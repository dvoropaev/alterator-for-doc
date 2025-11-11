#pragma once

#include <QAbstractItemModel>
#include "data/Parameter.h"

class ParameterModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    inline ParameterModel(const std::vector<Parameter*>& data = {}) { setItems(data); }

    void setItems(const std::vector<Parameter*>& items);
    void setScope(Parameter::ValueScope scope);

    int indexOf(const Parameter* param) const;
    QModelIndex indexOf(const Property::Value*) const;

    inline void refresh(){
        beginResetModel();
        endResetModel();
    }

    // QAbstractItemModel interface
public:
    QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
    QModelIndex parent(const QModelIndex& child) const override;

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

protected:
    Property::Value* getValue(Parameter*) const;

    std::vector<Parameter*> m_items;
    Parameter::ValueScope m_scope{Parameter::ValueScope::Current};
};
