#pragma once

#include "Service.h"
#include <QJsonObject>

class Action {
public:
    class TestSet : public std::set<DiagTool::Test*>
    {
    public:
        void toggle(DiagTool::Test* test)
        {
            auto hint = lower_bound(test);
            if ( *hint == test )
                erase(hint);
            else
                insert(hint, test);
        }

        bool hasTool(DiagTool* tool) const;
        bool hasTest(DiagTool::Test* test) const;
    };

    Service* service{};
    Parameter::Context action;
    QJsonObject parameters;

    struct Options {
        bool force{false};
        bool autostart{false};

        bool prediag{false};
        TestSet prediagTests;

        bool postdiag{false};
        TestSet postdiagTests;
    } options;

    /*
     * Returns true if any of tool's tests are selected in specified mode:
     *  - for DiagTool::Test::Mode::PreDeploy:  options::prediagTests
     *  - for DiagTool::Test::Mode::PostDeploy: options::postdiagTests
     */
    bool hasTool(DiagTool*       tool, DiagTool::Test::Mode mode);
    bool hasTest(DiagTool::Test* test, DiagTool::Test::Mode mode);

    /*
     * Save to json.
     */
    QByteArray serialize();

private:
    QJsonObject saveTests(DiagTool::Test::Mode mode);
};
