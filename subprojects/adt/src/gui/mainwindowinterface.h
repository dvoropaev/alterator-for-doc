#ifndef MAIN_WINDOW_INTERFACE_H
#define MAIN_WINDOW_INTERFACE_H

#include "../controllers/guicontroller.h"
#include "testswidgetinterface.h"
#include "toolswidgetinterface.h"
#include "widgetstatus.h"

enum class WidgetOfStackedWidget
{
    None,
    ToolsWidget,
    TestsWidget
};

class MainWindowInterface
{
public:
    enum class StateNavigate
    {
        disabledAll,
        enabledAll,
        enabledBack,
        enabledForward
    };

    virtual ~MainWindowInterface() = default;

    virtual void setController(GuiController *controller) = 0;

    virtual void closeAll() = 0;

    virtual void toggleStackWidget() = 0;

    virtual void enabledActions(bool) = 0;

    virtual void enableResetTestsStatusAction(bool) = 0;

    virtual void enabledNavigateActions(StateNavigate) = 0;

    virtual void showMessStatusBar(QString message) = 0;

    virtual QSize getSize() = 0;

    virtual QPoint getPos() = 0;

    virtual WidgetOfStackedWidget getCurrentWidget() = 0;

    virtual void onDetailsDialogShow(const QString &toolId, const QString &testId) = 0;

    //Tests widget
    virtual void resetTests(const QStringList &testsIds)                                = 0;
    virtual void setTestsWidgetTool(const QString &toolId)                              = 0;
    virtual void setEnableStatusWidgetRunButtons(bool enable)                           = 0;
    virtual void disableTestWidgetButtons()                                             = 0;
    virtual void enableTestWidgetButtons()                                              = 0;
    virtual void setCommonWidgetStatus(const QString &id, WidgetStatus status)          = 0;
    virtual void resetToolTestsLogs(const QString &toolId, const QStringList &testsIds) = 0;
    virtual void runTestOfTool(const QString &tool, const QString &testId)              = 0;
    virtual QStringList getFilteredTests(const QString &toolId)                         = 0;
    virtual void setToolFilter(const QString &filter)                                   = 0;
    virtual QString getTestDisplayName(const QString &toolid, const QString &testId)    = 0;

    //Tools widget
    virtual void setToolsWidgetModel(std::shared_ptr<TreeModel> model)                  = 0;
    virtual void enableToolsWidgetbuttons()                                             = 0;
    virtual void disableToolsWidgetButtons()                                            = 0;
    virtual void setToolsWidgetDescription(const QString &text)                         = 0;
    virtual void onToolsWidgetBrowseButtonClicked()                                     = 0;
    virtual void changeSelectedTool(QModelIndex index)                                  = 0;
    virtual void setTestAvailability(bool systemTests, bool sessionTests)               = 0;
    virtual void setVarsModel(ADTTool *tool, std::unique_ptr<QStandardItemModel> model) = 0;
    virtual void setDescription(const QString &description)                             = 0;
};

#endif // MAIN_WINDOW_INTERFACE_H
