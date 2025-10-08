#include "basecontroller.h"
#include "model/adttool.h"

#include <memory>

BaseController::BaseController(std::shared_ptr<TreeModel> model)
    : m_model(model)
    , m_systemConn(QDBusConnection::systemBus())
    , m_sessionConn(QDBusConnection::sessionBus())
    , m_currentTool()
    , m_currentTest()
    , m_dbusConfig()
{}

void BaseController::resetToolTestsLogs(const QString &toolId, const QStringList &testsList)
{
    auto tests = getAllTests(toolId);

    auto testExists{[&testsList](QString &testId) -> bool {
        for (auto &currentTest : testsList)
        {
            if (currentTest == testId)
                return true;
        }

        return false;
    }};

    for (auto &testId : tests)
    {
        if (testExists(testId))
        {
            auto test = getTest(m_currentTool, testId);
            if (!test)
                continue;

            test->clearLogs();
        }
    }
}

bool BaseController::hasSystemTests(const QString &toolId)
{
    auto tool = getTool(toolId);
    if (!tool)
        return false;

    return tool->bus() == ADTTool::BusType::System || tool->bus() == ADTTool::BusType::All ? true : false;
}

bool BaseController::hasSessionTests(const QString &toolId)
{
    auto tool = getTool(toolId);
    if (!tool)
        return false;

    return tool->bus() == ADTTool::BusType::Session || tool->bus() == ADTTool::BusType::All ? true : false;
}

QStringList BaseController::getFilteredTests(const QString &toolId)
{
    auto tool = getTool(toolId);
    if (!tool)
        return {};

    return tool->getFilteredTests();
}

QStringList BaseController::getAllTests(const QString &toolId)
{
    auto tool = getTool(toolId);
    if (!tool)
        return {};

    return tool->getTests();
}

void BaseController::setToolFilter(const QString &filter)
{
    auto tool = getTool(m_currentTool);
    if (!tool)
        return;

    tool->setFilter(filter);
}

QString BaseController::getToolFilter(const QString &toolId)
{
    auto tool = getTool(toolId);
    if (!tool)
        return {};

    return tool->getFilter();
}

QString BaseController::getTestDisplayName(const QString &toolId, const QString &testId)
{
    auto test = getTest(toolId, testId);
    if (!test)
        return {};

    return test->displayName();
}

int BaseController::getTestExitCode(const QString &toolId, const QString &testId)
{
    auto test = getTest(toolId, testId);
    if (!test)
        return -255;

    return test->exitCode();
}

QString BaseController::getToolDisplayName(const QString &toolId)
{
    auto tool = getTool(toolId);
    if (!tool)
        return {};

    return tool->displayName();
}

QString BaseController::getToolComment(const QString &toolId)
{
    auto tool = getTool(toolId);
    if (!tool)
        return {};

    return tool->comment();
}

QString BaseController::getReportSuffix(const QString &toolId, ADTTool::BusType bus)
{
    auto tool = getTool(m_currentTool);
    return tool->reportSuffix(bus);
}

ADTTool *BaseController::getTool(const QString &toolId)
{
    return m_model->getTool(toolId);
}

std::unique_ptr<QStandardItemModel> BaseController::getVarsModel(const QString &toolId)
{
    auto tool = getTool(toolId);
    if (!tool)
        return std::unique_ptr<QStandardItemModel>(new QStandardItemModel());

    return tool->getVarsModel();
}

ADTTest *BaseController::getTest(const QString &toolId, const QString &testId)
{
    return m_model->getTest(toolId, testId);
}

QString BaseController::getCurrentTool()
{
    return m_currentTool;
}

QString BaseController::getCurrentTest()
{
    return m_currentTest;
}

bool BaseController::setCurrentTool(const QString &toolId)
{
    auto tool = getTool(toolId);
    if (!tool)
        return false;

    m_currentTool = toolId;
    return true;
}

int BaseController::getToolReports(QByteArray &array, ADTTool::BusType bus)
{
    auto tool = getTool(m_currentTool);
    if (!tool)
        return 1;

    return tool->getReport( bus == ADTTool::System ? m_systemConn : m_sessionConn, array);
}

QString BaseController::getAllTestLogs(const QString &toolId)
{
    QString logsLines;

    QStringList tests = getAllTests(toolId);

    for (auto testId : tests)
    {
        auto test = getTest(m_currentTool, testId);
        if (!test)
            continue;

        logsLines = logsLines + test->getStdout();
        logsLines = logsLines + test->getStderr();
    }
    return logsLines;
}

bool BaseController::setCurrentTest(const QString &toolId, const QString &testId)
{
    auto test = getTest(toolId, testId);
    if (!test)
        return false;

    setCurrentTool(toolId);
    m_currentTest = testId;
    return true;
}
