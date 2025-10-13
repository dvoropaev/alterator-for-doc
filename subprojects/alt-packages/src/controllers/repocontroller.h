#ifndef REPOCONTROLLER_H
#define REPOCONTROLLER_H

#include "basecontroller.h"
#include "entities/repo.h"

#include <QSortFilterProxyModel>
#include <QStandardItemModel>

enum RepoModelColumn
{
    columnRepo,
    columnType,
    columnSign,
    columnUrl,
    columnComponent
};

class RepoManagerWidget;

class RepoController : public BaseController
{
    Q_OBJECT

public:
    RepoController(DataSourceInterface *dataSource, MainController *mc);
    ~RepoController();

public:
    void updateModel() override;

public slots:
    void onRemoveRepoRequested(const QModelIndexList &indexes);
    void onAddRepoRequested();
    void onSelected();
    void onDeselected();
    void onWindowClose();

private:
    void updateModelHeaderData() override;
    std::vector<Repo> getModelData();

private:
    RepoController(const RepoController &)            = delete;
    RepoController(RepoController &&)                 = delete;
    RepoController &operator=(const RepoController &) = delete;
    RepoController &operator=(RepoController &&)      = delete;

private:
};

#endif // REPOCONTROLLER_H
