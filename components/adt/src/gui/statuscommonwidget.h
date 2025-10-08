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

#ifndef STATUS_COMMON_WIDGET_H
#define STATUS_COMMON_WIDGET_H

#include <mutex>

#include "../model/treeitem.h"
#include "testswidgetinterface.h"
#include "ui_statuscommonwidget.h"
#include "widgetstatus.h"

namespace Ui
{
class Ui_StatusCommonWidget;
}

class TestWidgetInterface;

class StatusCommonWidget : public QWidget
{
    Q_OBJECT

    struct WidgetBackgroundColor
    {
        WidgetBackgroundColor() {}
        WidgetBackgroundColor(int r, int g, int b, int a)
        {
            red   = r;
            green = g;
            blue  = b;
            alpha = a;
        }
        int red   = 0;
        int green = 0;
        int blue  = 0;
        int alpha = 0;
    };

    struct TestOkColor : public WidgetBackgroundColor
    {
        TestOkColor()
            : WidgetBackgroundColor(0, 170, 0, 50)
        {}
    };

    struct TestFailedColor : public WidgetBackgroundColor
    {
        TestFailedColor()
            : WidgetBackgroundColor(255, 0, 0, 50)
        {}
    };

    struct TestIdleColor : public WidgetBackgroundColor
    {
        TestIdleColor()
            : WidgetBackgroundColor(0, 0, 0, 0)
        {}
    };

    struct TestReadyColor : public WidgetBackgroundColor
    {
        TestReadyColor()
            : WidgetBackgroundColor(186, 189, 182, 50)
        {}
    };

    struct TestRunningColor : public WidgetBackgroundColor
    {
        TestRunningColor()
            : WidgetBackgroundColor(0, 0, 0, 50)
        {}
    };

public:
    StatusCommonWidget(TestWidgetInterface *widget, QString test, QWidget *parent = nullptr);
    virtual ~StatusCommonWidget();

    QString getToolId();
    QString getTestId();

    void setText(QString text);
    void setIcon(QIcon &icon);
    void setEnabledRunButton(bool isEnabled);
    void setWidgetStatus(WidgetStatus status);
    WidgetStatus getWidgetStatus();

    QString getLabelText();

    void setNameTestLength(int size, int verticalScrollBarWidth = 0);

    void showEvent(QShowEvent *event) override;

signals:
    void logsButtonClicked(StatusCommonWidget *widget);
    void runButtonCLicked(StatusCommonWidget *widget);

private:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void setColorWidget(int deltaColor);

private slots:
    void on_runPushButton_clicked();
    void on_logsPushButton_clicked();

private:
    Ui::StatusCommonWidget *m_ui{};
    TestWidgetInterface *m_widget;
    QString m_test;
    QString m_labelText{};

    WidgetBackgroundColor m_backColorWidget;

    std::recursive_mutex m_mutex;

    WidgetStatus m_status;

private:
    StatusCommonWidget(const StatusCommonWidget &)            = delete;
    StatusCommonWidget(StatusCommonWidget &&)                 = delete;
    StatusCommonWidget &operator=(const StatusCommonWidget &) = delete;
    StatusCommonWidget &operator=(StatusCommonWidget &&)      = delete;
};

#endif // STATUS_COMMON_WIDGET_H
