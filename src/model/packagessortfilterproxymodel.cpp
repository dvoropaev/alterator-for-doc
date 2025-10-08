#include "packagessortfilterproxymodel.h"

#include "entities/package.h"

void PackagesFilterProxyModel::setFilterGroup(const QString &group)
{
    m_group = group;
    invalidateFilter();
}

void PackagesFilterProxyModel::setFilterArch(const QString &arch)
{
    m_arch = arch;
    invalidateFilter();
}

bool PackagesFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    auto pkg          = index.data(Qt::UserRole).value<Package>();

    return (m_group.isEmpty() || pkg.m_group == m_group || pkg.m_group.startsWith(QString("%1/").arg(m_group)))
           && (m_arch.isEmpty() || pkg.m_arch == m_arch)
           && QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}
