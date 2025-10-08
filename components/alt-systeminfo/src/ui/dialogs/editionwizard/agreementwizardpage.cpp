#include "agreementwizardpage.h"

#include "ui_agreementwizardpage.h"

#include <QDebug>

namespace alt
{
AgreementWizardPage::AgreementWizardPage(QWidget *parent)
    : QWizardPage(parent)
    , m_ui(std::make_unique<Ui::AgreementWizardPage>())
    , m_controller(nullptr)
{
    m_ui->setupUi(this);
    connect(m_ui->agreementCheckBox, &QCheckBox::stateChanged, [this](int state) { emit completeChanged(); });
}

AgreementWizardPage::~AgreementWizardPage() = default;

bool AgreementWizardPage::isComplete() const
{
    return QWizardPage::isComplete() && m_ui->agreementCheckBox->isChecked();
}

void AgreementWizardPage::initializePage()
{
    QWizardPage::initializePage();
    const auto &selectedId = m_controller->getStateSelectedEditionId();
    m_ui->licenseTextBrowser->setText(m_controller->getLicense(selectedId));
}

void AgreementWizardPage::setController(EditionController *controller)
{
    m_controller = controller;
}

} // namespace alt
