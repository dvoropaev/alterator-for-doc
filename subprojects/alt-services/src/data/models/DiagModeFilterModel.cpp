#include "DiagModeFilterModel.h"
#include "DiagModel.h"

void DiagModeFilterModel::setMode(DiagTool::Test::Mode mode)
{
    beginFilterChange();
    m_mode = mode;
    invalidateFilter();
}

QVariant DiagModeFilterModel::data(const QModelIndex& index, int role) const
{
    if ( index.column() == 1 && role == Qt::DisplayRole )
        return mapToSource(index).data().value<DiagTool::Test::Modes>().testFlag(m_mode);

    return QSortFilterProxyModel::data(index, role);
}

bool DiagModeFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    auto index = sourceModel()->index(source_row, DiagModel::Modes, source_parent);

    return index.data().value<DiagTool::Test::Modes>().testFlag(m_mode);
}

bool DiagModeFilterModel::filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const
{
    return source_column == DiagModel::Name ||
           source_column == DiagModel::Required;
}
