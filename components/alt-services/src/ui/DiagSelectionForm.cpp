#include "DiagSelectionForm.h"
#include "ui_DiagSelectionForm.h"

#include "controller/Controller.h"

class DiagModel : public QAbstractItemModel {
    Q_OBJECT
public:
    DiagModel() = default;

    void setService(Service* s){
        beginResetModel();
        m_data = &s->diagTools();
        endResetModel();
    }

    void setMode(DiagTool::Test::Mode mode) {
        m_mode = mode;
        for ( int i = 0; i < m_data->size(); ++i ) {
            auto& tool = (*m_data)[i];
            auto toolIdx = index(i, 0);
            for ( int j = 0; j < tool->tests().size(); ++j ) {
                auto& test = tool->tests()[j];
                test->setSelected(m_mode, false);
            }
            emit dataChanged(index(0,0, toolIdx), index(tool->tests().size()-1, 0, toolIdx), {Qt::CheckStateRole});
        }

        emit dataChanged(index(0,0), index(rowCount()-1, 0), {Qt::CheckStateRole});
    }

    QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override {
        if ( parent.isValid() ) {
            if ( parent.parent().isValid() )
                return {};
            return createIndex(row, column, parent.row());
        }
        return createIndex(row, column, LONG_LONG_MAX);
    }

    QModelIndex parent(const QModelIndex& child) const override {
        if ( child.internalId() == LONG_LONG_MAX ) return {};
        return createIndex(child.internalId(), 0, LONG_LONG_MAX);
    }

    int rowCount(const QModelIndex& parent = {}) const override {
        if ( parent.isValid() ) {
            if ( parent.parent().isValid() ) return 0;
            return m_data->at(parent.row())->tests().size();
        }
        return m_data ? m_data->size() : 0;
    }

    int columnCount(const QModelIndex& parent = {}) const override {
        if ( parent.isValid() )
            return 3;
        return 1;
    }

    QVariant data(const QModelIndex& index, int role) const override {
        DiagTool* tool = nullptr;
        DiagTool::Test* test = nullptr;

        if ( index.internalId() == LONG_LONG_MAX )
            tool = m_data->at(index.row()).get();
        else {
            tool = m_data->at(index.internalId()).get();
            test = tool->tests().at(index.row()).get();
        }

        switch ( role ) {
            case Qt::DisplayRole:
                return index.internalId() == LONG_LONG_MAX
                    ? tool->displayName()
                    : test->displayName();

            case Qt::DecorationRole:
                return index.internalId() == LONG_LONG_MAX
                    ? tool->icon()
                    : test->icon();

            case Qt::ToolTipRole:
                return index.internalId() == LONG_LONG_MAX
                    ? tool->comment()
                    : test->comment();

            case Qt::CheckStateRole:
            {
                if ( index.internalId() == LONG_LONG_MAX ) {

                    if ( std::none_of( tool->tests().cbegin(), tool->tests().cend(), [this](const auto& test){
                            return test->modes().testFlag(m_mode) && test->isEnabled(m_mode);
                        }))
                        return Qt::Unchecked;

                    return std::any_of( tool->tests().cbegin(), tool->tests().cend(), [this](const auto& test){
                               return test->modes().testFlag(m_mode) && !test->isEnabled(m_mode);
                           })
                               ? Qt::PartiallyChecked
                               : Qt::Checked;
                } else {
                    return test->isEnabled(m_mode) ? Qt::Checked : Qt::Unchecked;
                }

                break;
            }

            case Qt::ForegroundRole: {
                return QApplication::palette().brush(
                    index.data(Qt::CheckStateRole).toInt() == Qt::Unchecked
                         ? QPalette::Disabled
                         : QPalette::Current
                    , QPalette::Text );
            }

            case Qt::UserRole:
                if ( index.internalId() < LONG_LONG_MAX )
                    return test->modes().toInt();
                break;

        }

        return {};
    }

    // bool setData(const QModelIndex& idx, const QVariant& value, int role) override {
    //     if ( role == Qt::CheckStateRole ) {
    //         bool check = value.toBool();

    //         if ( idx.internalId() == LONG_LONG_MAX ) {
    //             auto& tool = m_data->at(idx.row());

    //             for ( auto& test : tool->tests() )
    //                 test->setSelected(m_mode, check);

    //             emit dataChanged(idx, idx, {Qt::CheckStateRole});
    //             emit dataChanged(index(0,0, idx), index(tool->tests().size()-1, 0, idx), {Qt::CheckStateRole});

    //             return true;

    //         } else {
    //             auto& tool = m_data->at(idx.internalId());
    //             auto& test = tool->tests().at(idx.row());

    //             test->setSelected(m_mode, check);

    //             emit dataChanged(idx.parent(), idx.parent(), {Qt::CheckStateRole});
    //             emit dataChanged(idx.siblingAtRow(0), idx.siblingAtRow(tool->tests().size()-1), {Qt::CheckStateRole});

    //             return check == test->isEnabled(m_mode);
    //         }
    //     }
    //     return false;
    // }

    void toggle(const QModelIndex& idx) {
        if ( idx.internalId() == LONG_LONG_MAX ) {
            auto& tool = m_data->at(idx.row());

            bool isChecked = idx.data(Qt::CheckStateRole).toInt() == Qt::Checked;

            for ( auto& test : tool->tests() )
                if ( !test->isRequired(m_mode) )
                    test->setSelected(m_mode, !isChecked);

            emit dataChanged(idx, idx, {Qt::CheckStateRole, Qt::ForegroundRole});
            emit dataChanged(index(0,0, idx), index(tool->tests().size()-1, 0, idx), {Qt::CheckStateRole, Qt::ForegroundRole});

        } else {
            auto& tool = m_data->at(idx.internalId());
            auto& test = tool->tests().at(idx.row());

            if ( !test->isRequired(m_mode) )
                test->setSelected(m_mode, !test->isEnabled(m_mode));

            emit dataChanged(idx.parent(), idx.parent(), {Qt::CheckStateRole, Qt::ForegroundRole});
            emit dataChanged(idx.siblingAtRow(0), idx.siblingAtRow(tool->tests().size()-1), {Qt::CheckStateRole, Qt::ForegroundRole});
        }
    }

    Qt::ItemFlags flags(const QModelIndex& index) const override {
        auto flags = QAbstractItemModel::flags(index);

        DiagTool* tool = nullptr;
        DiagTool::Test* test = nullptr;

        if ( index.internalId() == LONG_LONG_MAX ) {
            tool = m_data->at(index.row()).get();
            flags.setFlag(Qt::ItemIsUserCheckable, !std::all_of(
                tool->tests().cbegin(),
                tool->tests().cend(),
                [this](const auto& test){
                    return !test->modes().testFlag(m_mode) ||
                           test->isRequired(m_mode);
                }
            ));
        } else {
            tool = m_data->at(index.internalId()).get();
            test = tool->tests().at(index.row()).get();

            flags.setFlag(Qt::ItemIsUserCheckable, !test->isRequired(m_mode) );
        }

        return flags;
    }

signals:
    void selectionChanged(bool);

private:
    const PtrVector<DiagTool>* m_data{nullptr};
    DiagTool::Test::Mode m_mode{DiagTool::Test::PreDeploy};
};

#include "DiagSelectionForm.moc"

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
        d->m_diag_proxy_edit.setData(d->m_diag_proxy_edit.index(i,0), true, Qt::CheckStateRole);
}

void DiagSelectionForm::on_clearButton_clicked()
{
    for ( int i = 0; i < d->m_diag_proxy_edit.rowCount(); ++i )
        d->m_diag_proxy_edit.setData(d->m_diag_proxy_edit.index(i,0), false, Qt::CheckStateRole);
}

void DiagSelectionForm::onDataChanged(){
    bool any = false;
    bool all = d->m_diag_proxy_edit.rowCount();

    for ( int i = 0; i < d->m_diag_proxy_edit.rowCount(); ++i ) {
        auto checkData = d->m_diag_proxy_edit.index(i, 0).data(Qt::CheckStateRole);
        int state = checkData.isValid() ? checkData.toInt() : Qt::Checked;
        any |= ( state != Qt::Unchecked );
        all &= ( state == Qt::Checked   );
    }

    d->ui.clearButton->setEnabled(any);
    d->ui.selectAllButton->setEnabled( d->m_diag_proxy_edit.rowCount() > 0 && !all);
}
