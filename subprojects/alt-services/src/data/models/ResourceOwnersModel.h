#pragma once

#include "ResourceModel.h"
class Controller;

class ResourceOwnersModel : public ResourceModel {
    Q_OBJECT
public:
    using ResourceModel::ResourceModel;

    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void setItems(const PtrVector<Resource>& items);

    inline bool hasConflicts() const { return !m_conflicts.empty(); }

    inline bool conflictsResolvable() const { return std::all_of(
        m_conflicts.cbegin(), m_conflicts.cend(),
        [](const ConflictMap::const_iterator::value_type& conflict){
            const auto& [resource, owner] = conflict;
            return resource->override();
        }
    );}

public slots:
    void parametersChanged();

private:
    using ConflictMap = std::map<Resource*, std::pair<Service*, Resource*>>;
    ConflictMap m_conflicts;
};
