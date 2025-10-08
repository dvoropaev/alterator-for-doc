#include "hardwarepropertieswidget.h"

#include "ui_hardwarepropertieswidget.h"

namespace alt
{
HardwarePropertiesWidget::HardwarePropertiesWidget(QWidget *parent)
    : QWidget(parent)
    , m_ui(std::make_unique<Ui::HardwarePropertiesWidget>())
{
    m_ui->setupUi(this);
}

HardwarePropertiesWidget::~HardwarePropertiesWidget() = default;

void HardwarePropertiesWidget::setModel(QAbstractItemModel *model)
{
    m_ui->propertyTableView->setModel(model);
}
} // namespace alt
