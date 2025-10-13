#include "mainwindow.h"

#include "mainwindowsettings.h"
#include "ui_mainwindow.h"

#include "controllers/aptcontroller.h"
#include "controllers/maincontroller.h"

#include <QShortcut>
#include <QStandardItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(std::make_unique<Ui::MainWindow>())
    , m_quitShortcut(std::make_unique<QShortcut>(QKeySequence(Qt::CTRL | Qt::Key_Q), this))
    , m_settings(std::make_unique<MainWindowSettings>(this, m_ui.get()))
{
    m_ui->setupUi(this);

    m_settings->restoreSettings();

    QObject::connect(m_quitShortcut.get(), &QShortcut::activated, this, &MainWindow::close);
}

MainWindow::~MainWindow() = default;

void MainWindow::addActionsToLanguageMenu(const QList<QAction *> &actions)
{
    m_ui->languageMenu->addActions(actions);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_settings->saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        for (auto action : m_ui->languageMenu->actions())
        {
            const auto &value = action->data().value<std::pair<QLocale, std::function<QString()>>>();
            auto trText       = value.second;
            auto locale       = value.first;
            action->setText(trText());
        }

        m_ui->retranslateUi(this);
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::connect(AptController *controller)
{
    m_ui->centralwidget->connect(controller);
    QObject::connect(m_ui->disableRemoveManuallyAction, &QAction::triggered, this, [this, controller](bool checked) {
        if (!controller->disableSafeMode(checked))
        {
            m_ui->disableRemoveManuallyAction->setChecked(true);
        }
    });
}

void MainWindow::connect(RpmController *controller)
{
    m_ui->centralwidget->connect(controller);
}

void MainWindow::connect(MainController *controller)
{
    QObject::connect(m_ui->aboutAction, &QAction::triggered, controller, &MainController::onAboutDialogRequested);
    QObject::connect(m_ui->manualAction, &QAction::triggered, controller, &MainController::onManualRequested);
}

void MainWindow::connect(RepoController *controller)
{
    m_ui->centralwidget->connect(controller);
}

void MainWindow::setActive(const QString &objectPath)
{
    m_ui->centralwidget->setActive(objectPath);
}
