#include "transactiontabwidget.h"

#include "repository/componentrepository.h"
#include "service/transaction.h"
#include "service/transactionservice.h"
#include "ui_transactiontabwidget.h"

namespace alt
{
TransactionTabWidget::TransactionTabWidget(QWidget *parent)
    : QWidget(parent)
    , ui(std::make_unique<Ui::TransactionTabWidget>())
{
    ui->setupUi(this);
    clear();
}

TransactionTabWidget::~TransactionTabWidget() = default;

void TransactionTabWidget::setTransaction(const Transaction &transaction)
{
    setComponentList(transaction);
    setPackageList(transaction);
}

void TransactionTabWidget::setComponentList(const Transaction &transaction)
{
    ui->installComponents->clear();
    ui->removeComponents->clear();

    QIcon icon = QIcon::fromTheme("dialog-warning", QIcon(":warning"));
    for (const auto &[name, component] : transaction.components())
    {
        QListWidgetItem *item = new QListWidgetItem(component.get().displayName());
        if (transaction.action(component) == Transaction::Action::Install)
        {
            ui->installComponents->addItem(item);
        }
        else
        {
            if (transaction.status(component).isBase
                && TransactionService::safeMode().test(TransactionService::SafeMode::Base))
            {
                item->setIcon(icon);
                item->setToolTip(tr("Base component"));
            }
            ui->removeComponents->addItem(item);
        }
    }

    int instCount = ui->installComponents->count();
    int remvCount = ui->removeComponents->count();
    ui->installComponentLabel->setText(ui->installComponentLabel->text().arg(instCount));
    ui->removeComponentLabel->setText(ui->removeComponentLabel->text().arg(remvCount));

    ui->installComponents->setVisible(instCount != 0);
    ui->installComponentLabel->setVisible(instCount != 0);
    ui->removeComponents->setVisible(remvCount != 0);
    ui->removeComponentLabel->setVisible(remvCount != 0);
}

void TransactionTabWidget::setPackageList(const Transaction &transaction)
{
    ui->installPackages->clear();
    ui->removePackages->clear();

    QIcon icon = QIcon::fromTheme("dialog-warning", QIcon(":warning"));
    for (const auto &[name, package] : transaction.packages())
    {
        QListWidgetItem *item = new QListWidgetItem(name);
        if (transaction.action(*package) == Transaction::Action::Install)
        {
            ui->installPackages->addItem(item);
        }
        else
        {
            auto status = transaction.status(*package);
            if (status.isBase && TransactionService::safeMode().test(TransactionService::SafeMode::Base))
            {
                QStringList displayNames;
                for (const auto &name : status.relation)
                {
                    auto optionalComponent = ComponentRepository::get<Component>(name);
                    if (!optionalComponent.has_value())
                    {
                        continue;
                    }
                    displayNames.push_back(optionalComponent->get().displayName());
                }
                QString message = tr("The package belongs to the basic components: %1").arg(displayNames.join(", "));
                item->setIcon(icon);
                item->setToolTip(message);
            }
            else if (status.isManuallyInstalled)
            {
                item->setIcon(icon);
                item->setToolTip(tr("Manually installed package"));
            }
            ui->removePackages->addItem(item);
        }
    }

    int instCount = ui->installPackages->count();
    int remvCount = ui->removePackages->count();
    ui->installLabel->setText(ui->installLabel->text().arg(instCount));
    ui->removeLabel->setText(ui->removeLabel->text().arg(remvCount));

    ui->installPackages->setVisible(instCount != 0);
    ui->installLabel->setVisible(instCount != 0);
    ui->removePackages->setVisible(remvCount != 0);
    ui->removeLabel->setVisible(remvCount != 0);
}

void TransactionTabWidget::setStage(Stage stage)
{
    this->stage = stage;
    if (stage == Stage::Request)
    {
        ui->installComponentLabel->setText(tr("For installation (%1):"));
        ui->removeComponentLabel->setText(tr("For removal (%1):"));
        ui->installLabel->setText(tr("For installation (%1):"));
        ui->removeLabel->setText(tr("For removal (%1):"));
    }
    else if (stage == Stage::Resolve)
    {
        ui->installComponentLabel->setText(tr("Will be installed (%1):"));
        ui->removeComponentLabel->setText(tr("Will be removed (%1):"));
        ui->installLabel->setText(tr("Will be installed (%1):"));
        ui->removeLabel->setText(tr("Will be removed (%1):"));
    }
}

void TransactionTabWidget::clear()
{
    ui->installComponents->setVisible(false);
    ui->removeComponents->setVisible(false);
    ui->installComponentLabel->setVisible(false);
    ui->removeComponentLabel->setVisible(false);
    ui->installPackages->setVisible(false);
    ui->removePackages->setVisible(false);
    ui->installLabel->setVisible(false);
    ui->removeLabel->setVisible(false);
    ui->installPackages->clear();
    ui->removePackages->clear();
    ui->installComponents->clear();
    ui->removeComponents->clear();
    ui->retranslateUi(this);
    ui->tabWidget->setCurrentIndex(0);
}
} // namespace alt
