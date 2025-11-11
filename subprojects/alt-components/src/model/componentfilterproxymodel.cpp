#include "componentfilterproxymodel.h"

#include "model.h"

namespace alt
{
ComponentFilterProxyModel::ComponentFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setRecursiveFilteringEnabled(true);
}

QVariant ComponentFilterProxyModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole || !index.isValid())
    {
        return QSortFilterProxyModel::data(index, role);
    }

    using Type = ModelItem::Type;
    const auto type = index.data(CustomRoles::TypeRole).value<Type>();
    if (type == Type::Component || type == Type::Category)
    {
        return QSortFilterProxyModel::data(index, role);
    }

    const auto name = QSortFilterProxyModel::data(index, role).toString();
    return QString("%1 (%2)").arg(name).arg(countComponents(index).total);
}

void ComponentFilterProxyModel::setFilter(FilterOptions option, bool value)
{
    options.setFlag(option, value);
}

bool ComponentFilterProxyModel::testFilter(FilterOptions option)
{
    return options.testFlag(option);
}

bool ComponentFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    std::ignore = std::tie(left, right);
    return false;
}

bool ComponentFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const auto index = sourceModel()->index(sourceRow, 0, sourceParent);
    if (const auto *component = index.data(alt::ObjectRole).value<Component *>())
    {
        if (Model::current_edition != nullptr && options.testFlag(NonEdition) && !component->hasRelationToEdition)
        {
            return false;
        }

        if (options.testFlag(Draft) && component->isDraft)
        {
            return false;
        }

        return true;
    }

    return false;
}

ComponentFilterProxyModel::ComponentsCount ComponentFilterProxyModel::countComponents(const QModelIndex &index) const
{
    ComponentsCount count = {
        .total = 0,
        .installed = 0,
    };
    const int rowCount = this->rowCount(index);
    for (int row = 0; row < rowCount; ++row)
    {
        const auto childCount = countComponents(this->index(row, 0, index));
        count.total += childCount.total;
        count.installed += childCount.installed;
    }
    if (const auto *component = index.data(CustomRoles::ObjectRole).value<Component *>())
    {
        ++count.total;
        count.installed += component->state == ComponentState::installed;
    }
    return count;
}
} // namespace alt
