#ifndef TOOL_WIDGET_H
#define TOOL_WIDGET_H

#include <QObject>
#include <QStandardItemModel>
#include <QWidget>

#include <model/adttool.h>

namespace Ui
{
class ToolWidget;
}

class ToolWidget : public QWidget
{
    Q_OBJECT
public:
    ToolWidget(QWidget *parent = nullptr);
    virtual ~ToolWidget();

    void setTestAvailability(bool systemTests, bool sessionTests);
    void setVarsModel(ADTTool *tool, std::unique_ptr<QStandardItemModel> model);
    void setDescription(const QString &description);

private:
    Ui::ToolWidget *ui;

private:
    ToolWidget(const ToolWidget &)            = delete;
    ToolWidget(ToolWidget &&)                 = delete;
    ToolWidget &operator=(const ToolWidget &) = delete;
    ToolWidget &operator=(ToolWidget &&)      = delete;
};

#endif //TOOL_WIDGET_H
