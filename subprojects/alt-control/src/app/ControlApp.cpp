#include "ControlApp.h"

#include "controller/Controller.h"
#include "ui/MainWindow.h"

#include <QLocale>
#include <QTranslator>

#include "version.h"

class ControlApp::Private {
public:
    AppSettings m_settings;
    Controller  m_controller;
};

ControlApp::ControlApp(int& argc, char** argv)
    : QApplication{argc, argv}
    , d{new Private}
{
    setOrganizationName("ALTLinux");
    setOrganizationDomain("altlinux.org");
    setApplicationName("alt-control");
    setApplicationVersion(getApplicationVersion());
}

ControlApp::~ControlApp() { delete d; }

AppSettings& ControlApp::settings() { return d->m_settings; }

int ControlApp::exec()
{
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        if (translator.load(":/i18n/" + QLocale(locale).name())) {
            installTranslator(&translator);
            break;
        }
    }
    QString language = QLocale::system().name();
    if ( translator.load(language, ":/") )
        installTranslator(&translator);

    MainWindow w;
    w.show();

    w.setController(&d->m_controller);
    d->m_controller.refresh();

    return QApplication::exec();
}
