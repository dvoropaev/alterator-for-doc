#include "dialog.h"

#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>

#include <memory>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    QTranslator translator;
    QString language = QLocale::system().name();
    translator.load(language, ":/");
    application.installTranslator(&translator);

    QTranslator baseTranslator;
    baseTranslator.load("qt_"+language.split("_")[0], QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    application.installTranslator(&baseTranslator);

    auto dialog = std::make_unique<alt::Dialog>();
    QObject::connect(dialog.get(), &QDialog::rejected, []() { exit(1); });
    dialog->show();

    return application.exec();
}
