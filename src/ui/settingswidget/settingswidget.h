#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>

#include <memory>

namespace alt
{
namespace Ui
{
class SettingsWidget;
} // namespace Ui

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    SettingsWidget(QWidget *parent = nullptr);
    ~SettingsWidget();

private:
    void desktopSettingsConnectorHelper(std::string path);

private:
    SettingsWidget(const SettingsWidget &) = delete;
    SettingsWidget(SettingsWidget &&) = delete;
    SettingsWidget &operator=(const SettingsWidget &) = delete;
    SettingsWidget &operator=(SettingsWidget &&) = delete;

private:
    std::unique_ptr<Ui::SettingsWidget> m_ui;
};
} // namespace alt
#endif // SETTINGSWIDGET_H
