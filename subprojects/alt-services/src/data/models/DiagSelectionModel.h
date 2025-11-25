#pragma once

#include <QSortFilterProxyModel>
#include "data/Action.h"
#include "DiagModeFilterModel.h"

/*
 *  Uses DiagModeFilterModel "required" column
 *  and given Action::TestSet to map them into CheckState.
 *  Also allows to modify Action::TestSet by SetData
 */
class DiagSelectionModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Action::TestSet* m_selection{};
public:
    explicit inline DiagSelectionModel(DiagModeFilterModel* source)
    { setSourceModel(source); }

    inline void setSourceModel(DiagModeFilterModel* sourceModel)
    { QSortFilterProxyModel::setSourceModel(sourceModel); }

    void setSelection(Action::TestSet* selection);

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

protected:
    bool filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const override;
};
