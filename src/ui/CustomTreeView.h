#pragma once

#include <QTreeView>

class CustomTreeView : public QTreeView
{
    Q_OBJECT
public:
    CustomTreeView(QWidget* parent = nullptr);
    void setModel(QAbstractItemModel* model) override;

public slots:
    void highlight(const QModelIndex& index);
};

