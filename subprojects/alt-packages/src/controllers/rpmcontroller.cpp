#include "rpmcontroller.h"

#include "entities/package.h"

#include "ui/dialogs/rpmfilesdialog.h"
#include "ui/dialogs/rpminfodialog.h"
#include "ui/mainwindow/mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QFuture>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QtConcurrent/QtConcurrent>

RpmController::RpmController(DataSourceInterface *dataSource, MainController *mc)
    : BaseController(dataSource, mc)
    , m_groupsOfAllPackages()
    , m_archesOfAllPackages()
{
    m_proxyModel->sort(RpmModelColumn::columnName, Qt::AscendingOrder);

    updateModel();
}

RpmController::~RpmController() = default;

void RpmController::onSelected() {}

void RpmController::onDeselected() {}

void RpmController::onWindowClose() {}

void RpmController::onRemovePackagesRequested(const QModelIndexList &indexes)
{
    const auto &packagesNames = getSelectedPackagesNames(indexes);
    if (packagesNames.isEmpty())
    {
        return;
    }

    emit wait(true, tr("Removing packages..."));
    QStringList result;
    int code = getDataSource()->rpmRemove(result, packagesNames.join(" "));
    emit wait(false, code ? QString{} : tr("Packages removed successfully"));
    if (code)
    {
        emit showError(code, result);
        return;
    }
    emit requestToUpdateData();
}

void RpmController::onInstallPackageRequested()
{
    const auto &rpmFiles = QFileDialog::getOpenFileNames(nullptr, tr("RPM file"), "~/", tr("RPM Files (*.rpm)"));

    if (rpmFiles.isEmpty())
    {
        return;
    }

    emit wait(true, tr("Installing packages..."));
    QStringList result;
    int exitCode = getDataSource()->rpmInstall(result, rpmFiles.join(" "));
    emit wait(false, exitCode ? QString{} : tr("Packages installed successfully"));
    if (exitCode)
    {
        emit showError(exitCode, result);
        return;
    }

    emit requestToUpdateData();
}

void RpmController::onInfoAboutPackagesRequested(const QModelIndexList &indexes)
{
    for (const auto &pkgName : getSelectedPackagesNames(indexes))
    {
        auto model = buildPackageInfoModel(pkgName);
        if (!model)
        {
            return;
        }

        auto dialog = std::make_unique<RpmInfoDialog>(m_window.get()).release();
        dialog->setModel(model);

        dialog->show();

        connect(dialog, &RpmInfoDialog::finished, [dialog, model]() { delete dialog, model; });
    }
}

void RpmController::onFilesOfPackagesRequested(const QModelIndexList &indexes)
{
    for (const auto &pkgName : getSelectedPackagesNames(indexes))
    {
        auto model = buildPackageFilesModel(pkgName);
        if (!model)
        {
            return;
        }

        auto dialog = std::make_unique<RpmFilesDialog>(m_window.get()).release();
        dialog->setModel(model);

        dialog->show();

        connect(dialog, &RpmFilesDialog::finished, [dialog, model]() { delete dialog, model; });
    }
}

void RpmController::updateModel()
{
    const auto &data = getModelData();
    if (data.empty())
    {
        return;
    }

    m_model->clear();

    for (auto &pkg : data)
    {
        QList<QStandardItem *> list;

        auto pkgItem = std::make_unique<QStandardItem>();
        pkgItem->setData(QVariant::fromValue(pkg), Qt::UserRole);
        list << pkgItem.release();

        list << std::make_unique<QStandardItem>(pkg.m_name).release();
        list << std::make_unique<QStandardItem>(QString("%1-%2").arg(pkg.m_version).arg(pkg.m_release)).release();

        m_model.get()->appendRow(list);
    }

    updateModelHeaderData();
}

void RpmController::updateModelHeaderData()
{
    m_model->setHeaderData(RpmModelColumn::columnName, Qt::Horizontal, tr("Name"), Qt::DisplayRole);
    m_model->setHeaderData(RpmModelColumn::columnVersionRelease, Qt::Horizontal, tr("Version-Release"), Qt::DisplayRole);
}

std::vector<Package> RpmController::getModelData()
{
    m_groupsOfAllPackages.clear();
    std::vector<Package> data;

    QStringList packages;
    int exitCode = getDataSource()->rpmList(packages);
    if (exitCode)
    {
        qWarning() << "Failed to get package list from RPM";
        emit showError(exitCode, packages);
        return {};
    }

    for (auto &pkgInfo : packages)
    {
        auto splittedPkgInfo = pkgInfo.split(" ");
        m_groupsOfAllPackages.insert(splittedPkgInfo[4]);
        m_archesOfAllPackages.insert(splittedPkgInfo[3]);
        data.push_back({std::move(splittedPkgInfo[0]),
                        std::move(splittedPkgInfo[1]),
                        std::move(splittedPkgInfo[2]),
                        std::move(splittedPkgInfo[3]),
                        std::move(splittedPkgInfo[4]),
                        true});
    }

    return data;
}

QStringList RpmController::getSelectedPackagesNames(const QModelIndexList &indexes)
{
    QStringList packages;

    for (auto index : indexes)
    {
        if (index.column() == RpmModelColumn::columnName)
        {
            packages << index.data().value<QString>();
        }
    }

    return packages;
}

QStringListModel *RpmController::buildPackageFilesModel(const QString &packageName)
{
    QStringList files;
    auto exitCode = getDataSource()->rpmFiles(files, packageName);
    if (exitCode)
    {
        qWarning() << "Failed to get package list from RPM";
        emit showError(exitCode, files);
        return nullptr;
    }

    return std::make_unique<QStringListModel>(files).release();
}

QStandardItemModel *RpmController::buildPackageInfoModel(const QString &packageName)
{
    QString info;
    QStringList backendResult;
    auto exitCode = getDataSource()->rpmInfo(backendResult, packageName);
    if (exitCode)
    {
        qWarning() << "Failed to get package info from RPM";
        emit showError(exitCode, backendResult);
        return nullptr;
    }
    info = std::move(backendResult[0]);

    auto model             = std::make_unique<QStandardItemModel>();
    auto descriptionRegexp = QRegularExpression("^((?:.+\\: .+\\n)*)(.+\\:\\n[\\s\\S]+)$");
    auto keyValueRegexp    = QRegularExpression("(.+)\\: (.+)\\n");

    auto descriptionBlockMatch = descriptionRegexp.match(info);
    if (!descriptionBlockMatch.hasMatch())
    {
        return nullptr;
    }

    auto keyValueIter = keyValueRegexp.globalMatch(descriptionBlockMatch.captured(1));
    while (keyValueIter.hasNext())
    {
        auto rowMatch = keyValueIter.next();
        model->appendRow(QList<QStandardItem *>()
                         << new QStandardItem(rowMatch.captured(1)) << new QStandardItem(rowMatch.captured(2)));
    }
    auto description = descriptionBlockMatch.captured(2);
    auto t1          = description.section("\n", 0, 0);
    auto t2          = description.section("\n", 1, -1);
    model->appendRow(QList<QStandardItem *>() << new QStandardItem(description.section("\n", 0, 0))
                                              << new QStandardItem(description.section("\n", 1, -1)));

    return model.release();
}

QStringList RpmController::groupsOfAllPackages()
{
    auto list = QStringList(m_groupsOfAllPackages.begin(), m_groupsOfAllPackages.end());
    list.sort();
    return list;
}

QStringList RpmController::archesOfAllPackages()
{
    auto list = QStringList(m_archesOfAllPackages.begin(), m_archesOfAllPackages.end());
    list.sort();
    return list;
}
