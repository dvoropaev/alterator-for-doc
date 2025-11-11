#include "application.h"
#include "controller/controller.h"

#include <QLibraryInfo>
#include <QTranslator>

namespace alt
{
Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
{
    QMetaType::registerConverter<Component *, Object *>();
    QMetaType::registerConverter<Category *, Object *>();
    QMetaType::registerConverter<Section *, Object *>();
    QMetaType::registerConverter<Tag *, Object *>();

    setupTranslator();
}

Application::~Application() = default;

QLocale Application::getLocale()
{
    return Application::currentLocale;
}

void Application::setLocale(const QLocale &locale)
{
    currentLocale = locale;
    setupTranslator();
    Controller::instance().changeLocale(locale);
}

int Application::exec()
{
    Controller::instance().init();
    return QApplication::exec();
}

void Application::setupTranslator()
{
    const auto installTranslator = [](const QString &id, auto &translator, const QString &lang, const QString &path) {
        if (translator != nullptr)
        {
            alt::Application::removeTranslator(translator);
            delete translator;
        }
        translator = new QTranslator(qApp);

        // NOTE(chernigin): qtbase does not have translations into en, it is en by default
        if (lang == "qtbase_en")
        {
            return;
        }

        if (!translator->load(lang, path))
        {
            auto message = QString("Failed to load translations %1").arg(id);
            qWarning().noquote() << message;
            Controller::instance().issueMessage(QtMsgType::QtInfoMsg, message);
        }

        if (!alt::Application::installTranslator(translator))
        {
            auto message = QString("Failed to install translations %1").arg(id);
            qWarning().noquote() << message;
            Controller::instance().issueMessage(QtMsgType::QtInfoMsg, message);
        }
    };

    static QTranslator *appTranslator;
    static QTranslator *qtBaseTranslator;

    const auto lang = alt::Application::getLocale().name().split("_")[0];
    installTranslator("app", appTranslator, lang, ":/");
    installTranslator("qt", qtBaseTranslator, "qtbase_" + lang, QLibraryInfo::path(QLibraryInfo::TranslationsPath));
}
} // namespace alt
