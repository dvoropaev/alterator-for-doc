#include "mainwindow.h"
#include "application.h"
#include "controller/controller.h"
#include "model/item.h"
#include "model/objects/component.h"
#include "ui/aboutdialog/aboutdialog.h"
#include "ui/componentswidget/componentswidget.h"
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
MainWindow::MainWindow(Model *model, ProxyModel *proxyModel, QWidget *parent)
    : QMainWindow(parent)
    , ui(std::make_unique<Ui::MainWindow>())
    , settings(std::make_unique<MainWindowSettings>(this))
    , model(model)
    , proxyModel(proxyModel)
{
    this->ui->setupUi(this);
    this->settings->restoreSettings();
    this->setWindowIcon(QIcon(":logo.png"));

    this->ui->componentsWidget->setComponentsModel(this->proxyModel);

    connect(this->model, &Model::itemChanged, this, &MainWindow::onItemChanged);
    connect(this->ui->languageMenu, &QMenu::triggered, this, &MainWindow::onLanguageChanged);

    initializeLanguageMenu();
    initializeViewModeMenu();
}

MainWindow::~MainWindow() = default;

MainStatusBar *MainWindow::getStatusBar()
{
    return this->ui->statusbar;
}

ComponentsWidget *MainWindow::getComponentWidget()
{
    return this->ui->componentsWidget;
}

QString MainWindow::languageToString(QLocale::Language language)
{
    switch (language)
    {
    case QLocale::English:
        return tr("English");
    case QLocale::Russian:
        return tr("Russian");
    default:
        return {};
    }
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
        languageAction->setText(QString("&%1").arg(languageToString(language)));
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

void MainWindow::setDescription(const QModelIndex &proxyIndex,
                                alt::ModelItem::Type type,
                                const QString &displayName,
                                const QString &description)
{
    QString quotedDisplayName;
    if (alt::Application::getLocale().language() == QLocale::Russian)
    {
        quotedDisplayName = QString::fromUtf8("«%1»").arg(displayName);
    }
    else
    {
        quotedDisplayName = QString("\"%1\"").arg(displayName);
    }

    QString prefix = QString("<h2>%1 %2</h2>\n").arg(alt::ModelItem::typeToString(type), quotedDisplayName);

    if (type == alt::ModelItem::Type::Category || type == alt::ModelItem::Type::Section
        || type == alt::ModelItem::Type::Tag)
    {
        const auto index = this->proxyModel->mapToSource(proxyIndex);
        const auto componentsCount = this->model->countComponents(this->model->itemFromIndex(index));

        prefix += QString("<ul>\n  <li>%1: %2</li>\n  <li>%3: %4</li>\n</ul>")
                      .arg(tr("Total components"))
                      .arg(componentsCount.total)
                      .arg(tr("Installed components"))
                      .arg(componentsCount.installed);
    }
    else if (type == alt::ModelItem::Type::Component)
    {
        const auto *component = proxyIndex.data(alt::ObjectRole).value<Component *>();
        if (component && !component->tags.empty() && alt::Model::current_edition)
        {
            QStringList tagsNames;
            for (const auto &tag : alt::Model::current_edition->tags)
            {
                if (component->tags.contains(tag.name))
                {
                    const QString tagHtml = QString("<span class=\"tag\">%1</span>").arg(tag.displayName);
                    tagsNames.append(tagHtml);
                }
            }

            prefix += QString("<ul>\n  <li>%1: %2</li>\n</ul>").arg(tr("Tags")).arg(tagsNames.join(", "));
        }
    }

    this->ui->componentsWidget->setDescription(prefix + description);
}

void MainWindow::setDescription(const QString &description)
{
    this->ui->componentsWidget->setDescription(description);
}

void MainWindow::setContentList(const std::vector<std::shared_ptr<Package>> &packages)
{
    this->ui->componentsWidget->setContentList(packages);
}

void MainWindow::setContentList(QAbstractItemModel *model, const QModelIndex &index)
{
    this->ui->componentsWidget->setContentList(model, index);
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

void MainWindow::showActionOtherComponents(bool show)
{
    this->ui->actionOther->setVisible(show);
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

void MainWindow::onItemChanged(QStandardItem *item)
{
    Controller::instance().itemChanged(dynamic_cast<ModelItem *>(item));
}

void MainWindow::on_action_Show_logs_triggered()
{
    Controller::instance().showWarnings();
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
    if (Controller::instance().getViewMode() == MainWindow::ViewMode::BySections)
    {
        showActionOtherComponents(true);
        Controller::instance().setFilterNonEditionComponents(this->ui->actionOther->isChecked());
    }
    else
    {
        showActionOtherComponents(false);
        Controller::instance().setFilterNonEditionComponents(true);
    }
}

void MainWindow::on_actionShow_drafts_toggled(bool isChecked)
{
    Controller::instance().showDrafts(isChecked);
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

void MainWindow::on_actionOther_toggled(bool isChecked)
{
    Controller::instance().setFilterNonEditionComponents(isChecked);
}

void MainWindow::on_actionBySections_triggered()
{
    this->showActionOtherComponents(true);
    Controller::instance().setViewMode(ViewMode::BySections);
    Controller::instance().setFilterNonEditionComponents(this->ui->actionOther->isChecked());
}

void MainWindow::on_actionPlain_triggered()
{
    this->showActionOtherComponents(false);
    Controller::instance().setViewMode(ViewMode::Plain);
    Controller::instance().setFilterNonEditionComponents(true);
}

void MainWindow::on_actionByTags_triggered()
{
    this->showActionOtherComponents(false);
    Controller::instance().setViewMode(ViewMode::ByTags);
    Controller::instance().setFilterNonEditionComponents(true);
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

        for (auto *languageAction : this->ui->languageMenu->actions())
        {
            const auto &locale = languageAction->data().toLocale();
            languageAction->setText(languageToString(locale.language()));
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
