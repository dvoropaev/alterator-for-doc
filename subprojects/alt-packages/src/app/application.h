#ifndef AMPAPP_H
#define AMPAPP_H

#include "commandlineparser.h"
#include <memory>
#include <QApplication>

class MainController;

class Application : public QApplication
{
public:
    explicit Application(int argc, char **argv);
    ~Application();

public:
    int exec();
    static void setApplicationLocale(const QLocale &locale);
    static QLocale applicationLocale();

private:
    Application(const Application &)            = delete;
    Application(Application &&)                 = delete;
    Application &operator=(const Application &) = delete;
    Application &operator=(Application &&)      = delete;

private:
    static QLocale m_locale;
    std::unique_ptr<MainController> m_mainController;
    std::unique_ptr<CommandLineParser> m_parser;
    std::unique_ptr<CommandLineOptions> m_options;
    QString m_parseErrorMessage;
    CommandLineParser::CommandLineParseResult m_parserResult;
};

#endif // AMPAPP_H
