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

#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "AboutDialog.h"

#include <memory>
#include <QDebug>

class MainWindowPrivate
{
public:
    MainWindowPrivate(MainWindow *mWindow, Ui::MainWindow *ui)
        : m_mainWindow(mWindow)
        , m_mainWindowUi(ui)
        , m_toolsModel(nullptr)
        , m_about{mWindow}
    {}
    ~MainWindowPrivate() = default;

public:
    MainWindow *m_mainWindow{};
    Ui::MainWindow *m_mainWindowUi{};
    GuiController *m_controller{};

    //Tests widget

    //Tools widget
    std::shared_ptr<TreeModel> m_toolsModel;

    AboutDialog m_about;

private:
    MainWindowPrivate(const MainWindowPrivate &)            = delete;
    MainWindowPrivate(MainWindowPrivate &&)                 = delete;
    MainWindowPrivate &operator=(const MainWindowPrivate &) = delete;
    MainWindowPrivate &operator=(MainWindowPrivate &&)      = delete;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow())
    , d(new MainWindowPrivate(this, ui))
{
    ui->setupUi(this);

    ui->menuView->addAction(ui->toolBar->toggleViewAction());

    ui->toolsPage->setMainWindow(this);
    ui->testsPage->setMainWindow(this);

    connect(ui->actionNavigateBack, &QAction::triggered, this, &MainWindow::on_navigateBack);
    connect(ui->actionNavigateForward, &QAction::triggered, this, &MainWindow::on_navigateForward);
    connect(ui->actionRunAllTests, &QAction::triggered, this, &MainWindow::on_runAllTests);
    connect(ui->actionStopRunTests, &QAction::triggered, this, &MainWindow::on_stopRunTests);
    connect(ui->actionReportExport, &QAction::triggered, this, &MainWindow::on_reportExport);
    connect(ui->actionResetTestsStatus, &QAction::triggered, this, &MainWindow::on_resetAllTestsStatus);
    connect(ui->actionSaveAllLogs, &QAction::triggered, this, &MainWindow::on_saveAllLogs);
    connect(ui->actionAbout, &QAction::triggered, &d->m_about, &AboutDialog::open);
}

MainWindow::~MainWindow()
{
    delete d;
    delete ui;
}

void MainWindow::closeAll()
{
    d->m_controller->saveMainWindowSettings();
    close();
}

void MainWindow::setController(GuiController *controller)
{
    d->m_controller = controller;
    d->m_controller->restoreMainWindowSettings();
}

void MainWindow::toggleStackWidget()
{
    if (ui->stackedWidget->currentIndex() == 0)
    {
        ui->stackedWidget->setCurrentIndex(1);
        enabledNavigateActions(StateNavigate::enabledBack);
    }
    else
    {
        ui->stackedWidget->setCurrentIndex(0);
        enabledNavigateActions(StateNavigate::enabledForward);
    }
}

WidgetOfStackedWidget MainWindow::getCurrentWidget()
{
    QWidget *currentWidget = ui->stackedWidget->currentWidget();

    if (currentWidget == ui->toolsPage)
        return WidgetOfStackedWidget::ToolsWidget;

    if (currentWidget == ui->testsPage)
        return WidgetOfStackedWidget::TestsWidget;

    return WidgetOfStackedWidget::None;
}

void MainWindow::onDetailsDialogShow(const QString &toolId, const QString &testId)
{
    d->m_controller->onDetailsDialogShow(toolId, testId);
}

void MainWindow::setTestsWidgetTool(const QString &toolId)
{
    ui->testsPage->setTool(toolId);
}

void MainWindow::resetTests(const QStringList &testsIds)
{
    ui->testsPage->resetTests(testsIds);
}

void MainWindow::setEnableStatusWidgetRunButtons(bool enable)
{
    ui->testsPage->setEnabledRunButtonOfStatusWidgets(enable);
}

void MainWindow::disableTestWidgetButtons()
{
    ui->testsPage->disableButtons();
}

void MainWindow::enableTestWidgetButtons()
{
    ui->testsPage->enableButtons();
}

void MainWindow::setCommonWidgetStatus(const QString &id, WidgetStatus status)
{
    ui->testsPage->setWidgetStatus(id, status);
}

void MainWindow::resetToolTestsLogs(const QString &toolId, const QStringList &testsIds)
{
    d->m_controller->resetToolTestsLogs(toolId, testsIds);
}

void MainWindow::runTestOfTool(const QString &toolId, const QString &testId)
{
    d->m_controller->runTest(toolId, testId);
}

QStringList MainWindow::getFilteredTests(const QString &toolId)
{
    return d->m_controller->getFilteredTests(toolId);
}

void MainWindow::setToolFilter(const QString &filter)
{
    d->m_controller->setToolFilter(filter);
}

QString MainWindow::getTestDisplayName(const QString &toolId, const QString &testId)
{
    return d->m_controller->getTestDisplayName(toolId, testId);
}

void MainWindow::closeEvent(QCloseEvent *closeEvent)
{
    d->m_controller->closeAll();
}

void MainWindow::showMessStatusBar(QString message)
{
    ui->statusbar->showMessage(message);
}

void MainWindow::on_navigateBack()
{
    d->m_controller->backTestsWigdet();
    ui->actionResetTestsStatus->setEnabled(false);
}

void MainWindow::on_navigateForward()
{
    d->m_controller->backTestsWigdet();
}

void MainWindow::on_runAllTests()
{
    d->m_controller->resetToolStatus();
    ui->stackedWidget->currentIndex() == 0 ? d->m_controller->runAllToolsWidget()
                                           : d->m_controller->runCurrentToolTest();
}

void MainWindow::on_stopRunTests()
{
    ui->actionStopRunTests->setEnabled(false);
    d->m_controller->stopAllTests();
}

void MainWindow::on_reportExport()
{
    d->m_controller->reportTestWidget();
}

void MainWindow::on_resetAllTestsStatus()
{
    ui->actionResetTestsStatus->setEnabled(false);
    d->m_controller->resetToolStatus();
}

void MainWindow::on_saveAllLogs()
{
    d->m_controller->allLogsTool();
}

void MainWindow::enabledActions(bool stateEnable)
{
    ui->actionRunAllTests->setEnabled(stateEnable);
    ui->actionStopRunTests->setEnabled(!stateEnable);
    ui->actionReportExport->setEnabled(stateEnable);
    ui->actionSaveAllLogs->setEnabled(stateEnable);
}

void MainWindow::enableResetTestsStatusAction(bool stateEnable)
{
    ui->actionResetTestsStatus->setEnabled(stateEnable);
}

void MainWindow::enabledNavigateActions(StateNavigate stateEnable)
{
    bool stateEnableBack    = false;
    bool stateEnableForward = false;
    switch (stateEnable)
    {
    case StateNavigate::enabledAll:
        stateEnableBack    = true;
        stateEnableForward = true;
        break;
    case StateNavigate::enabledBack:
        stateEnableBack    = true;
        stateEnableForward = false;
        break;
    case StateNavigate::enabledForward:
        stateEnableBack    = false;
        stateEnableForward = true;
        break;
    default:
        break;
    }
    ui->actionNavigateBack->setEnabled(stateEnableBack);
    ui->actionNavigateForward->setEnabled(stateEnableForward);
}

QSize MainWindow::getSize()
{
    return this->size();
}

QPoint MainWindow::getPos()
{
    return this->pos();
}

void MainWindow::setToolsWidgetModel(std::shared_ptr<TreeModel> model)
{
    d->m_toolsModel = model;
    ui->toolsPage->setModel(model.get());
}

void MainWindow::enableToolsWidgetbuttons()
{
    ui->toolsPage->enableButtons();
}

void MainWindow::disableToolsWidgetButtons()
{
    ui->toolsPage->disableButtons();
}

void MainWindow::setToolsWidgetDescription(const QString &text)
{
    ui->toolsPage->setDescription(text);
}

void MainWindow::onToolsWidgetBrowseButtonClicked()
{
    d->m_controller->onToolsWidgetBrowseButtonClicked();
}

void MainWindow::changeSelectedTool(QModelIndex index)
{
    d->m_controller->changeSelectedTool(index);
}

void MainWindow::setTestAvailability(bool systemTests, bool sessionTests)
{
    ui->toolsPage->setTestAvailability(systemTests, sessionTests);
}

void MainWindow::setVarsModel(ADTTool *tool, std::unique_ptr<QStandardItemModel> model)
{
    ui->toolsPage->setVarsModel(tool, std::move(model));
}

void MainWindow::setDescription(const QString &description)
{
    ui->toolsPage->setDescription(description);
}
