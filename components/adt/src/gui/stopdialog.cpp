#include "stopdialog.h"
#include "ui_stopdialog.h"

StopDialog::StopDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StopDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint, 1);
}

StopDialog::~StopDialog()
{
    delete ui;
}

QSize StopDialog::getSize()
{
    return this->size();
}
