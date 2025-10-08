#ifndef VARS_TABLE_VIEW_H
#define VARS_TABLE_VIEW_H

#include <QTableView>

class VarsTableView : public QTableView
{
public:
    VarsTableView(QWidget *parent = nullptr);
    ~VarsTableView() = default;

    void setModel(QAbstractItemModel *newModel) override;
    void updateColumnsSizes();

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // VARS_TABLE_VIEW_H
