#include "basemanagerwidget.h"

#include "ui_basemanagerwidget.h"

#include <QDebug>
#include <QRegularExpression>

BaseManagerWidget::BaseManagerWidget(QWidget *parent)
    : QWidget(parent)
    , m_ui(std::make_unique<Ui::BaseManagerWidget>())
    , m_findShortcut(std::make_unique<QShortcut>(QKeySequence(Qt::CTRL | Qt::Key_F), this))
    , m_selectedSourceIndexes()
    , m_onFilterTextChanged(false)
{
    m_ui->setupUi(this);
    m_ui->progressBar->setVisible(false);
    m_ui->loadIndicator->setVisible(false);
    m_ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(m_ui->searchLineEdit, &QLineEdit::returnPressed, this, &BaseManagerWidget::onFilterTextChanged);
    connect(m_findShortcut.get(), &QShortcut::activated, this, [this]() { m_ui->searchLineEdit->setFocus(); });
}

BaseManagerWidget::~BaseManagerWidget() = default;

void BaseManagerWidget::setModel(QSortFilterProxyModel *model)
{
    return m_ui->tableView->setModel(model);
}

QModelIndexList BaseManagerWidget::getSelection()
{
    return m_ui->tableView->selectionModel()->selectedIndexes();
}

QHeaderView *BaseManagerWidget::getHorizontalHeader()
{
    return m_ui->tableView->horizontalHeader();
}

QItemSelectionModel *BaseManagerWidget::getSelectionModel()
{
    return m_ui->tableView->selectionModel();
}

void BaseManagerWidget::onFilterTextChanged()
{
    onFilterChanged(&PackagesFilterProxyModel::setFilterFixedString, m_ui->searchLineEdit->text());
}

void BaseManagerWidget::onFilterChanged(void (PackagesFilterProxyModel::*filterMethod)(const QString &text),
                                        const QString &text)
{
    m_onFilterTextChanged = true;
    saveSelectedIndexes();
    const auto &model = dynamic_cast<PackagesFilterProxyModel *>(m_ui->tableView->model());
    (model->*filterMethod)(text);
    restoreSelectedIndexes();
    m_onFilterTextChanged = false;
    getHorizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void BaseManagerWidget::saveSelectedIndexes()
{
    const auto &selectedIndexes = getSelectionModel()->selectedIndexes();
    for (auto &proxyIndex : selectedIndexes)
    {
        auto sourceIndex = static_cast<QSortFilterProxyModel *>(m_ui->tableView->model())->mapToSource(proxyIndex);
        m_selectedSourceIndexes.insert(sourceIndex);
    }
}

void BaseManagerWidget::restoreSelectedIndexes()
{
    for (const QModelIndex &sourceIndex : m_selectedSourceIndexes)
    {
        QModelIndex proxyIndex = static_cast<QSortFilterProxyModel *>(m_ui->tableView->model())
                                     ->mapFromSource(sourceIndex);
        if (proxyIndex.isValid())
        {
            getSelectionModel()->select(proxyIndex, QItemSelectionModel::Select);
        }
    }
}

void BaseManagerWidget::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    std::ignore = selected;
    if (!m_onFilterTextChanged)
    {
        for (auto &proxyIndex : deselected.indexes())
        {
            auto sourceIndex = static_cast<QSortFilterProxyModel *>(m_ui->tableView->model())->mapToSource(proxyIndex);
            m_selectedSourceIndexes.remove(sourceIndex);
        }
    }

    auto selectionModel = getSelectionModel();
    setButtonsEnabled(!(selectionModel->selectedIndexes().isEmpty() && m_selectedSourceIndexes.empty()));
}

void BaseManagerWidget::setSelectionMode(QAbstractItemView::SelectionMode mode)
{
    m_ui->tableView->setSelectionMode(mode);
}

void BaseManagerWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        m_ui->retranslateUi(this);
    }

    QWidget::changeEvent(event);
}

void BaseManagerWidget::onProgressChanged(int state, const QString &message)
{
    m_ui->progressBar->setVisible(state);
    m_ui->progressBar->setValue(state);
    m_ui->statusLabel->setText(message);

    for (int i = 0; i < m_ui->buttonsHorizontalLayout->count(); ++i)
    {
        auto widget = m_ui->buttonsHorizontalLayout->itemAt(i)->widget();
        if (widget)
        {
            widget->setDisabled(state || !message.isEmpty());
        }
    }

    setButtonsEnabled(!(getSelectionModel()->selectedIndexes().isEmpty() && m_selectedSourceIndexes.empty()));
}

void BaseManagerWidget::onWait(bool start, const QString &message)
{
    m_ui->progressBar->setVisible(start);
    m_ui->progressBar->setMaximum(start ? 0 : 100);
    m_ui->statusLabel->setText(message);
    for (int i = 0; i < m_ui->buttonsHorizontalLayout->count(); ++i)
    {
        if (auto widget = m_ui->buttonsHorizontalLayout->itemAt(i)->widget())
        {
            widget->setDisabled(start);
        }
    }

    setButtonsEnabled(!(getSelectionModel()->selectedIndexes().isEmpty() && m_selectedSourceIndexes.empty()));
}

void BaseManagerWidget::onShowLoadingIndicator(bool visible)
{
    m_ui->loadIndicator->setVisible(visible);
    m_ui->progressBar->setVisible(!visible && m_ui->progressBar->value() > 0);
}
