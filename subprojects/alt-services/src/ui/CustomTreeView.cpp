#include "CustomTreeView.h"

#include <QTimer>
#include <QHeaderView>
#include <QKeyEvent>
#include "ObjectInfoDelegate.h"
#include "app/ServicesApp.h"
#include "controller/Controller.h"

CustomTreeView::CustomTreeView(QWidget* parent)
    : QTreeView{parent}
{
    setSelectionMode(QAbstractItemView::NoSelection);
    setAnimated(true);
    setTabKeyNavigation(true);

    header()->setStretchLastSection(true);
    setUniformRowHeights(false);

    connect(ServicesApp::instance()->settings(), &AppSettings::tablesDetailedMultilineChanged, this, [this]{
        setWordWrap(ServicesApp::instance()->settings()->tablesDetailedMultiline());
    });
    setWordWrap(ServicesApp::instance()->settings()->tablesDetailedMultiline());

    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    connect(header(), &QHeaderView::sectionResized, [this](int index, int oldSize, int newSize){
        if ( auto d = qobject_cast<ObjectInfoDelegate*>(itemDelegateForColumn(index)) )
            d->setColumnSize(newSize);
    });

    setContextMenuPolicy(Qt::ActionsContextMenu);
    addActions(ServicesApp::instance()->controller()->tableActions());
}

void CustomTreeView::setModel(QAbstractItemModel* model)
{
    QTreeView::setModel(model);
    for (int i = 0; i < model->columnCount(); ++i)
        QTreeView::setItemDelegateForColumn(i, new ObjectInfoDelegate{this});

    connect(model, &QAbstractItemModel::dataChanged,
            [this](const QModelIndex &tl, const QModelIndex &br, const QList<int> &roles){
        if ( roles.contains(Qt::DecorationRole) )
            for (int i = tl.column(); i <= br.column(); ++i)
                if ( auto d = qobject_cast<ObjectInfoDelegate*>(itemDelegateForColumn(i)) )
                    emit d->sizeHintChanged({});
    });
}

void CustomTreeView::setColumnNeverDetailed(int column, bool restrict)
{
    if ( auto* d = dynamic_cast<ObjectInfoDelegate*>(itemDelegateForColumn(column)) )
        d->setNeverDetailed(restrict);
}

void CustomTreeView::insertAction(int index, QAction* action)
{
    if ( index >= actions().size() )
        QTreeView::addAction(action);
    else
        QTreeView::insertAction(actions().at(index), action);
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

void CustomTreeView::keyPressEvent(QKeyEvent* event)
{
    auto index = currentIndex();
    if ( event->key() == Qt::Key_Space && index.isValid() )
        emit clicked(index);
    QTreeView::keyPressEvent(event);
}
