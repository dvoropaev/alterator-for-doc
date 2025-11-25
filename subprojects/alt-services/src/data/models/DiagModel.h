#pragma once

#include <QAbstractItemModel>
#include "data/Service.h"

/*
 *  This is a tree model, consisting of 2 layers:
 *  1. Diagnostic tools
 *  2. Diagnostic tests
 */
class DiagModel : public QAbstractItemModel {
    Q_OBJECT
public:
    enum Column {
        Name,
        Modes,
        Required
    };

    DiagModel() = default;

    void setService(Service* s);

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    QVariant data(const QModelIndex& index, int role) const override;

private:
    const PtrVector<DiagTool>* m_data{nullptr};
};
