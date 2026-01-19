#include "ServicesApp.h"

#include "controller/Controller.h"
#include "ui/MainWindow.h"
#include "RichToolTipEventFilter.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <KDBusService>
#include <KWindowSystem>

#include "version.h"

#include <QDir>
#include <QCommandLineParser>
#include <QThread>
#include "sysexits.h"


class ServicesApp::Private {
public:
    AppSettings m_settings;
    std::unique_ptr<Controller> m_controller;
    std::unique_ptr<MainWindow> m_window;
    bool m_started{false};

    bool m_mayQuit{true};
};

ServicesApp::ServicesApp(int& argc, char** argv)
    : QApplication{argc, argv}
    , d{ new Private }
{
    setDesktopFileName("org.altlinux.alt-services");
    setOrganizationName("ALTLinux");
    setOrganizationDomain("altlinux.org");
    setApplicationName("alt-services");
    setApplicationVersion(getApplicationVersion());

    installEventFilter(new RichToolTipEventFilter{this});
}

ServicesApp::~ServicesApp() { delete d; }

int ServicesApp::run()
{
    QCommandLineParser parser;
    auto replace = QCommandLineOption{"replace", tr("Close existing window and open a new one")};
    auto object  = QCommandLineOption{{"o", "object"}, tr("Go to a specific service, specified by dbus path"), tr("path")};
    parser.addOptions({replace, object});
    auto help = parser.addHelpOption();
    parser.process(arguments());

    {
        // NOTE: until there is no any non-closable window, we can't handle this other way.
        auto session = QDBusConnection::sessionBus();
        if ( !session.isConnected() )
        {
            qCritical() << tr("Unable to connect to a session bus");
            return EX_UNAVAILABLE;
        }

        auto* interface = session.interface();
        if ( interface->isServiceRegistered("org.altlinux.alt-services") )
            qWarning() << tr("Application already running");
    }

    KDBusService::StartupOptions options{KDBusService::Unique | KDBusService::NoExitOnFailure};
    if ( parser.isSet(replace) )
    {
        qWarning() << tr("Attempting to replace existing window...");
        options.setFlag(KDBusService::Replace);
    }

    KDBusService service(options);

    if ( !service.isRegistered() )
    {
        qWarning() << tr("Replacing window failed. You should close it manually");
        return 4;
    }

    auto activate = [&, this](const QStringList& arguments, const QString& workingDirectory)
    {
        if ( d->m_started )
        {
            parser.process(arguments);
            raiseMainWindow();
        }

        if ( parser.isSet(object) )
            d->m_controller->selectByPath(parser.value(object));
    };

    connect(&service, &KDBusService::activateRequested, this, activate);

    d->m_controller.reset(new Controller);

    d->m_window.reset(new MainWindow);
    d->m_window->show();

    auto* t = QThread::create(&Controller::refresh, d->m_controller.get());
    connect(this, &QApplication::aboutToQuit, t, &QThread::quit);
    connect(t, &QThread::finished, this, [=]
    {
        activate(arguments(), QDir::current().absolutePath());
        d->m_started = true;
        t->deleteLater();
    });
    t->start();

    return exec();
}

AppSettings* ServicesApp::settings() { return &d->m_settings; }
Controller* ServicesApp::controller() { return d->m_controller.get(); }

bool ServicesApp::event(QEvent* event)
{
    if ( event->type() == QEvent::Quit && !d->m_mayQuit )
    {
        qDebug() << "rejecting quit: operation in progress!";
        event->ignore();
        return false;
    }
    return QApplication::event(event);
}


void ServicesApp::raiseMainWindow()
{
    d->m_window->show();
    KWindowSystem::updateStartupId(d->m_window->windowHandle());
    d->m_window->raise();
    KWindowSystem::activateWindow(d->m_window->windowHandle());
}

ServicesApp::QuitLock::QuitLock(bool& flag) : m_flag{flag} { m_flag = false; }

ServicesApp::QuitLock::~QuitLock() { m_flag = true; }

ServicesApp::QuitLock ServicesApp::quitLock()
{ return QuitLock{d->m_mayQuit}; }
