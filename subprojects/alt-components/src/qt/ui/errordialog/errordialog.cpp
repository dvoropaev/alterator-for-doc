#include "errordialog.h"
#include "ui_errordialog.h"

namespace alt
{
ErrorDialog::ErrorDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ErrorDialog)
{
    ui->setupUi(this);
    ui->titleLabel->clear();
}

ErrorDialog::~ErrorDialog()
{
    delete ui;
}

void ErrorDialog::showError(int code, const QString &log)
{
    ui->plainTextEdit->setPlainText(log);
    open();
}
} // namespace alt
