#include "../core/logger/prelude.h"
#include "controller.h"
#include "mainwindow.h"
#include "model/model.h"

#include "../aobuilder/builders/aobuilderimpl.h"
#include "../aobuilder/constants.h"
#include "../aobuilder/datasource/datasourcedbusimpl.h"

#include <iostream>
#include <memory>
#include <QtSolutions/QtSingleApplication>
#include <QDebug>
#include <QStandardItemModel>
#include <QTranslator>

int main(int argc, char *argv[])
{
    auto loggerManager = ab::logger::LoggerManager::globalInstance();

    loggerManager->addLogger<ab::logger::ConsoleLogger>(QtDebugMsg);
    loggerManager->addLogger<ab::logger::FileLogger>(QtWarningMsg);
    loggerManager->addLogger<ab::logger::SyslogLogger>(LOG_LEVEL_DISABLED);

    QtSingleApplication app(argc, argv);

    if ( app.isRunning() ) {
        qWarning() << "application is already running";
        return 0;
    }

    QLocale locale;
    QTranslator translator;
    QString language = locale.system().name().split("_").at(0);
    if (!translator.load(language, ":/"))
        qWarning() << "failed to install translator";
    app.installTranslator(&translator);

    auto mainWindow = std::make_shared<ab::MainWindow>();
    app.setActivationWindow(mainWindow.get());

    auto model = std::make_unique<ab::model::Model>();
    auto dataSource = std::make_unique<ao_builder::DataSourceDBusImpl>(ao_builder::dbus::SERVICE_NAME);
    auto modelBuilder = std::make_unique<ao_builder::AOBuilderImpl>();
    ab::Controller controller(mainWindow, std::move(model), std::move(dataSource), std::move(modelBuilder));
    mainWindow->setController(&controller);
    mainWindow->show();

    return app.exec();
}
