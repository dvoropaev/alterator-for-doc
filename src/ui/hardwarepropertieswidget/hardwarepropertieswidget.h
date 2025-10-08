#ifndef HARDWAREPROPERTIESWIDGET_H
#define HARDWAREPROPERTIESWIDGET_H

#include <QWidget>

#include <memory>

class QAbstractItemModel;

namespace alt
{
namespace Ui
{
class HardwarePropertiesWidget;
} // namespace Ui

class HardwarePropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    HardwarePropertiesWidget(QWidget *parent = nullptr);
    ~HardwarePropertiesWidget();

public:
    void setModel(QAbstractItemModel *model);

private:
    HardwarePropertiesWidget(const HardwarePropertiesWidget &) = delete;
    HardwarePropertiesWidget(HardwarePropertiesWidget &&) = delete;
    HardwarePropertiesWidget &operator=(const HardwarePropertiesWidget &) = delete;
    HardwarePropertiesWidget &operator=(HardwarePropertiesWidget &&) = delete;

private:
    std::unique_ptr<Ui::HardwarePropertiesWidget> m_ui;
};
} // namespace alt
#endif // HARDWAREPROPERTIESWIDGET_H
