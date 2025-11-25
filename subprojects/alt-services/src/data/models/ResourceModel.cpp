#include "ResourceModel.h"

#include <QSize>
#include <QApplication>
#include <QFontDatabase>
#include <QIcon>
#include <QPalette>

void ResourceModel::setItems(const PtrVector<Resource>& items)
{
    beginResetModel();
    m_resources.clear();
    for ( const auto& resource : items )
        m_resources[resource->type()].push_back(resource.get());
    endResetModel();
}

int ResourceModel::rowCount(const QModelIndex& parent) const {
    if ( !parent.isValid() )
        return m_resources.size();

    if ( parent.internalId() == ULONG_LONG_MAX ) {
        auto typeIt = m_resources.begin();
        std::advance(typeIt, parent.row());
        if ( typeIt == m_resources.end() )
            return {};

        auto& [type,resources] = *typeIt;
        return resources.size();
    }

    return 0;
}

int ResourceModel::columnCount(const QModelIndex& parent) const {
    return 2;
}

QModelIndex ResourceModel::index(int row, int column, const QModelIndex& parent) const {
    if ( column >= columnCount(parent) || row >= rowCount(parent) )
        return {};

    if ( parent.isValid() && parent.internalId() == ULONG_LONG_MAX )
        return createIndex(row, column, parent.row());

    if ( !parent.isValid() )
        return createIndex(row, column, ULONG_LONG_MAX);

    return {};
}

QModelIndex ResourceModel::parent(const QModelIndex& child) const {
    if ( child.internalId() == ULONG_LONG_MAX )
        return {};
    return createIndex(child.internalId(), 0, ULONG_LONG_MAX);
}

QSize ResourceModel::span(const QModelIndex& index) const {
    if ( index.internalId() == ULONG_LONG_MAX )
        return {index.column() == 0 ? 2 : 0 , 1};
    return QAbstractItemModel::span(index);
}

QVariant ResourceModel::data(const QModelIndex& index, int role) const
{
    auto typeIt = m_resources.begin();
    std::advance(typeIt, index.internalId() == ULONG_LONG_MAX ? index.row() : index.internalId());
    if ( typeIt == m_resources.end() )
        return {};

    auto& [type,resources] = *typeIt;

    static const std::map<Resource::Type, QString> typeNames {
        { Resource::Type::Path, tr("Directories"    ) },
        { Resource::Type::File, tr("Files"          ) },
        { Resource::Type::Unit, tr("Systemd units"  ) },
        { Resource::Type::Port, tr("Ports"          ) },
    };

    if ( !index.parent().isValid() ) {
        if ( index.column() == 0 ) switch (role) {
            case Qt::DisplayRole:
                return typeNames.at(type);

            case Qt::DecorationRole:
                return resourceIcon(type);

            case Qt::BackgroundRole:
                return QApplication::palette().button();
            case Qt::ForegroundRole:
                return QApplication::palette().buttonText();

            case Qt::UserRole: return -1;
        }

        return {};
    }

    Resource* resource = resources.at( index.row() );

    static const std::map<int, QString> protoNames {
        { Resource::Protocol::TCP,  "TCP"      },
        { Resource::Protocol::UDP,  "UDP"      },
        { Resource::Protocol::BOTH, "TCP/UDP"  }
    };

    if ( role == Qt::DisplayRole ) switch ( index.column() ) {
        case 0: return resource->displayName();
        case 1: {
            auto value = resource->value(m_scope).toString();
            if ( resource->type() == Resource::Type::Port )
                return value.rightJustified(5, ' ').append(' ')
                    .append(protoNames.at(resource->portProtocol()));
            return value;
        }
        }

    if ( index.column() == 0 && role == Qt::ToolTipRole )
        return resource->comment();


    Parameter* override = resource->override();

    if ( index.column() == 0 && role == Qt::DecorationRole )
        return override
                   ? QIcon::fromTheme("changes-allow")
                   : QIcon{};

    switch (role) {
        case Qt::UserRole:
            return QVariant::fromValue(override);

        case Qt::ToolTipRole:
            if ( override )
                return tr("This resource may be overriden by a parameter.\nClick to highlight it.");
            break;

        case Qt::FontRole: {
            QFont f;
            if ( index.column() == 1 )
                f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
            f.setBold(override);
            f.setUnderline( override && index.column() == 0 );
            return f;
        }
    }

    return {};
}

QVariant ResourceModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole ) {
        switch ( section ) {
            case 0: return QObject::tr("Name");
            case 1: return QObject::tr("Value");
        }
    }

    return {};
}

inline const QIcon& ResourceModel::resourceIcon(Resource::Type t)
{
    static const std::map<Resource::Type, QIcon> typeIcons {
        { Resource::Type::Path, QIcon::fromTheme("folder"        ) },
        { Resource::Type::File, QIcon::fromTheme("text-x-generic") },
        { Resource::Type::Unit, QIcon::fromTheme("system-run"    ) },
        { Resource::Type::Port, QIcon::fromTheme("network-server") },
    };

    return typeIcons.at(t);
}

QModelIndex ResourceModel::indexOf(Resource* resource)
{
    auto it = m_resources.find(resource->type());

    if ( it == m_resources.end() ) {
        qWarning() << "internal error: cannot find resource parent index";
        return {};
    }

    int row = std::distance(m_resources.begin(), it);
    auto parent = createIndex(row, 0, ULONG_LONG_MAX);

    auto resourceIt = std::find(it->second.begin(), it->second.end(), resource);
    if ( resourceIt == it->second.end() ) {
        qWarning() << "internal error: cannot find resource index";
        return {};
    }

    return createIndex(std::distance(it->second.begin(), resourceIt), 0, row);
}

Resource* ResourceModel::resource(const QModelIndex& index) const
{
    if ( index.internalId() == ULONG_LONG_MAX  )
        return nullptr;

    auto typeIt = m_resources.begin();
    std::advance( typeIt, index.internalId() );

    if ( typeIt == m_resources.end() )
        return nullptr;

    auto& [type,resources] = *typeIt;

    return resources.at( index.row() );
}
