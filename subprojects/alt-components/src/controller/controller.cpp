#include "controller.h"
#include "application.h"
#include "constants.h"
#include "dbus/dbusproxy.h"
#include "model/item.h"
#include "model/model.h"
#include "model/modelbuilder.h"
#include "model/objects/category.h"
#include "model/objects/component.h"
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
#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>
#include <qnamespace.h>

namespace alt
{
Controller::Controller()
    : model(std::make_unique<Model>())
    , proxyModel(std::make_unique<ProxyModel>())
    , warningsModel(std::make_unique<QStandardItemModel>(0, 2))
{
    DBusProxy::get().resetManagerLocale();
    modelBuilder = std::make_unique<ModelBuilder>(this);

    this->proxyModel->setSourceModel(this->model.get());
    mainWindow = std::make_unique<MainWindow>(this->model.get(), this->proxyModel.get());
    transactionWizard = std::make_unique<TransactionWizard>(this->mainWindow.get());
    updatingWizard = std::make_unique<UpdatingSourcesWizard>(this->mainWindow.get());

    connect(modelBuilder.get(), &ModelBuilder::buildStarted, mainWindow->getStatusBar(), &MainStatusBar::onStarted);
    connect(modelBuilder.get(), &ModelBuilder::buildDone, mainWindow->getStatusBar(), &MainStatusBar::onDone);

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

    mainWindow->showActionOtherComponents(Model::current_edition != nullptr);

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
    TransactionService::create();

    mainWindow->getStatusBar()->resetProgressBarValue();

    // Temporarily disconnect the proxy model
    proxyModel->setSourceModel(nullptr);

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

    model->translate(Application::getLocale().name());

    if (Model::current_edition != nullptr)
    {
        modelBuilder->setEditionRelationshipForAllComponents();
    }

    // Reconnect and reset the proxy model
    proxyModel->setSourceModel(model.get());
    proxyModel->invalidate();

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
        setEnableBaseComponents(!TransactionService::safeMode().test(TransactionService::SafeMode::Base));
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

void Controller::selectObject(QModelIndex index)
{
    if (!index.isValid())
    {
        qWarning() << "Selection index is invalid";
        return;
    }

    if (const auto *component = index.data(alt::ObjectRole).value<Component *>())
    {
        const QString &description = DBusProxy::get().getComponentDescription(component->dbusPath);
        this->mainWindow->setDescription(index, alt::ModelItem::Type::Component, component->displayName, description);
        // TODO(chernigin): maybe we don't need to convert std::set into std::vector
        this->mainWindow->setContentList({component->packages.begin(), component->packages.end()});
    }
    else if (const auto *category = index.data(alt::ObjectRole).value<Category *>())
    {
        const QString &description = DBusProxy::get().getCategoryDescription(category->name);
        this->mainWindow->setDescription(index, alt::ModelItem::Type::Category, category->displayName, description);
        this->mainWindow->setContentList(this->proxyModel.get(), index);
    }
    else if (const auto *section = index.data(alt::ObjectRole).value<Section *>())
    {
        const QString &description = ""; // TODO(chernigin): implement description
        this->mainWindow->setDescription(index, alt::ModelItem::Type::Section, section->displayName, description);
        this->mainWindow->setContentList(this->proxyModel.get(), index);
    }
    else if (const auto *tag = index.data(alt::ObjectRole).value<Tag *>())
    {
        const QString &description = ""; // TODO(chernigin): implement description
        this->mainWindow->setDescription(index, alt::ModelItem::Type::Tag, tag->displayName, description);
        this->mainWindow->setContentList(this->proxyModel.get(), index);
    }
    else
    {
        this->mainWindow->setDescription("");
        this->mainWindow->setContentList(this->proxyModel.get(), index);
    }
}

void Controller::itemChanged(ModelItem *item)
{
    if (!item)
    {
        return;
    }

    if (auto *component = item->data().value<Component *>())
    {
        auto systemState = component->state;
        auto checkState = item->checkState();

        auto comp = ComponentRepository::get<Component>(component->name);
        if (systemState != static_cast<ComponentState>(checkState))
        {
            TransactionService::current().add(comp.value());
            item->setBackground(checkState == Qt::Checked ? Model::CHECKED_BACKGROUND_BRUSH
                                                          : Model::UNCHECKED_BACKGROUND_BRUSH);
        }
        else
        {
            TransactionService::current().discard(comp.value());
            item->setBackground(Application::palette().color(QPalette::Base));
        }

        setButtonsStatus(!isStatusEquivalent());
        updateParentCategoryBackground(static_cast<ModelItem *>(item->parent()));
    }
    else if (item->data().value<Category *>())
    {
        updateParentCategoryBackground(item);
    }
    model->correctCheckItemStates();
}

void Controller::updateParentCategoryBackground(ModelItem *parentItem)
{
    if (!parentItem || !parentItem->data().value<Category *>())
    {
        return;
    }

    bool hasGreenChild = false;
    bool hasRedChild = false;

    for (int i = 0; i < parentItem->rowCount(); ++i)
    {
        QBrush childBgBrush = parentItem->child(i)->background();
        if (childBgBrush == Model::CHECKED_BACKGROUND_BRUSH)
        {
            hasGreenChild = true;
        }
        else if (childBgBrush == Model::UNCHECKED_BACKGROUND_BRUSH)
        {
            hasRedChild = true;
        }
        else if (childBgBrush == Model::MIXED_STATE_BACKGROUND_BRUSH)
        {
            hasGreenChild = true;
            hasRedChild = true;
        }
        if (hasGreenChild && hasRedChild)
        {
            break;
        }
    }

    QBrush newBackgroundBrush = Application::palette().color(QPalette::Base);
    if (hasGreenChild && hasRedChild)
    {
        newBackgroundBrush = Model::MIXED_STATE_BACKGROUND_BRUSH;
    }
    else if (hasGreenChild)
    {
        newBackgroundBrush = Model::CHECKED_BACKGROUND_BRUSH;
    }
    else if (hasRedChild)
    {
        newBackgroundBrush = Model::UNCHECKED_BACKGROUND_BRUSH;
    }

    if (parentItem->background() != newBackgroundBrush)
    {
        parentItem->setBackground(newBackgroundBrush);
        if (auto *grandParentItem = dynamic_cast<ModelItem *>(parentItem->parent()))
        {
            updateParentCategoryBackground(grandParentItem);
        }
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
    this->model->resetCurrentState();
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
    this->model->translate(locale.name());
    retranslateWarningsModel();
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

void Controller::showDrafts(bool showDrafts)
{
    proxyModel->setShowDrafts(showDrafts);
    proxyModel->invalidate();
}

void Controller::setFilterNonEditionComponents(bool show)
{
    proxyModel->setFilterNonEditionComponents(show);
    proxyModel->invalidate();

    if (viewMode != MainWindow::ViewMode::Plain)
    {
        mainWindow->getComponentWidget()->expandTopLevel();
    }
}

void Controller::resetBaseComponentState(QStandardItem *parent,
                                         const QSet<QString> &baseComponents,
                                         QList<ModelItem *> &installedBaseComponents)
{
    if (!parent)
        return;

    if (auto *modelItem = dynamic_cast<ModelItem *>(parent))
    {
        if (auto *component = modelItem->data(CustomRoles::ObjectRole).value<Component *>())
        {
            if (baseComponents.contains(component->name) && component->state == ComponentState::installed)
            {
                installedBaseComponents.append(modelItem);
                modelItem->setCheckState(static_cast<Qt::CheckState>(ComponentState::installed));
                return;
            }
        }
    }
    for (int i = 0; i < parent->rowCount(); ++i)
    {
        resetBaseComponentState(parent->child(i), baseComponents, installedBaseComponents);
    }
}

void Controller::setEnableBaseComponents(bool isEnable)
{
    QList<ModelItem *> installedBaseComponents;

    auto &sections = alt::Model::current_edition->sections;
    auto it = std::find_if(sections.begin(), sections.end(), [](const auto &section) { return section.name == "base"; });

    if (it != sections.end())
    {
        auto baseComponents = it->components;

        QStandardItem *rootItem = this->model->invisibleRootItem();
        resetBaseComponentState(rootItem, baseComponents, installedBaseComponents);
        for (auto *item : installedBaseComponents)
        {
            this->model->itemSetEnable(item, isEnable);
        }
    }
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
        setEnableBaseComponents(!value);
    }
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
    reset();

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

    model->translate(Application::getLocale().name());
    model->resetCurrentState();
    if (Model::current_edition != nullptr)
    {
        setEnableBaseComponents(!TransactionService::safeMode().test(TransactionService::SafeMode::Base));
    }
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

    model->translate(Application::getLocale().name());
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
