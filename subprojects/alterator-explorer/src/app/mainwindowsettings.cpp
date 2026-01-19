#include "mainwindowsettings.h"
#include "mainwindow.h"

#include <QSettings>

namespace ab
{
const QString MAIN_WINDOW_GEOMETRY = "mainwindow/geometry";
const QString MAIN_WINDOW_STATE = "mainwindow/state";
const QString MAIN_WINDOW_DISABLE_BRANDING = "mainwindow/disable_branding";

class MainWindowSettingsPrivate
{
public:
    MainWindowSettingsPrivate(MainWindow *mWindow, Ui::MainWindow *ui)
        : m_mainWindow(mWindow)
        , m_ui(ui)
    {}
    ~MainWindowSettingsPrivate() = default;

public:
    MainWindowSettingsPrivate(const MainWindowSettingsPrivate &) = delete;
    MainWindowSettingsPrivate(MainWindowSettingsPrivate &&) = delete;
    MainWindowSettingsPrivate &operator=(const MainWindowSettingsPrivate &) = delete;
    MainWindowSettingsPrivate &operator=(MainWindowSettingsPrivate &&) = delete;

    MainWindow *m_mainWindow = nullptr;
    Ui::MainWindow *m_ui = nullptr;
    QSettings m_settings{};
};

MainWindowSettings::MainWindowSettings(MainWindow *mWindow, Ui::MainWindow *ui)
    : d(new MainWindowSettingsPrivate(mWindow, ui))
{}

MainWindowSettings::~MainWindowSettings()
{
    delete d;
}

void MainWindowSettings::restoreSettings()
{
    const QByteArray geometry = d->m_settings.value(MAIN_WINDOW_GEOMETRY).toByteArray();
    d->m_mainWindow->restoreGeometry(geometry);

    const QByteArray state = d->m_settings.value(MAIN_WINDOW_STATE).toByteArray();
    d->m_mainWindow->restoreState(state);

    d->m_mainWindow->setBrandingEnabled( !d->m_settings.value(MAIN_WINDOW_DISABLE_BRANDING).toBool() );
}

void MainWindowSettings::saveSettings()
{
    const QByteArray geometry = d->m_mainWindow->saveGeometry();
    d->m_settings.setValue(MAIN_WINDOW_GEOMETRY, geometry);

    const QByteArray state = d->m_mainWindow->saveState();
    d->m_settings.setValue(MAIN_WINDOW_STATE, state);

    d->m_settings.setValue(MAIN_WINDOW_DISABLE_BRANDING, !d->m_mainWindow->isBrandingEnabled() );
}
} // namespace ab
