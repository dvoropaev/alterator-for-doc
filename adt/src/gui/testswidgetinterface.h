#ifndef TESTS_WIDGET_INTERFACE_H
#define TESTS_WIDGET_INTERFACE_H

#include "../model/adttool.h"
#include "controllers/basecontroller.h"
#include "mainwindowinterface.h"
#include "statuscommonwidget.h"
#include "widgetstatus.h"

#include <QIcon>

class StatusCommonWidget;
class MainWindowInterface;

class TestWidgetInterface
{
public:
    virtual ~TestWidgetInterface() = default;

    virtual void setMainWindow(MainWindowInterface *window) = 0;
    virtual void setTool(const QString &tool)               = 0;

    virtual void enableButtons()                                    = 0;
    virtual void disableButtons()                                   = 0;
    virtual void resetTests(const QStringList &testsIds)            = 0;
    virtual void setEnabledRunButtonOfStatusWidgets(bool isEnabled) = 0;

    virtual QString getDisplayName(const QString &testId) = 0;

    virtual QString getTool() = 0;

    virtual void setWidgetStatus(const QString &testId, WidgetStatus status, bool moveScroll = true) = 0;
};

#endif // TESTS_WIDGET_INTERFACE_H
