#ifndef AMPPARSER_H
#define AMPPARSER_H

#include <QApplication>
#include <QCommandLineParser>

struct CommandLineOptions
{
public:
    QString objectName{};
};

class CommandLineParser
{
public:
    enum CommandLineParseResult
    {
        CommandLineOk,
        CommandLineError,
        CommandLineVersionRequested,
        CommandLineHelpRequested,
        CommandLineObjectRequested
    };

public:
    CommandLineParser(QApplication &application);
    ~CommandLineParser();

public:
    CommandLineParseResult parseCommandLine(CommandLineOptions *options, QString *errorMessage);
    void showHelp();
    void showVersion();

private:
    CommandLineParser(const CommandLineParser &)            = delete;
    CommandLineParser(CommandLineParser &&)                 = delete;
    CommandLineParser &operator=(const CommandLineParser &) = delete;
    CommandLineParser &operator=(CommandLineParser &&)      = delete;

private:
    QApplication &m_application;
    std::unique_ptr<QCommandLineParser> m_parser;
};

#endif // AMPPARSER_H
