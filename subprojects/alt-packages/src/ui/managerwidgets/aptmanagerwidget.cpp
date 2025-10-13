#include "aptmanagerwidget.h"
#include "ui_basemanagerwidget.h"

#include <QDebug>
#include <QItemSelectionModel>
#include <QPushButton>

AptManagerWidget::AptManagerWidget(QWidget *parent)
    : BaseManagerWidget(parent)
    , m_applyPushButton(std::make_unique<QPushButton>())
    , m_resetPushButton(std::make_unique<QPushButton>())
    , m_updatePushButton(std::make_unique<QPushButton>())
    , m_distUpgradePushButton(std::make_unique<QPushButton>())
{
    retranslateButtons();
    setButtonsEnabled(false);
    m_ui->buttonsHorizontalLayout->addWidget(m_resetPushButton.get());
    m_ui->buttonsHorizontalLayout->addWidget(m_applyPushButton.get());
    m_ui->buttonsHorizontalLayout->addWidget(m_updatePushButton.get());
    m_ui->buttonsHorizontalLayout->addWidget(m_distUpgradePushButton.get());
    setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);

    QObject::connect(m_applyPushButton.get(), &QPushButton::clicked, this, &AptManagerWidget::onApplyButtonClicked);
    QObject::connect(m_resetPushButton.get(), &QPushButton::clicked, this, &AptManagerWidget::onResetButtonClicked);

    m_ui->filterLabel->hide();
    m_ui->loadIndicator->setVisible(true);
}

AptManagerWidget::~AptManagerWidget() = default;

void AptManagerWidget::connect(AptController *controller)
{
    setModel(controller->getModel());

    QObject::connect(this, &AptManagerWidget::requestApplyChanges, controller, &AptController::onApplyChangesRequested);
    QObject::connect(m_updatePushButton.get(), &QPushButton::clicked, controller, &AptController::onUpdateRequested);
    QObject::connect(m_distUpgradePushButton.get(),
                     &QPushButton::clicked,
                     controller,
                     &AptController::onDistUpgradeRequested);

    QObject::connect(controller->getModel(),
                     &QSortFilterProxyModel::dataChanged,
                     this,
                     &AptManagerWidget::onItemClicked);

    QObject::connect(getSelectionModel(),
                     &QItemSelectionModel::selectionChanged,
                     this,
                     &AptManagerWidget::onSelectionChanged);

    QObject::connect(controller, &AptController::progressChanged, this, &AptManagerWidget::onProgressChanged);
    QObject::connect(controller, &AptController::wait, this, &AptManagerWidget::onWait);
    QObject::connect(controller, &AptController::showLoadingIndicator, this, &AptManagerWidget::onShowLoadingIndicator);
}

void AptManagerWidget::onItemClicked(const QModelIndex &topLeft,
                                     const QModelIndex &bottomRight,
                                     const QVector<int> &roles)
{
    std::ignore         = std::tie(bottomRight, roles);
    auto selectionModel = getSelectionModel();
    auto isSelected     = selectionModel->isSelected(topLeft);
    selectionModel->select(topLeft, isSelected ? QItemSelectionModel::Deselect : QItemSelectionModel::Select);
}

void AptManagerWidget::setButtonsEnabled(bool isEnable)
{
    m_applyPushButton->setEnabled(isEnable);
    m_resetPushButton->setEnabled(isEnable);
}

void AptManagerWidget::onApplyButtonClicked()
{
    QModelIndexList savedSelection(m_selectedSourceIndexes.begin(), m_selectedSourceIndexes.end());
    auto fullSelection = getSelection() + savedSelection;
    emit requestApplyChanges(fullSelection);
}

void AptManagerWidget::onResetButtonClicked()
{
    for (auto &proxyIndex : getSelectionModel()->selection().indexes())
    {
        auto proxyModel  = static_cast<QSortFilterProxyModel *>(m_ui->tableView->model());
        auto sourceIndex = proxyModel->mapToSource(proxyIndex);
        auto sourceModel = qobject_cast<QStandardItemModel *>(proxyModel->sourceModel());
        if (sourceModel)
        {
            auto item = sourceModel->itemFromIndex(sourceIndex);
            item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
        }
    }
    for (auto &sourceIndex : m_selectedSourceIndexes)
    {
        auto proxyModel  = static_cast<QSortFilterProxyModel *>(m_ui->tableView->model());
        auto sourceModel = qobject_cast<QStandardItemModel *>(proxyModel->sourceModel());
        if (sourceModel)
        {
            auto item = sourceModel->itemFromIndex(sourceIndex);
            item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
        }
    }
    m_selectedSourceIndexes.clear();
    getSelectionModel()->clearSelection();
    setButtonsEnabled(false);
}

void AptManagerWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        retranslateButtons();
    }

    BaseManagerWidget::changeEvent(event);
}

void AptManagerWidget::retranslateButtons()
{
    m_applyPushButton->setText(tr("Apply"));
    m_resetPushButton->setText(tr("Reset"));
    m_updatePushButton->setText(tr("Update"));
    m_distUpgradePushButton->setText(tr("Upgrade All"));
}
