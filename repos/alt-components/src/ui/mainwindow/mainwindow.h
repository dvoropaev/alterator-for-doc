#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "mainwindowsettings.h"
#include "model/model.h"
#include "model/proxymodel.h"
#include "ui/componentswidget/componentswidget.h"
#include "ui/statusbar/mainstatusbar.h"

#include <QMainWindow>

namespace alt::Ui
{
class MainWindow;
} // namespace alt::Ui

namespace alt
{
class MainWindowSettings;
class Controller;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum class ViewMode
    {
        Undefined,
        BySections,
        ByTags,
        Plain,
    };

    enum class NameViewMode
    {
        NamesAndIDs,
        NamesOnly,
        IDsOnly,
    };

    MainWindow(Controller *controller, Model *model, ProxyModel *proxyModel, QWidget *parent = nullptr);
    ~MainWindow() override;

public:
    void setupTranslator();

    void closeEvent(QCloseEvent *event) override;

    void setDescription(const QModelIndex &index,
                        alt::ModelItem::Type type,
                        const QString &displayName,
                        const QString &description);
    void setDescription(const QString &description);
    void setContentList(const std::vector<std::shared_ptr<Package>> &packages);
    void setContentList(QAbstractItemModel *model, const QModelIndex &index);

    void setEnabledApplyButton(bool isEnabled);
    void setEnabledResetButton(bool isEnabled);

    void setViewModeActionStatus(ViewMode viewMode);
    void setViewModeByTagsActionEnabled(bool enabled);
    void setViewModeBySectionsActionEnabled(bool enabled);

    void showDisableRemoveBaseComponent(bool show);

    MainStatusBar *getStatusBar();
    ComponentsWidget *getComponentWidget();

    static void showInfo(const QString &message);
    static void showWarning(const QString &message);
    void showProblem(const QString &message);

private:
    static QString languageToString(QLocale::Language language);
    void initializeLanguageMenu();
    void initializeViewModeMenu();

public:
    MainWindow(const MainWindow &) = delete;
    MainWindow(MainWindow &&) = delete;
    MainWindow &operator=(const MainWindow &) = delete;
    MainWindow &operator=(MainWindow &&) = delete;

private slots:
    void changeEvent(QEvent *event) override;
    void on_resetPushButton_clicked();
    void on_applyPushButton_clicked();

    void onItemChanged(QStandardItem *item);

    void on_manualAction_triggered();
    void on_aboutAction_triggered();
    void on_action_Show_logs_triggered();
    void on_actionReload_components_triggered();
    void on_actionShow_drafts_toggled(bool);
    void on_actionDisable_deletion_base_components_toggled(bool);
    void on_actionDisable_the_removal_of_packages_installed_manually_toggled(bool);
    void on_actionContent_toggled(bool);
    void on_actionBySections_triggered();
    void on_actionByTags_triggered();
    void on_actionPlain_triggered();
    void on_actionQuit_triggered();
    void on_actionNames_and_IDs_triggered();
    void on_actionNames_only_triggered();
    void on_actionIDs_only_triggered();
    void onLanguageChanged(QAction *action);

private:
    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<MainWindowSettings> settings;

    Controller *controller = nullptr;
    Model *model = nullptr;
    ProxyModel *proxyModel = nullptr;
};
} // namespace alt

#endif // MAIN_WINDOW_H
