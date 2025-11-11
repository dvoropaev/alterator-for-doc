#include "DiagModel.h"

#include <QApplication>
#include <QPalette>

void DiagModel::setService(Service* s){
    beginResetModel();
    m_data = &s->diagTools();
    endResetModel();
}

void DiagModel::setMode(DiagTool::Test::Mode mode) {
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

QModelIndex DiagModel::index(int row, int column, const QModelIndex& parent) const {
    if ( parent.isValid() ) {
        if ( parent.parent().isValid() ) return {};
        return createIndex(row, column, parent.row());
    }
    return createIndex(row, column, LONG_LONG_MAX);
}

QModelIndex DiagModel::parent(const QModelIndex& child) const {
    return child.internalId() == LONG_LONG_MAX
        ? QModelIndex{}
        : createIndex(child.internalId(), 0, LONG_LONG_MAX);
}

int DiagModel::rowCount(const QModelIndex& parent) const {
    if ( parent.isValid() ) {
        if ( parent.parent().isValid() ) return 0;
        return m_data->at(parent.row())->tests().size();
    }
    return m_data ? m_data->size() : 0;
}

int DiagModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 3 : 1;
}

QVariant DiagModel::data(const QModelIndex& index, int role) const {
    DiagTool*       tool = nullptr;
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
            } else
                return test->isEnabled(m_mode)
                           ? Qt::Checked
                           : Qt::Unchecked;

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

void DiagModel::toggle(const QModelIndex& idx) {
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

Qt::ItemFlags DiagModel::flags(const QModelIndex& index) const {
    auto flags = QAbstractItemModel::flags(index);

    DiagTool* tool = nullptr;
    DiagTool::Test* test = nullptr;

    if ( index.internalId() == LONG_LONG_MAX ) {
        tool = m_data->at(index.row()).get();
        flags.setFlag(Qt::ItemIsUserCheckable,
            std::any_of( tool->tests().cbegin(), tool->tests().cend(), [this](const auto& test){
                return test->modes().testFlag(m_mode) && !test->isRequired(m_mode);
            })
        );
    } else {
        tool = m_data->at(index.internalId()).get();
        test = tool->tests().at(index.row()).get();

        flags.setFlag(Qt::ItemIsUserCheckable, !test->isRequired(m_mode) );
    }

    return flags;
}
