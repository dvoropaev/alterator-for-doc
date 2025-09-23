#include "aboutdialog.h"

#include "appversion.h"
#include "ui_aboutdialog.h"

namespace alt
{
AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    QString text = ui->versionLabel->text();

    ui->versionLabel->setText(text.append(getApplicationVersion()));
}

AboutDialog::~AboutDialog()
{
    delete ui;
};

} // namespace alt
