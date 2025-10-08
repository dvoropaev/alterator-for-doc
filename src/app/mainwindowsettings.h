#ifndef AB_MAIN_WINDOW_SETTINGS_H
#define AB_MAIN_WINDOW_SETTINGS_H

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QObject>

namespace ab
{
class MainWindow;
class MainWindowSettingsPrivate;

class MainWindowSettings : public QObject
{
    Q_OBJECT
public:
    MainWindowSettings(MainWindow *mWindow, Ui::MainWindow *ui);
    ~MainWindowSettings() override;

public:
    MainWindowSettings(const MainWindowSettings &) = delete;
    MainWindowSettings(MainWindowSettings &&) = delete;
    MainWindowSettings &operator=(const MainWindowSettings &) = delete;
    MainWindowSettings &operator=(MainWindowSettings &&) = delete;

    void restoreSettings();

public slots:
    void saveSettings();

private:
    MainWindowSettingsPrivate *d;
};
} // namespace ab

#endif // AB_MAIN_WINDOW_SETTINGS_H
