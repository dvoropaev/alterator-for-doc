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

#ifndef MAIN_TESTS_WIDGET_H
#define MAIN_TESTS_WIDGET_H

#include <QHash>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "model/adttool.h"
#include "testswidgetinterface.h"

#include "statuscommonwidget.h"

namespace Ui
{
class MainTestsWidget;
}

class MainWindowInterface;

class MainTestsWidget : public QWidget, public TestWidgetInterface
{
    Q_OBJECT
public:
    MainTestsWidget(QWidget *parent = nullptr);
    virtual ~MainTestsWidget();

    virtual void setMainWindow(MainWindowInterface *window) override;
    virtual void setTool(const QString &tool) override;

    void enableButtons() override;
    void disableButtons() override;
    void resetTests(const QStringList &testsIds) override;
    virtual void setEnabledRunButtonOfStatusWidgets(bool isEnabled) override;

    void setWidgetStatus(const QString &testId, WidgetStatus status, bool moveScroll = true) override;

    QString getTool() override;
    virtual QString getDisplayName(const QString &testId) override;

    void setStatusCommonWidgetsWidth();
    void resizeEvent(QResizeEvent *event) override;

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onRunButtonCurrentStatusWidgetClicked(StatusCommonWidget *widget);
    void onDetailsButtonCurrentStatusWidgetClicked(StatusCommonWidget *widget);
    void on_checkfilter_textChanged(const QString &arg1);

private:
    void clearUi();
    void updateStatusWidgets();
    void updateListOfStatusCommonWidgets();
    StatusCommonWidget *findWidgetByTask(QString testId);

private:
    Ui::MainTestsWidget *ui{};
    MainWindowInterface *m_mainWindow;
    QVBoxLayout *m_summaryLayout{};
    QString m_tool{};
    std::vector<StatusCommonWidget *> m_statusWidgets{};

private:
    MainTestsWidget(const MainTestsWidget &)            = delete;
    MainTestsWidget(MainTestsWidget &&)                 = delete;
    MainTestsWidget &operator=(const MainTestsWidget &) = delete;
    MainTestsWidget &operator=(MainTestsWidget &&)      = delete;
};

#endif // MAIN_TESTS_WIDGET_H
