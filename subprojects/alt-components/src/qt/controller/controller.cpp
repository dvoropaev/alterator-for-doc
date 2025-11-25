#include "controller.h"
#include "../application/application.h"
#include "../model/item.h"
#include "../model/model.h"
#include "../ui/applydialog/updatingsourceswizard.h"
#include "../ui/componentswidget/componentswidget.h"
#include "../ui/mainwindow/mainwindow.h"
#include "../ui/statusbar/mainstatusbar.h"
#include "dbus/dbusmanager.h"
#include "repository/componentrepository.h"
#include "repository/editionrepository.h"
#include "repository/packagerepository.h"
#include "service/transaction.h"
#include "service/transactionservice.h"
#include <memory>

#include <QColor>
#include <QDebug>
#include <QFontDatabase>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>

namespace alt
{
const size_t COMPONENT_FILTER_WEIGHT = 10;
const size_t TRANSACTION_FILTER_WEIGHT = 50;
const size_t SORT_FILTER_WEIGHT = 100;

Controller::Controller()
    : dbus(std::make_shared<DBusManager>())
    , packageRepository(std::make_shared<PackageRepository>(dbus))
    , componentRepository(std::make_shared<ComponentRepository>(dbus, packageRepository))
    , editionRepository(std::make_shared<EditionRepository>(dbus))
    , transactionService(std::make_shared<TransactionService>(packageRepository, componentRepository, editionRepository))
    , model(std::make_unique<Model>(componentRepository, editionRepository))
    , filterProxyModel(std::make_unique<ComponentFilterProxyModel>(editionRepository))
    , transactionProxyModel(std::make_unique<TransactionFilterProxyModel>(transactionService, editionRepository))
    , proxyModel(std::make_unique<ProxyModel>())
    , ensemble(std::make_unique<EnsembleProxyModel>())
    , packagesListModel(std::make_unique<QStandardItemModel>())
{
    dbus->setLocale(QString("%1.UTF-8").arg(Application::getLocale().name()).toStdString());

    filterProxyModel->setFilter(ComponentFilterProxyModel::FilterOptions::Draft, true);
    filterProxyModel->setFilter(ComponentFilterProxyModel::FilterOptions::NonEdition, true);

    ensemble->setRootModel(model.get());
    ensemble->insert(COMPONENT_FILTER_WEIGHT, filterProxyModel.get());
    ensemble->insert(TRANSACTION_FILTER_WEIGHT, transactionProxyModel.get());
    ensemble->insert(SORT_FILTER_WEIGHT, proxyModel.get());

    mainWindow = std::make_unique<MainWindow>(this->ensemble.get());
    transactionWizard = std::make_unique<TransactionWizard>(transactionService,
                                                            componentRepository,
                                                            this->mainWindow.get());
    updatingWizard = std::make_unique<UpdatingSourcesWizard>(this->mainWindow.get());
}

Controller::~Controller() = default;

Controller &Controller::instance()
{
    // NOTE(sheriffkorov): one memory leak but in the end of process.
    // If you know how do it better, do it, please.
    static Controller *controller = new Controller();
    return *controller;
}

void Controller::setLogger(const std::shared_ptr<ILogger> &logger)
{
    this->logger = logger;
    dbus->setLogger(logger);
    mainWindow->setLogger(logger);
    componentRepository->setLogger(logger);
    editionRepository->setLogger(logger);
}

void Controller::init()
{
    rebuildModel();

    mainWindow->showActionNonEditionComponents(editionRepository->current().has_value());

    this->setButtonsStatus(false);
    this->mainWindow->show();

    auto result = packageRepository->lastUpdateOfSources().map(
        +[](std::chrono::system_clock::time_point timestamp) -> QDate {
            return QDateTime::fromMSecsSinceEpoch(timestamp.time_since_epoch().count(), QTimeZone::UTC).date();
        });
    auto lastUpdate = result ? std::make_optional(result.value()) : std::nullopt;

    if (checkDate(lastUpdate, 0, tr("It seems your apt package lists weren't updated since %1")))
    {
        auto reply = QMessageBox::warning(
            QApplication::activeWindow(),
            tr("Warning"),
            tr("The system's list of software sources has not been updated%1.\n"
               "This may cause errors in the application..\n\nDo you want to update your sources?")
                .arg(lastUpdate.has_value() ? tr(" since %1").arg(Application::getLocale().toString(lastUpdate.value()))
                                            : ""),
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            updateSources();
        }
    }

    result = packageRepository->lastUpdateOfSystem().map(+[](std::chrono::system_clock::time_point timestamp) -> QDate {
        return QDateTime::fromMSecsSinceEpoch(timestamp.time_since_epoch().count(), QTimeZone::UTC).date();
    });
    auto lastUpgrade = result ? std::make_optional(result.value()) : std::nullopt;
    checkDate(lastUpgrade, 7, tr("It seems your system wasn't upgraded since %1"));
}

void Controller::rebuildModel()
{
    auto *mainStatusBar = mainWindow->getStatusBar();
    mainStatusBar->resetProgressBarValue();
    mainStatusBar->onStarted();

    transactionService->create();
    editionRepository->update();

    // Temporarily disconnect the proxy model
    ensemble->setRootModel(nullptr);

    updateViewMode();

    if (viewMode == MainWindow::ViewMode::BySections)
    {
        model->buildBySections();
    }
    else if (viewMode == MainWindow::ViewMode::ByTags)
    {
        model->buildByTags();
    }
    else
    {
        model->buildPlain();
    }

    // Reconnect and reset the proxy model
    ensemble->setRootModel(model.get());
    filterProxyModel->invalidate();

    const int totalCount = filterProxyModel->countComponents().total;
    const int editionCount = countEditionComponents();
    const auto currentEdition = editionRepository->current();
    mainStatusBar->onDone(currentEdition ? &currentEdition.value().get() : nullptr, totalCount, editionCount);

    if (!currentEdition)
    {
        mainWindow->setViewModeByTagsActionEnabled(false);
        mainWindow->setViewModeBySectionsActionEnabled(false);
        mainWindow->showDisableRemoveBaseComponent(false);
    }
    else
    {
        mainWindow->setViewModeByTagsActionEnabled(!currentEdition->get().tags.empty());
        mainWindow->setViewModeBySectionsActionEnabled(true);
        mainWindow->showDisableRemoveBaseComponent(true);
        transactionProxyModel->setDisabledBaseComponents(
            transactionService->safeMode().test(TransactionService::SafeMode::Base));
    }

    mainWindow->setViewModeActionStatus(viewMode);

    if (viewMode != MainWindow::ViewMode::Plain)
    {
        mainWindow->getComponentWidget()->expandTopLevel();
    }
}

void Controller::updateViewMode()
{
    const auto currentEdition = editionRepository->current();
    if (viewMode == MainWindow::ViewMode::Undefined)
    {
        if (currentEdition)
        {
            viewMode = MainWindow::ViewMode::BySections;
        }
        else
        {
            viewMode = MainWindow::ViewMode::Plain;
        }
    }
    else if (!currentEdition)
    {
        if (viewMode == MainWindow::ViewMode::ByTags || viewMode == MainWindow::ViewMode::BySections)
        {
            viewMode = MainWindow::ViewMode::Plain;
        }
    }
    else if (viewMode == MainWindow::ViewMode::ByTags && currentEdition->get().tags.empty())
    {
        viewMode = MainWindow::ViewMode::BySections;
    }
}

QString Controller::getDescription(const QModelIndex &index) const
{
    const auto *object = index.data(CustomRoles::ObjectRole).value<Object *>();
    const auto type = index.data(CustomRoles::TypeRole).value<ModelItem::Type>();
    return getDescriptionPrefixWithStats(index, object, type) + getObjectDescription(object);
}

QString Controller::getObjectDescription(const Object *object) const
{
    if (const auto *component = dynamic_cast<const Component *>(object))
    {
        return QString::fromStdString(componentRepository->getDescription(*component).value_or(""));
    }

    return {};
}

std::pair<ModelItem::Type, QAbstractItemModel *> Controller::getContent(const QModelIndex &index) const
{
    if (const auto *component = index.data(CustomRoles::ObjectRole).value<Component *>())
    {
        this->packagesListModel->clear();
        for (const auto &package : component->packages)
        {
            auto *item = new QStandardItem(package.getPackageName().data());
            item->setCheckState(package.installed ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
            item->setCheckable(false);
            item->setEnabled(false);
            item->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
            this->packagesListModel->insertRow(this->packagesListModel->rowCount(), item);
        }
        return std::make_pair(ModelItem::Type::Component, packagesListModel.get());
    }
    return std::make_pair(ModelItem::Type::Category, ensemble.get());
}

QString Controller::getDescriptionPrefixWithStats(const QModelIndex &index,
                                                  const Object *object,
                                                  ModelItem::Type type) const
{
    auto lang = QLocale::languageToCode(Application::getLocale().language()).toStdString();
    QString quotedDisplayName = Application::getLocale().quoteString(
        object != nullptr ? object->displayName(lang).data() : "");
    QString prefix = QString("<h2>%1 %2</h2>\n").arg(ModelItem::typeToString(type), quotedDisplayName);
    const auto currentEdition = editionRepository->current();

    if (type == ModelItem::Type::Component && currentEdition)
    {
        const auto *component = dynamic_cast<const Component *>(object);
        if (component != nullptr && !component->tags.empty())
        {
            QStringList tagsNames;
            for (const auto &tag : currentEdition->get().tags)
            {
                if (component->tags.contains(tag.name))
                {
                    const QString tagHtml = QString("<span class=\"tag\">%1</span>").arg(tag.displayName(lang));
                    tagsNames.append(tagHtml);
                }
            }

            prefix += QString("<ul>\n  <li>%1: %2</li>\n</ul>").arg(tr("Tags")).arg(tagsNames.join(", "));
        }
    }
    else if (type != ModelItem::Type::Component)
    {
        const auto filterIndex = ensemble->mapToMember(index, filterProxyModel.get());
        const auto componentsCount = this->filterProxyModel->countComponents(filterIndex);
        prefix += QString("<ul>\n  <li>%1: %2</li>\n  <li>%3: %4</li>\n</ul>")
                      .arg(tr("Total components"))
                      .arg(componentsCount.total)
                      .arg(tr("Installed components"))
                      .arg(componentsCount.installed);
    }

    return prefix;
}

void Controller::updateCurrentTransaction(const QModelIndex &index)
{
    if (!index.isValid())
    {
        return;
    }

    const auto *object = index.data(CustomRoles::ObjectRole).value<Component *>();
    if (object == nullptr)
    {
        return;
    }

    auto component = componentRepository->get<Component>(object->name);
    if (component.has_value())
    {
        auto systemState = component->get().state;
        auto checkState = index.data(Qt::CheckStateRole).value<Qt::CheckState>();
        if (systemState != static_cast<ComponentState>(checkState))
        {
            transactionService->add(transactionService->current(), component.value());
        }
        else
        {
            transactionService->discard(transactionService->current(), component.value());
        }

        setButtonsStatus(!isStatusEquivalent());
    }
}

void Controller::apply()
{
    this->transactionWizard->setPage(0);
    if (this->transactionWizard->exec() == QDialog::Accepted)
    {
        rebuildModel();
        setButtonsStatus(false);
    }
}

void Controller::reset()
{
    this->transactionProxyModel->reset();
    transactionService->create();
    setButtonsStatus(false);
}

tl::expected<void, PackageRepository::Error> Controller::update(const std::function<void(const std::string &)> &callback)
{
    return packageRepository->updateSources(callback);
}

void Controller::updateSources()
{
    this->updatingWizard->setPage(0);
    if (this->updatingWizard->exec() == QDialog::Accepted)
    {
        return;
    }
}

bool Controller::isStatusEquivalent()
{
    return transactionService->current().components().empty();
}

void Controller::setButtonsStatus(bool status)
{
    this->mainWindow->setEnabledApplyButton(status);
    this->mainWindow->setEnabledResetButton(status);
}

void Controller::changeLocale(const QLocale &locale)
{
    dbus->setLocale(QString("%1.UTF-8").arg(Application::getLocale().name()).toStdString());
    this->proxyModel->invalidate();
}

bool Controller::checkDate(std::optional<QDate> backendResult, int interval, const QString &warningMessage)
{
    if (!backendResult.has_value())
    {
        return false;
    }
    auto updateDate = backendResult.value();
    if (updateDate.daysTo(QDate::currentDate()) > interval)
    {
        if (logger)
        {
            logger->write(ILogger::Level::Warning,
                          warningMessage.arg(updateDate.toString("dd MMMM yyyy")).toStdString());
        }
        return true;
    }

    return false;
}

MainWindow::ViewMode Controller::getViewMode() const
{
    return viewMode;
}

void Controller::setFilterDrafts(bool showDrafts)
{
    setFilter(ComponentFilterProxyModel::FilterOptions::Draft, !showDrafts);
}

void Controller::setFilterNonEditionComponents(bool show)
{
    setFilter(ComponentFilterProxyModel::FilterOptions::NonEdition, !show);
    if (viewMode != MainWindow::ViewMode::Plain)
    {
        mainWindow->getComponentWidget()->expandTopLevel();
    }
}

void Controller::setFilter(ComponentFilterProxyModel::FilterOptions option, bool value)
{
    filterProxyModel->setFilter(option, value);
    filterProxyModel->invalidate();
    setButtonsStatus(!isStatusEquivalent());

    const int totalCount = filterProxyModel->countComponents().total;
    const int editionCount = countEditionComponents();
    const auto currentEdition = editionRepository->current();
    mainWindow->getStatusBar()->onDone(currentEdition ? &currentEdition.value().get() : nullptr,
                                       totalCount,
                                       editionCount);
}

void Controller::showContent(bool showContent)
{
    mainWindow->getComponentWidget()->ComponentsWidget::setVisibleContent(showContent);
}

void Controller::setSafeModeForBaseComponents(bool value)
{
    auto target = TransactionService::SafeMode::Base;
    auto newMode = transactionService->safeMode();
    value ? newMode.set(target) : newMode.reset(target);
    if (transactionService->safeMode() == newMode)
    {
        return;
    }

    transactionService->setSafeMode(newMode);
    if (editionRepository->current())
    {
        transactionProxyModel->setDisabledBaseComponents(
            transactionService->safeMode().test(TransactionService::SafeMode::Base));
    }
    setButtonsStatus(!isStatusEquivalent());
}

bool Controller::setSafeMode(bool value)
{
    auto target = TransactionService::SafeMode::Manually;
    auto newMode = transactionService->safeMode();
    value ? newMode.set(target) : newMode.reset(target);
    if (transactionService->safeMode() == newMode)
    {
        return true;
    }

    if (!value)
    {
        auto reply = QMessageBox::warning(QApplication::activeWindow(),
                                          tr("Confirmation"),
                                          tr("Disabling this option may lead to the removal of packages required for "
                                             "proper system operation.\n\nConfirm the action?"),
                                          QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::No)
        {
            return false;
        }
    }

    transactionService->setSafeMode(newMode);
    return true;
}

void Controller::setViewMode(MainWindow::ViewMode viewMode)
{
    this->viewMode = viewMode;
    if (viewMode == MainWindow::ViewMode::BySections)
    {
        model->buildBySections(false);
        mainWindow->getComponentWidget()->ComponentsWidget::expandTopLevel();
    }
    else if (viewMode == MainWindow::ViewMode::ByTags)
    {
        model->buildByTags(false);
        mainWindow->getComponentWidget()->ComponentsWidget::expandTopLevel();
    }
    else if (viewMode == MainWindow::ViewMode::Plain)
    {
        model->buildPlain(false);
    }

    if (editionRepository->current())
    {
        transactionProxyModel->setDisabledBaseComponents(
            transactionService->safeMode().test(TransactionService::SafeMode::Base));
    }

    reset();
}

void Controller::setTextFilter(const QString &query)
{
    if (!this->proxyModel)
    {
        return;
    }

    this->proxyModel->setFilterFixedString(query);
    if (viewMode == MainWindow::ViewMode::BySections || viewMode == MainWindow::ViewMode::ByTags)
    {
        mainWindow->getComponentWidget()->expandTopLevel();
    }
}

void Controller::setNameViewMode(MainWindow::NameViewMode viewMode)
{
    this->nameViewMode = viewMode;
    if (viewMode == MainWindow::NameViewMode::NamesAndIDs)
    {
        this->model->setTextMode(Model::TextMode::NamesAndIDs);
    }
    else if (viewMode == MainWindow::NameViewMode::NamesOnly)
    {
        this->model->setTextMode(Model::TextMode::NamesOnly);
    }
    else if (viewMode == MainWindow::NameViewMode::IDsOnly)
    {
        this->model->setTextMode(Model::TextMode::IDsOnly);
    }
    this->proxyModel->invalidate();
}

void Controller::showError(int code, const QString &text)
{
    transactionWizard->reject();
    errorDialog = std::make_unique<ErrorDialog>(this->mainWindow.get());
    errorDialog->showError(code, text);
}

int Controller::countEditionComponents() const
{
    auto currentEdition = editionRepository->current();
    return currentEdition
               ? std::accumulate(currentEdition->get().sections.begin(),
                                 currentEdition->get().sections.end(),
                                 0,
                                 [](int acc, const Section &section) { return acc + section.components.size(); })
               : 0;
}

} // namespace alt
