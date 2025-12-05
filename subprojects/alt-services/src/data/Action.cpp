#include "Action.h"

#include <QJsonArray>

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
