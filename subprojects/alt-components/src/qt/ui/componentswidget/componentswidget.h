#ifndef COMPONENTSWIDGET_H
#define COMPONENTSWIDGET_H

#include <memory>

#include <QItemSelection>
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

    void setComponentsModel(QAbstractItemModel *model);
    void expandTopLevel();
    void updateDescription();

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
    std::unique_ptr<QMenu> treeContextMenu;
    std::unique_ptr<QMenu> packagesContextMenu;
    std::unique_ptr<QShortcut> findShortcut;
};

} // namespace alt

#endif // COMPONENTSWIDGET_H
