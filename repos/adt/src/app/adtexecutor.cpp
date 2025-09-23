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

#include "adtexecutor.h"

#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDebug>
#include <QThread>
#include <QTimer>

const QString STDOUT_SIGNAL_NAME = "diag1_stdout_signal";
const QString STDERR_SIGNAL_NAME = "diag1_stderr_signal";

const char *managerService        = "org.altlinux.alterator";
const char *managerPath           = "/org/altlinux/alterator";
const char *managerInterface      = "org.altlinux.alterator.manager";
const char *managerVarSetMethod   = "SetEnvValue";
const char *managerVarUnsetMethod = "UnsetEnvValue";

class ADTExecutorPrivate
{
public:
    ADTExecutorPrivate(BaseController *controller)
        : m_controller(controller)
    {}

    QStringList m_tests{};
    QString m_tool{};

    volatile bool stopFlag  = false;
    volatile bool waitFlag  = false;
    volatile bool isRunning = false;

    QDBusConnection m_systemCon  = QDBusConnection::systemBus();
    QDBusConnection m_sessionCon = QDBusConnection::sessionBus();

    BaseController *m_controller;

private:
    ADTExecutorPrivate(const ADTExecutorPrivate &)            = delete;
    ADTExecutorPrivate(ADTExecutorPrivate &&)                 = delete;
    ADTExecutorPrivate &operator=(const ADTExecutorPrivate &) = delete;
    ADTExecutorPrivate &operator=(ADTExecutorPrivate &&)      = delete;
};

ADTExecutor::ADTExecutor(BaseController *controller)
    : d(new ADTExecutorPrivate(controller))
{}

ADTExecutor::~ADTExecutor()
{
    delete d;
}

int ADTExecutor::getAmountOfExecutables()
{
    return d->m_tests.size();
}

void ADTExecutor::cancelTasks()
{
    d->stopFlag = true;
}

void ADTExecutor::resetStopFlag()
{
    d->stopFlag = false;
}

void ADTExecutor::wait()
{
    d->waitFlag = true;
}

void ADTExecutor::resetWaitFlag()
{
    d->waitFlag = false;
}

bool ADTExecutor::isRunning()
{
    return d->isRunning;
}

void ADTExecutor::setTasks(QString toolId, QStringList tests)
{
    d->m_tool  = toolId;
    d->m_tests = tests;
}

void ADTExecutor::runTasks()
{
    emit allTaskBegin();

    if (d->m_tests.size() == 0)
    {
        this->moveToThread(QApplication::instance()->thread());

        emit allTasksFinished();

        QThread::currentThread()->quit();

        return;
    }

    d->isRunning = true;

    setToolVariables(d->m_tool);

    for (QString &testId : d->m_tests)
    {
        if (d->stopFlag)
        {
            break;
        }

        while (d->waitFlag)
        {
            QThread::currentThread()->yieldCurrentThread();
        }

        emit beginTask(d->m_tool, testId);

        executeTask(testId);

        emit finishTask(d->m_tool, testId);
    }
    d->isRunning = false;

    unsetToolVariables(d->m_tool);

    this->moveToThread(QApplication::instance()->thread());

    emit allTasksFinished();

    QThread::currentThread()->quit();
}

void ADTExecutor::executeTask(QString &testId)
{
    auto tool = d->m_controller->getTool(d->m_tool);
    if (!tool)
        return;

    auto test = d->m_controller->getTest(d->m_tool, testId);
    if (!test)
        return;

    QDBusConnection dbus = d->m_sessionCon;

    if (test->bus() == ADTTest::BusType::System)
    {
        dbus = d->m_systemCon;
    }

    QDBusInterface dbusIface(tool->dbusServiceName(), tool->dbusPath(), tool->dbusIface(), dbus);

    test->clearLogs();

    QString signalPrefix = dbus.baseService();
    signalPrefix.replace(':', '_');
    signalPrefix.replace('.', '_');

    QString stdoutSignal = STDOUT_SIGNAL_NAME + signalPrefix;
    QString stderrSignal = STDERR_SIGNAL_NAME + signalPrefix;

    connectTaskSignals(dbusIface, tool, test, stdoutSignal, stderrSignal);

    QDBusReply<int> reply = dbusIface.call(tool->runMethodName(), test->testId());

    if (!reply.isValid())
    {
        test->setExitCode(-255);
        test->appendToStderr(reply.error().message());

        disconnectTaskSignals(dbusIface, tool, test, stdoutSignal, stderrSignal);

        return;
    }

    disconnectTaskSignals(dbusIface, tool, test, stdoutSignal, stderrSignal);

    test->setExitCode(reply.value());
}

bool ADTExecutor::setToolVariables(const QString &toolId)
{
    auto tool = d->m_controller->getTool(toolId);
    if (!tool)
        return false;

    return applyToolsVars(tool, d->m_systemCon, true) && applyToolsVars(tool, d->m_sessionCon, true);
}

bool ADTExecutor::unsetToolVariables(const QString &toolId)
{
    auto tool = d->m_controller->getTool(toolId);
    if (!tool)
        return false;

    return applyToolsVars(tool, d->m_systemCon, false) && applyToolsVars(tool, d->m_sessionCon, false);
}

bool ADTExecutor::applyToolsVars(ADTTool *tool, QDBusConnection conn, bool set)
{
    QDBusInterface iface(managerService, managerPath, managerInterface, conn);
    if (!iface.isValid())
        return false;

    QStringList vars = tool->getVarNames();

    for (auto &varName : vars)
    {
        ADTVarInterface *var = tool->getVar(varName);
        if (!var)
            return false;

        auto varType = var->getType();

        QString value;
        int val;
        switch (varType)
        {
        case ADTVarInterface::ADTVarType::INT:
            if (!var->get(&val))
                return false;
            value = QString::number(val);

            break;

        case ADTVarInterface::ADTVarType::STRING:
            if (!var->get(&value))
                return false;

            break;

        case ADTVarInterface::ADTVarType::ENUM_STRING:
            if (!var->get(&value))
                return false;

            break;

        case ADTVarInterface::ADTVarType::ENUM_INT:
            if (!var->get(&val))
                return false;

            value = QString::number(val);

            break;

        default: //unsupported type
            return false;
        }

        QDBusMessage reply;
        if (set)
            reply = iface.call(managerVarSetMethod, varName, value);
        else
            reply = iface.call(managerVarUnsetMethod, varName);

        if (reply.type() == QDBusMessage::ErrorMessage)
            return false;

        int exitCode = reply.arguments().at(0).toInt();
        if (exitCode)
            return false;
    }

    return true;
}

void ADTExecutor::connectTaskSignals(
    QDBusInterface &iface, ADTTool *tool, ADTTest *test, QString stdoutSignalName, QString stderrSignalName)
{
    if (!iface.connection().connect(tool->dbusServiceName(),
                                    tool->dbusPath(),
                                    tool->dbusIface(),
                                    stdoutSignalName,
                                    test,
                                    SLOT(getStdout(QString))))
    {
        qWarning() << "ERROR: Can't connect stdout signal from test: " << test->id() << " in tool: " << tool->id();
    }

    if (!iface.connection().connect(tool->dbusServiceName(),
                                    tool->dbusPath(),
                                    tool->dbusIface(),
                                    stderrSignalName,
                                    test,
                                    SLOT(getStderr(QString))))
    {
        qWarning() << "ERROR: Can't connect stderrsignal from test: " << test->id() << " in tool: " << tool->id();
    }

    connect(test, &ADTTest::getStdoutText, d->m_controller, &BaseController::getStdoutFromTest);
    connect(test, &ADTTest::getStderrText, d->m_controller, &BaseController::getStderrFromTest);
}

void ADTExecutor::disconnectTaskSignals(
    QDBusInterface &iface, ADTTool *tool, ADTTest *test, QString stdoutSignalName, QString stderrSignalName)
{
    if (!iface.connection().disconnect(tool->dbusServiceName(),
                                       tool->dbusPath(),
                                       tool->dbusIface(),
                                       stdoutSignalName,
                                       test,
                                       SLOT(getStdout(QString))))
    {
        qWarning() << "ERROR: Can't disconnect stdout signal from test: " << test->id() << " in tool: " << tool->id();
    }

    if (!iface.connection().disconnect(tool->dbusServiceName(),
                                       tool->dbusPath(),
                                       tool->dbusIface(),
                                       stderrSignalName,
                                       test,
                                       SLOT(getStderr(QString))))
    {
        qWarning() << "ERROR: Can't disconnect stderr signal from test: " << test->id() << " in tool: " << tool->id();
    }

    disconnect(test, &ADTTest::getStdoutText, d->m_controller, &BaseController::getStdoutFromTest);
    disconnect(test, &ADTTest::getStderrText, d->m_controller, &BaseController::getStderrFromTest);
}
