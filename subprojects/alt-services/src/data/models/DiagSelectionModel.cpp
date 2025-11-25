#include "DiagSelectionModel.h"
#include <qapplication.h>
#include <qpalette.h>

static const QList<int> rolesToChange{Qt::CheckStateRole, Qt::ForegroundRole};

void DiagSelectionModel::setSelection(Action::TestSet* selection)
{
    m_selection = selection;
    emit dataChanged(index(0,0), index(rowCount()-1,0), rolesToChange);
    for ( int row = 0; row < rowCount(); ++row )
    {
        auto parent = index(row, 0);
        emit dataChanged(index(0,0, parent), index(rowCount(parent)-1,0), rolesToChange);
    }
}

std::pair<DiagTool*, DiagTool::Test*> getPointers(const QModelIndex& index){
    auto* ptr = index.data(Qt::UserRole).value<void*>();

    DiagTool* tool{};
    DiagTool::Test* test{};

    if ( index.parent().isValid() )
    {
        test = static_cast<DiagTool::Test*>(ptr);
        tool = test->tool();
    }
    else
    {
        tool = static_cast<DiagTool*>(ptr);
    }

    return {tool, test};
}

QVariant DiagSelectionModel::data(const QModelIndex& index, int role) const
{
    if (!m_selection) return {};

    if ( rolesToChange.contains(role) )
    {
        auto [tool, test] = getPointers(index);

        bool tool_all = true;
        bool any = false;

        if ( !test )
            for ( int row = 0; row < rowCount(index); ++row )
            {
                bool checked = DiagSelectionModel::index( row, 0, index )
                    .data(Qt::CheckStateRole).toInt() == Qt::Checked;

                if ( checked )
                    any = true;

                tool_all &= checked;
            }
        else
            any = mapToSource(index).siblingAtColumn(1).data().toBool()
                  || m_selection->hasTest(test);

        switch (role) {
            case Qt::CheckStateRole:
            {
                if (test)
                    return any ? Qt::Checked : Qt::Unchecked;
                else
                    return tool_all  ? Qt::Checked
                         : (any      ? Qt::PartiallyChecked
                                     : Qt::Unchecked);
            } break;

            case Qt::ForegroundRole:
                return QApplication::palette().brush(
                    any ? QPalette::Current : QPalette::Disabled ,
                    QPalette::Text
                );
            break;

            default: break;
        }
    }

    return QSortFilterProxyModel::data(index, role);
}

bool DiagSelectionModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if ( role == Qt::CheckStateRole )
    {
        auto [tool, test] = getPointers(index);

        if ( test )
        {
            m_selection->toggle(test);
            emit dataChanged(index,          index,          rolesToChange);
            emit dataChanged(index.parent(), index.parent(), rolesToChange);
        }
        else
        {
            Qt::CheckState currentState = (Qt::CheckState)index.data(Qt::CheckStateRole).toInt();
            for ( int row = 0; row < rowCount(index); ++row )
            {
                auto child = DiagSelectionModel::index( row, 0, index );
                test = getPointers(child).second;

                if ( currentState == Qt::Checked )
                    m_selection->erase(test);
                else
                    m_selection->insert(test);

                emit dataChanged(child, child, rolesToChange);
            }
            emit dataChanged(index, index, rolesToChange);
        }

        return true;
    }

    return false;
}

Qt::ItemFlags DiagSelectionModel::flags(const QModelIndex& index) const
{
    auto flags = QAbstractItemModel::flags(index);
    flags.setFlag(Qt::ItemIsUserCheckable,
                  !mapToSource(index).siblingAtColumn(1).data().toBool());
    return flags;
}

bool DiagSelectionModel::filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const
{ return source_column == 0; }
