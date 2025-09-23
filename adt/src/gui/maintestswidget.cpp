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

#include "maintestswidget.h"
#include "statuscommonwidget.h"
#include "ui_maintestswidget.h"
#include <cstddef>

#include <QDebug>
#include <QScrollBar>
#include <QStyle>
#include <QThread>

const int LAYOUT_STRETCH_INDEX  = -1;
const int LAYOUT_STRETCH_FACTOR = 4000;
const int LAYOUT_INDEX          = 100;

MainTestsWidget::MainTestsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainTestsWidget)
    , m_mainWindow(nullptr)
    , m_summaryLayout(new QVBoxLayout())
{
    ui->setupUi(this);

    ui->summaryScrollAreaWidgetContents->setLayout(m_summaryLayout);
    ui->summaryScrollArea->horizontalScrollBar()->setEnabled(false);
}

MainTestsWidget::~MainTestsWidget()
{
    delete ui;
}

void MainTestsWidget::setMainWindow(MainWindowInterface *window)
{
    m_mainWindow = window;
}

void MainTestsWidget::setTool(const QString &tool)
{
    m_tool = tool;

    clearUi();
    updateStatusWidgets();
    updateListOfStatusCommonWidgets();
}

void MainTestsWidget::enableButtons()
{
    ui->checkfilter->setEnabled(true);
}

void MainTestsWidget::disableButtons()
{
    ui->checkfilter->setEnabled(false);
}

void MainTestsWidget::resetTests(const QStringList &testsIds)
{
    m_mainWindow->resetToolTestsLogs(m_tool, testsIds);

    for (auto testId : testsIds)
        setWidgetStatus(testId, WidgetStatus::idle);
}

void MainTestsWidget::setEnabledRunButtonOfStatusWidgets(bool isEnabled)
{
    std::for_each(m_statusWidgets.begin(), m_statusWidgets.end(), [isEnabled](StatusCommonWidget *widget) {
        widget->setEnabledRunButton(isEnabled);
    });
}

void MainTestsWidget::setWidgetStatus(const QString &testId, WidgetStatus status, bool moveScroll)
{
    StatusCommonWidget *currentWidget = findWidgetByTask(testId);

    if (!currentWidget)
    {
        return;
    }

    currentWidget->setWidgetStatus(status);

    if (moveScroll)
    {
        ui->summaryScrollArea->ensureWidgetVisible(currentWidget);
    }
}

void MainTestsWidget::onRunButtonCurrentStatusWidgetClicked(StatusCommonWidget *widget)
{
    m_mainWindow->runTestOfTool(m_tool, widget->getTestId());
}

void MainTestsWidget::onDetailsButtonCurrentStatusWidgetClicked(StatusCommonWidget *widget)
{
    m_mainWindow->onDetailsDialogShow(widget->getToolId(), widget->getTestId());
}

void MainTestsWidget::updateStatusWidgets()
{
    m_statusWidgets.clear();

    QStringList tests = m_mainWindow->getFilteredTests(m_tool);

    if (tests.empty())
        return;

    for (auto &testId : tests)
    {
        StatusCommonWidget *currentWidget = new StatusCommonWidget(this, testId);
        connect(currentWidget,
                &StatusCommonWidget::logsButtonClicked,
                this,
                &MainTestsWidget::onDetailsButtonCurrentStatusWidgetClicked);

        connect(currentWidget,
                &StatusCommonWidget::runButtonCLicked,
                this,
                &MainTestsWidget::onRunButtonCurrentStatusWidgetClicked);

        m_statusWidgets.push_back(currentWidget);
    }
}

void MainTestsWidget::updateListOfStatusCommonWidgets()
{
    for (size_t i = 0; i < m_statusWidgets.size(); ++i)
    {
        m_summaryLayout->insertWidget(i, static_cast<StatusCommonWidget *>(m_statusWidgets[i]), Qt::AlignTop);
    }
    m_summaryLayout->insertStretch(LAYOUT_STRETCH_INDEX, LAYOUT_STRETCH_FACTOR);
}

void MainTestsWidget::clearUi()
{
    delete ui->summaryScrollAreaWidgetContents;

    ui->summaryScrollAreaWidgetContents = new QWidget();

    ui->summaryScrollArea->setWidget(ui->summaryScrollAreaWidgetContents);

    m_summaryLayout = new QVBoxLayout();
    m_summaryLayout->setAlignment(Qt::AlignTop);

    ui->summaryScrollAreaWidgetContents->setLayout(m_summaryLayout);

    ui->stackedWidget->setCurrentIndex(0);
}

StatusCommonWidget *MainTestsWidget::findWidgetByTask(QString testId)
{
    const auto find = std::find_if(m_statusWidgets.begin(),
                                   m_statusWidgets.end(),
                                   [this, &testId](StatusCommonWidget *widget) {
                                       return widget->getTestId() == testId;
                                   });

    if (find == m_statusWidgets.end())
    {
        qWarning() << "ERROR: can't find status widget by task";
        return nullptr;
    }

    return *find;
}

QString MainTestsWidget::getTool()
{
    return m_tool;
}

QString MainTestsWidget::getDisplayName(const QString &testId)
{
    return m_mainWindow->getTestDisplayName(m_tool, testId);
}

void MainTestsWidget::setStatusCommonWidgetsWidth()
{
    int verticalScrollBarWidth = 0;
    if (ui->summaryScrollArea->verticalScrollBar()->isVisible())
        verticalScrollBarWidth = ui->summaryScrollArea->verticalScrollBar()->width();

    std::for_each(m_statusWidgets.begin(), m_statusWidgets.end(), [=](StatusCommonWidget *widget) {
        widget->setNameTestLength(ui->commonPage->width(), verticalScrollBarWidth);
    });
}

void MainTestsWidget::resizeEvent(QResizeEvent *event)
{
    setStatusCommonWidgetsWidth();
}

void MainTestsWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    setStatusCommonWidgetsWidth();
}

void MainTestsWidget::on_checkfilter_textChanged(const QString &arg1)
{
    m_mainWindow->setToolFilter(arg1);

    clearUi();

    updateStatusWidgets();
    updateListOfStatusCommonWidgets();
}
