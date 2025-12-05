#include "DiagSelectionForm.h"
#include "ui_DiagSelectionForm.h"

#include "app/ServicesApp.h"
#include "controller/Controller.h"

#include "data/models/DiagModel.h"
#include "data/models/DiagModeFilterModel.h"
#include "data/models/DiagSelectionModel.h"

class ReadOnlyProxy : public QSortFilterProxyModel {
    // QAbstractItemModel interface
public:
    bool setData(const QModelIndex& index, const QVariant& value, int role) override {return false;};

    QVariant data(const QModelIndex& index, int role) const override
    {
        if ( role == Qt::CheckStateRole )
            return {};
        return QSortFilterProxyModel::data(index, role);
    }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
    {
        return sourceModel()->index(source_row, 0, source_parent)
            .data(Qt::CheckStateRole).value<Qt::CheckState>() != Qt::Unchecked;
    }
};

class DiagSelectionForm::Private {
public:
    Private()
        : m_diag_filter_proxy{&m_diag_model}
        , m_diag_selection_model{&m_diag_filter_proxy}
    {
        m_read_only_model.setSourceModel(&m_diag_selection_model);
    }

    Ui::DiagSelectionForm ui;
    Service*    m_service{nullptr};

    DiagModel           m_diag_model;
    DiagModeFilterModel m_diag_filter_proxy;
    DiagSelectionModel  m_diag_selection_model;

    ReadOnlyProxy       m_read_only_model;
};

DiagSelectionForm::DiagSelectionForm(QWidget *parent)
    : QWidget(parent)
    , d {new Private}
{
    d->ui.setupUi(this);

    d->ui.diagTreeView->setModel(&d->m_diag_selection_model);

    connect(&d->m_diag_selection_model, &QAbstractItemModel::modelReset, this, &DiagSelectionForm::onDataChanged);
    connect(&d->m_diag_selection_model, &QAbstractItemModel::modelReset, d->ui.diagTreeView, &QTreeView::expandAll);
    connect(&d->m_diag_selection_model, &QAbstractItemModel::dataChanged, this,
    [this](const QModelIndex& tl, const QModelIndex& be, const QList<int>& roles)
    {
        if ( roles.contains(Qt::CheckStateRole) )
            onDataChanged();
    });

    d->ui. selectAllButton->setIcon(QIcon::fromTheme("edit-select-all"));
    d->ui.     clearButton->setIcon(QIcon::fromTheme("edit-clear"));
}

DiagSelectionForm::~DiagSelectionForm() { delete d; }


void DiagSelectionForm::setService(Service* service, Action::TestSet& tests)
{
    d->m_service = service;
    d->m_diag_model.setService(service);
    d->m_diag_selection_model.setSelection(&tests);
    d->ui.diagTreeView->expandAll();
    onDataChanged();
}

void DiagSelectionForm::setMode(DiagTool::Test::Mode mode)
{   
    d->m_diag_filter_proxy.setMode(mode);
    d->ui.diagTreeView->expandAll();
}

QAbstractItemModel& DiagSelectionForm::model()
{
    return d->m_read_only_model;
}

bool DiagSelectionForm::anySelected()
{
    bool any = false;

    for ( int i = 0; i < d->m_diag_selection_model.rowCount(); ++i ) {
        auto checkData = d->m_diag_selection_model.index(i, 0).data(Qt::CheckStateRole);
        int state = checkData.isValid() ? checkData.value<Qt::CheckState>() : Qt::Checked;
        any |= ( state != Qt::Unchecked );
    }

    return any;
}

void DiagSelectionForm::on_selectAllButton_clicked()
{
    for ( int i = 0; i < d->m_diag_selection_model.rowCount(); ++i )
        d->m_diag_selection_model.setData(d->m_diag_selection_model.index(i, 0), true, Qt::CheckStateRole);
}

void DiagSelectionForm::on_clearButton_clicked()
{
    for ( int i = 0; i < d->m_diag_selection_model.rowCount(); ++i )
        d->m_diag_selection_model.setData(d->m_diag_selection_model.index(i, 0), false, Qt::CheckStateRole);
}

void DiagSelectionForm::onDataChanged(){
    bool anySelected = false;
    bool allSelected = d->m_diag_selection_model.rowCount();
    bool mayClear = false;

    for ( int i = 0; i < d->m_diag_selection_model.rowCount(); ++i ) {
        auto parent_index = d->m_diag_selection_model.index(i, 0);
        for ( int j = 0; j < d->m_diag_selection_model.rowCount(parent_index); ++j ) {
            auto index = d->m_diag_selection_model.index(j, 0, parent_index);
            auto checkData = index.data(Qt::CheckStateRole);
            int state = checkData.isValid() ? checkData.toInt() : Qt::Checked;
            mayClear |= ( state != Qt::Unchecked && d->m_diag_selection_model.flags(index).testFlag(Qt::ItemIsUserCheckable) );
            anySelected |= ( state != Qt::Unchecked );
            allSelected &= ( state == Qt::Checked   );
        }
    }

    d->ui.clearButton->setEnabled(mayClear && anySelected);
    d->ui.selectAllButton->setEnabled( d->m_diag_selection_model.rowCount() > 0 && !allSelected);
}
