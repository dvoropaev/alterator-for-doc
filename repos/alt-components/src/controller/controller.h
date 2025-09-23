#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "model/item.h"
#include "model/model.h"
#include "model/modelbuilder.h"
#include "model/objects/component.h"
#include "model/proxymodel.h"
#include "service/transactionservice.h"
#include "ui/applydialog/transactionwizard.h"
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

    void init();
    void selectObject(QModelIndex index);
    void itemChanged(ModelItem *item);
    void changeLocale(const QLocale &locale);

    void apply();
    void reset();

    void showError(int code, const QString &text);
    void showWarnings();
    void showDrafts(bool showDrafts);
    void showContent(bool showContent);
    void setViewMode(MainWindow::ViewMode viewMode);
    void setNameViewMode(MainWindow::NameViewMode viewMode);
    void setTextFilter(const QString &query);
    void issueMessage(const QtMsgType &level, const QString &message);
    void setSafeModeForBaseComponents(bool safeMode);
    bool setSafeMode(bool safeMode);
    MainWindow::ViewMode getViewMode() const;

    void rebuildModel();

signals:
    void packagesReady(const std::set<QString> &toInstall,
                       const std::set<QString> &toRemove,
                       const std::set<QString> &toExtraRemove,
                       const std::map<QString, QList<QString>> &toRemoveBasePackagesWithComponents);

public:
    Controller(const Controller &) = delete;
    Controller(Controller &&) = delete;
    Controller &operator=(const Controller &) = delete;
    Controller &operator=(Controller &&) = delete;

private:
    bool isStatusEquivalent();
    void setButtonsStatus(bool status);
    void showWaitingDialog();

    void checkDate(std::optional<QDate> backendResult, int interval, const QString &warningMessage);

    void updateViewMode();

    void issueLog(const QList<QStandardItem *> &items);
    void updateParentCategoryBackground(ModelItem *parentItem);

    QString toLocalizedString(const QtMsgType &level);
    void retranslateWarningsModel();

    void setEnableBaseComponents(bool isEnable);
    void resetBaseComponentState(QStandardItem *parent,
                                 const QSet<QString> &baseComponents,
                                 QList<ModelItem *> &installedBaseComponents);

    QBrush initMixedStateBackgroundBrush();

private:
    std::unique_ptr<Model> model;
    std::unique_ptr<ProxyModel> proxyModel;
    std::unique_ptr<ModelBuilder> modelBuilder;
    std::unique_ptr<alt::MainWindow> mainWindow;
    std::unique_ptr<TransactionWizard> transactionWizard;
    std::unique_ptr<ErrorDialog> errorDialog;
    std::unique_ptr<QStandardItemModel> warningsModel;
    MainWindow::ViewMode viewMode = MainWindow::ViewMode::Undefined;
    MainWindow::NameViewMode nameViewMode = MainWindow::NameViewMode::NamesOnly;
};

} // namespace alt

#endif // CONTROLLER_H
