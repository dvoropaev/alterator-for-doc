#include "FacilityModel.h"

#include <QApplication>
#include <qpalette.h>

FacilityModel::FacilityModel(const std::vector<std::unique_ptr<Facility>>& facilities, QObject *parent)
    : QAbstractTableModel{parent}
    , m_facilities{facilities}
{}

int FacilityModel::rowCount(const QModelIndex& parent) const { return m_facilities.size(); }
int FacilityModel::columnCount(const QModelIndex& parent) const { return 2; }

QVariant FacilityModel::data(const QModelIndex& index, int role) const
{
    const auto* facility     = m_facilities.at(index.row()).get();
    const auto* currentState = facility->currentState();

    if ( index.column() == 0 ) switch ( role ) {
        case Qt::DisplayRole: return facility->displayName();
        case Qt::ToolTipRole: return facility->comment();
    }

    else if ( index.column() == 1 ) switch ( role ) {
        case Qt::ForegroundRole:
            return QApplication::palette().brush(
                currentState
                    ? QPalette::Current
                    : QPalette::Disabled,
                QPalette::Text
            );

        case Qt::DisplayRole:
            if ( currentState ) switch ( currentState->type() ) {
                case Facility::State::Type::String:
                case Facility::State::Type::Integer:
                    return QString{R"("%0": %1)"}
                        .arg(currentState->displayName())
                        .arg(currentState->value().toString());
                    break;
                default:
                    return currentState->displayName();
                    break;
            }
            return tr("(unknown)");

        case Qt::ToolTipRole:
            return currentState
                       ? currentState->comment()
                       : tr("value is not set");
    }

    return {};
}

QVariant FacilityModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole ) {
        if ( section == 0 ) return tr("Facility");
        else if ( section == 1 ) return tr("State");
    }
    return {};
}

Facility* FacilityModel::facility(const QModelIndex& index) const
{
    return m_facilities.at(index.row()).get();
}

QModelIndex FacilityModel::indexOf(Facility* f) const
{
    for ( int row = 0; row < m_facilities.size(); ++row )
        if ( m_facilities[row].get() == f )
            return index(row, 0);
    return {};
}
