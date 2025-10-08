#ifndef PROPERTYTABLEVIEW_H
#define PROPERTYTABLEVIEW_H

#include <QWidget>

#include <memory>

class QAbstractItemModel;
class QTableView;

namespace alt
{
namespace Ui
{
class PropertyTableView;
} // namespace Ui

class PropertyTableView : public QWidget
{
    Q_OBJECT

public:
    PropertyTableView(QWidget *parent = nullptr);
    ~PropertyTableView();

public:
    void setModel(QAbstractItemModel *model);
    QTableView *tableView();

private slots:
    void onDataChanged(const QModelIndex &topLeft,
                       const QModelIndex &bottomRight,
                       const QVector<int> &roles = QVector<int>());

private:
    PropertyTableView(const PropertyTableView &) = delete;
    PropertyTableView(PropertyTableView &&) = delete;
    PropertyTableView &operator=(const PropertyTableView &) = delete;
    PropertyTableView &operator=(PropertyTableView &&) = delete;

private:
    std::unique_ptr<Ui::PropertyTableView> m_ui;
};
} // namespace alt
#endif // PROPERTYTABLEVIEW_H
