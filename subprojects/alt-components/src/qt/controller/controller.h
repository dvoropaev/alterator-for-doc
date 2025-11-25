#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <tl/expected.hpp>

#include "../model/item.h"
#include "../model/model.h"
#include "../model/proxy/componentfilterproxymodel.h"
#include "../model/proxy/ensembleproxymodel.h"
#include "../model/proxy/proxymodel.h"
#include "../model/proxy/transactionfilterproxymodel.h"
#include "../ui/applydialog/transactionwizard.h"
#include "../ui/applydialog/updatingsourceswizard.h"
#include "../ui/errordialog/errordialog.h"
#include "../ui/mainwindow/mainwindow.h"
#include "dbus/dbusmanager.h"
#include "interface/ilogger.h"
#include "repository/packagerepository.h"

namespace alt
{
class ControllerPrivate;
class MainWindow;

class Controller : public QObject
{
    Q_OBJECT;

public:
    Controller();
    ~Controller() override;

public:
    static Controller &instance();

    void setLogger(const std::shared_ptr<ILogger> &logger);

    void init();
    void changeLocale(const QLocale &locale);

    QString getDescription(const QModelIndex &index) const;
    std::pair<ModelItem::Type, QAbstractItemModel *> getContent(const QModelIndex &index) const;

    void apply();
    void reset();
    tl::expected<void, PackageRepository::Error> update(
        const std::function<void(const std::string &)> &callback = nullptr);
    void updateSources();
    void updateCurrentTransaction(const QModelIndex &index);

    void showError(int code, const QString &text);
    void showContent(bool showContent);
    void setFilterDrafts(bool showDrafts);
    void setFilterNonEditionComponents(bool show);
    void setViewMode(MainWindow::ViewMode viewMode);
    void setNameViewMode(MainWindow::NameViewMode viewMode);
    void setTextFilter(const QString &query);
    void setSafeModeForBaseComponents(bool safeMode);
    bool setSafeMode(bool safeMode);
    MainWindow::ViewMode getViewMode() const;

    void rebuildModel();

public:
    Controller(const Controller &) = delete;
    Controller(Controller &&) = delete;
    Controller &operator=(const Controller &) = delete;
    Controller &operator=(Controller &&) = delete;

private:
    QString getObjectDescription(const Object *object) const;
    QString getDescriptionPrefixWithStats(const QModelIndex &index, const Object *object, ModelItem::Type type) const;
    bool isStatusEquivalent();
    void setButtonsStatus(bool status);
    void showWaitingDialog();
    void setFilter(ComponentFilterProxyModel::FilterOptions option, bool value);

    bool checkDate(std::optional<QDate> backendResult, int interval, const QString &warningMessage);

    void updateViewMode();

    QBrush initMixedStateBackgroundBrush();
    int countEditionComponents() const;

private:
    std::shared_ptr<ILogger> logger;
    std::shared_ptr<DBusManager> dbus;
    std::shared_ptr<PackageRepository> packageRepository;
    std::shared_ptr<ComponentRepository> componentRepository;
    std::shared_ptr<EditionRepository> editionRepository;
    std::shared_ptr<TransactionService> transactionService;
    std::unique_ptr<Model> model;
    std::unique_ptr<ComponentFilterProxyModel> filterProxyModel;
    std::unique_ptr<TransactionFilterProxyModel> transactionProxyModel;
    std::unique_ptr<ProxyModel> proxyModel;
    std::unique_ptr<EnsembleProxyModel> ensemble;
    mutable std::unique_ptr<QStandardItemModel> packagesListModel;
    std::unique_ptr<alt::MainWindow> mainWindow;
    std::unique_ptr<TransactionWizard> transactionWizard;
    std::unique_ptr<UpdatingSourcesWizard> updatingWizard;
    std::unique_ptr<ErrorDialog> errorDialog;
    MainWindow::ViewMode viewMode = MainWindow::ViewMode::Undefined;
    MainWindow::NameViewMode nameViewMode = MainWindow::NameViewMode::NamesOnly;
};

} // namespace alt

#endif // CONTROLLER_H
