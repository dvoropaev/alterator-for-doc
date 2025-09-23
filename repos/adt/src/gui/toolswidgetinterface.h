#ifndef TOOLS_WIDGET_INTERFACE_H
#define TOOLS_WIDGET_INTERFACE_H

#include <QAbstractItemModel>
#include <QString>

#include "mainwindowinterface.h"

class MainWindowInterface;

class ToolsWidgetInterface
{
public:
    virtual ~ToolsWidgetInterface() = default;

    virtual void setModel(QAbstractItemModel *model)        = 0;
    virtual void setMainWindow(MainWindowInterface *window) = 0;

    virtual void enableButtons()  = 0;
    virtual void disableButtons() = 0;

    virtual void setTestAvailability(bool systemTests, bool sessionTests)               = 0;
    virtual void setVarsModel(ADTTool *tool, std::unique_ptr<QStandardItemModel> model) = 0;
    virtual void setDescription(const QString &description)                             = 0;
};

#endif // TOOLS_WIDGET_INTERFACE_H
