#include "DiagSelectionForm.h"
#include "ui_DiagSelectionForm.h"

#include "controller/Controller.h"
#include "../data/models/DiagModel.h"


#include <QSortFilterProxyModel>
class DiagFilterModel : public QSortFilterProxyModel {
public:
    DiagFilterModel(DiagModel* m, bool editable)
        : m_editable{editable}
    {
        setSourceModel(m);
    }

    void setMode(DiagTool::Test::Mode mode) {
        ((DiagModel*)sourceModel())->setMode(mode);

#if QT_VERSION >= QT_VERSION_CHECK(6,9,0)
        beginFilterChange();
#endif
        m_filter = mode;
        invalidateRowsFilter();
    }

    // QAbstractItemModel interface
public:
    Qt::ItemFlags flags(const QModelIndex& index) const override {
        auto flags = QSortFilterProxyModel::flags(index);
        if ( !m_editable )
            flags.setFlag(Qt::ItemIsUserCheckable, false);
        return flags;
    }

    // QSortFilterProxyModel interface
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override {
        if ( !source_parent.isValid() ) return true;
        auto index = sourceModel()->index(source_row, 0, source_parent);
        auto modes = DiagTool::Test::Modes::fromInt(index.data(Qt::UserRole).toInt());
        return modes.testFlag(m_filter);
    }

    DiagTool::Test::Mode m_filter;
    const bool m_editable;
};

class DiagSelectionForm::Private {
public:
    Private()
        : m_diag_proxy{&m_diag_model, false}
        , m_diag_proxy_edit{&m_diag_model, true}
    {}

    Ui::DiagSelectionForm ui;
    Service*    m_service{nullptr};
    Controller* m_controller{nullptr};

    DiagModel       m_diag_model;
    DiagFilterModel m_diag_proxy_edit;
    DiagFilterModel m_diag_proxy;
};

DiagSelectionForm::DiagSelectionForm(QWidget *parent)
    : QWidget(parent)
    , d {new Private}
{
    d->ui.setupUi(this);

    d->ui.diagTreeView->setModel(&d->m_diag_proxy_edit);

    connect(d->ui.diagTreeView, &QAbstractItemView::clicked, this, [this](const QModelIndex& index){
        auto checked = index.data(Qt::CheckStateRole);
        if ( checked.isValid() )
            //d->m_model.setData(index, !checked.toBool(), Qt::CheckStateRole);

            /*
             * QAbstractItemView::clicked is emitted even on checkboxes
             * so we do not use setData here to avoid calling it twice
             */
            d->m_diag_model.toggle( d->m_diag_proxy_edit.mapToSource(index) );
    });

    d->ui. selectAllButton->setIcon(QIcon::fromTheme("edit-select-all"));
    d->ui.     clearButton->setIcon(QIcon::fromTheme("edit-clear"));
}

DiagSelectionForm::~DiagSelectionForm() { delete d; }



void DiagSelectionForm::setController(Controller* c)
{
    d->m_controller = c;
}

void DiagSelectionForm::setService(Service* service)
{
    d->m_service = service;

    d->m_diag_model.setService(service);
    d->ui.diagTreeView->expandAll();

    connect(&d->m_diag_model, &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex& tl, const QModelIndex& be, const QList<int> roles) {
                if ( roles.contains(Qt::CheckStateRole) ) {
                    onDataChanged();
                }
    });

    onDataChanged();
}

void DiagSelectionForm::setMode(DiagTool::Test::Mode mode)
{
    d->m_diag_proxy.setMode(mode);
    d->m_diag_proxy_edit.setMode(mode);
}

QAbstractItemModel* DiagSelectionForm::model()
{
    return &d->m_diag_proxy;
}

bool DiagSelectionForm::anySelected()
{
    bool any = false;

    for ( int i = 0; i < d->m_diag_proxy_edit.rowCount(); ++i ) {
        auto checkData = d->m_diag_proxy_edit.index(i, 0).data(Qt::CheckStateRole);
        int state = checkData.isValid() ? checkData.toInt() : Qt::Checked;
        any |= ( state != Qt::Unchecked );
    }

    return any;
}

void DiagSelectionForm::on_selectAllButton_clicked()
{
    for ( int i = 0; i < d->m_diag_proxy_edit.rowCount(); ++i )
        d->m_diag_model.toggle( d->m_diag_proxy_edit.mapToSource(d->m_diag_proxy_edit.index(i,0)) );
}

void DiagSelectionForm::on_clearButton_clicked()
{
    for ( int i = 0; i < d->m_diag_proxy_edit.rowCount(); ++i )
        d->m_diag_model.toggle( d->m_diag_proxy_edit.mapToSource(d->m_diag_proxy_edit.index(i,0)) );
}

void DiagSelectionForm::onDataChanged(){
    bool anySelected = false;
    bool allSelected = d->m_diag_proxy_edit.rowCount();
    bool mayClear = false;

    for ( int i = 0; i < d->m_diag_proxy_edit.rowCount(); ++i ) {
        auto parent_index = d->m_diag_proxy_edit.index(i, 0);
        for ( int j = 0; j < d->m_diag_proxy_edit.rowCount(parent_index); ++j ) {
            auto index = d->m_diag_proxy_edit.index(j, 0, parent_index);
            auto checkData = index.data(Qt::CheckStateRole);
            int state = checkData.isValid() ? checkData.toInt() : Qt::Checked;
            mayClear |= ( state != Qt::Unchecked && d->m_diag_proxy_edit.flags(index).testFlag(Qt::ItemIsUserCheckable) );
            anySelected |= ( state != Qt::Unchecked );
            allSelected &= ( state == Qt::Checked   );
        }
    }

    d->ui.clearButton->setEnabled(mayClear && anySelected);
    d->ui.selectAllButton->setEnabled( d->m_diag_proxy_edit.rowCount() > 0 && !allSelected);
}
