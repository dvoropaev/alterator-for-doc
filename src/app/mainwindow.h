#ifndef AB_MAIN_WINDOW_H
#define AB_MAIN_WINDOW_H

#include "model/model.h"

#include <QMainWindow>
#include <QStandardItemModel>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

namespace ab
{
class CategoryWidget;
class Controller;
class MainWindowSettings;
class PushButton;

class MainWindowPrivate;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public:
    MainWindow(const MainWindow &) = delete;
    MainWindow(MainWindow &&) = delete;
    MainWindow &operator=(const MainWindow &) = delete;
    MainWindow &operator=(MainWindow &&) = delete;

public:
    void closeEvent(QCloseEvent *event) override;
    bool event(QEvent* event) override;
    void changeEvent(QEvent*) override;

    void setController(Controller *c);

    void setModel(model::Model *m);
    void clearUi();

    void onModuleClicked(PushButton *button);

    void showInfo();

    void loadBranding();
    void onColorSchemeChanged();

    bool isBrandingEnabled();
    void setBrandingEnabled(bool how);

private slots:
    void preserveStyle();

private: signals:
    void styleChanged();

private:
    MainWindowPrivate *d;
};
} // namespace ab

#endif // AB_MAIN_WINDOW_H
