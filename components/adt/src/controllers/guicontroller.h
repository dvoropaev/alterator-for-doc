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

#ifndef GUI_CONTROLLER_H
#define GUI_CONTROLLER_H

#include "../app/adtcommandlineoptions.h"
#include "../app/adtsettings.h"
#include "basecontroller.h"
#include "model/builder/dbusconfig.h"
#include "model/treemodel.h"

#include <QApplication>

class GuiControllerPrivate;

class GuiController : public BaseController
{
    Q_OBJECT

public:
    GuiController(std::shared_ptr<TreeModel> model,
                  DBusConfig dbusConfig,
                  ADTSettings *settings,
                  ADTCommandLineOptions *options,
                  QApplication *app);
    ~GuiController();

    int listTools() override;
    int listToolTests(const QString &object) override;
    int runAllTests(const QString &toolId) override;
    int runTest(const QString &toolId, const QString &testId) override;
    int getToolReport(const QString &toolId, const QString &file) override;
    int runApp() override;

    void onDetailsDialogShow(const QString &toolId, const QString &testId) override;

    // Run All button pressed in tools widget
    void runAllToolsWidget();

    void onToolsWidgetBrowseButtonClicked();

    // Exit button pressed in tools widget
    void exitToolsWidget();

    // Changes current selected tool
    void changeSelectedTool(QModelIndex index);

    // Return Model Item of slected tool
    TreeItem *getTreeItemByIndex(QModelIndex index);

    // Switch to tests widget and runs task in test vector
    void runTestsWidget(const QString &toolId, const QStringList &tests);

    // Switch to test widget and runs all filtered tests
    void runCurrentToolTest();

    // Reset all tests in specified tool reset
    void resetToolStatus();

    // Stop all runnign tests
    void stopAllTests();

    // Back button pressed in tests widget
    void backTestsWigdet();

    // Exit button pressed in tests widget
    void exitTestsWidget();

    // Report button pressed in testswidget
    void reportTestWidget();

    // All Logs button pressed in toolbar
    void allLogsTool();

    // Saves mainwindow settings in common settings object
    void saveMainWindowSettings();

    // Restores mainwindow settings form common settings object
    void restoreMainWindowSettings();

    //Main window receives close event
    void closeAll();

public slots:
    virtual void on_serviceUnregistered() override;
    virtual void on_serviceRegistered() override;
    virtual void on_serviceOwnerChanged() override;

    virtual void getStdoutFromTest(QString toolId, QString testId, QString text) override;
    virtual void getStderrFromTest(QString toolId, QString testId, QString text) override;

private:
    void changeSelectedToolById(QString id);
    void updateDetailsDialog(const QString &toolId, const QString &testId);
    void doReport(ADTTool::BusType);

private:
    GuiControllerPrivate *d{};

private slots:
    void onAllTasksBegin() override;
    void onAllTasksFinished() override;

    void onBeginTask(QString toolId, QString testId) override;
    void onFinishTask(QString toolId, QString testId) override;

    void onCloseAndExitButtonPressed();
    void on_closeButtonPressed();

private:
    GuiController(const GuiController &)            = delete;
    GuiController(GuiController &&)                 = delete;
    GuiController &operator=(const GuiController &) = delete;
    GuiController &operator=(GuiController &&)      = delete;
};

#endif // GUI_CONTROLLER_H
