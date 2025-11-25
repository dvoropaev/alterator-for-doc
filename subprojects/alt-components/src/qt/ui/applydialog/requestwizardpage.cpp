#include "requestwizardpage.h"

#include "transactiontabwidget.h"
#include "transactionwizard.h"
#include <QPushButton>

namespace alt
{
RequestWizardPage::RequestWizardPage(const std::shared_ptr<TransactionService> &service,
                                     const std::shared_ptr<ComponentRepository> &components,
                                     QWidget *parent)
    : WizardPage(parent)
    , service(service)
    , componentRepository(components)
    , centralWidget(std::make_unique<TransactionTabWidget>(service, componentRepository))
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
    centralWidget->setTransaction(service->current());

    wizard->button(TransactionWizard::BackButton)->hide();
    wizard->button(TransactionWizard::ApplyButton)->hide();
    wizard->button(TransactionWizard::FinishButton)->hide();
    wizard->button(TransactionWizard::RetryButton)->hide();
    wizard->button(TransactionWizard::CancelButton)->show();
    wizard->button(TransactionWizard::NextButton)->show();

    wizard->statusBar()->StatusBar::onDone({});
}
} // namespace alt
