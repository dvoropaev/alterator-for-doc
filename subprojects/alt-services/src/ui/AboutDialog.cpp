#include "AboutDialog.h"

#include "version.h"
#include "ui_AboutDialog.h"


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
