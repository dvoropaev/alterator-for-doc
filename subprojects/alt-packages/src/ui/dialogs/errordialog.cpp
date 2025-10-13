#include "errordialog.h"
#include "ui_errordialog.h"

ErrorDialog::ErrorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ErrorDialog)
{
    ui->setupUi(this);
}

ErrorDialog::~ErrorDialog()
{
    delete ui;
}

void ErrorDialog::showError(int code, const QStringList &log)
{
    ui->titleLabel->setText(tr("Command exited with code %1:").arg(code));
    ui->plainTextEdit->setPlainText(log.join("\n"));
    open();
}
