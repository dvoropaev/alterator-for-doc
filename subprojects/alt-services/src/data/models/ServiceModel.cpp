#include "ServiceModel.h"

#include <QIcon>

void ServiceModel::setItems(const PtrVector<Service>& items)
{
    beginResetModel();
    m_services.clear();
    endResetModel();

    if ( items.size() )
        beginInsertRows({}, 0, items.size()-1);

    m_services.reserve(items.size());
    for ( const auto& service : items )
        m_services.push_back(service.get());

    if ( items.size() )
        endInsertRows();
}

int ServiceModel::rowCount(const QModelIndex& parent) const {return m_services.size();}

QVariant ServiceModel::data(const QModelIndex& index, int role) const
{
    if ( index.row() >= m_services.size() ) return {};
    auto service = m_services[index.row()];

    if ( role == Qt::DisplayRole && index.column() == 0 )
        return service->displayName();

    if ( role == Qt::DecorationRole && index.column() == 0 )
        return service->icon();


    if ( role == Qt::DecorationRole && index.column() == 1 ) {
        if ( service->isDeployed() )
            return service->isStarted()
                    ? QIcon::fromTheme("media-playback-start")
                    : QIcon::fromTheme("media-playback-stop");
        else
            return QIcon::fromTheme("window-close", QIcon::fromTheme("action-unavailable-symbolic"));
    }

    if ( role == Qt::ToolTipRole ) switch ( index.column() ) {
        case 0: return service->comment();
        case 1: {
            QString state = service->isDeployed() ? tr("Deployed") : tr("Not deployed");
            if ( service->isDeployed() )
                state.append(", ").append( service->isStarted() ? tr("started") : tr("stopped") );
            return state;
        }
    }

    return {};
}

int ServiceModel::columnCount(const QModelIndex& parent) const { return 2; }

QVariant ServiceModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ( orientation == Qt::Horizontal ) {
        if ( role == Qt::DisplayRole && section == 0 )
            return tr("Service");

        if ( role == Qt::DecorationRole && section == 1 )
            return QIcon::fromTheme("network-server");

        if ( role == Qt::ToolTipRole && section == 1 )
            return tr("State");

        if ( role == Qt::TextAlignmentRole )
            return Qt::AlignLeft;
    }
    return {};
}
