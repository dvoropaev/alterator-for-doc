#include "ServicesApp.h"

#include "controller/Controller.h"
#include "ui/MainWindow.h"

#include <QEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFile>
#include <QMessageBox>
#include <QJsonParseError>
#include <QJsonArray>

#include "version.h"

class ServicesApp::Private {
public:
    AppSettings m_settings;
    Controller* m_controller;
};

ServicesApp::ServicesApp(int& argc, char** argv)
    : QtSingleApplication{argc, argv}
    , d{ new Private }
{
    setOrganizationName("ALTLinux");
    setOrganizationDomain("altlinux.org");
    setApplicationName("alt-services");
    setApplicationVersion(getApplicationVersion());
}

ServicesApp::~ServicesApp() { delete d; }

int ServicesApp::run()
{
    Controller controller;
    d->m_controller = &controller;

    MainWindow window;
    window.show();

    controller.refresh();

    auto args = arguments();
    if ( args.size() >= 3 && args[1] == "-o" )
        controller.selectByPath(args[2]);

    return exec();
}

AppSettings* ServicesApp::settings() { return &d->m_settings; }
Controller* ServicesApp::controller() { return d->m_controller; }



template <typename E> bool checkEvent(E* event) {
    if ( event->dropAction() != Qt::CopyAction )
        return true;

    auto urls = event->mimeData()->urls();
    if ( urls.empty() ) return true;

    if ( urls.size() == 1 &&
         urls.at(0).isLocalFile() &&
         urls.at(0).fileName().toLower().endsWith(".json")
        )
    {
        event->acceptProposedAction();
        return true;
    }

    return false;
}

bool ServicesApp::notify(QObject* object, QEvent* event)
{
    if ( auto widget = qobject_cast<QWidget*>(object) ) {
        if ( widget->acceptDrops() ) {
            switch ( event->type() ) {
                case QEvent::Type::DragEnter:
                    if ( checkEvent((QDragEnterEvent*)event) ) break;
                    return false;

                case QEvent::Type::Drop:
                    if ( checkEvent((QDropEvent*)event) ) break;
                    return false;

                default:
                    break;
            }
        }
    }

    return QApplication::notify(object, event);
}


// NOTE: returns false only if options is invalid
bool getTests(Action::TestSet& result, const QJsonObject& options, Service* service)
{
    for ( auto entry = options.constBegin(); entry != options.constEnd(); ++entry )
    {
        const auto& key = entry.key();
        if ( key.isEmpty() )
            return false;

        auto toolIt = std::find_if( service->diagTools().cbegin(), service->diagTools().cend(),
                                   [&](const auto& tool){return tool->name() == key;} );

        if ( toolIt == service->diagTools().cend() )
        {
            qWarning() << "skipping non-existing diag tool: " << key;
            continue;
        }

        DiagTool* tool = toolIt->get();

        const auto& value = entry.value();

        if ( !value.isArray() )
            return false;

        auto tests = value.toArray();
        for ( const auto& testItem : tests )
        {
            QString testName = testItem.toString();

            if (testName.isEmpty())
                return false;

            auto testIt = std::find_if( tool->tests().cbegin(), tool->tests().cend(),
                                       [&](const auto& test){ return test->name() == testName; });

            if ( testIt == tool->tests().cend() )
            {
                qWarning() << "skipping non-existing diag tool " << key << " test: " << testName;
                continue;
            }

            result.insert(testIt->get());
        }
    }

    return true;
}


std::optional<Action> ServicesApp::importParameters(const QString& fileName)
{

    if ( fileName.isEmpty() ) return {};

    QFile f{fileName};
    if ( ! f.open(QIODevice::ReadOnly) ) {
        QMessageBox::critical(activeModalWidget(), tr("File open error"), tr("Could not open file."));
        return {};
    }

    QByteArray data = f.readAll();

    f.close();


#define MESSAGEBOX(severity, title, text) \
    QMessageBox:: severity (QApplication::activeModalWidget(), title, text)

#define CRITICAL(...) MESSAGEBOX(critical, __VA_ARGS__)
#define WARNING(...) MESSAGEBOX(warning, __VA_ARGS__)

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(data, &err);
    if ( err.error != QJsonParseError::NoError ) {
        CRITICAL( QObject::tr("File parse error"), QObject::tr("Invalid file format.").append('\n').append(err.errorString()));
        return {};
    }

    QJsonObject object = doc.object();
    if ( object["version"].toInt() != 1)
        CRITICAL( QObject::tr("File parse error"), QObject::tr("Unsupported format version."));

    auto name = object["service"].toString();
    Service* service = ServicesApp::instance()->controller()->findByName( name );

    if ( !service ) {
        CRITICAL( QObject::tr("Error"), QObject::tr("Service \"%0\" does not exist").arg(name));
        return {};
    }
    static const std::map<QString, Parameter::Context> ctxmap {
        { "configure" , Parameter::Context::Configure },
        { "deploy"    , Parameter::Context::Deploy    },
        { "undeploy"  , Parameter::Context::Undeploy  },
        { "diag"      , Parameter::Context::Diag      },
        { "backup"    , Parameter::Context::Backup    },
        { "restore"   , Parameter::Context::Restore   },
    };
    Parameter::Context loaded_ctx{};

    try { loaded_ctx = ctxmap.at(object["action"].toString()); }
    catch (std::out_of_range&) {
        CRITICAL( QObject::tr("File parse error"), QObject::tr("Invalid file format.") );
        return {};
    }


    if ( service->isDeployed() && loaded_ctx == Parameter::Context::Undeploy )
    {
        CRITICAL( QObject::tr("Action not applicable"), QObject::tr("Service is already deployed") );
        return {};
    }

    if ( !service->isDeployed() && !Parameter::Contexts{Parameter::Context::Deploy | Parameter::Context::Diag}.testFlag(loaded_ctx) )
    {
        CRITICAL( QObject::tr("Action not applicable"), QObject::tr("Service is not deployed") );
        return {};
    }

    Action::Options options;

    if ( object.contains("options") )
    {
        QJsonObject optionsObj = object["options"].toObject();

        switch ( loaded_ctx )
        {
        case Parameter::Context::Deploy:
            options.autostart = optionsObj["autostart"].toBool();
            options.force     = optionsObj["force"    ].toBool();

            if ( service->isDeployed() && !options.force )
            {
                CRITICAL( QObject::tr("Action not applicable"), QObject::tr("Service is already deployed") );
                return {};
            }

            if ( optionsObj.contains("prediag") )
            {
                QJsonObject prediag = optionsObj["prediag"].toObject();
                options.prediag = !options.force && prediag["enable"].toBool();

                if ( !getTests(options.prediagTests, prediag["options"].toObject(), service) )
                {
                    CRITICAL( QObject::tr("File parse error"), QObject::tr("Invalid file format.") );
                    return {};
                }

                if ( options.prediag && options.prediagTests.empty() )
                    WARNING( QObject::tr("Not enough data"),
                            QObject::tr("Premilinary diagnostics:").append('\n')
                                .append(QObject::tr("The file does not contains any existing diagnostic tests. A default diagnostic subset will be used.")) );

            }
            // NOTE: both actions may have postdiag
            [[fallthrough]];

        case Parameter::Context::Configure:
            if ( optionsObj.contains("postdiag") )
            {
                QJsonObject postdiag = optionsObj["postdiag"].toObject();
                options.postdiag = postdiag["enable"].toBool();

                if ( !getTests(options.postdiagTests, postdiag["options"].toObject(), service) )
                {
                    CRITICAL( QObject::tr("File parse error"), QObject::tr("Invalid file format.") );
                    return {};
                }

                if ( options.postdiag && options.postdiagTests.empty() )
                    WARNING( QObject::tr("Not enough data"),
                            QObject::tr("Post-diagnostics:").append('\n')
                                .append(QObject::tr("The file does not contains any existing diagnostic tests. A default diagnostic subset will be used.")) );
            }
            break;

        case Parameter::Context::Diag:
            (service->isDeployed() ? options.postdiag: options.prediag) = true;
            if ( !getTests(service->isDeployed() ? options.postdiagTests : options.prediagTests, optionsObj, service) )
            {
                CRITICAL( QObject::tr("File parse error"), QObject::tr("Invalid file format.") );
                return {};
            }

            if ( (service->isDeployed() ? options.postdiagTests : options.prediagTests).empty() )
            {
                WARNING( QObject::tr("Not enough data"),
                        QObject::tr("The file does not contains any existing diagnostic tests. A default diagnostic subset will be used.") );
                return {};
            }
            break;

        default: break;
        }
    }

    return { Action{service, loaded_ctx, object["parameters"].toObject(), std::move(options)} };
}
