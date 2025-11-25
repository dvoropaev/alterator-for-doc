#include "transactionwizard.h"

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
TransactionWizard::TransactionWizard(const std::shared_ptr<TransactionService> &service,
                                     const std::shared_ptr<ComponentRepository> &components,
                                     QWidget *parent)
    : Wizard(parent)
    , service(service)
    , componentRepository(components)
    , requestPage(std::make_unique<RequestWizardPage>(service, components))
    , resolvePage(std::make_unique<ResolveWizardPage>(service, components))
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
    qApp->setOverrideCursor(Qt::WaitCursor);
    QtConcurrent::run([this] {
        return service->run(service->current(), [this](const std::string &signal) {
            progressPage->onNewLine(QString::fromStdString(signal));
        });
    }).then(this, [this](const tl::expected<void, TransactionService::Error> &result) {
        qApp->restoreOverrideCursor();
        if (currentPage() != progressPage.get())
        {
            return;
        }
        result.has_value() ? onSuccess() : onFailure(QString::fromStdString(result.error().details));
    });
}

void TransactionWizard::onSuccess()
{
    Wizard::onSuccess();
    currentPage()->setSubTitle(tr("The transaction has completed."));
}

void TransactionWizard::onFailure(const QString &error)
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
