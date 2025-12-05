#pragma once

#include "../Resource.h"
#include <QAbstractItemModel>

class ResourceModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    inline ResourceModel(const PtrVector<Resource>& data = {}) { setItems(data); }

    void setItems(const PtrVector<Resource>& items);

    inline void setScope(Parameter::ValueScope scope){
        m_scope = scope;
        refresh();
    }
    inline auto scope() const { return m_scope; }

    inline void refresh(){
        beginResetModel();
        endResetModel();
    }
    
    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    QSize span(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    static const QIcon& resourceIcon(Resource::Type);
    QModelIndex indexOf(Resource*);
    Resource* resource(const QModelIndex& i) const ;
    
protected:
    std::map<Resource::Type, std::vector<Resource*>> m_resourcesByType;
    Parameter::ValueScope m_scope{Parameter::ValueScope::Default};
};
