#include "mainwindowsettings.h"

#include "mainwindow.h"

const QString MAIN_WINDOW_GEOMETRY = "mainwindow/geometry";
const QString MAIN_WINDOW_STATE    = "mainwindow/state";

MainWindowSettings::MainWindowSettings(MainWindow *mWindow, Ui::MainWindow *ui)
    : m_mainWindow(mWindow)
    , m_ui(ui)
    , m_settings("BaseALT", "alt-packages")
{}

MainWindowSettings::~MainWindowSettings() = default;

void MainWindowSettings::restoreSettings()
{
    const QByteArray geometry = m_settings.value(MAIN_WINDOW_GEOMETRY).toByteArray();
    m_mainWindow->restoreGeometry(geometry);

    const QByteArray state = m_settings.value(MAIN_WINDOW_STATE).toByteArray();
    m_mainWindow->restoreState(state);
}

void MainWindowSettings::saveSettings()
{
    const QByteArray geometry = m_mainWindow->saveGeometry();
    m_settings.setValue(MAIN_WINDOW_GEOMETRY, geometry);

    const QByteArray state = m_mainWindow->saveState();
    m_settings.setValue(MAIN_WINDOW_STATE, state);
}
