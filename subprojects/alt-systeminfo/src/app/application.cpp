#include "application.h"

#include "app/version.h"

#include <QCommandLineParser>
#include <QDebug>
#include <QLibraryInfo>
#include <QTranslator>

namespace alt
{
Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_controller(nullptr)
    , m_parseResult(parseCommandLine())
{
    setupTranslator();

    m_controller = std::make_unique<MainController>();
}

CommandLineParseResult Application::parseCommandLine()
{
    setApplicationVersion(getVersion());
    QCommandLineParser parser;

    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();
    auto licenseOption = QCommandLineOption(QStringList() << "l" << "show-license", "Runs dialog with edition license");
    if (!parser.addOption(licenseOption))
    {
        qWarning() << "Failed to add `show-license` command line option";
    }
    auto releaseNotesOption = QCommandLineOption(QStringList() << "r" << "release-notes", "Runs with Release Notes tab");
    if (!parser.addOption(releaseNotesOption))
    {
        qWarning() << "Failed to add `release-notes` command line option";
    }

    parser.process(*this);

    if (parser.isSet(licenseOption))
    {
        return CommandLineParseResult::ParseResultLicenseRequested;
    }
    else if (parser.isSet(releaseNotesOption))
    {
        return CommandLineParseResult::ParseResultReleaseNotesRequested;
    }

    return CommandLineParseResult::ParseResultOk;
}

void Application::installTranslator(const QString &id,
                                    QTranslator *translator,
                                    const QString &locale,
                                    const QString &path)
{
    if (translator != nullptr)
    {
        QApplication::removeTranslator(translator);
        delete translator;
    }

    translator = new QTranslator(this);

    bool loaded = translator->load(locale, path);
    if (!loaded)
    {
        qWarning() << "Failed to load translations" << id;
    }

    bool installed = QApplication::installTranslator(translator);
    if (!installed)
    {
        qWarning() << "Failed to install translations" << id;
    }
}

void Application::setupTranslator()
{
    static QTranslator *appTranslator;
    static QTranslator *qtBaseTranslator;

    installTranslator("app", appTranslator, QLocale::system().name(), ":/");
    installTranslator("qt",
                      qtBaseTranslator,
                      "qtbase_" + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
}

int Application::exec()
{
    if (m_parseResult == CommandLineParseResult::ParseResultLicenseRequested)
    {
        m_controller->showLicenseDialog();
    }
    else if (m_parseResult == CommandLineParseResult::ParseResultReleaseNotesRequested)
    {
        m_controller->showReleaseNotes();
    }
    else
    {
        m_controller->showMainWindow();
    }

    return QApplication::exec();
}
} // namespace alt
