#include "errordialog.h"
#include "ui_errordialog.h"

namespace alt
{
ErrorDialog::ErrorDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ErrorDialog)
{
    ui->setupUi(this);
}

ErrorDialog::~ErrorDialog()
{
    delete ui;
}

void ErrorDialog::showError(int code, const QString &log)
{
    ui->titleLabel->setText(tr("Operation is exited with code %1:").arg(code));
    ui->plainTextEdit->setPlainText(log);
    open();
}
} // namespace alt
