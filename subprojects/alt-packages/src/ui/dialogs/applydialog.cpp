#include "applydialog.h"
#include "ui_applydialog.h"

#include <QDialogButtonBox>
#include <QPushButton>

ApplyDialog::ApplyDialog(const QStringList &installPackages,
                         const QStringList &removePackages,
                         const QStringList &extraRemovePackages,
                         bool safe,
                         QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::ApplyDialog)
    , safeMode(safe)
{
    m_ui->setupUi(this);

    initPackageViews(installPackages, removePackages, extraRemovePackages);

    connect(m_ui->cancelButton, &QPushButton::clicked, this, &ApplyDialog::reject);
    connect(m_ui->applyButton, &QPushButton::clicked, this, &QDialog::accept);
}

ApplyDialog::~ApplyDialog()
{
    delete m_ui;
}

void ApplyDialog::initPackageViews(const QStringList &installPackages,
                                   const QStringList &removePackages,
                                   const QStringList &extraRemovePackages)
{
    if (installPackages.empty())
    {
        m_ui->installPackages->hide();
        m_ui->installLabel->hide();
    }
    else
    {
        m_ui->installPackages->show();
        m_ui->installLabel->show();
        m_ui->installPackages->addItems(installPackages);
        m_ui->installLabel->setText(m_ui->installLabel->text().arg(installPackages.size()));
    }

    if (removePackages.empty() && extraRemovePackages.empty())
    {
        m_ui->removePackages->hide();
        m_ui->removeLabel->hide();
    }
    else
    {
        m_ui->removePackages->show();
        m_ui->removeLabel->show();
        m_ui->removePackages->addItems(removePackages);
        m_ui->removeLabel->setText(m_ui->removeLabel->text().arg(removePackages.size()));
        for (const auto &package : extraRemovePackages)
        {
            QListWidgetItem *item = new QListWidgetItem(package, m_ui->removePackages);
            item->setIcon(QIcon::fromTheme("dialog-warning", QIcon(":warning")));
            item->setToolTip(tr("Manually installed package"));
        }
    }
    if (!extraRemovePackages.empty() && safeMode)
    {
        m_ui->applyButton->setEnabled(false);
    }
}
