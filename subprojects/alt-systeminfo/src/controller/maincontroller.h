#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QSortFilterProxyModel>

namespace alt
{
class MainWindow;
class LicenseDialog;
} // namespace alt

namespace alt
{
class MainController : public QObject
{
public:
    MainController();
    ~MainController() override;

public:
    void showMainWindow();
    void showLicenseDialog();
    void showReleaseNotes();

public:
    void onLicenseDialogRequested();

private:
    MainController(const MainController &) = delete;
    MainController(MainController &&) = delete;
    MainController &operator=(const MainController &) = delete;
    MainController &operator=(MainController &&) = delete;

private:
    std::unique_ptr<QSortFilterProxyModel> m_hardwareModel;
    std::unique_ptr<QSortFilterProxyModel> m_osModel;
    std::unique_ptr<MainWindow> m_mainWindow;
    std::unique_ptr<LicenseDialog> m_licenseDialog;
};
} //namespace alt

#endif // MAINCONTROLLER_H
