#pragma once

#include <QAbstractItemModel>
#include "data/Service.h"

class DiagModel : public QAbstractItemModel {
    Q_OBJECT
public:
    DiagModel() = default;

    void setService(Service* s);

    void setMode(DiagTool::Test::Mode mode);

    QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void toggle(const QModelIndex& idx);

signals:
    void selectionChanged(bool);

private:
    const PtrVector<DiagTool>* m_data{nullptr};
    DiagTool::Test::Mode m_mode{DiagTool::Test::PreDeploy};
};
