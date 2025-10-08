#include "wizardpage.h"

#include "ui/applydialog/wizard.h"
#include "ui/errordialog/errordialog.h"
#include "ui_wizardpage.h"

#include <QBoxLayout>

namespace alt
{
WizardPage::WizardPage(QWidget *parent)
    : QWidget(parent)
    , ui(std::make_unique<Ui::WizardPage>())
    , wizard(nullptr)
{
    ui->setupUi(this);
}

WizardPage::~WizardPage() = default;

void WizardPage::setWizard(Wizard *wizard)
{
    this->wizard = wizard;
}

void WizardPage::initialize() {}

void WizardPage::setTitle(const QString &text)
{
    ui->titleLabel->setText(text);
}

void WizardPage::setSubTitle(const QString &text)
{
    ui->subtitleLabel->setText(text);
}

QVBoxLayout *WizardPage::layout()
{
    return ui->verticalLayout;
}

void WizardPage::onError(const DBusError &error)
{
    auto dialog = std::make_unique<ErrorDialog>(wizard);
    dialog->showError(error.code, error.text);
    dialog->exec();
}
} // namespace alt
