#include "controller.h"
#include "application.h"
#include "constants.h"
#include "dbus/dbusproxy.h"
#include "model/componentfilterproxymodel.h"
#include "model/item.h"
#include "model/model.h"
#include "model/modelbuilder.h"
#include "repository/componentrepository.h"
#include "service/transaction.h"
#include "service/transactionservice.h"
#include "ui/applydialog/updatingsourceswizard.h"
#include "ui/componentswidget/componentswidget.h"
#include "ui/mainwindow/mainwindow.h"
#include "ui/statusbar/mainstatusbar.h"
#include "ui/warnings/warningsdialog.h"
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
    : model(std::make_unique<Model>())
    , filterProxyModel(std::make_unique<ComponentFilterProxyModel>())
    , transactionProxyModel(std::make_unique<TransactionFilterProxyModel>())
    , proxyModel(std::make_unique<ProxyModel>())
    , ensemble(std::make_unique<EnsembleProxyModel>())
    , packagesListModel(std::make_unique<QStandardItemModel>())
    , warningsModel(std::make_unique<QStandardItemModel>(0, 2))
{
    DBusProxy::get().resetManagerLocale();
    modelBuilder = std::make_unique<ModelBuilder>();

    filterProxyModel->setFilter(ComponentFilterProxyModel::FilterOptions::Draft, true);
    filterProxyModel->setFilter(ComponentFilterProxyModel::FilterOptions::NonEdition, true);

    ensemble->setRootModel(model.get());
    ensemble->insert(COMPONENT_FILTER_WEIGHT, filterProxyModel.get());
    ensemble->insert(TRANSACTION_FILTER_WEIGHT, transactionProxyModel.get());
    ensemble->insert(SORT_FILTER_WEIGHT, proxyModel.get());

    mainWindow = std::make_unique<MainWindow>(this->ensemble.get());
    transactionWizard = std::make_unique<TransactionWizard>(this->mainWindow.get());
    updatingWizard = std::make_unique<UpdatingSourcesWizard>(this->mainWindow.get());

    auto signalSuffix = DBusProxy::rawConnection().baseService();
    signalSuffix.replace(':', '_');
    signalSuffix.replace('.', '_');

    const auto aptSignals = {
        APT_UPDATE_STDOUT_SIGNAL_NAME,
        APT_UPDATE_STDERR_SIGNAL_NAME,
    };

    for (const auto &signal : aptSignals)
    {
        DBusProxy::rawConnection().connect(ALTERATOR_MANAGER_SERVICE_NAME,
                                           APT1_PATH,
                                           APT1_INTERFACE_NAME,
                                           signal + signalSuffix,
                                           this,
                                           SLOT(onAptUpdateNewLine(const QString &)));
    }

    connect(&DBusProxy::get(), &DBusProxy::errorOccured, this, &Controller::issueMessage);
}

Controller::~Controller() = default;

Controller &Controller::instance()
{
    // NOTE(sheriffkorov): one memory leak but in the end of process.
    // If you know how do it better, do it, please.
    static Controller *controller = new Controller();
    return *controller;
}

void Controller::init()
{
    rebuildModel();

    mainWindow->showActionNonEditionComponents(Model::current_edition != nullptr);

    this->setButtonsStatus(false);
    this->mainWindow->show();

    auto lastUpdate = DBusProxy::get().getDateResult(APT1_LAST_UPDATE_METHOD_NAME);
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

    checkDate(DBusProxy::get().getDateResult(APT1_LAST_UPGRADE_METHOD_NAME),
              7,
              tr("It seems your system wasn't upgraded since %1"));
}

void Controller::rebuildModel()
{
    auto *mainStatusBar = mainWindow->getStatusBar();
    mainStatusBar->resetProgressBarValue();
    mainStatusBar->onStarted();

    TransactionService::create();

    // Temporarily disconnect the proxy model
    ensemble->setRootModel(nullptr);

    Model::current_edition = modelBuilder->buildEdition();

    updateViewMode();

    if (viewMode == MainWindow::ViewMode::BySections)
    {
        modelBuilder->buildBySections(model.get());
    }
    else if (viewMode == MainWindow::ViewMode::ByTags)
    {
        modelBuilder->buildByTags(model.get());
    }
    else
    {
        modelBuilder->buildPlain(model.get());
    }

    if (Model::current_edition != nullptr)
    {
        modelBuilder->setEditionRelationshipForAllComponents();
    }

    // Reconnect and reset the proxy model
    ensemble->setRootModel(model.get());
    filterProxyModel->invalidate();

    const int totalCount = filterProxyModel->countComponents().total;
    const int editionCount = model->countEditionComponents();
    mainStatusBar->onDone(Model::current_edition.get(), totalCount, editionCount);

    if (Model::current_edition == nullptr)
    {
        mainWindow->setViewModeByTagsActionEnabled(false);
        mainWindow->setViewModeBySectionsActionEnabled(false);
        mainWindow->showDisableRemoveBaseComponent(false);
    }
    else
    {
        mainWindow->setViewModeByTagsActionEnabled(!Model::current_edition->tags.empty());
        mainWindow->setViewModeBySectionsActionEnabled(true);
        mainWindow->showDisableRemoveBaseComponent(true);
        transactionProxyModel->setDisabledBaseComponents(
            TransactionService::safeMode().test(TransactionService::SafeMode::Base));
    }

    mainWindow->setViewModeActionStatus(viewMode);

    if (viewMode != MainWindow::ViewMode::Plain)
    {
        mainWindow->getComponentWidget()->expandTopLevel();
    }
}

void Controller::updateViewMode()
{
    if (viewMode == MainWindow::ViewMode::Undefined)
    {
        if (Model::current_edition != nullptr)
        {
            viewMode = MainWindow::ViewMode::BySections;
        }
        else
        {
            viewMode = MainWindow::ViewMode::Plain;
        }
    }
    else if (Model::current_edition == nullptr)
    {
        if (viewMode == MainWindow::ViewMode::ByTags || viewMode == MainWindow::ViewMode::BySections)
        {
            viewMode = MainWindow::ViewMode::Plain;
        }
    }
    else if (viewMode == MainWindow::ViewMode::ByTags && Model::current_edition->tags.empty())
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

QString Controller::getObjectDescription(const Object *object)
{
    if (const auto *component = dynamic_cast<const Component *>(object))
    {
        return DBusProxy::get().getComponentDescription(component->dbusPath);
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
            auto *item = new QStandardItem(package->getPackageName());
            item->setCheckState(package->installed ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
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
    QString quotedDisplayName = Application::getLocale().quoteString(object != nullptr ? object->displayName() : "");
    QString prefix = QString("<h2>%1 %2</h2>\n").arg(ModelItem::typeToString(type), quotedDisplayName);

    if (type == ModelItem::Type::Component && Model::current_edition)
    {
        const auto *component = dynamic_cast<const Component *>(object);
        if (component != nullptr && !component->tags.empty())
        {
            QStringList tagsNames;
            for (const auto &tag : alt::Model::current_edition->tags)
            {
                if (component->tags.contains(tag.name))
                {
                    const QString tagHtml = QString("<span class=\"tag\">%1</span>").arg(tag.displayName());
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

    auto component = ComponentRepository::get<Component>(object->name);
    if (component.has_value())
    {
        auto systemState = component->get().state;
        auto checkState = index.data(Qt::CheckStateRole).value<Qt::CheckState>();
        if (systemState != static_cast<ComponentState>(checkState))
        {
            TransactionService::current().add(component.value());
        }
        else
        {
            TransactionService::current().discard(component.value());
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
    TransactionService::create();
    setButtonsStatus(false);
}

tl::expected<void, DBusError> Controller::update()
{
    QDBusInterface iface(ALTERATOR_MANAGER_SERVICE_NAME, APT1_PATH, APT1_INTERFACE_NAME, DBusProxy::rawConnection());
    constexpr auto dayInSeconds = 24 * 60 * 60 * 1000;
    iface.setTimeout(dayInSeconds);

    const auto dbusResult = DBusProxy::get().callDBus(&iface, APT1_UPDATE_METHOD_NAME);
    if (!dbusResult.has_value())
    {
        return tl::make_unexpected(DBusError{.code = 101, .text = ""});
    }

    auto errorMessage = DBusProxy::get().checkSuccess(dbusResult.value(), APT1_UPDATE_METHOD_NAME);
    if (errorMessage.has_value())
    {
        return tl::make_unexpected(DBusError{.code = dbusResult->first().toInt(), .text = errorMessage.value()});
    }

    return {};
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
    return TransactionService::current().components().empty();
}

void Controller::setButtonsStatus(bool status)
{
    this->mainWindow->setEnabledApplyButton(status);
    this->mainWindow->setEnabledResetButton(status);
}

void Controller::changeLocale(const QLocale &locale)
{
    DBusProxy::get().resetManagerLocale();
    retranslateWarningsModel();
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
        issueMessage(QtMsgType::QtWarningMsg, warningMessage.arg(updateDate.toString("dd MMMM yyyy")));
        return true;
    }

    return false;
}

void Controller::issueMessage(const QtMsgType &level, const QString &message)
{
    auto *typeItem = new QStandardItem();
    typeItem->setData(level, Qt::UserRole);
    typeItem->setText(toLocalizedString(level));

    auto *item = new QStandardItem(message);
    item->setData(level, Qt::UserRole);
    issueLog({typeItem, item});
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
    const int editionCount = model->countEditionComponents();
    mainWindow->getStatusBar()->onDone(Model::current_edition.get(), totalCount, editionCount);
}

void Controller::showContent(bool showContent)
{
    mainWindow->getComponentWidget()->ComponentsWidget::setVisibleContent(showContent);
}

void Controller::setSafeModeForBaseComponents(bool value)
{
    auto target = TransactionService::SafeMode::Base;
    auto newMode = TransactionService::safeMode();
    value ? newMode.set(target) : newMode.reset(target);
    if (TransactionService::safeMode() == newMode)
    {
        return;
    }

    TransactionService::setSafeMode(newMode);
    if (alt::Model::current_edition != nullptr)
    {
        transactionProxyModel->setDisabledBaseComponents(
            TransactionService::safeMode().test(TransactionService::SafeMode::Base));
    }
    setButtonsStatus(!isStatusEquivalent());
}

bool Controller::setSafeMode(bool value)
{
    auto target = TransactionService::SafeMode::Manually;
    auto newMode = TransactionService::safeMode();
    value ? newMode.set(target) : newMode.reset(target);
    if (TransactionService::safeMode() == newMode)
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

    TransactionService::setSafeMode(newMode);
    return true;
}

void Controller::onAptUpdateNewLine(const QString &line)
{
    emit aptUpdateNewLine(line);
}

void Controller::setViewMode(MainWindow::ViewMode viewMode)
{
    this->viewMode = viewMode;
    if (viewMode == MainWindow::ViewMode::BySections)
    {
        this->modelBuilder->buildBySections(this->model.get(), false);
        mainWindow->getComponentWidget()->ComponentsWidget::expandTopLevel();
    }
    else if (viewMode == MainWindow::ViewMode::ByTags)
    {
        this->modelBuilder->buildByTags(this->model.get(), false);
        mainWindow->getComponentWidget()->ComponentsWidget::expandTopLevel();
    }
    else if (viewMode == MainWindow::ViewMode::Plain)
    {
        this->modelBuilder->buildPlain(this->model.get(), false);
    }

    if (Model::current_edition != nullptr)
    {
        transactionProxyModel->setDisabledBaseComponents(
            TransactionService::safeMode().test(TransactionService::SafeMode::Base));
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

void Controller::showWarnings()
{
    auto *warningsDialog = new WarningsDialog(warningsModel.get(), mainWindow.get());
    connect(warningsDialog, &WarningsDialog::messagesRemoved, this, [this] { warningsModel->clear(); });
    warningsDialog->show();
}

void Controller::showError(int code, const QString &text)
{
    transactionWizard->reject();
    errorDialog = std::make_unique<ErrorDialog>(this->mainWindow.get());
    errorDialog->showError(code, text);
}

void Controller::issueLog(const QList<QStandardItem *> &items)
{
    auto row = QList<QStandardItem *>() << new QStandardItem(QDateTime::currentDateTime().toString("hh:mm:ss"));
    row.append(items);
    warningsModel->appendRow(row);
}

QString Controller::toLocalizedString(const QtMsgType &level)
{
    switch (level)
    {
    case QtMsgType::QtCriticalMsg:
        return tr("Error");
    case QtMsgType::QtWarningMsg:
        return tr("Warning");
    default:
        return tr("Info");
    }
}

void Controller::retranslateWarningsModel()
{
    if (!warningsModel)
        return;

    for (int row = 0; row < warningsModel->rowCount(); ++row)
    {
        QStandardItem *typeItem = warningsModel->item(row, 1);
        if (typeItem)
        {
            QVariant levelData = typeItem->data(Qt::UserRole);
            if (levelData.isValid())
            {
                QtMsgType level = static_cast<QtMsgType>(levelData.toInt());
                typeItem->setText(toLocalizedString(level));
            }
        }
    }
}
} // namespace alt
