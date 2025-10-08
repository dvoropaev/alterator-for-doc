#ifndef APPLICATION_H
#define APPLICATION_H

#include "controller/maincontroller.h"

#include <QApplication>

#include <memory>

namespace alt
{
enum CommandLineParseResult
{
    ParseResultOk,
    ParseResultLicenseRequested,
    ParseResultReleaseNotesRequested,
};

class Application : QApplication
{
public:
    explicit Application(int &argc, char **argv);

public:
    int exec();

public:
    Application(const Application &) = delete;
    Application(Application &&) = delete;
    Application &operator=(const Application &) = delete;
    Application &operator=(Application &&) = delete;

private:
    CommandLineParseResult parseCommandLine();
    void installTranslator(const QString &id, QTranslator *translator, const QString &locale, const QString &path);
    void setupTranslator();

private:
    std::unique_ptr<MainController> m_controller;
    CommandLineParseResult m_parseResult;
};
} // namespace alt

#endif // APPLICATION_H
