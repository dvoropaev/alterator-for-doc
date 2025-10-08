#include "operationsystempropertieswidget.h"

#include "ui_operationsystempropertieswidget.h"

#include "ui/dialogs/editionwizard/editionwizard.h"

#include "controller/maincontroller.h"

#include "databuilder/databuilder.h"

#include <QDebug>
#include <QLineEdit>
#include <QTableView>

namespace alt
{
OperationSystemPropertiesWidget::OperationSystemPropertiesWidget(QWidget *parent)
    : QWidget(parent)
    , m_ui(std::make_unique<Ui::OperationSystemPropertiesWidget>())
    , m_editionFieldWidget(nullptr)
    , m_licenseShortcut(QKeySequence(Qt::CTRL | Qt::Key_L), this)
{
    m_ui->setupUi(this);
}

OperationSystemPropertiesWidget::~OperationSystemPropertiesWidget() = default;

void OperationSystemPropertiesWidget::onEditRequested()
{
    disconnect(m_editionFieldWidget.get());
    auto dialog = std::make_unique<EditionWizard>(this->parentWidget()->parentWidget());
    dialog->exec();
    if (dialog->result() == QDialog::Accepted)
    {
        DataBuilder::rebuildProduct();
        setModel(DataBuilder::buildPropertyModel(DataBuilder::PropertyModelTypeOperationSystem).release());
        QPixmap p;
        p.loadFromData(DataBuilder::product().logo().value());
        m_ui->logoLabel->setPixmap(p);
    }
}

void OperationSystemPropertiesWidget::connect(MainController *controller)
{
    QObject::connect(m_ui->licensePushButton,
                     &QPushButton::released,
                     controller,
                     &MainController::onLicenseDialogRequested);
    QObject::connect(&m_licenseShortcut, &QShortcut::activated, controller, &MainController::onLicenseDialogRequested);
}

void OperationSystemPropertiesWidget::setModel(QAbstractItemModel *model)
{
    m_ui->propertyTableView->setModel(model);

    auto *tableView = m_ui->propertyTableView->tableView();

    for (size_t i = 0; i < model->rowCount(); ++i)
    {
        auto propertyNameModelIndex = model->index(i, 0);
        auto propertyValueModelIndex = model->index(i, 1);
        auto text = propertyValueModelIndex.data(Qt::DisplayRole).toString();
        bool isEditable = propertyValueModelIndex.data(Qt::UserRole).toBool();
        if (isEditable)
        {
            m_editionFieldWidget = std::make_unique<EditionFieldWidget>();
            m_editionFieldWidget->setText(text);

            QObject::connect(m_editionFieldWidget.get(),
                             &EditionFieldWidget::requestEdit,
                             this,
                             &OperationSystemPropertiesWidget::onEditRequested);

            tableView->setIndexWidget(propertyValueModelIndex, m_editionFieldWidget.get());
            model->setData(propertyValueModelIndex, "", Qt::DisplayRole);
        }
    }
}

void OperationSystemPropertiesWidget::setLogo(const QPixmap &logo)
{
    m_ui->logoLabel->setPixmap(logo);
}
} // namespace alt
