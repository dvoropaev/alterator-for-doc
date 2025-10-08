#ifndef APTCONTROLLER_H
#define APTCONTROLLER_H

#include "basecontroller.h"
#include "entities/package.h"

#include "ui/dialogs/applydialog.h"

#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

class AptManagerWidget;

class AptController : public BaseController
{
    Q_OBJECT

public:
    AptController(DataSourceInterface *dataSource, MainController *mc);
    ~AptController();

public:
    void updateModel() override;

public slots:
    void onApplyChangesRequested(const QModelIndexList &indexes);
    void onUpdateRequested();
    void onDistUpgradeRequested();
    void onSelected();
    void onDeselected();
    void onWindowClose();
    void onInstall(QString message);
    void onUpdate(QString message);
    void onDistUpgrade(QString message);
    void onCloseWaitDialog();
    bool disableSafeMode(bool value);

private:
    void install(const QStringList &packages);
    void update();
    void distUpgrade();
    void updateModelHeaderData() override;
    std::vector<Package> getModelData();
    QMessageBox::StandardButton questionApply(const QModelIndexList &indexes);
    QMessageBox::StandardButton questionUpdate();
    QMessageBox::StandardButton questionDistUpgrade();

private:
    AptController(const AptController &)            = delete;
    AptController(AptController &&)                 = delete;
    AptController &operator=(const AptController &) = delete;
    AptController &operator=(AptController &&)      = delete;

private:
    bool safeMode = true;
    QStringList excludePackages;
};

#endif // APTCONTROLLER_H
