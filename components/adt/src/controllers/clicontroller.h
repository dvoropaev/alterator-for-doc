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

#ifndef CLI_CONTROLLER_H
#define CLI_CONTROLLER_H

#include "../app/adtcommandlineoptions.h"
#include "../app/adtsettings.h"
#include "../model/builder/dbusconfig.h"
#include "basecontroller.h"
#include "model/treemodel.h"

#include <QDBusConnection>
#include <QString>

class CliControllerPrivate;

class CliController : public BaseController
{
    Q_OBJECT

public:
    CliController(std::shared_ptr<TreeModel> model,
                  DBusConfig dbusConfig,
                  ADTSettings *settings,
                  ADTCommandLineOptions *options);
    ~CliController();

    int listTools() override;
    int listToolTests(const QString &object) override;
    int runAllTests(const QString &object) override;
    int runTest(const QString &object, const QString &test) override;
    int getToolReport(const QString &toolId, const QString &file) override;
    int runApp() override;
    void onDetailsDialogShow(const QString &toolId, const QString &testId);

public slots:
    virtual void on_serviceUnregistered() override;
    virtual void on_serviceRegistered() override;
    virtual void on_serviceOwnerChanged() override;

    virtual void getStdoutFromTest(QString toolId, QString testId, QString text) override;
    virtual void getStderrFromTest(QString toolId, QString testId, QString text) override;

private slots:
    void onAllTasksBegin() override;
    void onAllTasksFinished() override;

    void onExecutorStops();

    void onBeginTask(QString toolId, QString testId) override;
    void onFinishTask(QString toolId, QString testId) override;

private:
    CliControllerPrivate *d;

private:
    CliController(const CliController &)            = delete;
    CliController(CliController &&)                 = delete;
    CliController &operator=(const CliController &) = delete;
    CliController &operator=(CliController &&)      = delete;
};

#endif // CLI_CONTROLLER_H
