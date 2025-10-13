#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>

#include "app/ServicesApp.h"

int main(int argc, char *argv[])
{
    ServicesApp app(argc, argv);

    if ( app.isRunning() ) {
        qWarning() << "application is already running";
        return 0;
    }

    QTranslator systemTranslator;
    if ( systemTranslator.load("qt_" + QLocale::languageToCode(QLocale::system().language()),
                                QLibraryInfo::path(QLibraryInfo::TranslationsPath)) )
        app.installTranslator(&systemTranslator);

    QTranslator internalTranslator;
    QString language = QLocale::system().name();
    if ( internalTranslator.load(language, ":/") )
        app.installTranslator(&internalTranslator);

    return app.run();
}
