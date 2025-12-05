#include "ResourceOwnersModel.h"

#include "data/Service.h"
#include "app/ServicesApp.h"
#include "controller/Controller.h"

#include <QIcon>

ResourceOwnersModel::ResourceOwnersModel(const PtrVector<Resource>& data)
    : ResourceModel{data}
{
    connect(this, &QAbstractItemModel::modelReset,
            this, &ResourceOwnersModel::checkConflicts);
}

void ResourceOwnersModel::checkConflicts()
{
    m_owners = m_resourcesByType
            | ranges::views::values
            | ranges::views::join
            | ranges::views::transform([](Resource* r){ return std::make_pair(r, qApp->controller()->findOwner(r)); })
            | ranges::to<OwnersMap>();

    emit dataChanged(index(0, 2),
                     index(rowCount()-1, 2),
                     {Qt::DisplayRole, Qt::DecorationRole});
}

int ResourceOwnersModel::columnCount(const QModelIndex& parent) const {
    return ResourceModel::columnCount(parent) + 1;
}

QVariant ResourceOwnersModel::data(const QModelIndex& index, int role) const {
    if ( !index.parent().isValid() || index.column() < 2 )
        return ResourceModel::data(index, role);

    const Resource* resource      = ResourceModel::resource(index);
    const Resource* otherResource = m_owners.at( resource );

    switch ( role )
    {
        case Qt::DisplayRole:
            return otherResource
                ? tr("Owned by '%0'")
                    .arg(otherResource->service()->displayName())
                : tr("Free");

        case Qt::ToolTipRole:
            return otherResource
                ? otherResource->service()->comment()
                : QVariant{};

        case Qt::DecorationRole:
            return otherResource &&
                   otherResource != resource &&
                   otherResource->service() != resource->service()
                ? QIcon::fromTheme(QIcon::ThemeIcon::DialogWarning)
                : QIcon::fromTheme("dialog-ok");

        default:
            return {};
    }
}

QVariant ResourceOwnersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( orientation == Qt::Horizontal && section == 2 && role == Qt::DisplayRole )
        return tr("Current state");

    return ResourceModel::headerData(section, orientation, role);
}

bool ResourceOwnersModel::hasConflicts() const
{
    return ranges::any_of(m_owners, [](const auto& pair){
        const auto& [a,b] = pair;
        return a != b && b != nullptr;
    });
}

bool ResourceOwnersModel::conflictsResolvable() const
{
    return ranges::all_of(m_owners, [](const auto& pair){
        const auto& [a,b] = pair;
        return a == b || b == nullptr || b->override();
    });
}
