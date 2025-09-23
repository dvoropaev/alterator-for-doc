#include "transactionwizard.h"
#include "service/transactionservice.h"
#include "ui_transactionwizard.h"

#include <QApplication>
#include <QPushButton>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <qnamespace.h>

namespace alt
{
TransactionWizard::TransactionWizard(QWidget *parent)
    : QDialog(parent)
    , ui(std::make_unique<Ui::TransactionWizard>())
{
    ui->setupUi(this);

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

    ui->stackedWidget->setCurrentIndex(0);
    for (int i = 0; i < ui->stackedWidget->count(); ++i)
    {
        auto *page = qobject_cast<WizardPage *>(ui->stackedWidget->widget(i));
        page->setWizard(this);
    }

    ui->requestPage->initialize();
}

TransactionWizard::~TransactionWizard() = default;

QPushButton *TransactionWizard::button(WizardButton button)
{
    switch (button)
    {
    case WizardButton::ApplyButton:
        return ui->applyButton;
    case WizardButton::BackButton:
        return ui->backButton;
    case WizardButton::NextButton:
        return ui->nextButton;
    case WizardButton::CancelButton:
        return ui->cancelButton;
    case WizardButton::FinishButton:
        return ui->finishButton;
    case WizardButton::RetryButton:
        return ui->retryButton;
    default:
        return nullptr;
    }
}

ApplyStatusBar *TransactionWizard::statusBar()
{
    return ui->statusBar;
}

void TransactionWizard::setNextPage()
{
    advancePage(+1);
}

void TransactionWizard::setPreviousPage()
{
    qApp->restoreOverrideCursor();
    advancePage(-1);
}

void TransactionWizard::setPage(int index)
{
    advancePage(index - ui->stackedWidget->currentIndex());
}

WizardPage *TransactionWizard::currentPage()
{
    return qobject_cast<WizardPage *>(ui->stackedWidget->currentWidget());
}

void TransactionWizard::advancePage(int offset)
{
    const auto curr = ui->stackedWidget->currentIndex();
    const auto next = curr + offset;
    if (next < 0 || next >= ui->stackedWidget->count())
    {
        return;
    }

    ui->stackedWidget->setCurrentIndex(next);
    currentPage()->initialize();
    statusBar()->setWarning(false);
}

void TransactionWizard::applyRequested()
{
    disconnect(&TransactionService::instance(),
               &TransactionService::aptNewLine,
               this->ui->progressPage,
               &ProgressWizardPage::onNewLine);
    connect(&TransactionService::instance(),
            &TransactionService::aptNewLine,
            this->ui->progressPage,
            &ProgressWizardPage::onNewLine);
    qApp->setOverrideCursor(Qt::WaitCursor);

    auto future = QtConcurrent::run(&Transaction::run, &TransactionService::current());
    future.then(this, [this](const tl::expected<void, DBusError> &result) {
        qApp->restoreOverrideCursor();
        if (currentPage() != ui->progressPage)
        {
            return;
        }
        result.has_value() ? onApplied() : onError(result.error());
    });
}

void TransactionWizard::onApplied()
{
    statusBar()->onDone({});
    currentPage()->setSubTitle(tr("The transaction is completed."));
    button(TransactionWizard::FinishButton)->setEnabled(true);
}

void TransactionWizard::onError(const DBusError &error)
{
    std::ignore = error;
    statusBar()->onDone({});
    currentPage()->setSubTitle(tr("The transaction is failed."));
    button(TransactionWizard::FinishButton)->hide();
    button(TransactionWizard::CancelButton)->show();
    button(TransactionWizard::RetryButton)->show();
}

void TransactionWizard::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        this->ui->retranslateUi(this);
    }
}
} // namespace alt
