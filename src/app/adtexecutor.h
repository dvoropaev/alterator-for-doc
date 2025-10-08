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

#ifndef ADT_EXECUTOR_H
#define ADT_EXECUTOR_H

#include "model/treeitem.h"

#include <QDBusInterface>
#include <QObject>

#include "controllers/guicontroller.h"
#include "model/treemodel.h"

class ADTExecutorPrivate;

class ADTExecutor : public QObject
{
    Q_OBJECT

public:
    ADTExecutor(BaseController *controller);

    ~ADTExecutor();

    int getAmountOfExecutables();

    void wait();
    bool isRunning();

    void resetStopFlag();
    void resetWaitFlag();

    void cancelTasks();
    void setTasks(QString toolId, QStringList tests);

public slots:
    void runTasks();

signals:
    void beginTask(QString toolId, QString testId);
    void finishTask(QString toolId, QString testId);

    void allTaskBegin();
    void allTasksFinished();

private:
    void executeTask(QString &testId);

    bool setToolVariables(const QString &toolId);
    bool unsetToolVariables(const QString &toolId);
    bool applyToolsVars(ADTTool *tool, QDBusConnection conn, bool set);

    void connectTaskSignals(
        QDBusInterface &iface, ADTTool *tool, ADTTest *test, QString stdoutSignalName, QString stderrSignalName);

    void disconnectTaskSignals(
        QDBusInterface &iface, ADTTool *tool, ADTTest *test, QString stdoutSignalName, QString stderrSignalName);

private:
    ADTExecutorPrivate *d;

private:
    ADTExecutor(const ADTExecutor &)            = delete;
    ADTExecutor(ADTExecutor &&)                 = delete;
    ADTExecutor &operator=(const ADTExecutor &) = delete;
    ADTExecutor &operator=(ADTExecutor &&)      = delete;
};

#endif // ADT_EXECUTOR_H
