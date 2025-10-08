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

#include "guicontroller.h"
#include "../app/adtexecutor.h"
#include "../gui/detailsdialog.h"
#include "gui/detailsdialog.h"
#include "gui/mainwindow.h"
#include "gui/serviceunregisteredwidget.h"
#include "gui/stopdialog.h"

#include <fstream>
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QSortFilterProxyModel>
#include <QThread>

class GuiControllerPrivate
{
public:
    GuiControllerPrivate(std::shared_ptr<TreeModel> model,
                         ADTSettings *settings,
                         ADTCommandLineOptions *options,
                         QApplication *app)
        : m_model(model)
        , m_proxyModel(new QSortFilterProxyModel())
        , m_settings(settings)
        , m_mainWindow(new MainWindow())
        , m_serviceUnregisteredWidget(new ServiceUnregisteredWidget())
        , m_stopDialog(new StopDialog())
        , m_detailsDialog(new DetailsDialog())
        , m_executor(nullptr)
        , m_isWorkingThreadActive(false)
        , m_options(options)
        , m_application(app)
    {
        m_serviceUnregisteredWidget->setWindowFlags(Qt::FramelessWindowHint);
    }

    ~GuiControllerPrivate()
    {
        delete m_proxyModel;
        delete m_serviceUnregisteredWidget;
        delete m_mainWindow;
        delete m_stopDialog;
        delete m_detailsDialog;
    }

    std::shared_ptr<TreeModel> m_model{};
    QSortFilterProxyModel *m_proxyModel{};

    ADTSettings *m_settings{};

    MainWindowInterface *m_mainWindow{};

    ServiceUnregisteredWidget *m_serviceUnregisteredWidget{};
    StopDialog *m_stopDialog{};
    DetailsDialog *m_detailsDialog;

    std::unique_ptr<ADTExecutor> m_executor{};
    QThread *m_workerThread{};
    bool m_isWorkingThreadActive;

    ADTCommandLineOptions *m_options{};

    QApplication *m_application{};

private:
    GuiControllerPrivate(const GuiControllerPrivate &)            = delete;
    GuiControllerPrivate(GuiControllerPrivate &&)                 = delete;
    GuiControllerPrivate &operator=(const GuiControllerPrivate &) = delete;
    GuiControllerPrivate &operator=(GuiControllerPrivate &&)      = delete;
};

GuiController::GuiController(std::shared_ptr<TreeModel> model,
                             DBusConfig dbusConfig,
                             ADTSettings *settings,
                             ADTCommandLineOptions *options,
                             QApplication *app)
    : BaseController(model)
    , d(new GuiControllerPrivate(model, settings, options, app))
{
    d->m_mainWindow->setController(this);
    d->m_proxyModel->setSourceModel(d->m_model.get());
    //d->m_proxyModel->sort(Qt::DisplayRole);

    d->m_mainWindow->disableToolsWidgetButtons();
    d->m_mainWindow->setToolsWidgetModel(d->m_model);

    d->m_executor = std::make_unique<ADTExecutor>(this);

    connect(d->m_executor.get(), &ADTExecutor::beginTask, this, &GuiController::onBeginTask);
    connect(d->m_executor.get(), &ADTExecutor::finishTask, this, &GuiController::onFinishTask);
    connect(d->m_executor.get(), &ADTExecutor::allTaskBegin, this, &GuiController::onAllTasksBegin);
    connect(d->m_executor.get(), &ADTExecutor::allTasksFinished, this, &GuiController::onAllTasksFinished);

    connect(d->m_serviceUnregisteredWidget,
            &ServiceUnregisteredWidget::closeAndExit,
            this,
            &GuiController::onCloseAndExitButtonPressed);
    connect(d->m_serviceUnregisteredWidget,
            &ServiceUnregisteredWidget::closeAll,
            this,
            &GuiController::on_closeButtonPressed);
}

GuiController::~GuiController()
{
    delete d;
}

void GuiController::runAllToolsWidget()
{
    d->m_mainWindow->toggleStackWidget();

    setToolFilter(QString());

    runTestsWidget(getCurrentTool(), getAllTests(getCurrentTool()));
}

void GuiController::onToolsWidgetBrowseButtonClicked()
{
    d->m_mainWindow->toggleStackWidget();
}

void GuiController::exitToolsWidget()
{
    d->m_mainWindow->closeAll();
}

void GuiController::changeSelectedTool(QModelIndex index)
{
    TreeItem *item = getTreeItemByIndex(index);

    if (!item)
        return;

    changeSelectedToolById(item->getTool()->id());
}

TreeItem *GuiController::getTreeItemByIndex(QModelIndex index)
{
    if (!index.isValid())
    {
        return nullptr;
    }

    TreeItem *item = static_cast<TreeItem *>(index.internalPointer());

    if (!item)
    {
        qWarning() << "ERROR: Can't get TreeItem by index";
        return nullptr;
    }
    return item;
}

void GuiController::runTestsWidget(const QString &toolId, const QStringList &tests)
{
    if (!d->m_isWorkingThreadActive)
    {
        d->m_executor->setTasks(toolId, tests);

        d->m_executor->resetStopFlag();

        d->m_workerThread = new QThread();

        connect(d->m_workerThread, &QThread::started, d->m_executor.get(), &ADTExecutor::runTasks);
        connect(d->m_workerThread, &QThread::finished, d->m_workerThread, &QObject::deleteLater);

        d->m_executor->moveToThread(d->m_workerThread);

        d->m_workerThread->start();
    }
}

void GuiController::runCurrentToolTest()
{
    d->m_mainWindow->resetTests(getAllTests(getCurrentTool()));

    runTestsWidget(getCurrentTool(), getAllTests(getCurrentTool()));
}

void GuiController::resetToolStatus()
{
    d->m_mainWindow->resetTests(getFilteredTests(getCurrentTool()));
}

void GuiController::backTestsWigdet()
{
    d->m_mainWindow->toggleStackWidget();
    resetToolStatus();
}

void GuiController::exitTestsWidget()
{
    if (d->m_executor->isRunning())
    {
        // TODO: show stopping dialog!
        d->m_executor->cancelTasks();
    }
    else
    {
        d->m_mainWindow->closeAll();
    }
}

void GuiController::reportTestWidget()
{
    ADTTool::BusType selectedBus = getTool( getCurrentTool() )->bus();

    if ( selectedBus == ADTTool::All )
    {
        QStringList items {
            tr("system & session bus"),
            tr("system bus"),
            tr("session bus"),
        };

        QMap<QString, ADTTool::BusType> bustypes {
            {items[0], ADTTool::All},
            {items[1], ADTTool::System},
            {items[2], ADTTool::Session}
        };

        bool ok{};
        QString item = QInputDialog::getItem(nullptr, tr("More than one report available"),
                                             tr("Please, choose where do you want to get report from:"),
                                             items, 0, false, &ok);

        selectedBus = ok && !item.isEmpty()
            ? bustypes.value(item, ADTTool::None)
            : ADTTool::None;
    }

    if ( selectedBus & ADTTool::System )
        doReport(ADTTool::System);

    if ( selectedBus & ADTTool::Session )
        doReport(ADTTool::Session);
}

void GuiController::allLogsTool()
{
    QByteArray logs = getAllTestLogs(getCurrentTool()).toLocal8Bit();

    QDate date = QDate::currentDate();

    QString fileSuffix("txt");

    QString filename(d->m_settings->getLogFilenameTemplate());
    filename.replace("%d", QString::number(date.day()));
    filename.replace("%m", QString::number(date.month()));
    filename.replace("%y", QString::number(date.year()));
    filename.replace("%name", getToolDisplayName(getCurrentTool()));
    filename.replace("%s", fileSuffix);

    QString fullFilename(d->m_settings->getLogPath() + filename);
    QString fileType(fileSuffix + QString(tr(" file")) + QString("(*.") + fileSuffix
                     + QString(tr(");;Any files (*.*)")));

    QString file = QFileDialog::getSaveFileName(nullptr, tr("Save a log of all tests"), fullFilename, fileType);

    if (file.isEmpty())
    {
        return;
    }

    std::ofstream reportFile;
    reportFile.open(file.toStdString(), std::fstream::out);

    if (reportFile.good())
    {
        reportFile.write(logs.data(), logs.size());
    }
    else
    {
        QMessageBox warningFile;
        warningFile.setText(tr("Can't save a log of all tests: ") + getToolDisplayName(getCurrentTool())
                            + " to file: " + file);
        warningFile.setIcon(QMessageBox::Warning);
        warningFile.exec();
    }
    reportFile.close();
}

void GuiController::onDetailsDialogShow(const QString &toolId, const QString &testId)
{
    d->m_detailsDialog->clearDetailsText();

    auto test = d->m_model->getTest(toolId, testId);
    if (!test)
        return;

    d->m_detailsDialog->setTestId(toolId, testId);
    updateDetailsDialog(toolId, testId);
    d->m_detailsDialog->show();
}

int GuiController::listTools()
{
    return 0;
}

int GuiController::listToolTests(const QString &toolId)
{
    auto tool = d->m_model->getTool(toolId);
    if (!tool)
    {
        qWarning() << "ERROR: can't find tool with id: " + toolId;
        // TODO: show messagebox with warning
        return 1;
    }

    changeSelectedToolById(toolId);
    d->m_mainWindow->toggleStackWidget();

    return 0;
}

int GuiController::runAllTests(const QString &toolId)
{
    auto tool = d->m_model->getTool(toolId);
    if (!tool)
    {
        qWarning() << "ERROR: can't find tool with id: " + toolId;
        // TODO: show messagebox with warning
        return 1;
    }

    changeSelectedToolById(toolId);
    d->m_mainWindow->toggleStackWidget();

    setToolFilter(QString());

    runTestsWidget(toolId, getAllTests(toolId));
    return 0;
}

void GuiController::stopAllTests()
{
    d->m_executor->cancelTasks();

    QSize sizeWindow = d->m_mainWindow->getSize();
    QPoint posWindow = d->m_mainWindow->getPos();
    QSize sizeDialog = d->m_stopDialog->getSize();
    d->m_stopDialog->move(posWindow.x() + sizeWindow.width() / 2 - sizeDialog.width() / 2,
                          posWindow.y() + sizeWindow.height() / 2 - sizeDialog.height() / 2);

    d->m_stopDialog->show();
    d->m_mainWindow->showMessStatusBar(tr("Finishing the last running test..."));
}

int GuiController::runTest(const QString &toolId, const QString &testId)
{
    if (!setCurrentTool(toolId))
    {
        qWarning() << "ERROR: can't find tool with id: " + toolId;
        // TODO: show messagebox with warning
        return 1;
    }

    if (!setCurrentTest(toolId, testId))
    {
        qWarning() << "ERROR: can't find test with id: " + testId + " in tool: " + toolId;
        // TODO: show messagebox with warning
        return 1;
    }

    runTestsWidget(toolId, {testId});
    return 0;
}

int GuiController::runApp()
{
    int result = -1;
    QString toolId;

    if (d->m_options->toolName.contains('/'))
    {
        toolId = d->m_model->getToolByPath(d->m_options->toolName);
        if (toolId.isEmpty())
        {
            toolId = d->m_options->toolName;
        }
    }
    else
    {
        toolId = d->m_options->toolName;
    }

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
    default:
        break;
    }

    auto mainWindow = dynamic_cast<MainWindow *>(d->m_mainWindow);

    mainWindow->show();

    d->m_application->exec();

    return result;
}

int GuiController::getToolReport(const QString &toolId, const QString &file)
{
    if (!setCurrentTool(toolId))
    {
        qWarning() << "ERROR: can't find tool with id: " + toolId;
        // TODO: show messagebox with warning
        return 1;
    }

    changeSelectedToolById(toolId);
    d->m_mainWindow->toggleStackWidget();
    reportTestWidget();

    return 0;
}

void GuiController::saveMainWindowSettings()
{
    d->m_settings->saveWindowsSettings(dynamic_cast<QMainWindow *>(d->m_mainWindow));
}

void GuiController::restoreMainWindowSettings()
{
    d->m_settings->restoreWindowSettings(dynamic_cast<QMainWindow *>(d->m_mainWindow));
}

void GuiController::closeAll()
{
    saveMainWindowSettings();

    WidgetOfStackedWidget widget = d->m_mainWindow->getCurrentWidget();

    switch (widget)
    {
    case WidgetOfStackedWidget::ToolsWidget:
        //Some logic here when tools widget is visible
        break;

    case WidgetOfStackedWidget::TestsWidget:
        exitTestsWidget();
        break;

    default:
        qWarning() << "ERROR: unknown widget in stacked widget!";
    }

    d->m_mainWindow->closeAll();
}

void GuiController::on_serviceUnregistered()
{
    if (d->m_executor->isRunning())
    {
        d->m_executor->wait();
    }
    d->m_serviceUnregisteredWidget->show();
    d->m_serviceUnregisteredWidget->startAnimation();
}

void GuiController::on_serviceRegistered()
{
    if (d->m_serviceUnregisteredWidget->isVisible())
    {
        d->m_serviceUnregisteredWidget->close();
    }

    if (d->m_executor->isRunning())
    {
        d->m_executor->resetWaitFlag();
    }
}

void GuiController::on_serviceOwnerChanged() {}

void GuiController::getStdoutFromTest(QString toolId, QString testId, QString text)
{
    if (toolId == getCurrentTool() && testId == getCurrentTest())
    {
        updateDetailsDialog(toolId, testId);
    }
}

void GuiController::getStderrFromTest(QString toolId, QString testId, QString text)
{
    if (toolId == getCurrentTool() && testId == getCurrentTest())
    {
        updateDetailsDialog(toolId, testId);
    }
}

void GuiController::changeSelectedToolById(QString id)
{
    if (!setCurrentTool(id))
        return;

    d->m_mainWindow->setToolsWidgetDescription(getToolComment(id));
    d->m_mainWindow->setTestAvailability(hasSystemTests(getCurrentTool()), hasSessionTests(getCurrentTool()));

    d->m_mainWindow->enableToolsWidgetbuttons();
    d->m_mainWindow->enabledActions(true);
    d->m_mainWindow->enableResetTestsStatusAction(false);
    d->m_mainWindow->enabledNavigateActions(MainWindowInterface::StateNavigate::disabledAll);

    d->m_mainWindow->setTestsWidgetTool(id);
    d->m_mainWindow->setVarsModel(getTool(id), getVarsModel(id));
    d->m_mainWindow->setDescription(getTool(id)->comment());
}

void GuiController::updateDetailsDialog(const QString &toolId, const QString &testId)
{
    auto test = getTest(toolId, testId);

    if (!test)
        return;

    if (d->m_detailsDialog->getToolId() == toolId && d->m_detailsDialog->getTestId() == testId)
    {
        d->m_detailsDialog->clearDetailsText();

        d->m_detailsDialog->setDetailsText(test->log());
        if (d->m_detailsDialog->isVisible())
            d->m_detailsDialog->repaint();
    }
}

void GuiController::doReport(ADTTool::BusType bus)
{
    QByteArray report;

    if (getToolReports(report, bus) != 0)
    {
        QMessageBox warning;
        warning.setText(tr("Can't get report from tool: ") + getToolDisplayName(getCurrentTool()));
        warning.setIcon(QMessageBox::Warning);
        warning.exec();

        return;
    }

    QDate date = QDate::currentDate();

    QString fileSuffix(getReportSuffix(getCurrentTool(), bus));

    QString filename(d->m_settings->getReportFilenameTemplate());
    filename.replace("%d", QString::number(date.day()));
    filename.replace("%m", QString::number(date.month()));
    filename.replace("%y", QString::number(date.year()));
    filename.replace("%name", getToolDisplayName(getCurrentTool()));
    filename.replace("%s", fileSuffix);

    QString fullFilename(d->m_settings->getReportPath() + filename);
    QString fileType(fileSuffix + QString(tr(" file")) + QString("(*.") + fileSuffix
                     + QString(tr(");;Any files (*.*)")));

    QString file = QFileDialog::getSaveFileName(nullptr, tr("Save report"), fullFilename, fileType);

    if (file.isEmpty())
    {
        return;
    }

    std::ofstream reportFile;
    reportFile.open(file.toStdString(), std::fstream::out);

    if (reportFile.good())
    {
        reportFile.write(report.data(), report.size());
    }
    else
    {
        QMessageBox warningFile;
        warningFile.setText(tr("Can't save report from tool: ") + getToolDisplayName(getCurrentTool())
                            + " to file: " + file);
        warningFile.setIcon(QMessageBox::Warning);
        warningFile.exec();
    }
    reportFile.close();
}

void GuiController::onAllTasksBegin()
{
    d->m_isWorkingThreadActive = true;
    d->m_mainWindow->setEnableStatusWidgetRunButtons(false);
    d->m_mainWindow->disableTestWidgetButtons();
    d->m_mainWindow->enabledActions(false);
    d->m_mainWindow->enableResetTestsStatusAction(false);
    d->m_mainWindow->enabledNavigateActions(MainWindowInterface::StateNavigate::disabledAll);
}

void GuiController::onAllTasksFinished()
{
    d->m_isWorkingThreadActive = false;
    d->m_mainWindow->setEnableStatusWidgetRunButtons(true);
    d->m_mainWindow->enableTestWidgetButtons();
    d->m_mainWindow->enabledActions(true);
    d->m_mainWindow->enableResetTestsStatusAction(true);
    d->m_mainWindow->enabledNavigateActions(MainWindowInterface::StateNavigate::enabledBack);
}

void GuiController::onBeginTask(QString toolId, QString testId)
{
    auto test = d->m_model->getTest(toolId, testId);
    if (!test)
        return;

    if (!setCurrentTool(toolId))
        return;

    if (!setCurrentTest(toolId, testId))
        return;

    d->m_mainWindow->setCommonWidgetStatus(testId, WidgetStatus::running);
    d->m_mainWindow->showMessStatusBar(tr("Running: ") + getTestDisplayName(toolId, testId));
}

void GuiController::onFinishTask(QString toolId, QString testId)
{
    if (getTestExitCode(toolId, testId) == 0)
    {
        d->m_mainWindow->setCommonWidgetStatus(testId, WidgetStatus::finishedOk);
        d->m_mainWindow->showMessStatusBar(tr("OK: ") + getTestDisplayName(toolId, testId));
    }
    else
    {
        d->m_mainWindow->setCommonWidgetStatus(testId, WidgetStatus::finishedFailed);
        d->m_mainWindow->showMessStatusBar(tr("FAILED: ") + getTestDisplayName(toolId, testId));
    }

    //TODO: add check for exist
    d->m_stopDialog->close();
}

void GuiController::onCloseAndExitButtonPressed()
{
    d->m_executor->cancelTasks();
    d->m_executor->resetWaitFlag();
    d->m_serviceUnregisteredWidget->close();
    d->m_mainWindow->closeAll();
}

void GuiController::on_closeButtonPressed()
{
    d->m_executor->cancelTasks();
    d->m_executor->resetWaitFlag();
    d->m_serviceUnregisteredWidget->close();
}
