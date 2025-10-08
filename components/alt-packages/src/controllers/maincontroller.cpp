#include "maincontroller.h"

#include "app/application.h"

#include "aptcontroller.h"
#include "dynamictranslator.h"
#include "repocontroller.h"
#include "rpmcontroller.h"

#include "constants.h"
#include "datasource/dbusdatasource.h"

#include "ui/mainwindow/mainwindow.h"

#include "ui/dialogs/aboutdialog.h"
#include "ui/dialogs/errordialog.h"
#include "ui/dialogs/waitdialog.h"

#include <functional>
#include <QActionGroup>
#include <QDesktopServices>
#include <QPair>
#include <QUrl>

#if QT_VERSION_MAJOR == 6
#define libraryPath(PATH) (QLibraryInfo::path(PATH))
#else
#define libraryPath(PATH) (QLibraryInfo::location(PATH))
#endif

MainController::MainController()
    : m_translator(buildTranslator())
    , m_mainWindow(nullptr)
    , m_waitDialog(nullptr)
    , m_errorDialog(nullptr)
    , m_dataSource(std::make_shared<DBusDataSource>(ALTERATOR_SERVICE_NAME,
                                                    ALTERATOR_PATH,
                                                    ALTERATOR_INTERFACE_NAME,
                                                    ALTERATOR_GETOBJECTS_METHOD_NAME))
    , m_aptController(nullptr)
    , m_rpmController(nullptr)
    , m_repoController(nullptr)
{}

MainController::~MainController() = default;

void MainController::initMainWindow()
{
    m_mainWindow = std::make_shared<MainWindow>();
    m_mainWindow->addActionsToLanguageMenu(buildActionsForLanguageMenu());

    m_mainWindow->show();
    m_waitDialog  = std::make_unique<WaitDialog>(m_mainWindow.get());
    m_errorDialog = std::make_unique<ErrorDialog>(m_mainWindow.get());

    m_aptController  = std::make_unique<AptController>(m_dataSource.get(), this);
    m_rpmController  = std::make_unique<RpmController>(m_dataSource.get(), this);
    m_repoController = std::make_unique<RepoController>(m_dataSource.get(), this);

    m_mainWindow->connect(this);
    m_mainWindow->connect(m_aptController.get());
    m_mainWindow->connect(m_rpmController.get());
    m_mainWindow->connect(m_repoController.get());

    connect(m_aptController.get(), &BaseController::showLoadingIndicator, m_waitDialog.get(), &WaitDialog::setProgressBarVisible);

    m_aptController->setWindow(m_mainWindow);
    m_rpmController->setWindow(m_mainWindow);
    m_repoController->setWindow(m_mainWindow);
}

void MainController::onUpdateDataRequested()
{
    m_aptController->updateModel();
    m_rpmController->updateModel();
    m_repoController->updateModel();
}

void MainController::onAboutDialogRequested()
{
    auto dialog = std::make_unique<AboutDialog>(m_mainWindow.get());
    dialog->setVersion(QApplication::applicationVersion());

    dialog->exec();
}

void MainController::onManualRequested()
{
    const QUrl manualUrl("https://www.altlinux.org/AMP");
    QDesktopServices::openUrl(manualUrl);
}

void MainController::setActive(const QString &objectPath)
{
    m_mainWindow->setActive(objectPath);
}

void MainController::showError(int code, const QStringList &log)
{
    if (m_waitDialog && m_waitDialog->isEmpty())
    {
        m_waitDialog->close();
    }
    m_errorDialog->showError(code, log);
}

void MainController::openWaitDialog()
{
    m_waitDialog->clearText();
    m_waitDialog->exec();
}

void MainController::appendMessageToWaitDialog(const QString &message)
{
    m_waitDialog->appendText(message);
}

void MainController::closeWaitDialog()
{
    m_waitDialog->close();
}

std::unique_ptr<DynamicTranslator> MainController::buildTranslator()
{
    auto translator = std::make_unique<DynamicTranslator>();

    auto languages = {
        QLocale::English,
        QLocale::Russian,
    };

    for (auto language : languages)
    {
        auto appTranslator = std::make_unique<QTranslator>();
        if (!appTranslator->load(QLocale(language), ":/"))
        {
            qWarning() << "qtapp translator failed";
        }

        auto qtTranslator = std::make_unique<QTranslator>();

        if (!qtTranslator->load(QString("qtbase_%1").arg(QLocale(language).name()),
                                libraryPath(QLibraryInfo::TranslationsPath)))
        {
            qWarning() << "qtbase translator failed";
        }

        auto translators = std::make_unique<DynamicTranslator::Translators>();
        *translators     = {appTranslator.release(), qtTranslator.release()};

        translator->insert(language, std::move(translators));
    }

    // NOTE: This is temporary here until dynamic translations are ready.
    translator->retranslate();

    return translator;
}

QList<QAction *> MainController::buildActionsForLanguageMenu()
{
    auto languageGroup = std::make_unique<QActionGroup>(this).release();
    languageGroup->setExclusive(true);
    connect(languageGroup, &QActionGroup::triggered, this, &MainController::onLanguageSelected);

    std::map<QLocale::Language, std::function<QString()>> translatedLanguagesNames{
        {
            QLocale::Language::English,
            +[]() { return QString("&%1").arg(tr("English")); },
        },
        {
            QLocale::Language::Russian,
            +[]() { return QString("&%1").arg(tr("Russian")); },
        },
    };

    for (const auto &[language, trLanguageName] : translatedLanguagesNames)
    {
        auto languageAction = std::make_unique<QAction>();

        QLocale localeData(language);
        languageAction->setCheckable(true);
        languageAction->setText(QString("%1").arg(trLanguageName()));
        languageAction->setData(QVariant::fromValue(std::make_pair(localeData, trLanguageName)));

        if (localeData.language() == QLocale::system().language())
        {
            languageAction->setChecked(true);
        }

        languageGroup->addAction(languageAction.release());
    }

    return languageGroup->actions();
}

void MainController::onLanguageSelected(QAction *action)
{
    const auto &value  = action->data().value<std::pair<QLocale, std::function<QString()>>>();
    const auto &locale = value.first;

    retranslate(locale);
}

void MainController::retranslate(const QLocale &locale)
{
    m_translator->retranslate(locale);

    m_aptController->retranslate(locale);
    m_rpmController->retranslate(locale);
    m_repoController->retranslate(locale);
}
