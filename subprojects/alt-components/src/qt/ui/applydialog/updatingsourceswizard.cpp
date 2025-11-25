#include "updatingsourceswizard.h"

#include "../../controller/controller.h"
#include "progresswizardpage.h"
#include "repository/packagerepository.h"
#include "ui_wizard.h"
#include "wizard.h"

#include <QApplication>
#include <QPushButton>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

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

    auto future = QtConcurrent::run(&Controller::update, &Controller::instance(), [this](const std::string &signal) {
        progressPage->onNewLine(QString::fromStdString(signal));
    });
    future.then(this, [this](const tl::expected<void, PackageRepository::Error> &result) {
        qApp->restoreOverrideCursor();
        if (currentPage() != progressPage.get())
        {
            return;
        }
        result.has_value() ? onSuccess() : onFailure(QString::fromStdString(result.error().details));
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

void UpdatingSourcesWizard::onFailure(const QString &error)
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
