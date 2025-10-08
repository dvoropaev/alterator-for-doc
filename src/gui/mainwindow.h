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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "mainwindowinterface.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindowPrivate;

class MainWindow : public QMainWindow, public MainWindowInterface
{
public:
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void closeAll() override;
    void setController(GuiController *controller) override;
    void toggleStackWidget() override;
    void enabledActions(bool) override;
    void enableResetTestsStatusAction(bool stateEnable);
    void enabledNavigateActions(StateNavigate) override;
    void showMessStatusBar(QString message) override;
    QSize getSize() override;
    QPoint getPos() override;
    WidgetOfStackedWidget getCurrentWidget() override;
    void onDetailsDialogShow(const QString &toolId, const QString &testId) override;

    //Tests widget
    virtual void setTestsWidgetTool(const QString &toolId) override;
    virtual void resetTests(const QStringList &testsIds) override;
    virtual void setEnableStatusWidgetRunButtons(bool enable) override;
    virtual void disableTestWidgetButtons() override;
    virtual void enableTestWidgetButtons() override;
    virtual void setCommonWidgetStatus(const QString &id, WidgetStatus status) override;
    virtual void resetToolTestsLogs(const QString &toolId, const QStringList &testsIds) override;
    virtual void runTestOfTool(const QString &toolId, const QString &testId) override;
    virtual QStringList getFilteredTests(const QString &toolId) override;
    virtual void setToolFilter(const QString &filter) override;
    virtual QString getTestDisplayName(const QString &toolId, const QString &testId) override;

    //Tools widget
    virtual void setToolsWidgetModel(std::shared_ptr<TreeModel> model) override;
    virtual void enableToolsWidgetbuttons() override;
    virtual void disableToolsWidgetButtons() override;
    virtual void setToolsWidgetDescription(const QString &text) override;
    virtual void onToolsWidgetBrowseButtonClicked() override;
    virtual void changeSelectedTool(QModelIndex index) override;
    virtual void setTestAvailability(bool systemTests, bool sessionTests) override;
    void setVarsModel(ADTTool *tool, std::unique_ptr<QStandardItemModel> model) override;
    void setDescription(const QString &description) override;

private:
    void closeEvent(QCloseEvent *closeEvent) override;

private:
    Ui::MainWindow *ui{};
    MainWindowPrivate *d{};

private:
    MainWindow(const MainWindow &)            = delete;
    MainWindow(MainWindow &&)                 = delete;
    MainWindow &operator=(const MainWindow &) = delete;
    MainWindow &operator=(MainWindow &&)      = delete;

private slots:
    void on_navigateBack();
    void on_navigateForward();
    void on_runAllTests();
    void on_stopRunTests();
    void on_reportExport();
    void on_resetAllTestsStatus();
    void on_saveAllLogs();
};

#endif // MAIN_WINDOW_H
