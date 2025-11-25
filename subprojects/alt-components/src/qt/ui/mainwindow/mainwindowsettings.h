#ifndef MAIN_WINDOW_SETTINGS_H
#define MAIN_WINDOW_SETTINGS_H

#include <memory>

#include <QObject>

namespace alt
{
class MainWindow;
class MainWindowSettingsPrivate;

class MainWindowSettings : public QObject
{
    Q_OBJECT
public:
    explicit MainWindowSettings(MainWindow *mWindow);
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
    std::unique_ptr<MainWindowSettingsPrivate> d;
};
} // namespace alt

#endif // MAIN_WINDOW_SETTINGS_H
