#pragma once

#include <QStyledItemDelegate>

class ObjectInfoDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    ObjectInfoDelegate(QObject* parent = nullptr);
    ~ObjectInfoDelegate();

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index) override;

    void setColumnSize(int size);

    void setNeverDetailed(bool);

private slots:
    void setDetailed();
    void setMultiline();

private:
    class Private;
    Private* d;
};
