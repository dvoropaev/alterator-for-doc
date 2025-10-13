#include "updatingsourceswizard.h"

#include "controller/controller.h"
#include "progresswizardpage.h"
#include "ui/applydialog/wizard.h"
#include "ui_wizard.h"

#include <QApplication>
#include <QPushButton>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <qevent.h>

namespace alt
{
UpdatingSourcesWizard::UpdatingSourcesWizard(QWidget *parent)
    : Wizard(parent)
    , progressPage(std::make_unique<ProgressWizardPage>())
{
    ui->stackedWidget->addWidget(progressPage.get());

    connect(button(WizardButton::ApplyButton), &QAbstractButton::clicked, this, &UpdatingSourcesWizard::updateRequested);
    connect(button(WizardButton::RetryButton), &QAbstractButton::clicked, this, [this]() {
        currentPage()->initialize();
        progressPage->setTitle(tr("Update in Progress"));

        updateRequested();
    });
    connect(button(WizardButton::NextButton), &QAbstractButton::clicked, this, &UpdatingSourcesWizard::setNextPage);
    connect(button(WizardButton::BackButton), &QAbstractButton::clicked, this, &UpdatingSourcesWizard::setPreviousPage);
    connect(button(WizardButton::CancelButton), &QAbstractButton::clicked, this, []() {
        qApp->restoreOverrideCursor();
    });

    for (int i = 0; i < ui->stackedWidget->count(); ++i)
    {
        auto *page = qobject_cast<WizardPage *>(ui->stackedWidget->widget(i));
        page->setWizard(this);
    }
}

UpdatingSourcesWizard::~UpdatingSourcesWizard() = default;

void UpdatingSourcesWizard::updateRequested()
{
    qApp->setOverrideCursor(Qt::WaitCursor);

    disconnect(&Controller::instance(),
               &Controller::aptUpdateNewLine,
               progressPage.get(),
               &ProgressWizardPage::onNewLine);
    connect(&Controller::instance(), &Controller::aptUpdateNewLine, progressPage.get(), &ProgressWizardPage::onNewLine);

    auto future = QtConcurrent::run(&Controller::update, &Controller::instance());
    future.then(this, [this](const tl::expected<void, DBusError> &result) {
        qApp->restoreOverrideCursor();
        if (currentPage() != progressPage.get())
        {
            return;
        }
        result.has_value() ? onSuccess() : onFailure(result.error());
    });
}

void UpdatingSourcesWizard::showEvent(QShowEvent *event)
{
    std::ignore = event;
    currentPage()->initialize();
    progressPage->setTitle(tr("Update in Progress"));
    updateRequested();
    Wizard::showEvent(event);
}

void UpdatingSourcesWizard::onSuccess()
{
    Wizard::onSuccess();
    currentPage()->setSubTitle(tr("The software sources update has completed."));
}

void UpdatingSourcesWizard::onFailure(const DBusError &error)
{
    Wizard::onFailure(error);
    currentPage()->setSubTitle(tr("The software sources update has failed."));
}

void UpdatingSourcesWizard::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        setWindowTitle(tr("Update Wizard"));
    }
    Wizard::changeEvent(event);
}
} // namespace alt
