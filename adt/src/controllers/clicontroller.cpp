/***********************************************************************************************************************
**
** Copyright (C) 2023 BaseALT Ltd. <org@basealt.ru>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
***********************************************************************************************************************/

#include "clicontroller.h"
#include "../app/adtexecutor.h"
#include "model/treeitem.h"

#include <fstream>
#include <iostream>
#include <ostream>
#include <vector>
#include <QDebug>
#include <QThread>
#include <qobject.h>
#include <qobjectdefs.h>

class CliControllerPrivate
{
public:
    CliControllerPrivate(std::shared_ptr<TreeModel> model, ADTSettings *settings, ADTCommandLineOptions *options)
        : m_options(options)
        , m_model(model)
        , m_settings(settings)
        , m_executor(nullptr)
        , m_workerThread{nullptr}
        , m_loop(new QEventLoop())
    {}

    ~CliControllerPrivate() { delete m_loop; }

    ADTCommandLineOptions *m_options;
    std::shared_ptr<TreeModel> m_model;
    ADTSettings *m_settings;
    std::unique_ptr<ADTExecutor> m_executor;
    QThread *m_workerThread;
    QEventLoop *m_loop;

private:
    CliControllerPrivate(const CliControllerPrivate &)            = delete;
    CliControllerPrivate(CliControllerPrivate &&)                 = delete;
    CliControllerPrivate &operator=(const CliControllerPrivate &) = delete;
    CliControllerPrivate &operator=(CliControllerPrivate &&)      = delete;
};

CliController::CliController(std::shared_ptr<TreeModel> model,
                             DBusConfig dbusConfig,
                             ADTSettings *settings,
                             ADTCommandLineOptions *options)
    : BaseController(model)
    , d(new CliControllerPrivate(model, settings, options))
{
    d->m_executor = std::make_unique<ADTExecutor>(this);

    connect(d->m_executor.get(), &ADTExecutor::beginTask, this, &CliController::onBeginTask);
    connect(d->m_executor.get(), &ADTExecutor::finishTask, this, &CliController::onFinishTask);
    connect(d->m_executor.get(), &ADTExecutor::allTaskBegin, this, &CliController::onAllTasksBegin);
    connect(d->m_executor.get(), &ADTExecutor::allTasksFinished, this, &CliController::onAllTasksFinished);
}

CliController::~CliController()
{
    delete d;
}

int CliController::listTools()
{
    for (const auto &toolId : d->m_model->getTools())
    {
        auto tool = d->m_model->getTool(toolId);

        std::cout << tool->dbusPath().toStdString() << std::endl;
    }

    return 0;
}

int CliController::listToolTests(const QString &object)
{
    auto tool = d->m_model->getTool(object);
    if (!tool)
    {
        std::cerr << "ERROR: can't find object: " << object.toStdString() << std::endl;
        return 1;
    }

    auto tests = tool->getTests(d->m_options->bus);

    for (auto &testId : tests)
    {         
        auto test = d->m_model->getTest(tool->id(), testId);
        std::cout << test->testId().toStdString()
                  << std::string(test->bus() == ADTTest::BusType::System ? "\t on system bus" : "\t on session bus ")
                  << std::endl;
    }

    return 0;
}

int CliController::runAllTests(const QString &object)
{
    auto tool = d->m_model->getTool(object);
    if (!tool)
    {
        std::cerr << "ERROR: can't find object: " << object.toStdString() << std::endl;
        return 1;
    }

    auto tests = tool->getTests( d->m_options->bus );

    d->m_executor->setTasks(object, tests);

    d->m_executor->resetStopFlag();

    d->m_workerThread = new QThread();

    connect(d->m_workerThread, &QThread::started, d->m_executor.get(), &ADTExecutor::runTasks);
    connect(d->m_workerThread, &QThread::finished, this, &CliController::onExecutorStops);
    connect(d->m_workerThread, &QThread::finished, d->m_workerThread, &QObject::deleteLater);

    d->m_executor->moveToThread(d->m_workerThread);

    d->m_workerThread->start();

    d->m_loop->exec(); //Handle signals from executor

    // d->m_executor->resetStopFlag();
    // d->m_executor->setTasks(tool->id(), tests);
    // d->m_executor->runTasks();

    return 0;
}

int CliController::runTest(const QString &object, const QString &neededTest)
{
    auto* tool = d->m_model->getTool(object);
    if (!tool)
    {
        std::cerr << "ERROR: can't find object: " << object.toStdString() << std::endl;
        return 1;
    }

    QStringList tests;
    auto*  systemTest = tool->getTest(neededTest+"system");
    auto* sessionTest = tool->getTest(neededTest+"session");

    if ( d->m_options->bus & ADTTool::System )
    {
        if ( systemTest )
        {
            tests << systemTest->id();
        }
        else
        {
            std::cerr << "ERROR: can't find test: " << neededTest.toStdString() << " in object: " << object.toStdString() << " on system bus"
                      << std::endl;
            return 3;
        }
    }

    if ( d->m_options->bus & ADTTool::Session )
    {
        if ( sessionTest)
        {
            tests << sessionTest->id();
        }
        else
        {
            std::cerr << "ERROR: can't find test: " << neededTest.toStdString() << " in object: " << object.toStdString() << " on session bus"
                      << std::endl;
            return 3;
        }
    }

    if ( systemTest && sessionTest && d->m_options->bus == ADTTool::None )
    {
        std::cerr << "ERROR: Bus type is not specified, but test: " << neededTest.toStdString() << " is present on both system and session bus." << std::endl;
        return 4;
    }

    d->m_executor->resetStopFlag();
    d->m_executor->setTasks(tool->id(), tests);
    d->m_executor->runTasks();

    return 0;
}

int CliController::getToolReport(const QString &toolId, const QString &file)
{
    if (!setCurrentTool(toolId))
    {
        qWarning() << "ERROR: can't find tool with id: " + toolId;
        // TODO: show messagebox with warning
        return 1;
    }

    QByteArray report;


    ADTTool::BusType selectedBus = getTool(getCurrentTool())->bus();

    if ( selectedBus == ADTTool::All )
    {
        selectedBus = d->m_options->bus;
    }

    if ( selectedBus == ADTTool::None )
    {
        std::cerr << "ERROR: this tool has tests on both system and session bus. -b (--bus) parameter is required." << std::endl;
        return 1;
    }

    if ( selectedBus == ADTTool::All )
    {
        std::cerr << "ERROR: --bus=all is not supported. You should select either system or session bus." << std::endl;
        return 1;
    }

    if (getToolReports(report, selectedBus) != 0)
    {
        std::cerr << "ERROR: can't get report from object : " << toolId.toStdString();
        return 1;
    }

    std::ofstream reportFile(file.toStdString(), std::fstream::out);
    if (!reportFile.good())
    {
        std::cerr << "ERROR: can't save report to file: " << file.toStdString();
        return 1;
    }

    reportFile.write(report.data(), report.size());

    return 0;
}

int CliController::runApp()
{
    int result = -1;

    QString toolId;

    if (d->m_options->toolName.contains('/'))
    {
        auto tools = d->m_model->getTools();
        for (auto &currentToolId : tools)
        {
            auto tool = d->m_model->getTool(currentToolId);
            if (QString::compare(tool->dbusPath(), d->m_options->toolName) == 0)
            {
                toolId = tool->id();
                break;
            }
        }
        if (toolId.isEmpty())
        {
            toolId = d->m_options->toolName;
        }
    }
    else
        toolId = d->m_options->toolName;

    switch (d->m_options->action)
    {
    case ADTCommandLineOptions::Action::listOfObjects:
        result = listTools();
        break;
    case ADTCommandLineOptions::Action::listOfTestFromSpecifiedObject:
        result = listToolTests(toolId);
        break;
    case ADTCommandLineOptions::Action::runAllTestFromSpecifiedObject:
        result = runAllTests(toolId);
        break;
    case ADTCommandLineOptions::Action::runSpecifiedTestFromSpecifiedObject:
        result = runTest(toolId, d->m_options->testName);
        break;
    case ADTCommandLineOptions::Action::getReportTool:
        result = getToolReport(toolId, d->m_options->reportFilename);
        break;
    default:
        break;
    }

    return result;
}

void CliController::onDetailsDialogShow(const QString &toolId, const QString &testId) {}

void CliController::on_serviceUnregistered()
{
    if (d->m_executor->isRunning())
    {
        std::cout << "Service alterator-manager.service was unregistered! Please, restart the service! Waiting..."
                  << std::endl;
        d->m_executor->wait();
    }
}

void CliController::on_serviceRegistered()
{
    if (d->m_executor->isRunning())
    {
        std::cout << "Service alterator-manager.service was regictered! Working..." << std::endl;
        d->m_executor->resetWaitFlag();
    }
}

void CliController::on_serviceOwnerChanged()
{
    std::cout << "The owner of alterator-manager.service was changed! exiting..." << std::endl;
    if (d->m_executor->isRunning())
    {
        d->m_executor->cancelTasks();
        d->m_executor->resetWaitFlag();
    }
}

void CliController::getStdoutFromTest(QString toolId, QString testId, QString text)
{
    auto test = d->m_model->getTest(toolId, testId);
    if (!test)
        return;

    std::cout << toolId.toStdString() << "\t" << test->testId().toStdString() << "\t" << text.toStdString()
              << std::endl;
}

void CliController::getStderrFromTest(QString toolId, QString testId, QString text)
{
    auto test = d->m_model->getTest(toolId, testId);
    if (!test)
        return;

    std::cerr << toolId.toStdString() << "\t" << test->testId().toStdString() << "\t" << text.toStdString()
              << std::endl;
}

void CliController::onAllTasksBegin() {}

void CliController::onAllTasksFinished() {}

void CliController::onExecutorStops()
{
    d->m_loop->exit();
}

void CliController::onBeginTask(QString toolId, QString testId)
{
    auto test = d->m_model->getTest(toolId, testId);
    if (!test)
        return;

    std::cout << std::string(test->bus() == ADTTest::BusType::System ? "system bus" : "session bus") << "\t"
              << test->testId().toStdString() << std::endl;
}

void CliController::onFinishTask(QString toolId, QString testId)
{
    auto test = d->m_model->getTest(toolId, testId);
    if (!test)
        return;

    std::cout << (test->exitCode() == 0 ? "OK" : "ERROR") << std::endl;
}
