#include "Action.h"

#include <QJsonArray>

bool Action::TestSet::hasTool(DiagTool* tool) const
{
    return std::any_of(cbegin(), cend(), [tool](auto* test){return test->tool() == tool;});
}

bool Action::TestSet::hasTest(DiagTool::Test* test) const
{
    return std::find(cbegin(), cend(), test) != cend();
}

bool Action::hasTool(DiagTool* tool, DiagTool::Test::Mode mode) {
    return ( mode == DiagTool::Test::Mode::PreDeploy
         ? std::ref( options.prediagTests)
         : std::ref(options.postdiagTests) ).get().hasTool(tool);
}
bool Action::hasTest(DiagTool::Test* test, DiagTool::Test::Mode mode) {
    return ( mode == DiagTool::Test::Mode::PreDeploy
                ? std::ref(options. prediagTests)
                : std::ref(options.postdiagTests) ).get().hasTest(test);
}


QJsonObject Action::saveTests(DiagTool::Test::Mode mode) {
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

QByteArray Action::serialize()
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
