#ifndef BASEMANAGERWIDGET_H
#define BASEMANAGERWIDGET_H

#include "model/packagessortfilterproxymodel.h"
#include <QStandardItemModel>

#include <QAbstractItemView>
#include <QHeaderView>
#include <QShortcut>
#include <QWidget>

namespace Ui
{
class BaseManagerWidget;
}

class BaseManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BaseManagerWidget(QWidget *parent = nullptr);
    ~BaseManagerWidget();

public:
    void setSelectionMode(QAbstractItemView::SelectionMode mode);
    void setModel(QSortFilterProxyModel *model);
    QModelIndexList getSelection();
    QHeaderView *getHorizontalHeader();
    QItemSelectionModel *getSelectionModel();

    void onFilterChanged(void (PackagesFilterProxyModel::*filterMethod)(const QString &), const QString &text);

public slots:
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onFilterTextChanged();
    void onProgressChanged(int state, const QString &message);
    void onWait(bool start, const QString &message);
    void onShowLoadingIndicator(bool visible);

protected:
    virtual void setButtonsEnabled(bool isEnable) = 0;

protected slots:
    void changeEvent(QEvent *event) override;

private:
    void saveSelectedIndexes();
    void restoreSelectedIndexes();

private:
    BaseManagerWidget(const BaseManagerWidget &)            = delete;
    BaseManagerWidget(BaseManagerWidget &&)                 = delete;
    BaseManagerWidget &operator=(const BaseManagerWidget &) = delete;
    BaseManagerWidget &operator=(BaseManagerWidget &&)      = delete;

protected:
    std::unique_ptr<Ui::BaseManagerWidget> m_ui;
    std::unique_ptr<QShortcut> m_findShortcut;
    QSet<QModelIndex> m_selectedSourceIndexes;
    bool m_onFilterTextChanged;
};

#endif // BASEMANAGERWIDGET_H
