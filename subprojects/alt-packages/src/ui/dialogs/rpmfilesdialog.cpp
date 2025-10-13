#include "rpmfilesdialog.h"
#include "ui_rpmfilesdialog.h"

#include <QStringListModel>

RpmFilesDialog::RpmFilesDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(std::make_unique<Ui::RpmFilesDialog>())
{
    m_ui->setupUi(this);
}

RpmFilesDialog::~RpmFilesDialog() = default;

void RpmFilesDialog::setModel(QStringListModel *model)
{
    m_ui->filesListView->setModel(model);
}
