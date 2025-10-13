/***********************************************************************************************************************
**
** Copyright (C) 2023 BaseALT Ltd. <org@basealt.ru>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
***********************************************************************************************************************/

#include "adtapp.h"
#include "../controllers/basecontroller.h"
#include "../controllers/clicontroller.h"
#include "../controllers/guicontroller.h"
#include "../model/builder/adtmodelbuilderdbus.h"
#include "../model/builder/dbusconfig.h"
#include "../model/builder/dbusservicewatcher.h"
#include "adtcommandlineoptions.h"
#include "adtcommandlineparser.h"

#include <memory>
#include <QDebug>
#include <QtGlobal>

typedef ADTCommandLineParser::CommandLineParseResult CommandLineParseResult;

class ADTAppPrivate
{
public:
    ADTAppPrivate(QApplication *app, ADTSettings *settings, QDBusConnection conn, QString locale)
        : m_application(app)
        , m_settings(settings)
        , m_locale(locale)
        , m_options(std::make_unique<ADTCommandLineOptions>())
        , m_parser(std::make_unique<ADTCommandLineParser>(*app))
        , m_modelBuilder(std::make_unique<ADTModelBuilderDbus>())
        , m_dbusConnection(conn)
        , m_managerWatcher(std::make_unique<ADTServiceChecker>(DBusConfig{}.managerService,
                                                               DBusConfig{}.managerPath,
                                                               DBusConfig{}.managerInterface))
    {}

    QApplication *m_application{};
    ADTSettings *m_settings{};
    QString m_locale{};

    std::unique_ptr<ADTCommandLineOptions> m_options{};
    std::unique_ptr<ADTCommandLineParser> m_parser{};
    ADTCommandLineParser::CommandLineParseResult m_parserResult{};
    QString m_parseErrorMessage{};

    std::shared_ptr<TreeModel> m_model{};
    std::shared_ptr<ADTModelBuilderInterface> m_modelBuilder{};
    std::unique_ptr<BaseController> m_controller{};

    DBusConfig m_dbusConfig{};
    QDBusConnection m_dbusConnection;
    std::unique_ptr<ADTServiceChecker> m_managerWatcher{};

private:
    ADTAppPrivate(const ADTAppPrivate &)            = delete;
    ADTAppPrivate(ADTAppPrivate &&)                 = delete;
    ADTAppPrivate &operator=(const ADTAppPrivate &) = delete;
    ADTAppPrivate &operator=(ADTAppPrivate &&)      = delete;
};

ADTApp::ADTApp(QApplication *application, ADTSettings *settings, QDBusConnection conn, QString locale)
    : d(new ADTAppPrivate(application, settings, conn, locale))
{}

ADTApp::~ADTApp()
{
    delete d;
}

int ADTApp::runApp()
{
    d->m_parserResult = d->m_parser->parseCommandLine(d->m_options.get(), &d->m_parseErrorMessage);

    if (d->m_parserResult == CommandLineParseResult::CommandLineError)
    {
        qWarning() << d->m_parseErrorMessage;
        d->m_parser->showHelp();
        return 1;
    }

    if (d->m_parserResult == CommandLineParseResult::CommandLineHelpRequested)
    {
        d->m_parser->showHelp();
        return 0;
    }

    if (d->m_parserResult == CommandLineParseResult::CommandLineVersionRequested)
    {
        d->m_parser->showVersion();
        return 0;
    }

    buildModel();

    if (d->m_options->useGraphic)
    {
        // GUI mode
        d->m_controller = std::make_unique<GuiController>(d->m_model,
                                                          d->m_dbusConfig,
                                                          d->m_settings,
                                                          d->m_options.get(),
                                                          d->m_application);
    }
    else
    {
        // CLI mode
        d->m_controller = std::make_unique<CliController>(d->m_model,
                                                          d->m_dbusConfig,
                                                          d->m_settings,
                                                          d->m_options.get());
    }

    connect(d->m_managerWatcher.get(),
            &ADTServiceChecker::serviceOwnerChanged,
            d->m_controller.get(),
            &BaseController::on_serviceOwnerChanged);
    connect(d->m_managerWatcher.get(),
            &ADTServiceChecker::serviceRegistered,
            d->m_controller.get(),
            &BaseController::on_serviceRegistered);
    connect(d->m_managerWatcher.get(),
            &ADTServiceChecker::serviceUnregistered,
            d->m_controller.get(),
            &BaseController::on_serviceUnregistered);

    return d->m_controller->runApp();
}

void ADTApp::buildModel()
{
    d->m_model = d->m_modelBuilder->buildModel();
    d->m_model->setLocaleForElements(d->m_locale);
}
