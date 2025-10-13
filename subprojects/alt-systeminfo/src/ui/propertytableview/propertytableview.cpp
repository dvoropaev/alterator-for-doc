#include "propertytableview.h"
#include "ui_propertytableview.h"

namespace alt
{
enum PropertyColumn
{
    PropertyName,
    PropertyValue
};

PropertyTableView::PropertyTableView(QWidget *parent)
    : QWidget(parent)
    , m_ui(std::make_unique<Ui::PropertyTableView>())
{
    m_ui->setupUi(this);

    m_ui->tableView->horizontalHeader()->hide();
    m_ui->tableView->verticalHeader()->hide();

    m_ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    m_ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
}

void PropertyTableView::setModel(QAbstractItemModel *model)
{
    connect(model, &QAbstractItemModel::dataChanged, this, &PropertyTableView::onDataChanged);
    m_ui->tableView->setModel(model);
    onDataChanged({}, {}, {});
}

void PropertyTableView::onDataChanged(const QModelIndex &topLeft,
                                      const QModelIndex &bottomRight,
                                      const QVector<int> &roles)
{
    m_ui->tableView->resizeColumnsToContents();
    const size_t margin = 50;
    auto firstColumnWidth = m_ui->tableView->horizontalHeader()->sectionSize(PropertyColumn::PropertyName);
    m_ui->tableView->horizontalHeader()->resizeSection(PropertyColumn::PropertyName, firstColumnWidth + margin);
    auto recommendedSize = m_ui->tableView->sizeHint();
    m_ui->tableView->setMinimumSize(recommendedSize);
}

QTableView *PropertyTableView::tableView()
{
    return m_ui->tableView;
}

PropertyTableView::~PropertyTableView() = default;
} // namespace alt
