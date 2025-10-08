#include "aptinterface.h"

#include "utility/constants.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

namespace alt
{
std::optional<QStringList> AptInterface::lastDistUpgrade()
{
    return call(ALTERATOR_APT1_LAST_DIST_UPGRADE_METHOD_NAME);
}

std::optional<QStringList> AptInterface::call(const QString &methodName)
{
    static QDBusInterface interface{ALTERATOR_SERVICE_NAME,
                                    ALTERATOR_APT_OBJECT_PATH,
                                    ALTERATOR_APT1_INTERFACE_NAME,
                                    QDBusConnection::systemBus()};
    if (!interface.isValid())
    {
        qWarning() << "Invalid apt1 interface";
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

    return reply.arguments().first().toStringList();
}

} // namespace alt
