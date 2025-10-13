#include "mainwindow.h"

#include "ui_mainwindow.h"

#include <QDateTime>
#include <QDebug>
#include <QStandardItemModel>
#include <QTimeZone>

namespace alt
{
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(std::make_unique<Ui::MainWindow>())
    , m_quitShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q), this)
{
    m_ui->setupUi(this);

    QFont font = m_ui->hostNameLabel->font();
    font.setBold(true);
    m_ui->hostNameLabel->setFont(font);

    // NOTE(sheriffkorov): Temporary (when there's time to do well)
    m_ui->tabWidget->removeTab(m_ui->tabWidget->indexOf(m_ui->hardwareTab));
    QObject::connect(&m_quitShortcut, &QShortcut::activated, this, &MainWindow::close);
}

MainWindow::~MainWindow() = default;

void MainWindow::connect(MainController *controller)
{
    m_ui->osPropertiesWidget->connect(controller);
}

void MainWindow::setHostName(const QString &name)
{
    m_ui->hostNameLabel->setText(name);
}

void MainWindow::setOsLogo(const QPixmap &logo)
{
    m_ui->osPropertiesWidget->setLogo(logo);
}

void MainWindow::setOsModel(QAbstractItemModel *model)
{
    m_ui->osPropertiesWidget->setModel(model);
}

void MainWindow::setHardwareModel(QAbstractItemModel *model)
{
    m_ui->hardwarePropertiesWidget->setModel(model);
}

void MainWindow::setTabUsefulInformation()
{
    m_ui->tabWidget->setCurrentWidget(m_ui->usefulTab);
}
} // namespace alt
