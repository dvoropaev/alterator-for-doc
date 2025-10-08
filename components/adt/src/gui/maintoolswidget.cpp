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

#include "maintoolswidget.h"
#include "ui_maintoolswidget.h"

#include <QDebug>
#include <QMouseEvent>

MainToolsWidget::MainToolsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainToolsWidget)
{
    ui->setupUi(this);

    ui->toolsListView->viewport()->installEventFilter(this);

    QList<int> splitterSize = {geometry().width() / 5, geometry().width() * 4 / 5};
    ui->toolsSplitter->setSizes(splitterSize);
    ui->toolsSplitter->setChildrenCollapsible(false);
}

MainToolsWidget::~MainToolsWidget()
{
    delete ui;
}

void MainToolsWidget::enableButtons()
{
    ui->browseCheckPushButton->setEnabled(true);
}

void MainToolsWidget::disableButtons()
{
    ui->browseCheckPushButton->setEnabled(false);
}

void MainToolsWidget::setModel(QAbstractItemModel *model)
{
    ui->toolsListView->setModel(model);

    connect(ui->toolsListView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &MainToolsWidget::onSelectionChanged);
}

void MainToolsWidget::setMainWindow(MainWindowInterface *window)
{
    m_mainWindow = window;
}

void MainToolsWidget::setDescription(const QString &description)
{
    ui->toolWidget->setDescription(description);
}

void MainToolsWidget::setTestAvailability(bool systemTests, bool sessionTests)
{
    ui->toolWidget->setTestAvailability(systemTests, sessionTests);
}

void MainToolsWidget::setVarsModel(ADTTool *tool, std::unique_ptr<QStandardItemModel> model)
{
    ui->toolWidget->setVarsModel(tool, std::move(model));
}

bool MainToolsWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->toolsListView->viewport() && event->type() == QEvent::MouseButtonDblClick)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        QModelIndex index       = ui->toolsListView->indexAt(mouseEvent->pos());

        if (index.isValid())
        {
            m_mainWindow->changeSelectedTool(index);

            on_browseCheckPushButton_clicked();
        }
    }
    return QWidget::eventFilter(watched, event);
}

void MainToolsWidget::on_browseCheckPushButton_clicked()
{
    m_mainWindow->onToolsWidgetBrowseButtonClicked();
}

void MainToolsWidget::onSelectionChanged(const QItemSelection &newSelection, const QItemSelection &previousSelection)
{
    if (newSelection.isEmpty())
    {
        return;
    }

    QModelIndex currentIndex = newSelection.indexes().at(0);
    m_mainWindow->changeSelectedTool(currentIndex);
}
