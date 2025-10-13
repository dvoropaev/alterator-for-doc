#include "editionwizard.h"

#include "ui_editionwizard.h"

namespace alt
{
EditionWizard::EditionWizard(QWidget *parent)
    : QWizard(parent)
    , m_ui(std::make_unique<Ui::EditionWizard>())
    , m_controller(std::make_unique<EditionController>())
    , m_quitShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q), this)
    , m_nextShortcut(QKeySequence(Qt::CTRL | Qt::Key_Right), this)
    , m_backShortcut(QKeySequence(Qt::CTRL | Qt::Key_Left), this)
{
    m_ui->setupUi(this);
    m_ui->editionSelectionWizardPage->setController(m_controller.get());
    m_ui->licenseAgreementWizardPage->setController(m_controller.get());

    connect(&m_quitShortcut, &QShortcut::activated, this, &EditionWizard::reject);
    connect(&m_nextShortcut, &QShortcut::activated, this, &EditionWizard::next);
    connect(&m_backShortcut, &QShortcut::activated, this, &EditionWizard::back);
}

EditionWizard::~EditionWizard() = default;

void EditionWizard::done(int result)
{
    if (result == QDialog::Accepted)
    {
        m_controller->setEdition(m_controller->getStateSelectedEditionId());
    }

    QWizard::done(result);
}
} // namespace alt
