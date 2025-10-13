#include "application.h"

#include "app/version.h"
#include "commandlineparser.h"

#include "controllers/maincontroller.h"

#include "constants.h"

#include <iostream>
#include <QDebug>
#include <QLibraryInfo>
#include <QStandardItem>
#include <QTranslator>

QLocale Application::m_locale = QLocale::system();

Application::Application(int argc, char **argv)
    : QApplication(argc, argv)
    , m_mainController(std::make_unique<MainController>())
    , m_parser(std::make_unique<CommandLineParser>(*this))
    , m_options(std::make_unique<CommandLineOptions>())
    , m_parseErrorMessage()
    , m_parserResult(m_parser->parseCommandLine(m_options.get(), &m_parseErrorMessage))
{
    setOrganizationName(QCoreApplication::translate("main", "BaseALT"));
    setOrganizationDomain("altlinux.org");
    setApplicationName("alt-packages");
    setApplicationVersion(getVersion());
}

Application::~Application() {}

int Application::exec()
{
    if (m_parserResult == CommandLineParser::CommandLineParseResult::CommandLineError)
    {
        std::cerr << m_parseErrorMessage.toStdString() << std::endl;
        m_parser->showHelp();
        return 1;
    }

    if (m_parserResult == CommandLineParser::CommandLineParseResult::CommandLineHelpRequested)
    {
        m_parser->showHelp();
        return 0;
    }

    if (m_parserResult == CommandLineParser::CommandLineParseResult::CommandLineVersionRequested)
    {
        m_parser->showVersion();
        return 0;
    }

    m_mainController->initMainWindow();

    QString objectName = APT_OBJECT_PATH; // APT by default
    if (m_parserResult == CommandLineParser::CommandLineParseResult::CommandLineObjectRequested)
    {
        objectName = std::move(m_options->objectName);
    }

    m_mainController->setActive(objectName);

    return QApplication::exec();
}

QLocale Application::applicationLocale()
{
    return m_locale;
}

void Application::setApplicationLocale(const QLocale &locale)
{
    m_locale = locale;
}
