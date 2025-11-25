#include "application/application.h"

#include <QDebug>
#include <QTranslator>
#include <qbase/logger/prelude.h>

int main(int argc, char *argv[])
{
    auto *loggerManager = qbase::logger::LoggerManager::globalInstance();

    loggerManager->addLogger<qbase::logger::ConsoleLogger>(QtDebugMsg);
    loggerManager->addLogger<qbase::logger::FileLogger>(QtWarningMsg, "alt-components", "alt-components.log");
    loggerManager->addLogger<qbase::logger::SyslogLogger>(LOG_LEVEL_DISABLED, "alt-components");

    auto app = std::make_unique<alt::Application>(argc, argv);

    return app->exec();
}
