#include "maincontroller.h"

#include "databuilder/databuilder.h"

#include "ui/dialogs/licensedialog/licensedialog.h"
#include "ui/mainwindow/mainwindow.h"

#include "interface/managerinterface.h"

#include <QLocale>

namespace alt
{
MainController::MainController()
    : m_osModel(std::make_unique<QSortFilterProxyModel>())
    , m_hardwareModel(std::make_unique<QSortFilterProxyModel>())
    , m_mainWindow(std::make_unique<MainWindow>())
    , m_licenseDialog(std::make_unique<LicenseDialog>(m_mainWindow.get()))
{
    ManagerInterface::setEnvValue("LC_ALL", QLocale::system().name());

    DataBuilder::rebuildProduct();

    m_mainWindow->connect(this);

    m_osModel->setSourceModel(DataBuilder::buildPropertyModel(DataBuilder::PropertyModelTypeOperationSystem).release());

    // NOTE(sheriffkorov): Temporary (when there's time to do well)
    // m_hardwareModel->setSourceModel(DataBuilder::buildPropertyModel(DataBuilder::PropertyModelTypeHardware).release());

    m_mainWindow->setHostName(DataBuilder::buildHostName());
    m_mainWindow->setOsModel(m_osModel.get());
    // m_mainWindow->setHardwareModel(m_hardwareModel.get());

    const auto &logo = DataBuilder::product().logo();
    if (logo.has_value())
    {
        QPixmap p;
        p.loadFromData(logo.value());
        m_mainWindow->setOsLogo(p);
    }
}

MainController::~MainController()
{
    ManagerInterface::unsetEnvValue("LC_ALL");
}

void MainController::showMainWindow()
{
    m_mainWindow->show();
}

void MainController::showLicenseDialog()
{
    auto reply = DataBuilder::product().license();
    if (!reply.has_value())
    {
        return;
    }

    m_licenseDialog->setLicenseText(reply.value());

    if (m_mainWindow->isHidden())
    {
        m_licenseDialog->show();
    }
    else
    {
        m_licenseDialog->exec();
    }
}

void MainController::showReleaseNotes()
{
    m_mainWindow->setTabUsefulInformation();
    showMainWindow();
}

void MainController::onLicenseDialogRequested()
{
    showLicenseDialog();
}
} // namespace alt
