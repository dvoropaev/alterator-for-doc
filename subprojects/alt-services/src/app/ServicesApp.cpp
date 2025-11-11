#include "ServicesApp.h"

#include "controller/Controller.h"
#include "ui/MainWindow.h"
#include "ui/ActionWizard.h"

#include <QEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFile>
#include <QMessageBox>
#include <QJsonParseError>

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

std::optional<ServicesApp::ParsedParameters> ServicesApp::importParameters(const QString& fileName)
{
    static const std::map<QString, Parameter::Contexts> ctxmap {
        { "configure" , Parameter::Context::Configure },
        { "deploy"    , Parameter::Context::Deploy    },
        { "undeploy"  , Parameter::Context::Undeploy  },
        { "diag"      , Parameter::Context::Diag      },
      //{ "status"    , Parameter::Context::Status    },
        { "backup"    , Parameter::Context::Backup    },
        { "restore"   , Parameter::Context::Restore   },

        { "diag+deploy", Parameter::Context::Deploy | Parameter::Context::Diag },
    };

    if ( fileName.isEmpty() ) return {};

    QFile f{fileName};
    if ( ! f.open(QIODevice::ReadOnly) ) {
        QMessageBox::critical(activeModalWidget(), tr("File open error"), tr("Could not open file."));
        return {};
    }

    QByteArray data = f.readAll();

    f.close();

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(data, &err);
    if ( err.error != QJsonParseError::NoError ) {
        QMessageBox::critical(activeModalWidget(), tr("File parse error"), tr("Invalid file format.").append('\n').append(err.errorString()));
        return {};
    }

    QJsonObject object = doc.object();
    auto name = object["service"].toString();
    Service* service = d->m_controller->findByName( name );

    if ( !service ) {
        QMessageBox::critical(activeModalWidget(), tr("Error"), tr("Service \"%0\" does not exist").arg(name));
        return {};
    }

    Parameter::Contexts loaded_ctx{};

    try { loaded_ctx = ctxmap.at(object["context"].toString()); }
    catch (std::out_of_range&) {
        QMessageBox::critical(activeModalWidget(), tr("File parse error"), tr("Invalid file format.") );
        return {};
    }

    return { {service, loaded_ctx, object["parameters"].toObject()} };
}
