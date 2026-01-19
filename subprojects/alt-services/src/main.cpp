#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

#include "app/ServicesApp.h"

int main(int argc, char *argv[]) {
  ServicesApp app(argc, argv);

  QTranslator systemTranslator;
  if (systemTranslator.load(
          "qt_" + QLocale::languageToCode(QLocale::system().language()),
          QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
    app.installTranslator(&systemTranslator);

  QTranslator internalTranslator;
  QString language = QLocale::system().name();
  if (internalTranslator.load(language, ":/"))
    app.installTranslator(&internalTranslator);

  return app.run();
}
