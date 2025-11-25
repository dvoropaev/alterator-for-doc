#include "proxymodel.h"

#include "../item.h"
#include "entity/category.h"
#include "entity/constants.h"
#include "entity/section.h"

namespace alt
{
ProxyModel::ProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    this->setFilterCaseSensitivity(Qt::CaseInsensitive);
    this->setRecursiveFilteringEnabled(true);
}

bool ProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const auto *leftSection = left.data(alt::ObjectRole).value<Section *>();
    const auto *rightSection = right.data(alt::ObjectRole).value<Section *>();
    if ((leftSection != nullptr) && (rightSection != nullptr))
    {
        return leftSection->sort_weight > rightSection->sort_weight;
    }

    // NOTE(mchernigin): sort so default category is always on the bottom
    const auto *leftData = left.data(alt::ObjectRole).value<Category *>();
    if ((leftData != nullptr) && leftData->name == entity::DEFAULT_CATEGORY_ID)
    {
        return true;
    }
    const auto *rightData = right.data(alt::ObjectRole).value<Category *>();
    if ((rightData != nullptr) && rightData->name == entity::DEFAULT_CATEGORY_ID)
    {
        return false;
    }

    const QString leftDisplayName = left.data().toString();
    const QString rightDisplayName = right.data().toString();

    return leftDisplayName.compare(rightDisplayName, Qt::CaseInsensitive) > 0;
}

bool ProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const auto index = sourceModel()->index(sourceRow, 0, sourceParent);

    // NOTE(mchernigin): check if item itself passes filter
    if (index.data().toString().contains(filterRegularExpression()))
    {
        return true;
    }

    // NOTE(mchernigin): check if parent is pases filter
    for (QModelIndex i = sourceParent; i.isValid(); i = i.parent())
    {
        if (i.data().toString().contains(filterRegularExpression()))
        {
            return true;
        }
    }

    return false;
}
} // namespace alt
