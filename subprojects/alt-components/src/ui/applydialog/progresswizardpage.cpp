#include "progresswizardpage.h"

#include "transactionwizard.h"
#include "ui/applydialog/wizardpage.h"

#include <QPushButton>

namespace alt
{
ProgressWizardPage::ProgressWizardPage(QWidget *parent)
    : WizardPage(parent)
    , textBrowser(std::make_unique<QTextBrowser>())
{
    layout()->addWidget(textBrowser.get());
}

ProgressWizardPage::~ProgressWizardPage() = default;

void ProgressWizardPage::initialize()
{
    setTitle(tr("Transaction Progress"));
    setSubTitle(tr("Please wait..."));
    textBrowser->clear();

    wizard->button(TransactionWizard::BackButton)->hide();
    wizard->button(TransactionWizard::ApplyButton)->hide();
    wizard->button(TransactionWizard::FinishButton)->show();
    wizard->button(TransactionWizard::FinishButton)->setEnabled(false);
    wizard->button(TransactionWizard::RetryButton)->hide();
    wizard->button(TransactionWizard::CancelButton)->hide();
    wizard->button(TransactionWizard::NextButton)->hide();
}

void ProgressWizardPage::onNewLine(const QString &line)
{
    wizard->setNextPage();
    wizard->statusBar()->onProgress({});
    textBrowser->append(line);
}

} // namespace alt
