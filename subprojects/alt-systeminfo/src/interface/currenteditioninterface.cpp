#include "currenteditioninterface.h"

#include "utility/constants.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

namespace alt
{
std::optional<QByteArray> CurrentEditionInterface::license()
{
    return call(ALTERATOR_CURRENT_EDITION1_LICENSE_METHOD_NAME);
}

std::optional<QByteArray> CurrentEditionInterface::info()
{
    return call(ALTERATOR_CURRENT_EDITION1_INFO_METHOD_NAME);
}

int CurrentEditionInterface::set(const QString &editionId)
{
    static QDBusInterface interface{ALTERATOR_SERVICE_NAME,
                                    ALTERATOR_GLOBAL_OBJECT_PATH,
                                    ALTERATOR_CURRENT_EDITION1_INTERFACE_NAME,
                                    QDBusConnection::systemBus()};
    if (!interface.isValid())
    {
        qWarning() << "Invalid current_edition1 interface";
    }

    const QDBusMessage &reply = interface.call(ALTERATOR_CURRENT_EDITION1_SET_METHOD_NAME, editionId);

    if (reply.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "Invalid DBus reply: " << reply.errorMessage();
        return -1;
    }

    return reply.arguments().first().toInt();
}

std::optional<QStringList> CurrentEditionInterface::get()
{
    static QDBusInterface interface{ALTERATOR_SERVICE_NAME,
                                    ALTERATOR_GLOBAL_OBJECT_PATH,
                                    ALTERATOR_CURRENT_EDITION1_INTERFACE_NAME,
                                    QDBusConnection::systemBus()};
    if (!interface.isValid())
    {
        qWarning() << "Invalid current_edition1 interface";
    }

    const QDBusMessage &reply = interface.call(ALTERATOR_CURRENT_EDITION1_GET_METHOD_NAME);

    if (reply.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "Invalid DBus reply: " << reply.errorMessage();
        return std::nullopt;
    }

    if (reply.arguments().last().toBool())
    {
        return std::nullopt;
    }

    return reply.arguments().first().toStringList();
}

std::optional<QByteArray> CurrentEditionInterface::call(const QString &methodName)
{
    static QDBusInterface interface{ALTERATOR_SERVICE_NAME,
                                    ALTERATOR_GLOBAL_OBJECT_PATH,
                                    ALTERATOR_CURRENT_EDITION1_INTERFACE_NAME,
                                    QDBusConnection::systemBus()};
    if (!interface.isValid())
    {
        qWarning() << "Invalid current_edition1 interface";
    }

    const QDBusMessage &reply = interface.call(methodName);

    if (reply.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "Invalid DBus reply: " << reply.errorMessage();
        return std::nullopt;
    }

    if (reply.arguments().last().toBool())
    {
        return std::nullopt;
    }

    return reply.arguments().first().toByteArray();
}
} // namespace alt
