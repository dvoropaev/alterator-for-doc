#include "proxymodel.h"
#include "constants.h"
#include "model/item.h"
#include "model/model.h"
#include "model/objects/category.h"
#include "model/objects/component.h"
#include "model/objects/section.h"

namespace alt
{
ProxyModel::ProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    this->setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void ProxyModel::setShowDrafts(bool showDrafts)
{
    this->showDrafts = showDrafts;
}

void ProxyModel::setFilterNonEditionComponents(bool show)
{
    this->showNonEditionComponentsSection = show;
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
    if ((leftData != nullptr) && leftData->name == DEFAULT_CATEGORY_ID)
    {
        return true;
    }
    const auto *rightData = right.data(alt::ObjectRole).value<Category *>();
    if ((rightData != nullptr) && rightData->name == DEFAULT_CATEGORY_ID)
    {
        return false;
    }

    const QString leftDisplayName = left.data().toString();
    const QString rightDisplayName = right.data().toString();

    return leftDisplayName.compare(rightDisplayName, Qt::CaseInsensitive) > 0;
}

bool ProxyModel::hasRelationToEdition(const QModelIndex &index) const
{
    const auto *component = index.data(alt::ObjectRole).value<Component *>();
    if (component != nullptr && component->hasRelationToEdition)
    {
        return true;
    }

    for (int i = 0; i < sourceModel()->rowCount(index); ++i)
    {
        const QModelIndex childIndex = sourceModel()->index(i, 0, index);
        if (hasRelationToEdition(childIndex))
        {
            return true;
        }
    }

    return false;
}

bool ProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const auto index = sourceModel()->index(sourceRow, 0, sourceParent);

    const auto *category = index.data(alt::ObjectRole).value<Category *>();
    const auto *component = index.data(alt::ObjectRole).value<Component *>();

    // NOTE(alxvmr): filter of the "Other components" section
    if (alt::Model::current_edition != nullptr
        && !this->showNonEditionComponentsSection
        && !hasRelationToEdition(index))
    {
        return false;
    }

    // NOTE(chernigin): filter out drafts
    const bool isDraft = (category != nullptr && category->isDraft) || (component != nullptr && component->isDraft);
    if (!this->showDrafts && isDraft)
    {
        return false;
    }

    // NOTE(mchernigin): filter empty categories with no items
    if (category != nullptr && sourceModel()->rowCount(index) == 0)
    {
        return false;
    }

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

    // NOTE(mchernigin): check if any child passes filter
    const std::function<bool(const QModelIndex &)> checkChildren = [this, &checkChildren](const QModelIndex &index) {
        for (int i = 0; i < sourceModel()->rowCount(index); ++i)
        {
            const auto isItselfGood
                = sourceModel()->index(i, 0, index).data().toString().contains(filterRegularExpression());
            if (isItselfGood || checkChildren(sourceModel()->index(i, 0, index)))
            {
                return true;
            }
        }
        return false;
    };
    return checkChildren(index);
}
} // namespace alt
