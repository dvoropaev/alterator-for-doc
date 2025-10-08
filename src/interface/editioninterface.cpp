#include "editioninterface.h"

#include "utility/constants.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

namespace alt
{
std::optional<QByteArray> EditionInterface::license(const QString &object)
{
    return call(object, ALTERATOR_CURRENT_EDITION1_LICENSE_METHOD_NAME);
}

std::optional<QByteArray> EditionInterface::info(const QString &object)
{
    return call(object, ALTERATOR_CURRENT_EDITION1_INFO_METHOD_NAME);
}

std::optional<QByteArray> EditionInterface::description(const QString &object)
{
    return call(object, ALTERATOR_CURRENT_EDITION1_DESCRIPTION_METHOD_NAME);
}

std::optional<QByteArray> EditionInterface::call(const QString &object, const QString &methodName)
{
    QDBusInterface interface{ALTERATOR_SERVICE_NAME,
                             object,
                             ALTERATOR_EDITION1_INTERFACE_NAME,
                             QDBusConnection::systemBus()};
    if (!interface.isValid())
    {
        qWarning() << "Invalid edition1 interface";
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
