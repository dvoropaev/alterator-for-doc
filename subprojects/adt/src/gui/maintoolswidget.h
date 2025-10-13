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

#ifndef MAIN_TOOLS_WIDGET_H
#define MAIN_TOOLS_WIDGET_H

#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QWidget>

#include "mainwindowinterface.h"
#include "toolswidgetinterface.h"

namespace Ui
{
class MainToolsWidget;
}

class MainToolsWidget : public QWidget, public ToolsWidgetInterface
{
    Q_OBJECT
public:
    MainToolsWidget(QWidget *parent = nullptr);
    virtual ~MainToolsWidget();

public:
    void setModel(QAbstractItemModel *model) override;
    void setMainWindow(MainWindowInterface *window) override;

    void enableButtons() override;
    void disableButtons() override;

    void setDescription(const QString &description) override;
    void setTestAvailability(bool systemTests, bool sessionTests) override;
    void setVarsModel(ADTTool *tool, std::unique_ptr<QStandardItemModel> model) override;

    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_browseCheckPushButton_clicked();
    void onSelectionChanged(const QItemSelection &newSelection, const QItemSelection &previousSelection);

private:
    Ui::MainToolsWidget *ui{};
    MainWindowInterface *m_mainWindow{};

private:
    MainToolsWidget(const MainToolsWidget &)            = delete;
    MainToolsWidget(MainToolsWidget &&)                 = delete;
    MainToolsWidget &operator=(const MainToolsWidget &) = delete;
    MainToolsWidget &operator=(MainToolsWidget &&)      = delete;
};

#endif // MAIN_TOOLS_WIDGET_H
