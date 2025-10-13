#include "transactionwizard.h"

#include "service/transactionservice.h"

#include "progresswizardpage.h"
#include "requestwizardpage.h"
#include "resolvewizardpage.h"
#include "ui_wizard.h"

#include <QApplication>
#include <QPushButton>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <qnamespace.h>

namespace alt
{
TransactionWizard::TransactionWizard(QWidget *parent)
    : Wizard(parent)
    , requestPage(std::make_unique<RequestWizardPage>())
    , resolvePage(std::make_unique<ResolveWizardPage>())
    , progressPage(std::make_unique<ProgressWizardPage>())
{
    ui->stackedWidget->addWidget(requestPage.get());
    ui->stackedWidget->addWidget(resolvePage.get());
    ui->stackedWidget->addWidget(progressPage.get());

    connect(button(WizardButton::ApplyButton), &QAbstractButton::clicked, this, &TransactionWizard::applyRequested);
    connect(button(WizardButton::RetryButton), &QAbstractButton::clicked, this, [this]() {
        currentPage()->initialize();
        applyRequested();
    });
    connect(button(WizardButton::NextButton), &QAbstractButton::clicked, this, &TransactionWizard::setNextPage);
    connect(button(WizardButton::BackButton), &QAbstractButton::clicked, this, &TransactionWizard::setPreviousPage);
    connect(button(WizardButton::CancelButton), &QAbstractButton::clicked, this, []() {
        qApp->restoreOverrideCursor();
    });

    for (int i = 0; i < ui->stackedWidget->count(); ++i)
    {
        auto *page = qobject_cast<WizardPage *>(ui->stackedWidget->widget(i));
        page->setWizard(this);
    }

    currentPage()->initialize();
}

TransactionWizard::~TransactionWizard() = default;

void TransactionWizard::applyRequested()
{
    disconnect(&TransactionService::instance(),
               &TransactionService::aptNewLine,
               progressPage.get(),
               &ProgressWizardPage::onNewLine);
    connect(&TransactionService::instance(),
            &TransactionService::aptNewLine,
            progressPage.get(),
            &ProgressWizardPage::onNewLine);
    qApp->setOverrideCursor(Qt::WaitCursor);

    auto future = QtConcurrent::run(&Transaction::run, &TransactionService::current());
    future.then(this, [this](const tl::expected<void, DBusError> &result) {
        qApp->restoreOverrideCursor();
        if (currentPage() != progressPage.get())
        {
            return;
        }
        result.has_value() ? onSuccess() : onFailure(result.error());
    });
}

void TransactionWizard::onSuccess()
{
    Wizard::onSuccess();
    currentPage()->setSubTitle(tr("The transaction has completed."));
}

void TransactionWizard::onFailure(const DBusError &error)
{
    Wizard::onFailure(error);
    currentPage()->setSubTitle(tr("The transaction has failed."));
}

void TransactionWizard::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        setWindowTitle(tr("Transaction Wizard"));
    }
    Wizard::changeEvent(event);
}
} // namespace alt
