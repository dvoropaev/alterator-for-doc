#include "managerinterface.h"

#include "utility/constants.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

namespace alt
{
void ManagerInterface::setEnvValue(const QString &env, const QString &value)
{
    static QDBusInterface interface{ALTERATOR_SERVICE_NAME,
                                    ALTERATOR_OBJECT_PATH,
                                    ALTERATOR_MANAGER_INTERFACE_NAME,
                                    QDBusConnection::systemBus()};
    if (!interface.isValid())
    {
        qWarning() << "Invalid manager interface";
    }

    const QDBusMessage &reply = interface.call(ALTERATOR_MANAGER_SET_ENV_VALUE_METHOD_NAME, env, value);

    if (reply.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "Invalid DBus reply: " << reply.errorMessage();
    }

    if (reply.arguments().last().toBool())
    {
        qDebug() << QString("'%1.%2' returned exit code %3 with arguments (%4, %5)")
                        .arg(ALTERATOR_MANAGER_INTERFACE_NAME)
                        .arg(ALTERATOR_MANAGER_SET_ENV_VALUE_METHOD_NAME)
                        .arg(reply.arguments().last().toInt())
                        .arg(env)
                        .arg(value);
    }
}

void ManagerInterface::unsetEnvValue(const QString &env)
{
    static QDBusInterface interface{ALTERATOR_SERVICE_NAME,
                                    ALTERATOR_OBJECT_PATH,
                                    ALTERATOR_MANAGER_INTERFACE_NAME,
                                    QDBusConnection::systemBus()};
    if (!interface.isValid())
    {
        qWarning() << "Invalid manager interface";
    }

    const QDBusMessage &reply = interface.call(ALTERATOR_MANAGER_UNSET_ENV_VALUE_METHOD_NAME, env);

    if (reply.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "Invalid DBus reply: " << reply.errorMessage();
    }

    if (reply.arguments().last().toBool())
    {
        qDebug() << QString("'%1.%2' returned exit code %3 with arguments (%4)")
                        .arg(ALTERATOR_MANAGER_INTERFACE_NAME)
                        .arg(ALTERATOR_MANAGER_UNSET_ENV_VALUE_METHOD_NAME)
                        .arg(reply.arguments().last().toInt())
                        .arg(env);
    }
}

std::optional<QString> ManagerInterface::getEnvValue(const QString &env)
{
    static QDBusInterface interface{ALTERATOR_SERVICE_NAME,
                                    ALTERATOR_OBJECT_PATH,
                                    ALTERATOR_MANAGER_INTERFACE_NAME,
                                    QDBusConnection::systemBus()};
    if (!interface.isValid())
    {
        qWarning() << "Invalid manager interface";
    }

    const QDBusMessage &reply = interface.call(ALTERATOR_MANAGER_GET_ENV_VALUE_METHOD_NAME, env);

    if (reply.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "Invalid DBus reply: " << reply.errorMessage();
    }

    if (reply.arguments().last().toBool())
    {
        qDebug() << QString("%1.%2 returned exit code %3 with arguments ()")
                        .arg(ALTERATOR_MANAGER_INTERFACE_NAME)
                        .arg(ALTERATOR_MANAGER_GET_ENV_VALUE_METHOD_NAME)
                        .arg(reply.arguments().last().toInt());
    }

    return reply.arguments().first().toString();
}

QStringList ManagerInterface::getObjects(const QString &iface)
{
    static QDBusInterface interface{ALTERATOR_SERVICE_NAME,
                                    ALTERATOR_OBJECT_PATH,
                                    ALTERATOR_MANAGER_INTERFACE_NAME,
                                    QDBusConnection::systemBus()};
    if (!interface.isValid())
    {
        qWarning() << "Invalid manager interface";
    }

    const QDBusReply<QList<QDBusObjectPath>> reply = interface.call(ALTERATOR_MANAGER_GET_OBJECTS_METHOD_NAME, iface);

    if (!reply.isValid())
    {
        qWarning() << "Invalid DBus reply: " << reply.error();
    }

    QStringList result;
    for (auto path : reply.value())
    {
        result << path.path();
    }

    return result;
}
} // namespace alt
