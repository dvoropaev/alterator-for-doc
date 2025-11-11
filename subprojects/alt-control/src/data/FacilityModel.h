#pragma once

#include <QObject>
#include <QAbstractTableModel>
#include "Facilty.h"

class Controller;

class FacilityModel : public QAbstractTableModel
{
    Q_OBJECT
    friend class Controller;
public:
    explicit FacilityModel(const std::vector<std::unique_ptr<Facility>>& facilities, QObject *parent = nullptr);

    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Facility* facility(const QModelIndex& index) const;
    QModelIndex indexOf(Facility*) const;

private:
    const std::vector<std::unique_ptr<Facility>>& m_facilities;
};
