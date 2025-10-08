#include "licensedialog.h"

#include "ui_licensedialog.h"

namespace alt
{
LicenseDialog::LicenseDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(std::make_unique<Ui::LicenseDialog>())
    , m_quitShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q), this)
{
    m_ui->setupUi(this);
    connect(&m_quitShortcut, &QShortcut::activated, this, &LicenseDialog::accept);
}

LicenseDialog::~LicenseDialog() = default;

void LicenseDialog::setLicenseText(const QString &text)
{
    m_ui->textBrowser->setText(text);
}
} // namespace alt
