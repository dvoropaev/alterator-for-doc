#include "Controller.h"
#include "MainWindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        if (translator.load(":/i18n/" + QLocale(locale).name())) {
            a.installTranslator(&translator);
            break;
        }
    }
    QString language = QLocale::system().name();
    if ( translator.load(language, ":/") )
        a.installTranslator(&translator);

    MainWindow w;
    w.show();

    Controller c;
    w.setController(&c);
    c.refresh();

    return a.exec();
}
