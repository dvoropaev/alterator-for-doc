#include "wizard.h"
#include "ui_wizard.h"

#include <QApplication>
#include <QPushButton>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

namespace alt
{
Wizard::Wizard(QWidget *parent)
    : QDialog(parent)
    , ui(std::make_unique<Ui::Wizard>())
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
}

Wizard::~Wizard() = default;

QPushButton *Wizard::button(WizardButton button)
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

ApplyStatusBar *Wizard::statusBar()
{
    return ui->statusBar;
}

void Wizard::setNextPage()
{
    advancePage(+1);
}

void Wizard::setPreviousPage()
{
    qApp->restoreOverrideCursor();
    advancePage(-1);
}

void Wizard::setPage(int index)
{
    advancePage(index - ui->stackedWidget->currentIndex());
}

WizardPage *Wizard::currentPage()
{
    return qobject_cast<WizardPage *>(ui->stackedWidget->currentWidget());
}

void Wizard::advancePage(int offset)
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

void Wizard::onSuccess()
{
    statusBar()->onDone({});
    button(Wizard::FinishButton)->setEnabled(true);
}

void Wizard::onFailure(const QString &error)
{
    std::ignore = error;
    statusBar()->onDone({});
    button(Wizard::FinishButton)->hide();
    button(Wizard::CancelButton)->show();
    button(Wizard::RetryButton)->show();
}

void Wizard::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        this->ui->retranslateUi(this);
    }
}
} // namespace alt
