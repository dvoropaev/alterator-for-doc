#pragma once

#include "../Resource.h"
#include <QAbstractItemModel>

class ResourceModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ResourceModel(const PtrVector<Resource>& data);
    
    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    QSize span(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    static const QIcon& resourceIcon(Resource::Type);
    QModelIndex indexOf(Resource*);
    
protected:
    std::map<Resource::Type, std::vector<Resource*>> m_resources;
};
