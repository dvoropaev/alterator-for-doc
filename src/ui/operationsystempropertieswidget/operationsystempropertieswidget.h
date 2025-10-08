#ifndef OPERATIONSYSTEMPROPERTIESWIDGET_H
#define OPERATIONSYSTEMPROPERTIESWIDGET_H

#include "ui/editionswidget/editionfieldwidget.h"

#include <QWidget>

#include <memory>
#include <QShortcut>

class QAbstractItemModel;

namespace alt
{
namespace Ui
{
class OperationSystemPropertiesWidget;
} // namespace Ui
class MainController;

class OperationSystemPropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    OperationSystemPropertiesWidget(QWidget *parent = nullptr);
    ~OperationSystemPropertiesWidget();

public:
    void connect(MainController *controller);
    void setModel(QAbstractItemModel *model);
    void setLogo(const QPixmap &logo);

private slots:
    void onEditRequested();

private:
    OperationSystemPropertiesWidget(const OperationSystemPropertiesWidget &) = delete;
    OperationSystemPropertiesWidget(OperationSystemPropertiesWidget &&) = delete;
    OperationSystemPropertiesWidget &operator=(const OperationSystemPropertiesWidget &) = delete;
    OperationSystemPropertiesWidget &operator=(OperationSystemPropertiesWidget &&) = delete;

private:
    std::unique_ptr<Ui::OperationSystemPropertiesWidget> m_ui;
    std::unique_ptr<EditionFieldWidget> m_editionFieldWidget;
    QShortcut m_licenseShortcut;
};
} // namespace alt
#endif // OPERATIONSYSTEMPROPERTIESWIDGET_H
