#include "settingswidget.h"

#include "ui_settingswidget.h"

#include "databuilder/databuilder.h"

#include <filesystem>
#include <map>

#include <QDebug>
#include <QProcess>

namespace alt
{
SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget(parent)
    , m_ui(std::make_unique<Ui::SettingsWidget>())
{
    m_ui->setupUi(this);
    m_ui->applicationSettingsTitleLabel->setHidden(true);
    m_ui->hardwareCheckBox->setHidden(true);
    m_ui->startupCheckBox->setHidden(true);

    m_ui->runAccPushButton->setHidden(!std::filesystem::exists("/usr/bin/acc"));
    connect(m_ui->runAccPushButton, &QPushButton::clicked, this, []() { QProcess::startDetached("/usr/bin/acc", {}); });

    m_ui->runCommonSettingsPushButton->setHidden(true);
    std::map<std::string, std::string> desktopEnvironmentSettings = {
        {"plasma", "/usr/bin/systemsettings"},
        {"kde5", "/usr/bin/systemsettings"},
        {"kde6", "/usr/bin/systemsettings"},
        {"gnome", "/usr/bin/gnome-control-center"},
        {"mate", "/usr/bin/mate-control-center"},
        {"xfce", "/usr/bin/xfce4-settings-manager"},
        {"cinnamon", "/usr/bin/cinnamon-settings"},
    };

    // If edition selected and edition DE exists
    if (DataBuilder::product().m_edition.has_value()
        && DataBuilder::product().m_edition->m_desktopEnvironment.has_value())
    {
        const auto &editionDesktop = DataBuilder::product().m_edition->m_desktopEnvironment.value().toStdString();
        const auto &path = desktopEnvironmentSettings[editionDesktop];
        desktopSettingsConnectorHelper(path);
    }

    // DE Settings by current session
    auto desktopEnvValue = qgetenv("XDG_SESSION_DESKTOP").toStdString();
    if (desktopEnvValue.empty())
    {
        desktopEnvValue = qgetenv("DESKTOP_SESSION").toStdString();
    }

    std::transform(desktopEnvValue.begin(), desktopEnvValue.end(), desktopEnvValue.begin(), [](char c) {
        return std::tolower(c);
    });
    if (!desktopEnvValue.empty())
    {
        const auto &path = desktopEnvironmentSettings[desktopEnvValue];
        desktopSettingsConnectorHelper(path);
    }
    else
    {
        // Fallback if current session is unknown (first in)
        for (const auto &[de, path] : desktopEnvironmentSettings)
        {
            desktopSettingsConnectorHelper(path);
        }
    }
}

SettingsWidget::~SettingsWidget() = default;

void SettingsWidget::desktopSettingsConnectorHelper(std::string path)
{
    if (!path.empty() && std::filesystem::exists(path))
    {
        m_ui->runCommonSettingsPushButton->setHidden(false);
        connect(m_ui->runCommonSettingsPushButton, &QPushButton::clicked, [path]() {
            QProcess::startDetached(path.c_str(), {});
        });
    }
}
} // namespace alt
