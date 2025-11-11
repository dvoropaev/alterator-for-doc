#pragma once

#include <QStyledItemDelegate>

class ObjectInfoDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    ObjectInfoDelegate(QObject* parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void setColumnSize(int size);

private slots:
    void setDetailed();

private:
    bool m_detailed{true};
    int m_column_size{0};
};
