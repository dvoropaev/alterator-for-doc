#pragma once

#include <QAbstractItemModel>
#include "data/Parameter.h"

class ParameterModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    ParameterModel(const PtrVector<Parameter>& data);
    inline ParameterModel(const std::vector<Parameter*>& data = {}) { setItems(data); }

    void setItems(const std::vector<Parameter*>& items);

    int indexOf(const Parameter* param) const;
    QModelIndex indexOf(const Property::Value*) const;

    // QAbstractItemModel interface
public:
    QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
    QModelIndex parent(const QModelIndex& child) const override;

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

protected:
    virtual Property::Value* getValue(Parameter*) const = 0;

    std::vector<Parameter*> m_items;
};
