#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QShortcut>

#include <memory>

class QAbstractItemModel;

namespace alt
{
namespace Ui
{
class MainWindow;
} // namespace Ui

class MainController;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    void connect(MainController *controller);
    void setHostName(const QString &name);
    void setOsLogo(const QPixmap &logo);
    void setOsModel(QAbstractItemModel *model);
    void setHardwareModel(QAbstractItemModel *model);
    void setTabUsefulInformation();

private:
    MainWindow(const MainWindow &) = delete;
    MainWindow(MainWindow &&) = delete;
    MainWindow &operator=(const MainWindow &) = delete;
    MainWindow &operator=(MainWindow &&) = delete;

private:
    std::unique_ptr<Ui::MainWindow> m_ui;
    QShortcut m_quitShortcut;
};
} // namespace alt
#endif // MAINWINDOW_H
