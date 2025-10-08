#include "detailsdialog.h"
#include "ui_detailsdialog.h"

#include <QFont>
#include <QTextCursor>

DetailsDialog::DetailsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DetailsDialog)
{
    ui->setupUi(this);
    ui->closePushButton->setFocus();
}

DetailsDialog::~DetailsDialog()
{
    delete ui;
}

void DetailsDialog::setDetailsText(const QString &text)
{
    ui->detailsPlainTextEdit->appendPlainText(text);
}

void DetailsDialog::clearDetailsText()
{
    ui->detailsPlainTextEdit->clear();
}

void DetailsDialog::setTestId(const QString &toolId, const QString &testId)
{
    m_toolId = toolId;
    m_testId = testId;
}

QString DetailsDialog::getToolId() const
{
    return m_toolId;
}

QString DetailsDialog::getTestId() const
{
    return m_testId;
}

void DetailsDialog::closeEvent(QCloseEvent *event)
{
    m_toolId.clear();
    m_testId.clear();
}

void DetailsDialog::on_closePushButton_clicked()
{
    close();
}
