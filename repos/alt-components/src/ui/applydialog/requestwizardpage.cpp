#include "requestwizardpage.h"

#include "service/transactionservice.h"
#include "ui/applydialog/transactiontabwidget.h"
#include "ui/applydialog/transactionwizard.h"
#include <QPushButton>

namespace alt
{
RequestWizardPage::RequestWizardPage(QWidget *parent)
    : WizardPage(parent)
    , centralWidget(std::make_unique<TransactionTabWidget>())
{
    layout()->addWidget(centralWidget.get());
}

RequestWizardPage::~RequestWizardPage() = default;

void RequestWizardPage::initialize()
{
    setTitle(tr("Requested Components"));
    setSubTitle(tr("Component and package lists that you requested."));
    centralWidget->clear();
    centralWidget->setStage(TransactionTabWidget::Stage::Request);
    centralWidget->setTransaction(TransactionService::current());

    wizard->button(TransactionWizard::BackButton)->hide();
    wizard->button(TransactionWizard::ApplyButton)->hide();
    wizard->button(TransactionWizard::FinishButton)->hide();
    wizard->button(TransactionWizard::RetryButton)->hide();
    wizard->button(TransactionWizard::CancelButton)->show();
    wizard->button(TransactionWizard::NextButton)->show();

    wizard->statusBar()->StatusBar::onDone({});
}
} // namespace alt
