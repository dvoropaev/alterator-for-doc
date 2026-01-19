#include "Action.h"

#include <QJsonArray>
#include <QFile>
#include <QMessageBox>

#include "app/ServicesApp.h"
#include "controller/Controller.h"

#include <range/v3/algorithm.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/filter.hpp>

bool Action::TestSet::hasTool(DiagTool* tool) const
{
    return ranges::contains(*this, tool, &DiagTool::Test::tool);
}

bool Action::TestSet::hasTest(DiagTool::Test* test) const
{
    return find(test) != cend();
}

bool Action::modifiesResourses() const
{
    using namespace std::placeholders;
    switch ( action )
    {
        case Parameter::Context::Deploy:
        case Parameter::Context::Undeploy:
            return service->resources().size();

        case Parameter::Context::Configure:
            return ranges::any_of(
                service->resources()
                    | ranges::views::transform(&Resource::override)
                    | ranges::views::filter(ranges::identity()),
                std::bind(std::bit_and(), Parameter::Context::Configure, _1),
                &Parameter::contexts
            );

        default: return false;
    }
}

bool Action::preDiagAvailable() const
{
    return service->hasPreDiag() &&
           !service->isDeployed() &&
           action & ( Parameter::Context::Deploy );
}

bool Action::postDiagAvailable() const
{
    return service->hasPostDiag() &&
           action & ( Parameter::Context::Deploy | Parameter::Context::Configure | Parameter::Context::Restore );
}

bool Action::hasTool(DiagTool* tool, DiagTool::Test::Mode mode) const {
    return ( mode == DiagTool::Test::Mode::PreDeploy
         ? std::ref( options.prediagTests)
         : std::ref(options.postdiagTests) ).get().hasTool(tool);
}
bool Action::hasTest(DiagTool::Test* test, DiagTool::Test::Mode mode) const {
    return ( mode == DiagTool::Test::Mode::PreDeploy
                ? std::ref(options. prediagTests)
                : std::ref(options.postdiagTests) ).get().hasTest(test);
}


QJsonObject Action::saveTests(DiagTool::Test::Mode mode) const {
    const TestSet& testSet = mode == DiagTool::Test::Mode::PreDeploy
                            ? options.prediagTests
                            : options.postdiagTests;

    std::map<QString, QStringList> toolToTest;

    for ( const auto& tool : service->diagTools() )
        for ( const auto& test : tool->tests() )
            if ( test->required().testFlag(mode) || testSet.hasTest(test.get()) )
                toolToTest[tool->name()].push_back(test->name());

    QJsonObject result;
    for ( auto& [tool, tests] : toolToTest )
        result[tool] = QJsonValue::fromVariant(tests);

    return result;
}

QByteArray Action::serialize() const
{

    static const std::map<Parameter::Context, QString> ctxmap {
        { Parameter::Context::Configure , "configure" },
        { Parameter::Context::Deploy    , "deploy"    },
        { Parameter::Context::Undeploy  , "undeploy"  },
        { Parameter::Context::Diag      , "diag"      },
        { Parameter::Context::Backup    , "backup"    },
        { Parameter::Context::Restore   , "restore"   },
    };

    Parameter::Contexts contexts{action};
    contexts.setFlag(Parameter::Context::Diag, options.prediag || options.postdiag);

    QJsonObject options_json;
    switch (action) {
        case Parameter::Context::Deploy: {
            options_json["force"]     = options.force;
            options_json["autostart"] = options.autostart;

            options_json["prediag"] = QJsonObject{
                {"enable",  options.prediag},
                {"options", saveTests(DiagTool::Test::Mode::PreDeploy)}
            };

        } [[fallthrough]];

        case Parameter::Context::Configure:
            options_json["postdiag"] = QJsonObject{
                {"enable",  options.postdiag},
                {"options", saveTests(DiagTool::Test::Mode::PostDeploy)}
            };
            break;

        case Parameter::Context::Diag:
            options_json = saveTests(options.prediag
                                         ? DiagTool::Test::Mode::PreDeploy
                                         : DiagTool::Test::Mode::PostDeploy);
            break;

        default: break;
    }

    return QJsonDocument{{
        { "version",    1},
        { "service",    service->name() },
        { "action",     ctxmap.at(action) },
        { "parameters", service->getParameters(contexts, true) },
        { "options",    options_json }
    }}.toJson(QJsonDocument::Indented);
}


// NOTE: returns false only if options is invalid
bool getTests(Action::TestSet& result, const QJsonObject& options, Service* service)
{
    for ( auto entry = options.constBegin(); entry != options.constEnd(); ++entry )
    {
        const auto& key = entry.key();
        if ( key.isEmpty() )
            return false;

        auto toolIt = ranges::find(service->diagTools(), key, &DiagTool::name);

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

            auto testIt = ranges::find( tool->tests(), testName, &DiagTool::Test::name );

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

std::optional<Action> Action::importFromFile(const QString& fileName)
{
#define MESSAGEBOX(severity, title, text) \
    QMessageBox:: severity (qApp->activeModalWidget(), title, text)

#define CRITICAL(...) MESSAGEBOX(critical, __VA_ARGS__)
#define WARNING(...) MESSAGEBOX(warning, __VA_ARGS__)

    if ( fileName.isEmpty() ) return {};

    QFile f{fileName};
    if ( ! f.open(QIODevice::ReadOnly) )
    {
        CRITICAL( QObject::tr("File open error"), QObject::tr("Could not open file.") );
        return {};
    }

    QByteArray data = f.readAll();

    f.close();


    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(data, &err);
    if ( err.error != QJsonParseError::NoError ) {
        CRITICAL( QObject::tr("File parse error"), QObject::tr("Invalid file format.").append('\n').append(err.errorString()));
        return {};
    }

    QJsonObject object = doc.object();
    if ( object["version"].toInt() != 1 )
        CRITICAL( QObject::tr("File parse error"), QObject::tr("Unsupported format version."));

    auto name = object["service"].toString();
    Service* service = qApp->controller()->findByName( name );

    if ( !service )
    {
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
    catch ( std::out_of_range& )
    {
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
