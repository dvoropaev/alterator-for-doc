#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QShortcut>

#include "mainwindowsettings.h"

class AptController;
class RpmController;
class RepoController;
class MainController;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    void connect(MainController *controller);
    void connect(AptController *controller);
    void connect(RpmController *controller);
    void connect(RepoController *controller);
    void setActive(const QString &objectPath);
    void addActionsToLanguageMenu(const QList<QAction *> &actions);

private slots:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    MainWindow(const MainWindow &)            = delete;
    MainWindow(MainWindow &&)                 = delete;
    MainWindow &operator=(const MainWindow &) = delete;
    MainWindow &operator=(MainWindow &&)      = delete;

private:
    std::unique_ptr<Ui::MainWindow> m_ui;
    std::unique_ptr<QShortcut> m_quitShortcut;
    std::unique_ptr<MainWindowSettings> m_settings;
};

#endif // MAINWINDOW_H
