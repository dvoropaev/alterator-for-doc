#include "dynamictranslator.h"

#include "app/application.h"

#include "constants.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>

static void setupSystemBusLocale()
{
    static QDBusInterface interface(ALTERATOR_SERVICE_NAME,
                                    ALTERATOR_PATH,
                                    ALTERATOR_INTERFACE_NAME,
                                    QDBusConnection::systemBus());
    if (!interface.isValid())
    {
        qWarning() << "Invalid manager interface";
    }

    QDBusMessage reply = interface.call(MANAGER_SET_ENV_VALUE_METHOD_NAME,
                                        "LC_ALL",
                                        QString("%1.UTF-8").arg(Application::applicationLocale().name()));

    if (reply.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "Invalid DBus reply: " << reply.errorMessage();
    }
}

DynamicTranslator::DynamicTranslator()
    : m_translators()
    , m_currentTranslators(nullptr)
{}

DynamicTranslator::~DynamicTranslator() = default;

void DynamicTranslator::retranslate(const QLocale &locale)
{
    auto newTranslators = m_translators[locale.language()].get();
    if (newTranslators == m_currentTranslators)
    {
        return;
    }

    if (m_currentTranslators)
    {
        QApplication::removeTranslator(m_currentTranslators->m_appTranslator);
        QApplication::removeTranslator(m_currentTranslators->m_qtTranslator);
    }

    m_currentTranslators = newTranslators;
    QApplication::installTranslator(m_currentTranslators->m_appTranslator);
    QApplication::installTranslator(m_currentTranslators->m_qtTranslator);

    Application::setApplicationLocale(locale);

    setupSystemBusLocale();
}

void DynamicTranslator::insert(QLocale::Language key, std::unique_ptr<Translators> value)
{
    m_translators.insert({key, std::move(value)});
}
