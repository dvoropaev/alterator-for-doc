#include "warningsdialog.h"
#include "controller/controller.h"
#include "ui_warningsdialog.h"

#include <QClipboard>
#include <QMenu>

namespace alt
{
WarningsDialog::WarningsDialog(QStandardItemModel *model, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WarningsDialog)
{
    ui->setupUi(this);

    model->setHorizontalHeaderLabels(QStringList() << tr("Time") << tr("Log level") << tr("Message"));
    auto *proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterRole(Qt::UserRole);
    proxyModel->setFilterKeyColumn(2);
    ui->messagesTable->setModel(proxyModel);
    ui->messagesTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    auto *filterMenu = new QMenu(this);
    std::vector<std::pair<QString, QString>> warningLevels{
        {tr("&All"), QString()},
        {tr("&Critical"), QString::number(QtMsgType::QtCriticalMsg)},
        {tr("&Warnings"), QString::number(QtMsgType::QtWarningMsg)},
        {tr("&Info"), QString::number(QtMsgType::QtInfoMsg)},
    };
    for (const auto &level : warningLevels)
    {
        filterMenu->addAction(level.first, this, [this, proxyModel, level]() {
            proxyModel->setFilterRegularExpression(level.second);
            ui->filterButton->setText(level.first);
        });
    }
    ui->filterButton->setMenu(filterMenu);

    connect(ui->removeButton, &QPushButton::clicked, this, [this]() {
        emit messagesRemoved(ui->messagesTable->selectionModel());
    });

    connect(ui->copyButton, &QPushButton::clicked, this, [this] {
        auto *model = ui->messagesTable->selectionModel();
        QString buf("");
        for (auto rowIndex : model->selectedRows())
        {
            for (auto col = 0; col < ui->messagesTable->model()->columnCount(); col++)
            {
                buf += rowIndex.siblingAtColumn(col).data().toString() + " ";
            }
            buf += "\n";
        }
        QGuiApplication::clipboard()->setText(buf);
    });

    connect(ui->copyAllButton, &QPushButton::clicked, this, [this] {
        auto *model = ui->messagesTable->model();
        QString buf("");
        for (auto row = 0; row < model->rowCount(); row++)
        {
            for (auto col = 0; col < ui->messagesTable->model()->columnCount(); col++)
            {
                buf += model->data(model->index(row, col)).toString() + " ";
            }
            buf += "\n";
        }
        QGuiApplication::clipboard()->setText(buf);
    });
}

WarningsDialog::~WarningsDialog()
{
    delete ui;
}
} // namespace alt
