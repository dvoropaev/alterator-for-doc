#include "CustomTreeView.h"

#include <QTimer>
#include <QHeaderView>
#include "ObjectInfoDelegate.h"

CustomTreeView::CustomTreeView(QWidget* parent)
    : QTreeView{parent}
{
    setSelectionMode(QAbstractItemView::NoSelection);
    setAnimated(true);

    header()->setStretchLastSection(true);
    setUniformRowHeights(false);
    setWordWrap(true);

    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    connect(header(), &QHeaderView::sectionResized, [this](int index, int oldSize, int newSize){
        if ( auto d = qobject_cast<ObjectInfoDelegate*>(itemDelegateForColumn(index)) )
            d->setColumnSize(newSize);
    });
}

void CustomTreeView::setModel(QAbstractItemModel* model)
{
    QTreeView::setModel(model);
    for (int i = 0; i < model->columnCount(); ++i)
        setItemDelegateForColumn(i, new ObjectInfoDelegate{this});

    connect(model, &QAbstractItemModel::dataChanged,
            [this](const QModelIndex &tl, const QModelIndex &br, const QList<int> &roles){
        if ( roles.contains(Qt::DecorationRole) )
            for (int i = tl.column(); i <= br.column(); ++i)
                if ( auto d = qobject_cast<ObjectInfoDelegate*>(itemDelegateForColumn(i)) )
                    emit d->sizeHintChanged({});
    });
}

void CustomTreeView::highlight(const QModelIndex& index)
{
    if ( model()->checkIndex(index) ) {
        QTimer::singleShot(200, [=]{
            scrollTo(index);
            setSelectionMode(QAbstractItemView::SingleSelection);
            selectionModel()->select(index, QItemSelectionModel::SelectionFlag::Select | QItemSelectionModel::SelectionFlag::Rows);
            setSelectionMode(QAbstractItemView::NoSelection);

            QTimer::singleShot(500, this, &QAbstractItemView::clearSelection);
        });
    } else qWarning() << "incorrect index";
}
