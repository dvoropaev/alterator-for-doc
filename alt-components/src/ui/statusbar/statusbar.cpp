#include "statusbar.h"

#include "ui_statusbar.h"

namespace alt
{
StatusBar::StatusBar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StatusBar)
{
    ui->setupUi(this);
    ui->statusLabel->setVisible(true);
}

StatusBar::~StatusBar()
{
    delete ui;
}

void StatusBar::onStarted(const QString &text)
{
    ui->statusLabel->setText(text);
    ui->progressBar->setVisible(true);
}

void StatusBar::onProgress(const QString &text)
{
    ui->statusLabel->setText(text);
    ui->progressBar->setVisible(true);
}

void StatusBar::onDone(const QString &text)
{
    ui->statusLabel->setText(text);
    ui->progressBar->setVisible(false);
}

void StatusBar::setProgressBarMaximum(int maximum)
{
    ui->progressBar->setMaximum(maximum);
}

void StatusBar::incrementProgressBarValue()
{
    ui->progressBar->setValue(ui->progressBar->value() + 1);
}

void StatusBar::resetProgressBarValue()
{
    ui->progressBar->setValue(0);
}
} // namespace alt
