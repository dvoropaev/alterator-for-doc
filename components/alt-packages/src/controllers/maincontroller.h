#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <memory>
#include <QAction>
#include <QObject>

class DynamicTranslator;
class AptController;
class RpmController;
class RepoController;
class DBusDataSource;
class MainWindow;
class WaitDialog;
class ErrorDialog;

class MainController : public QObject
{
    Q_OBJECT

public:
    MainController();
    ~MainController();

public:
    void initMainWindow();
    void setActive(const QString &objectPath);

private:
    void retranslate(const QLocale &locale);
    static std::unique_ptr<DynamicTranslator> buildTranslator();
    QList<QAction *> buildActionsForLanguageMenu();

    friend class BaseController;

private slots:
    void onUpdateDataRequested();
    void onLanguageSelected(QAction *action);
    void showError(int, const QStringList &);
    void openWaitDialog();
    void appendMessageToWaitDialog(const QString &message);
    void closeWaitDialog();

public slots:
    void onAboutDialogRequested();
    void onManualRequested();

private:
    MainController(const MainController &)            = delete;
    MainController(MainController &&)                 = delete;
    MainController &operator=(const MainController &) = delete;
    MainController &operator=(MainController &&)      = delete;

private:
    std::unique_ptr<DynamicTranslator> m_translator;
    std::shared_ptr<MainWindow> m_mainWindow;
    std::unique_ptr<WaitDialog> m_waitDialog;
    std::unique_ptr<ErrorDialog> m_errorDialog;
    std::shared_ptr<DBusDataSource> m_dataSource;
    std::unique_ptr<AptController> m_aptController;
    std::unique_ptr<RpmController> m_rpmController;
    std::unique_ptr<RepoController> m_repoController;
};

Q_DECLARE_METATYPE(std::function<QString()>);

#endif // MAINCONTROLLER_H
