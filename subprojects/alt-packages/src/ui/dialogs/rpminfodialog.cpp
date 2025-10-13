#include "rpminfodialog.h"
#include "ui_rpminfodialog.h"

#include <QRegularExpression>
#include <QStandardItemModel>

RpmInfoDialog::RpmInfoDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(std::make_unique<Ui::RpmInfoDialog>())
{
    m_ui->setupUi(this);
    m_ui->infoView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_ui->infoView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

RpmInfoDialog::~RpmInfoDialog() = default;

void RpmInfoDialog::setModel(QStandardItemModel *model)
{
    m_ui->infoView->setModel(model);
}
