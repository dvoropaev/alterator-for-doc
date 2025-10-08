#ifndef MAIN_WINDOW_SETTINGS_H
#define MAIN_WINDOW_SETTINGS_H

#include "ui_mainwindow.h"

#include <QObject>
#include <QSettings>

class MainWindow;

class MainWindowSettings : public QObject
{
    Q_OBJECT
public:
    MainWindowSettings(MainWindow *mWindow, Ui::MainWindow *ui);
    ~MainWindowSettings() override;

public:
    MainWindowSettings(const MainWindowSettings &)            = delete;
    MainWindowSettings(MainWindowSettings &&)                 = delete;
    MainWindowSettings &operator=(const MainWindowSettings &) = delete;
    MainWindowSettings &operator=(MainWindowSettings &&)      = delete;

    void restoreSettings();

public slots:
    void saveSettings();

private:
    MainWindow *m_mainWindow;
    Ui::MainWindow *m_ui;
    QSettings m_settings{};
};

#endif // MAIN_WINDOW_SETTINGS_H
