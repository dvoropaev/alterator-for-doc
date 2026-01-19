#include "../core/logger/prelude.h"
#include "controller.h"
#include "mainwindow.h"
#include "model/model.h"

#include "../aobuilder/builders/aobuilderimpl.h"
#include "../aobuilder/constants.h"
#include "../aobuilder/datasource/datasourcedbusimpl.h"

#include <iostream>
#include <memory>
#include <QDebug>
#include <QStandardItemModel>
#include <QTranslator>
#include <QApplication>

#include <QCommandLineParser>
#include <KDBusService>
#include <KWindowSystem>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include "sysexits.h"
#include <QThread>

int main(int argc, char *argv[])
{
    auto loggerManager = ab::logger::LoggerManager::globalInstance();

    loggerManager->addLogger<ab::logger::ConsoleLogger>(QtDebugMsg);
    loggerManager->addLogger<ab::logger::FileLogger>(QtWarningMsg);
    loggerManager->addLogger<ab::logger::SyslogLogger>(LOG_LEVEL_DISABLED);

    QApplication app(argc, argv);
    app.setOrganizationDomain("altlinux.org");
    app.setOrganizationName("AltLinux");
    app.setApplicationName("alterator-explorer");


    {
        // NOTE: until there is no any non-closable window, we can't handle this other way.
        auto session = QDBusConnection::sessionBus();
        if ( !session.isConnected() )
        {
            qCritical() << QObject::tr("Unable to connect to a session bus");
            return EX_UNAVAILABLE;
        }

        auto* interface = session.interface();
        if ( interface->isServiceRegistered("org.altlinux.alterator-explorer") )
            qWarning() << QObject::tr("Application already running");
    }

    QCommandLineParser parser;
    auto replace = QCommandLineOption{"replace", QObject::tr("Close existing window and open a new one")};
    parser.addOption(replace);
    auto help = parser.addHelpOption();

    KDBusService::StartupOptions options{KDBusService::Unique | KDBusService::NoExitOnFailure};
    if ( parser.isSet(replace) )
    {
        qWarning() << QObject::tr("Attempting to replace existing window...");
        options.setFlag(KDBusService::Replace);
    }

    KDBusService service{options};

    if ( !service.isRegistered() )
    {
        qWarning() << QObject::tr("Replacing window failed. You should close it manually");
        return 4;
    }

    QLocale locale;
    QTranslator translator;
    QString language = locale.system().name().split("_").at(0);
    if (!translator.load(language, ":/"))
        qWarning() << "failed to install translator";
    app.installTranslator(&translator);

    auto mainWindow = std::make_shared<ab::MainWindow>();
    mainWindow->show();

    QObject::connect(&service, &KDBusService::activateRequested, &app, [mainWindow]
    {
        mainWindow->show();
        KWindowSystem::updateStartupId(mainWindow->windowHandle());
        mainWindow->raise();
        KWindowSystem::activateWindow(mainWindow->windowHandle());
    });

    auto model = std::make_unique<ab::model::Model>();
    auto dataSource = std::make_unique<ao_builder::DataSourceDBusImpl>(ao_builder::dbus::SERVICE_NAME);
    auto modelBuilder = std::make_unique<ao_builder::AOBuilderImpl>();
    ab::Controller controller(mainWindow, std::move(model), std::move(dataSource), std::move(modelBuilder));

    mainWindow->setController(&controller);

    auto* initThread = QThread::create(&ab::Controller::buildModel, &controller);
    QObject::connect(initThread, &QThread::finished, initThread, &QThread::deleteLater);
    initThread->start();

    return app.exec();
}
