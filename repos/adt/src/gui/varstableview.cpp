#include "varstableview.h"

#include <QHeaderView>

VarsTableView::VarsTableView(QWidget *parent)
    : QTableView(parent)
{}

void VarsTableView::setModel(QAbstractItemModel *newModel)
{
    QTableView::setModel(newModel);

    if (newModel)
        connect(newModel, &QAbstractItemModel::dataChanged, this, &VarsTableView::updateColumnsSizes);

    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    horizontalHeader()->setStretchLastSection(true);

    for (int col = 1; col < model()->columnCount(); col++)
    {
        horizontalHeader()->setSectionResizeMode(col, QHeaderView::ResizeToContents);
    }
}

void VarsTableView::resizeEvent(QResizeEvent *event)
{
    QTableView::resizeEvent(event);

    int totalWidth        = viewport()->width();
    int otherColumnsWidth = 0;

    updateColumnsSizes();
}

void VarsTableView::updateColumnsSizes()
{
    resizeColumnsToContents();
    resizeRowsToContents();

    int totalWidth        = viewport()->width();
    int otherColumnsWidth = 0;

    if (model())
    {
        for (int col = 1; col < model()->columnCount(); ++col)
        {
            if (!isColumnHidden(col))
                otherColumnsWidth += columnWidth(col);
        }

        int firstColumnWidth = totalWidth - otherColumnsWidth;

        if (firstColumnWidth > 0)
        {
            setColumnWidth(0, firstColumnWidth);
        }
    }
}
