#pragma once

#include "SearchAdapter.h"
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

/*
 * Base class for searching inside an item model
 */
class ItemModelSearcher : public SearchAdapter {
    Q_OBJECT

public:
    ItemModelSearcher(QAbstractItemModel* model = nullptr, QObject* parent = nullptr);
    ~ItemModelSearcher() override;
    void setModel(QAbstractItemModel& model);
    int search(const QString& text) override;
    QAbstractItemModel* model();

protected:
    QModelIndex prevIndex();
    QModelIndex nextIndex();
    QModelIndex currentIndex();

    QSortFilterProxyModel& filterModel();
    QModelIndex mapToSource(const QModelIndex&);

private:
    class Private;
    Private* d;
};
