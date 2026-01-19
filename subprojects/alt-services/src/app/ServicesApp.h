#pragma once

#include <QApplication>
#include <QObject>
#include "AppSettings.h"
#include "data/Service.h"

class Service;
class Controller;

class ServicesApp : public QApplication
{
    Q_OBJECT
public:
    ServicesApp(int& argc, char** argv);
    ~ServicesApp();

    int run();

    AppSettings* settings();
    Controller* controller();

    inline static ServicesApp* instance() {
        return (ServicesApp*)QApplication::instance();
    }

    bool event(QEvent* event) override;

    class QuitLock
    {
    private:
        bool& m_flag;
    protected:
        friend class ServicesApp;

        QuitLock(bool& flag);
    public:
        ~QuitLock();
    };
    [[nodiscard]] QuitLock quitLock();

private slots:
    void raiseMainWindow();

private:
    class Private;
    Private* d;
};

#if defined(qApp)
#undef qApp
#endif
#define qApp (static_cast<ServicesApp *>(QCoreApplication::instance()))
