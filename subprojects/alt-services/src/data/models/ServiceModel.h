#pragma once

#include <QAbstractItemModel>
#include <QObject>

#include "data/Service.h"

class ServiceModel : public QAbstractTableModel {
    Q_OBJECT
    std::vector<Service*> m_services;

public:
    ServiceModel() = default;

    void setItems(const PtrVector<Service>&);

    int rowCount(const QModelIndex& parent = {}) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole ) const override;

    int columnCount(const QModelIndex& parent) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};
