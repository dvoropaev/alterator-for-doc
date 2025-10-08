#include "selectionwizardpage.h"

#include "ui_selectionwizardpage.h"

#include <QDebug>

namespace alt
{
SelectionWizardPage::SelectionWizardPage(QWidget *parent)
    : QWizardPage(parent)
    , m_ui(std::make_unique<Ui::SelectionWizardPage>())
    , m_model(std::make_unique<QStandardItemModel>())
    , m_controller(nullptr)
{
    m_ui->setupUi(this);
    m_ui->editionsListView->setModel(m_model.get());

    connect(m_ui->editionsListView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &SelectionWizardPage::onSelectionChanged);
}

SelectionWizardPage::~SelectionWizardPage() = default;

bool SelectionWizardPage::isComplete() const
{
    return QWizardPage::isComplete() && !m_ui->editionsListView->selectionModel()->selectedIndexes().isEmpty();
}

void SelectionWizardPage::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if (selected.indexes().isEmpty())
    {
        m_ui->descriptionTextBrowser->setText({});
        return;
    }

    const QString &id = selected.indexes().constBegin()->data(Qt::UserRole).toString();
    m_ui->descriptionTextBrowser->setText(m_controller->getDescription(id));

    m_controller->setStateSelectedEditionId(id);

    emit completeChanged();
}

void SelectionWizardPage::setController(EditionController *controller)
{
    m_controller = controller;
    m_controller->buildModel(m_model.get());
    auto selectedEditionItems = m_model->findItems("•", Qt::MatchFlag::MatchContains);
    if (selectedEditionItems.empty())
    {
        return;
    }
    auto selectedEditionIndex = m_model->indexFromItem(selectedEditionItems[0]);
    m_ui->editionsListView->selectionModel()->select(selectedEditionIndex, QItemSelectionModel::SelectionFlag::Select);
}
} // namespace alt
