#include "waitdialog.h"
#include "ui_waitdialog.h"
#include <qnamespace.h>

WaitDialog::WaitDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(std::make_unique<Ui::WaitDialog>())
{
    m_ui->setupUi(this);
}

WaitDialog::~WaitDialog() = default;

void WaitDialog::setProgressBarVisible(bool visible)
{
    m_ui->progressBar->setVisible(visible);
}

void WaitDialog::appendText(QString text)
{
    auto oldText = m_ui->textBrowser->toPlainText();
    m_ui->textBrowser->setText(QString("%1%2").arg(oldText.isEmpty() ? "" : oldText.append('\n')).arg(text));
}

void WaitDialog::clearText()
{
    m_ui->textBrowser->clear();
}

bool WaitDialog::isEmpty()
{
    return m_ui->textBrowser->toPlainText().isEmpty();
}
