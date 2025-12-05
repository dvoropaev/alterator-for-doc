#pragma once

#include "ResourceModel.h"

#include <range/v3/view.hpp>

class ResourceOwnersModel : public ResourceModel {
    Q_OBJECT
public:
    explicit ResourceOwnersModel(const PtrVector<Resource>& data = {});

    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    bool hasConflicts() const;
    bool conflictsResolvable() const;

public slots:
    void checkConflicts();

private:
    using OwnersMap = std::map<const Resource*, const Resource*>;
    OwnersMap m_owners;
};
