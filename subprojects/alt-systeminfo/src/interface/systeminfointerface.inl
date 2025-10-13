#include "utility/constants.h"

#include <optional>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QPair>

namespace alt
{
template<typename T>
std::optional<T> SystemInfoInterface::call(const QString &methodName)
{
    QDBusInterface systeminfoInterface{ALTERATOR_SERVICE_NAME,
                                       ALTERATOR_SYSTEMINFO_OBJECT_PATH,
                                       ALTERATOR_SYSTEMINFO1_INTERFACE_NAME,
                                       QDBusConnection::systemBus()};
    if (!systeminfoInterface.isValid())
    {
        qWarning() << "Invalid systeminfo1 interface";
        return std::nullopt;
    }

    QDBusMessage message = systeminfoInterface.call(methodName);
    QDBusReply<T> reply = message;
    if (!reply.isValid())
    {
        qWarning() << "Invalid DBus reply: " << reply.error();
        return std::nullopt;
    }

    if (message.arguments().last().toInt())
    {
        return std::nullopt;
    }

    return reply.value();
}
} // namespace alt
