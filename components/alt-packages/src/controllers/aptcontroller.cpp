#include "aptcontroller.h"

#include "entities/package.h"

#include "ui/mainwindow/mainwindow.h"

#include <algorithm>
#include <nlohmann/json.hpp>
#include <vector>
#include <QDate>
#include <QDebug>
#include <QFutureWatcher>
#include <QInputDialog>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>

#include "datasource/dbusdatasource.h"

#if QT_VERSION_MAJOR == 6
#define concurrentRun(FUNC, THIS) (QtConcurrent::run(FUNC, THIS))
#define concurrentRunWithArgs(FUNC, THIS, ...) (QtConcurrent::run(FUNC, THIS, __VA_ARGS__))
#else
#define concurrentRun(FUNC, THIS) (QtConcurrent::run(THIS, FUNC))
#define concurrentRunWithArgs(FUNC, THIS, ...) (QtConcurrent::run(THIS, FUNC, __VA_ARGS__))
#endif

AptController::AptController(DataSourceInterface *dataSource, MainController *mc)
    : BaseController(dataSource, mc)
    , excludePackages({})
{
    m_proxyModel->sort(Qt::AscendingOrder);

    updateModel();

    // Note(sheriffkorov): It's bad, but I don't know how do it using inteface and virtual signals (Qt does not allow).
    // If you know how to fix it, fix it please.
    connect(dynamic_cast<DBusDataSource *>(getDataSource()),
            SIGNAL(onAptInstallAsync(QString)),
            this,
            SLOT(onInstall(QString)));
    connect(dynamic_cast<DBusDataSource *>(getDataSource()),
            SIGNAL(onAptUpdateAsync(QString)),
            this,
            SLOT(onUpdate(QString)));
    connect(dynamic_cast<DBusDataSource *>(getDataSource()),
            SIGNAL(onAptDistUpgradeAsync(QString)),
            this,
            SLOT(onDistUpgrade(QString)));

    connect(dynamic_cast<DBusDataSource *>(getDataSource()), SIGNAL(closeWaitDialog()), this, SLOT(onCloseWaitDialog()));
}

AptController::~AptController() = default;

void AptController::onSelected()
{
    emit showLoadingIndicator(false);
    QStringList backendResult;
    auto exitCode = getDataSource()->aptLastUpdate(backendResult);
    if (exitCode)
    {
        qWarning() << "Failed to get last package list update date from APT";
    }
    else
    {
        QString lastUpdateResult = backendResult[0];
        QRegularExpression re("^(?<datetime>\\d\\d\\d\\d-\\d\\d-\\d\\d \\d\\d:\\d\\d:\\d\\d) UTC$");
        QRegularExpressionMatch match = re.match(lastUpdateResult);

        if (match.hasMatch())
        {
            QString lastUpdate = match.captured("datetime");
            QDateTime current(QDateTime::currentDateTimeUtc());
            QDateTime last(QDateTime::fromString(lastUpdate, "yyyy-MM-dd hh:mm:ss"));
            last.setTimeZone(QTimeZone::UTC);

            if (last.addDays(1) > current)
            {
                return;
            }
        }
    }

    if (questionUpdate() == QMessageBox::Yes)
    {
        onUpdateRequested();
    }
}

void AptController::onDeselected() {}

void AptController::onWindowClose() {}

void AptController::onApplyChangesRequested(const QModelIndexList &indexes)
{
    emit showLoadingIndicator(true);

    if (indexes.isEmpty())
    {
        emit showLoadingIndicator(false);
        return;
    }

    QStringList pkgs;
    for (const auto &index : indexes)
    {
        const auto &pkg = index.data(Qt::UserRole).value<Package>();
        pkgs << (pkg.m_installed ? QString("%1-").arg(pkg.m_name) : pkg.m_name);
    }

    if (questionApply(indexes) != QMessageBox::StandardButton::Apply)
    {
        emit showLoadingIndicator(false);
        return;
    }

    emit showLoadingIndicator(false);
    auto result = concurrentRunWithArgs(&AptController::install, this, pkgs);
    emit openWaitDialog();
}

void AptController::install(const QStringList &packages)
{
    emit showLoadingIndicator(true);

    QStringList result;
    QString packagesStr = packages.join(" ");

    QString excludeStr = excludePackages.empty() ? "\"''\"" : QString("\"%1\"").arg(excludePackages.join(' '));

    int code = getDataSource()->aptApplyAsync(result, excludeStr, packagesStr);
    if (code)
    {
        emit showError(code, result.isEmpty() ? QStringList() << tr("Something gone wrong!") : result);
        emit showLoadingIndicator(false);
        return;
    }

    emit requestToUpdateData();
    emit showLoadingIndicator(false);
}

void AptController::update()
{
    emit showLoadingIndicator(true);
    QStringList result;
    int code = getDataSource()->aptUpdateAsync(result);
    if (code)
    {
        emit showError(code, result.isEmpty() ? QStringList() << tr("Something gone wrong!") : result);
        emit showLoadingIndicator(false);
        return;
    }

    emit requestToUpdateData();
    emit showLoadingIndicator(false);
}

void AptController::onInstall(QString message)
{
    emit appendWaitMessage(message);
}

void AptController::onDistUpgrade(QString message)
{
    emit appendWaitMessage(message);
}

void AptController::onCloseWaitDialog()
{
    emit closeWaitDialog();
}

void AptController::onUpdate(QString message)
{
    emit appendWaitMessage(message);
}

void AptController::onUpdateRequested()
{
    emit showLoadingIndicator(true);
    auto result = concurrentRun(&AptController::update, this);
    emit openWaitDialog();
}

void AptController::onDistUpgradeRequested()
{
    emit showLoadingIndicator(true);
    if (questionUpdate() == QMessageBox::StandardButton::Yes)
    {
        onUpdateRequested();
    }

    auto questionResult = questionDistUpgrade();
    if (questionResult == QMessageBox::StandardButton::Apply)
    {
        auto result = concurrentRun(&AptController::distUpgrade, this);
        emit openWaitDialog();
    }
    else if (questionResult == QMessageBox::StandardButton::Ignore)
    {
        QMessageBox::information(m_window.get(),
                                 tr("ALT Packages - Notification"),
                                 tr("The database of packages is already up to date."),
                                 QMessageBox::StandardButton::Ok,
                                 QMessageBox::StandardButton::Ok);
    }
}

void AptController::distUpgrade()
{
    emit showLoadingIndicator(true);
    QStringList result;
    int code = getDataSource()->aptDistUpgradeAsync(result);
    if (code)
    {
        emit showError(code, result.isEmpty() ? QStringList() << tr("Something gone wrong!") : result);
        emit showLoadingIndicator(false);
        return;
    }

    emit requestToUpdateData();
    emit showLoadingIndicator(false);
}

std::vector<Package> AptController::getModelData()
{
    std::vector<Package> data;

    QStringList packages;
    emit showLoadingIndicator(true);
    int exitCode = getDataSource()->aptListAllPackages(packages);
    emit showLoadingIndicator(false);
    if (exitCode)
    {
        qWarning() << "Failed to get package list from APT";
        emit showError(exitCode, packages);
        return {};
    }

    QStringList installedPackages;
    emit showLoadingIndicator(true);
    exitCode = getDataSource()->rpmList(installedPackages);
    emit showLoadingIndicator(false);
    if (exitCode)
    {
        emit showError(exitCode, installedPackages);
        qWarning() << "Failed to get installed packages list from RPM";
        return {};
    }

    std::transform(installedPackages.begin(),
                   installedPackages.end(),
                   installedPackages.begin(),
                   [](const QString &pkgInfo) { return pkgInfo.split(" ")[0]; });

    QSet<QString> installedSet(installedPackages.begin(), installedPackages.end());
    for (auto &pkg : packages)
    {
        bool isInstalled = installedSet.contains(pkg);
        data.push_back({std::move(pkg), {}, {}, {}, "", isInstalled});
    }

    return data;
}

void AptController::updateModel()
{
    emit showLoadingIndicator(true);

    const auto &data = getModelData();
    if (data.empty())
    {
        emit showLoadingIndicator(false);
        return;
    }

    m_model->clear();

    for (auto &pkg : data)
    {
        auto pkgItem = std::make_unique<QStandardItem>(pkg.m_name);
        pkgItem->setCheckable(true);
        pkgItem->setCheckState(pkg.m_installed ? Qt::Checked : Qt::Unchecked);
        pkgItem->setData(QVariant::fromValue(pkg), Qt::UserRole);
        m_model->appendRow(pkgItem.release());
    }

    updateModelHeaderData();
    emit showLoadingIndicator(false);
}
void onInstall(QString message);
void AptController::updateModelHeaderData()
{
    m_model->setHeaderData(0, Qt::Horizontal, tr("Name"), Qt::DisplayRole);
}

QMessageBox::StandardButton AptController::questionApply(const QModelIndexList &indexes)
{
    if (indexes.isEmpty())
    {
        return QMessageBox::StandardButton::Abort;
    }

    QStringList aptInstallPackages;
    QStringList aptRemovePackages;
    for (const auto &index : indexes)
    {
        const auto &pkg = index.data(Qt::UserRole).value<Package>();
        if (pkg.m_installed)
        {
            aptRemovePackages.push_back(pkg.m_name);
        }
        else
        {
            aptInstallPackages.push_back(pkg.m_name);
        }
    }

    QString aptInstallPackagesQuery = aptInstallPackages.join(" ") + aptRemovePackages.join("- ");
    if (!aptRemovePackages.empty())
    {
        aptInstallPackagesQuery.append("-");
    }

    QStringList packagesForTransaction, errors;
    int code = getDataSource()->aptCheckApply(packagesForTransaction, errors, aptInstallPackagesQuery);
    if (code || packagesForTransaction.empty())
    {
        return QMessageBox::StandardButton::Abort;
    }

    QString jsonString          = packagesForTransaction.first();
    auto json                   = nlohmann::json::parse(jsonString.toStdString());
    auto jsonArrayToQStringList = [](const nlohmann::json &jsonArray) {
        QStringList result;
        for (const auto &item : jsonArray)
        {
            result << (QString::fromStdString(item.get<std::string>()));
        }
        return result;
    };
    auto packagesForInstallation = jsonArrayToQStringList(json["install_packages"]);
    auto packagesForRemoval      = jsonArrayToQStringList(json["remove_packages"]);
    auto extraPackagesForRemoval = jsonArrayToQStringList(json["extra_remove_packages"]);

    if (!safeMode)
    {
        excludePackages = extraPackagesForRemoval;
    }

    for (const auto &pkg : aptRemovePackages)
    {
        extraPackagesForRemoval.removeAll(pkg);
    }
    for (const auto &pkg : extraPackagesForRemoval)
    {
        packagesForRemoval.removeAll(pkg);
    }

    auto dialog = std::make_unique<ApplyDialog>(packagesForInstallation,
                                                packagesForRemoval,
                                                extraPackagesForRemoval,
                                                safeMode,
                                                m_window.get());

    return dialog->exec() ? QMessageBox::StandardButton::Apply : QMessageBox::StandardButton::Abort;
}

QMessageBox::StandardButton AptController::questionUpdate()
{
    return QMessageBox::warning(
        m_window.get(),
        tr("APT update is required."),
        tr("It seems your package lists haven't been updated in a while. Would you like to update them?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes);
}

QMessageBox::StandardButton AptController::questionDistUpgrade()
{
    QStringList packagesForRemoval, packagesForInstallation;
    int code = getDataSource()->aptCheckDistUpgrade(packagesForInstallation, packagesForRemoval);
    if (code)
    {
        emit showError(code, packagesForRemoval);
        return QMessageBox::StandardButton::Abort;
    }

    if (packagesForRemoval.empty() && packagesForInstallation.empty())
    {
        return QMessageBox::StandardButton::Ignore;
    }

    auto dialog = std::make_unique<ApplyDialog>(packagesForInstallation,
                                                packagesForRemoval,
                                                QStringList(),
                                                m_window.get());

    return dialog->exec() ? QMessageBox::StandardButton::Apply : QMessageBox::StandardButton::Abort;
}

bool AptController::disableSafeMode(bool value)
{
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
    safeMode = value;
    return true;
}
