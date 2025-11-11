#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <tl/expected.hpp>

#include "model/componentfilterproxymodel.h"
#include "model/ensembleproxymodel.h"
#include "model/item.h"
#include "model/model.h"
#include "model/modelbuilder.h"
#include "model/proxymodel.h"
#include "model/transactionfilterproxymodel.h"
#include "ui/applydialog/transactionwizard.h"
#include "ui/applydialog/updatingsourceswizard.h"
#include "ui/errordialog/errordialog.h"
#include "ui/mainwindow/mainwindow.h"

namespace alt
{
class ControllerPrivate;
class MainWindow;

class Controller : public QObject
{
    Q_OBJECT
public:
    Controller();
    ~Controller() override;

public:
    static Controller &instance();

    void init();
    void changeLocale(const QLocale &locale);

    QString getDescription(const QModelIndex &index) const;
    std::pair<ModelItem::Type, QAbstractItemModel *> getContent(const QModelIndex &index) const;

    void apply();
    void reset();
    tl::expected<void, DBusError> update();
    void updateSources();
    void updateCurrentTransaction(const QModelIndex &index);

    void showError(int code, const QString &text);
    void showWarnings();
    void showContent(bool showContent);
    void setFilterDrafts(bool showDrafts);
    void setFilterNonEditionComponents(bool show);
    void setViewMode(MainWindow::ViewMode viewMode);
    void setNameViewMode(MainWindow::NameViewMode viewMode);
    void setTextFilter(const QString &query);
    void issueMessage(const QtMsgType &level, const QString &message);
    void setSafeModeForBaseComponents(bool safeMode);
    bool setSafeMode(bool safeMode);
    MainWindow::ViewMode getViewMode() const;

    void rebuildModel();

public slots:
    void onAptUpdateNewLine(const QString &line);

signals:
    void aptUpdateNewLine(const QString &line);

public:
    Controller(const Controller &) = delete;
    Controller(Controller &&) = delete;
    Controller &operator=(const Controller &) = delete;
    Controller &operator=(Controller &&) = delete;

private:
    static QString getObjectDescription(const Object *object);
    QString getDescriptionPrefixWithStats(const QModelIndex &index, const Object *object, ModelItem::Type type) const;
    bool isStatusEquivalent();
    void setButtonsStatus(bool status);
    void showWaitingDialog();
    void setFilter(ComponentFilterProxyModel::FilterOptions option, bool value);

    bool checkDate(std::optional<QDate> backendResult, int interval, const QString &warningMessage);

    void updateViewMode();

    void issueLog(const QList<QStandardItem *> &items);

    QString toLocalizedString(const QtMsgType &level);
    void retranslateWarningsModel();

    QBrush initMixedStateBackgroundBrush();

private:
    std::unique_ptr<Model> model;
    std::unique_ptr<ComponentFilterProxyModel> filterProxyModel;
    std::unique_ptr<TransactionFilterProxyModel> transactionProxyModel;
    std::unique_ptr<ProxyModel> proxyModel;
    std::unique_ptr<EnsembleProxyModel> ensemble;
    std::unique_ptr<ModelBuilder> modelBuilder;
    mutable std::unique_ptr<QStandardItemModel> packagesListModel;
    std::unique_ptr<alt::MainWindow> mainWindow;
    std::unique_ptr<TransactionWizard> transactionWizard;
    std::unique_ptr<UpdatingSourcesWizard> updatingWizard;
    std::unique_ptr<ErrorDialog> errorDialog;
    std::unique_ptr<QStandardItemModel> warningsModel;
    MainWindow::ViewMode viewMode = MainWindow::ViewMode::Undefined;
    MainWindow::NameViewMode nameViewMode = MainWindow::NameViewMode::NamesOnly;
};

} // namespace alt

#endif // CONTROLLER_H
