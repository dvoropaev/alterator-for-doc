#include "basecontroller.h"

#include "maincontroller.h"
#include "ui/mainwindow/mainwindow.h"

BaseController::BaseController(DataSourceInterface *dataSource, MainController *mc)
    : m_window(nullptr)
    , m_dataSource(dataSource)
    , m_model(std::make_unique<QStandardItemModel>())
    , m_proxyModel(std::make_unique<PackagesFilterProxyModel>())
{
    m_proxyModel->setSourceModel(m_model.get());
    m_proxyModel->setFilterKeyColumn(-1);

    updateModel();

    connect(this, &BaseController::requestToUpdateData, mc, &MainController::onUpdateDataRequested);
    connect(this, &BaseController::showError, mc, &MainController::showError);
    connect(this, &BaseController::openWaitDialog, mc, &MainController::openWaitDialog, Qt::DirectConnection);
    connect(this, &BaseController::appendWaitMessage, mc, &MainController::appendMessageToWaitDialog);
    connect(this, &BaseController::closeWaitDialog, mc, &MainController::closeWaitDialog);
}

BaseController::~BaseController() = default;

void BaseController::selected() {}

void BaseController::deSelected() {}

void BaseController::setWindow(std::shared_ptr<MainWindow> window)
{
    m_window = window;
}

DataSourceInterface *BaseController::getDataSource()
{
    return m_dataSource;
}

void BaseController::updateModel() {}

void BaseController::updateModelHeaderData() {}

void BaseController::retranslate(const QLocale &locale)
{
    std::ignore = locale;
    updateModelHeaderData();
}

PackagesFilterProxyModel *BaseController::getModel()
{
    return m_proxyModel.get();
}
