#include "mainwindow.h"

#include "../../application/application.h"
#include "../../application/applicationlogger.h"
#include "../../controller/controller.h"
#include "../aboutdialog/aboutdialog.h"
#include "../componentswidget/componentswidget.h"
#include "../warnings/warningsdialog.h"
#include "ui_mainwindow.h"

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>
#include <QDesktopServices>
#include <QMessageBox>
#include <QMouseEvent>
#include <QtGui/QtGui>
#include <qapplication.h>

namespace alt
{
MainWindow::MainWindow(QAbstractItemModel *model, QWidget *parent)
    : QMainWindow(parent)
    , ui(std::make_unique<Ui::MainWindow>())
    , settings(std::make_unique<MainWindowSettings>(this))
    , model(model)
{
    this->ui->setupUi(this);
    this->settings->restoreSettings();
    this->setWindowIcon(QIcon(":logo.svg"));

    this->ui->componentsWidget->setComponentsModel(this->model);

    connect(this->model, &QAbstractItemModel::dataChanged, this, &MainWindow::onDataChanged);
    connect(this->ui->languageMenu, &QMenu::triggered, this, &MainWindow::onLanguageChanged);

    initializeLanguageMenu();
    initializeViewModeMenu();
}

MainWindow::~MainWindow() = default;

void MainWindow::setLogger(const std::shared_ptr<ILogger> &logger)
{
    this->logger = logger;
}

MainStatusBar *MainWindow::getStatusBar()
{
    return this->ui->statusbar;
}

ComponentsWidget *MainWindow::getComponentWidget()
{
    return this->ui->componentsWidget;
}

void MainWindow::initializeLanguageMenu()
{
    auto *languageGroup = new QActionGroup(this);
    languageGroup->setExclusive(true);

    connect(languageGroup, &QActionGroup::triggered, this, &MainWindow::onLanguageChanged);

    const auto &languages = {QLocale::English, QLocale::Russian};

    for (const auto &language : languages)
    {
        auto languageAction = std::make_unique<QAction>();
        languageAction->setCheckable(true);
        languageAction->setText(QString("&%1").arg(QLocale(language, QLocale::AnyCountry).nativeLanguageName()));
        QLocale localeData(language);
        languageAction->setData(localeData);

        if (localeData.language() == Application::getLocale().language())
        {
            languageAction->setChecked(true);
        }

        languageGroup->addAction(languageAction.get());
        ui->languageMenu->addAction(languageAction.release());
    }
}

void MainWindow::initializeViewModeMenu()
{
    auto *viewModeGroup = new QActionGroup(this);
    viewModeGroup->setExclusive(true);

    this->ui->actionBySections->setActionGroup(viewModeGroup);
    this->ui->actionByTags->setActionGroup(viewModeGroup);
    this->ui->actionPlain->setActionGroup(viewModeGroup);

    this->ui->actionBySections->setChecked(true);

    auto *viewNameModeGroup = new QActionGroup(this);
    viewNameModeGroup->setExclusive(true);
    this->ui->actionNames_and_IDs->setActionGroup(viewNameModeGroup);
    this->ui->actionNames_only->setActionGroup(viewNameModeGroup);
    this->ui->actionIDs_only->setActionGroup(viewNameModeGroup);

    this->ui->actionNames_only->setChecked(true);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    this->settings->saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::setEnabledApplyButton(bool isEnabled)
{
    this->ui->applyPushButton->setEnabled(isEnabled);
}

void MainWindow::setEnabledResetButton(bool isEnabled)
{
    this->ui->resetPushButton->setEnabled(isEnabled);
}

void MainWindow::setViewModeActionStatus(ViewMode viewMode)
{
    this->ui->actionBySections->setChecked(viewMode == ViewMode::BySections);
    this->ui->actionByTags->setChecked(viewMode == ViewMode::ByTags);
    this->ui->actionPlain->setChecked(viewMode == ViewMode::Plain);
}

void MainWindow::setViewModeByTagsActionEnabled(bool enabled)
{
    this->ui->actionByTags->setEnabled(enabled);
}

void MainWindow::setViewModeBySectionsActionEnabled(bool enabled)
{
    this->ui->actionBySections->setEnabled(enabled);
}

void MainWindow::showDisableRemoveBaseComponent(bool show)
{
    this->ui->actionDisable_deletion_base_components->setVisible(show);
}

void MainWindow::showActionNonEditionComponents(bool show)
{
    this->ui->actionShowNonEditionComponents->setVisible(show);
}

void MainWindow::on_resetPushButton_clicked()
{
    Controller::instance().reset();
}

void MainWindow::on_applyPushButton_clicked()
{
    Controller::instance().apply();
}

void MainWindow::on_updatePushButton_clicked()
{
    Controller::instance().updateSources();
}

void MainWindow::onDataChanged(const QModelIndex &topLeft, const QModelIndex &, const QList<int> &)
{
    Controller::instance().updateCurrentTransaction(topLeft);
}

void MainWindow::on_action_Show_logs_triggered()
{
    if (!logger)
    {
        return;
    }
    const auto qtLogger = std::dynamic_pointer_cast<ApplicationLogger>(logger);
    if (!qtLogger)
    {
        return;
    }
    auto *warningsModel = dynamic_cast<QStandardItemModel *>(qtLogger->model());
    auto *warningsDialog = new WarningsDialog(warningsModel, this);
    connect(warningsDialog, &WarningsDialog::messagesRemoved, this, [warningsModel] { warningsModel->clear(); });
    warningsDialog->show();
}

void MainWindow::on_manualAction_triggered()
{
    const QUrl manualUrl("https://www.altlinux.org/AMC");
    QDesktopServices::openUrl(manualUrl);
}

void MainWindow::on_aboutAction_triggered()
{
    auto about = std::make_unique<AboutDialog>(this);
    about->exec();
}

void MainWindow::on_actionReload_components_triggered()
{
    Controller::instance().rebuildModel();
}

void MainWindow::on_actionShow_drafts_toggled(bool isChecked)
{
    Controller::instance().setFilterDrafts(isChecked);
    if (auto *widget = getComponentWidget())
    {
        widget->updateDescription();
    }
}

void MainWindow::on_actionDisable_deletion_base_components_toggled(bool isChecked)
{
    Controller::instance().setSafeModeForBaseComponents(isChecked);
}

void MainWindow::on_actionDisable_the_removal_of_packages_installed_manually_toggled(bool isChecked)
{
    bool accepted = Controller::instance().setSafeMode(isChecked);
    if (!accepted)
    {
        this->ui->actionDisable_the_removal_of_packages_installed_manually->blockSignals(true);
        this->ui->actionDisable_the_removal_of_packages_installed_manually->setChecked(!isChecked);
        this->ui->actionDisable_the_removal_of_packages_installed_manually->blockSignals(false);
    }
}

void MainWindow::on_actionContent_toggled(bool isChecked)
{
    Controller::instance().showContent(isChecked);
}

void MainWindow::on_actionShowNonEditionComponents_toggled(bool isChecked)
{
    Controller::instance().setFilterNonEditionComponents(isChecked);
    if (auto *widget = getComponentWidget())
    {
        widget->updateDescription();
    }
}

void MainWindow::on_actionBySections_triggered()
{
    Controller::instance().setViewMode(ViewMode::BySections);
}

void MainWindow::on_actionPlain_triggered()
{
    Controller::instance().setViewMode(ViewMode::Plain);
}

void MainWindow::on_actionByTags_triggered()
{
    Controller::instance().setViewMode(ViewMode::ByTags);
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionNames_and_IDs_triggered()
{
    Controller::instance().setNameViewMode(NameViewMode::NamesAndIDs);
}

void MainWindow::on_actionNames_only_triggered()
{
    Controller::instance().setNameViewMode(NameViewMode::NamesOnly);
}

void MainWindow::on_actionIDs_only_triggered()
{
    Controller::instance().setNameViewMode(NameViewMode::IDsOnly);
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        this->ui->retranslateUi(this);

        if (const auto qtLogger = std::dynamic_pointer_cast<ApplicationLogger>(logger))
        {
            qtLogger->retranslate();
        }
    }

    QWidget::changeEvent(event);
}

void MainWindow::onLanguageChanged(QAction *action)
{
    const QLocale &locale = action->data().toLocale();
    Application::setLocale(locale);
}

void MainWindow::showInfo(const QString &message)
{
    auto *msgBox = new QMessageBox(QMessageBox::Information, tr("Components"), message, QMessageBox::Ok);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setModal(false);
    msgBox->show();
}

void MainWindow::showWarning(const QString &message)
{
    auto *msgBox = new QMessageBox(QMessageBox::Warning, tr("Components"), message, QMessageBox::Ok);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setModal(false);
    msgBox->show();
}

void MainWindow::showProblem(const QString &message)
{
    auto *msgBox = new QMessageBox(QMessageBox::Critical, tr("Components"), message, QMessageBox::NoButton, this);
    auto *quitButton = new QPushButton(tr("Quit"), msgBox);
    msgBox->addButton(quitButton, QMessageBox::RejectRole);
    msgBox->addButton(QMessageBox::Cancel);
    connect(quitButton, &QPushButton::clicked, qApp, &QApplication::quit);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->open();
}
} // namespace alt
