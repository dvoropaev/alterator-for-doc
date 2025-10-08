#include "commandlineparser.h"

#include <memory>
#include <QApplication>
#include <QCommandLineParser>

CommandLineParser::CommandLineParser(QApplication &application)
    : m_application(application)
    , m_parser(std::make_unique<QCommandLineParser>())
{}

CommandLineParser::~CommandLineParser() = default;

CommandLineParser::CommandLineParseResult CommandLineParser::parseCommandLine(CommandLineOptions *options,
                                                                              QString *errorMessage)
{
    //TO DO add translations
    const QCommandLineOption helpOption = m_parser->addHelpOption();

    const QCommandLineOption versionOption = m_parser->addVersionOption();

    const QCommandLineOption objectOption(QStringList() << "o"
                                                        << "object",
                                          QObject::tr("Object to be used."),
                                          "obj");

    m_parser->setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    m_parser->addOption(objectOption);

    if (!m_parser->parse(m_application.arguments()))
    {
        *errorMessage = m_parser->errorText();
        return CommandLineError;
    }

    if (m_parser->isSet(versionOption))
    {
        return CommandLineVersionRequested;
    }

    if (m_parser->isSet(helpOption))
    {
        return CommandLineHelpRequested;
    }

    if (m_parser->isSet(objectOption))
    {
        const QString objectName = m_parser->value(objectOption);

        if (objectName.isNull() || objectName.isEmpty())
        {
            *errorMessage = QObject::tr("Bad object name: ") + objectName;
            return CommandLineError;
        }
        options->objectName = objectName;

        return CommandLineObjectRequested;
    }

    return CommandLineOk;
}

void CommandLineParser::showHelp()
{
    m_parser->showHelp();
}

void CommandLineParser::showVersion()
{
    m_parser->showVersion();
}
