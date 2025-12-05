#include "ItemModelSearcher.h"

#include <KDescendantsProxyModel>
#include <QSortFilterProxyModel>

class ItemModelSearcher::Private {
public:
    QAbstractItemModel* m_model{};
    QSortFilterProxyModel m_filter;
    KDescendantsProxyModel m_flat;
    int m_current_row{-1};

    QString m_text;
};

ItemModelSearcher::ItemModelSearcher(QAbstractItemModel* model)
    : d{new Private}
{
    if (model)
        setModel(*model);

    d->m_filter.setSourceModel(&d->m_flat);
}

ItemModelSearcher::~ItemModelSearcher() { delete d; }

void ItemModelSearcher::setModel(QAbstractItemModel& model)
{
    d->m_model = &model;

    d->m_flat.setSourceModel(d->m_model);

    d->m_filter.setFilterKeyColumn(-1);
    d->m_filter.setFilterCaseSensitivity(Qt::CaseInsensitive);
}

int ItemModelSearcher::search(const QString& text)
{
    d->m_current_row = -1;
    d->m_text = text;
    d->m_filter.setFilterFixedString(text);

    return d->m_filter.rowCount();
}

QAbstractItemModel* ItemModelSearcher::model() { return d->m_model; }

QModelIndex ItemModelSearcher::prevIndex()
{
    d->m_current_row--;
    return currentIndex();
}

QModelIndex ItemModelSearcher::nextIndex()
{
    d->m_current_row++;
    return currentIndex();
}

QModelIndex ItemModelSearcher::currentIndex()
{
    if ( d->m_current_row < 0 )
        return {};

    return mapToSource(d->m_filter.index(d->m_current_row,0));
}

QSortFilterProxyModel& ItemModelSearcher::filterModel() { return d->m_filter; }

QModelIndex ItemModelSearcher::mapToSource(const QModelIndex& index)
{
    return d->m_flat.mapToSource(d->m_filter.mapToSource(index));
}
