#pragma once

#include <QSortFilterProxyModel>
#include "DiagModel.h"

/*
 *  Filteres DiagModel by DiagTool::Test::Mode.
 *  Resulting columns:
 *  1. Tool/Test (unmodified)
 *  2. bool "Requred"
 */
class DiagModeFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    DiagTool::Test::Mode m_mode;
public:
    explicit inline DiagModeFilterModel(DiagModel* source)
    { setSourceModel(source); }

    void setMode(DiagTool::Test::Mode mode);

    QVariant data(const QModelIndex& index, int role) const override;

    inline void setSourceModel(DiagModel* sourceModel)
    { QSortFilterProxyModel::setSourceModel(sourceModel); }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
    bool filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const override;
};
