#include "DiagModel.h"

#include <QApplication>
#include <QPalette>

void DiagModel::setService(Service* s){
    beginResetModel();
    m_data = &s->diagTools();
    endResetModel();
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

int DiagModel::columnCount(const QModelIndex& parent) const { return 3; }

QVariant DiagModel::data(const QModelIndex& index, int role) const {
    DiagTool*       tool = nullptr;
    DiagTool::Test* test = nullptr;

    if ( index.internalId() == LONG_LONG_MAX )
        tool = m_data->at(index.row()).get();
    else {
        tool = m_data->at(index.internalId()).get();
        test = tool->tests().at(index.row()).get();
    }

    if ( role == Qt::DisplayRole )
    {
        switch ( index.column() )
        {
            case Column::Name:
                return index.internalId() == LONG_LONG_MAX
                    ? tool->displayName()
                    : test->displayName();
            case Column::Modes:
                return index.internalId() == LONG_LONG_MAX
                    ? std::accumulate(tool->tests().cbegin(), tool->tests().cend(),
                                      DiagTool::Test::Modes{},
                                      [](DiagTool::Test::Modes m, const auto& test){ return m | test->mode(); }
                                    )
                    : test->mode().toInt();
            case Column::Required:
                return index.internalId() == LONG_LONG_MAX
                    ? std::accumulate(tool->tests().cbegin(), tool->tests().cend(),
                                      DiagTool::Test::Modes{DiagTool::Test::Mode::All},
                                      [](DiagTool::Test::Modes m, const auto& test){ return m & test->required(); }
                                    )
                    : test->required().toInt();
            default: return {};
        }
    }

    switch ( role ) {
        case Qt::DecorationRole:
            return index.internalId() == LONG_LONG_MAX
                       ? tool->icon()
                       : test->icon();

        case Qt::ToolTipRole:
            return index.internalId() == LONG_LONG_MAX
                       ? tool->comment()
                       : test->comment();

        case Qt::UserRole:
            return QVariant::fromValue(index.internalId() == LONG_LONG_MAX ? (void*)tool : (void*)test);
    }

    return {};
}

QVariant DiagModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole ) switch ( section )
    {
        case Column::Name:      return tr("Tool/Test");
        case Column::Modes:     return tr("Modes");
        case Column::Required:  return tr("Required");
    }

    return {};
}
