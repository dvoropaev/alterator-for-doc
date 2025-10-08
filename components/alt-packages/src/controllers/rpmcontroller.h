#ifndef RPMCONTROLLER_H
#define RPMCONTROLLER_H

#include "basecontroller.h"

#include "entities/package.h"

#include <QSortFilterProxyModel>
#include <QStandardItemModel>

enum RpmModelColumn
{
    columnPackage,
    columnName,
    columnVersionRelease,
};

class RpmManagerWidget;

class RpmController : public BaseController
{
    Q_OBJECT

public:
    RpmController(DataSourceInterface *dataSource, MainController *mc);
    ~RpmController();

public:
    void updateModel() override;
    QStringList groupsOfAllPackages();
    QStringList archesOfAllPackages();
    void setManagerWidget(RpmManagerWidget *widget);

public slots:
    void onRemovePackagesRequested(const QModelIndexList &indexes);
    void onInstallPackageRequested();
    void onInfoAboutPackagesRequested(const QModelIndexList &indexes);
    void onFilesOfPackagesRequested(const QModelIndexList &indexes);
    void onSelected();
    void onDeselected();
    void onWindowClose();

signals:
    void packagesListChanged();

private:
    void updateModelHeaderData() override;
    std::vector<Package> getModelData();
    QStringList getSelectedPackagesNames(const QModelIndexList &indexes);
    QStandardItemModel *buildPackageInfoModel(const QString &packageName);
    QStringListModel *buildPackageFilesModel(const QString &packageName);

private:
    RpmController(const RpmController &)            = delete;
    RpmController(RpmController &&)                 = delete;
    RpmController &operator=(const RpmController &) = delete;
    RpmController &operator=(RpmController &&)      = delete;

private:
    QSet<QString> m_groupsOfAllPackages;
    QSet<QString> m_archesOfAllPackages;
};

#endif // RPMCONTROLLER_H
