#pragma once

#include <QTreeView>

class CustomTreeView : public QTreeView
{
    Q_OBJECT
public:
    CustomTreeView(QWidget* parent = nullptr);
    void setModel(QAbstractItemModel* model) override;

    void setColumnNeverDetailed(int, bool);

    void insertAction(int index, QAction* action);

public slots:
    void highlight(const QModelIndex& index);

private:
    void setItemDelegate(QAbstractItemDelegate*) = delete;
    void setItemDelegateForRow(int, QAbstractItemDelegate*) = delete;

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent* event) override;
    // TODO: re-order cells focus
    //bool focusNextPrevChild(bool next) override;
};

