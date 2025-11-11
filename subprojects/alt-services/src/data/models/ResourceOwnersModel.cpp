#include "ResourceOwnersModel.h"

#include "data/Service.h"
#include "app/ServicesApp.h"
#include "controller/Controller.h"

#include <QIcon>

void ResourceOwnersModel::parametersChanged()
{
    m_conflicts.clear();
    for ( const auto& [type,resources] : m_resources ) {
        for ( const auto& resource : resources ) {
            Service* otherService{};
            Resource* otherResource{};
            if ( ServicesApp::instance()->controller()->findConflict(resource->service(), resource, &otherService, &otherResource) )
                m_conflicts[resource] = { otherService, otherResource };
        }
    }
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

    auto it = m_conflicts.find( ResourceModel::resource(index) );
    if ( it != m_conflicts.cend() ) {
        auto& [otherService,otherResource] = it->second;

        switch (role) {
            case Qt::DisplayRole:    return tr("Owned by '%0'").arg(otherService->displayName());
            case Qt::ToolTipRole:    return otherService->comment();
            case Qt::DecorationRole: return QIcon::fromTheme("dialog-warning");
            default: return {};
        }
    } else
        switch (role) {
            case Qt::DisplayRole:    return tr("Free");
            case Qt::DecorationRole: return QIcon::fromTheme("dialog-ok");
            default: return {};
        }

    return {};

}

QVariant ResourceOwnersModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ( orientation == Qt::Horizontal && section == 2 && role == Qt::DisplayRole )
        return tr("State");
    return ResourceModel::headerData(section, orientation, role);
}

void ResourceOwnersModel::setItems(const PtrVector<Resource>& items)
{
    ResourceModel::setItems(items);
    parametersChanged();
}
