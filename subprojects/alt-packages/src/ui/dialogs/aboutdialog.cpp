#include "aboutdialog.h"

#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(std::make_unique<Ui::AboutDialog>())
{
    m_ui->setupUi(this);
}

AboutDialog::~AboutDialog() = default;

void AboutDialog::setVersion(const QString &version)
{
    QString text = m_ui->versionLabel->text();

    m_ui->versionLabel->setText(text.append(version));
}
