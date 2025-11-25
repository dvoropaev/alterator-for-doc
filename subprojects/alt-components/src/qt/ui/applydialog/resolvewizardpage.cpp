#include "resolvewizardpage.h"

#include "service/transaction.h"
#include "service/transactionservice.h"
#include "transactionwizard.h"

#include <QApplication>
#include <QPushButton>
#include <QtConcurrent/QtConcurrent>

namespace alt
{
ResolveWizardPage::ResolveWizardPage(const std::shared_ptr<TransactionService> &service,
                                     const std::shared_ptr<ComponentRepository> &components,
                                     QWidget *parent)
    : WizardPage(parent)
    , service(service)
    , componentRepository(components)
    , centralWidget(std::make_unique<TransactionTabWidget>(service, componentRepository))
{
    layout()->addWidget(centralWidget.get());
}

ResolveWizardPage::~ResolveWizardPage() = default;

void ResolveWizardPage::initialize()
{
    setTitle(tr("Resolved Components"));
    setSubTitle(tr("Components included in the transaction after dependency resolution."));
    centralWidget->clear();
    centralWidget->setStage(TransactionTabWidget::Stage::Resolve);
    centralWidget->setEnabled(false);

    wizard->statusBar()->onStarted(tr("Resolving list of components involved in the transaction..."));
    qApp->setOverrideCursor(Qt::WaitCursor);
    QtConcurrent::run([this] {
        return service->resolve(service->current());
    }).then(this, [this](const tl::expected<std::reference_wrapper<Transaction>, TransactionService::Error> &result) {
        if (result.has_value())
        {
            onResolve(result->get());
        }
        else
        {
            wizard->button(TransactionWizard::BackButton)->setEnabled(true);
            wizard->button(TransactionWizard::NextButton)->setEnabled(true);
            wizard->statusBar()->onDone({});
            qApp->restoreOverrideCursor();
            onError(QString::fromStdString(result.error().details));
        }
    });

    wizard->button(TransactionWizard::BackButton)->show();
    wizard->button(TransactionWizard::BackButton)->setEnabled(false);
    wizard->button(TransactionWizard::ApplyButton)->show();
    wizard->button(TransactionWizard::ApplyButton)->setEnabled(false);
    wizard->button(TransactionWizard::FinishButton)->hide();
    wizard->button(TransactionWizard::RetryButton)->hide();
    wizard->button(TransactionWizard::CancelButton)->show();
    wizard->button(TransactionWizard::NextButton)->hide();
    wizard->button(TransactionWizard::NextButton)->setEnabled(false);
}

void ResolveWizardPage::onResolve(const Transaction &transaction)
{
    wizard->button(TransactionWizard::BackButton)->setEnabled(true);
    wizard->button(TransactionWizard::NextButton)->setEnabled(true);
    qApp->restoreOverrideCursor();
    if (wizard->currentPage() != this)
    {
        return;
    }

    centralWidget->setEnabled(true);
    centralWidget->setTransaction(transaction);
    if (transaction.status() == Transaction::Status::Allowed)
    {
        wizard->button(TransactionWizard::ApplyButton)->setEnabled(true);
        wizard->statusBar()->onDone(tr("Check out the list of packages in \"Packages\" tab!"));
        wizard->statusBar()->setWarning(true);
        return;
    }

    wizard->button(TransactionWizard::ApplyButton)->setEnabled(false);
    wizard->statusBar()->setError(true);

    TransactionService::SafeMode accessDenied{TransactionService::SafeMode::No};
    for (const auto &name : transaction.packages())
    {
        auto action = transaction.action(name);
        if (action != Transaction::Action::Remove)
        {
            continue;
        }

        auto status = transaction.status(name);
        auto safeMode = service->safeMode();
        if (status.isBase && safeMode.test(TransactionService::SafeMode::Base))
        {
            accessDenied.set(TransactionService::SafeMode::Base);
            break;
        }
        if (status.isManuallyInstalled && safeMode.test(TransactionService::SafeMode::Manually))
        {
            accessDenied.set(TransactionService::SafeMode::Manually);
            if (!safeMode.test(TransactionService::SafeMode::Base))
            {
                break;
            }
        }
    }

    if (accessDenied.test(TransactionService::SafeMode::Base))
    {
        wizard->statusBar()->onDone(tr("The transaction contains base components!"));
    }
    else if (accessDenied.test(TransactionService::SafeMode::Manually))
    {
        wizard->statusBar()->onDone(tr("The transaction contains manually installed packages!"));
    }
}
} // namespace alt
