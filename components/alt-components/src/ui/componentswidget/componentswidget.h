#ifndef COMPONENTSWIDGET_H
#define COMPONENTSWIDGET_H

#include "model/objects/package.h"
#include "model/proxymodel.h"
#include <memory>

#include <QMenu>
#include <QShortcut>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QWidget>

namespace alt::Ui
{
class ComponentsWidget;
} // namespace alt::Ui

namespace alt
{
class Controller;

class ComponentsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ComponentsWidget(QWidget *parent = nullptr);
    ~ComponentsWidget() override;

    void setComponentsModel(alt::ProxyModel *model);
    void setDescription(const QString &description);
    void setContentList(const std::vector<std::shared_ptr<Package>> &packages);
    void setContentList(QAbstractItemModel *model, const QModelIndex &index);
    void expandTopLevel();

public:
    ComponentsWidget(const ComponentsWidget &) = delete;
    ComponentsWidget(ComponentsWidget &&) = delete;
    ComponentsWidget &operator=(const ComponentsWidget &) = delete;
    ComponentsWidget &operator=(ComponentsWidget &&) = delete;

private:
    void setupTreeCustomContextMenu();
    void setupPackagesContextMenu();
    void retranslateContextMenus();
    void setDefaultDescription();
    void loadDescriptionStyleSheet();

public slots:
    void changeEvent(QEvent *event) override;
    void treeCustomContextMenu(const QPoint &pos);
    void packagesCustomContextMenu(const QPoint &pos);
    void setVisibleContent(bool visible);

private slots:
    void onSelectionChanged(const QItemSelection &newSelection, const QItemSelection &previousSelection);
    void onSearchTextChanged(const QString &query);
    void onCopyAllPackages();

private:
    std::unique_ptr<Ui::ComponentsWidget> ui;
    std::unique_ptr<QStandardItemModel> packagesListModel;
    std::unique_ptr<QMenu> treeContextMenu;
    std::unique_ptr<QMenu> packagesContextMenu;
    std::unique_ptr<QShortcut> findShortcut;
};

} // namespace alt

#endif // COMPONENTSWIDGET_H
