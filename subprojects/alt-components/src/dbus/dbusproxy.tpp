#ifndef DBUSPROXY_TPP
#define DBUSPROXY_TPP

#include <optional>

#ifndef DBUSPROXY_H
#error __FILE__ should only be included from dbusproxy.h.
#endif // DBUSPROXY_H

#include "dbusproxy.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QString>

namespace alt
{
template<class... Args>
std::optional<QVariantList> DBusProxy::callDBus(QDBusInterface *iface, QString method, Args... args)
{
    if (!iface->isValid())
    {
        auto warning = QObject::tr(R"(No interface "%3" on object "%2" of service "%1")")
                           .arg(iface->service())
                           .arg(iface->path())
                           .arg(iface->interface());
        qWarning().noquote() << warning;
        emit errorOccured(QtMsgType::QtInfoMsg, warning);
        return std::nullopt;
    }

    QDBusMessage reply = iface->call(method, args...);
    if (reply.type() == QDBusMessage::ErrorMessage)
    {
        auto warning = QObject::tr("Error reply from manager: \"%1\"").arg(reply.errorMessage());
        qWarning().noquote() << warning;
        emit errorOccured(QtMsgType::QtInfoMsg, warning);
        return std::nullopt;
    }

    return reply.arguments();
}

template<typename T>
std::optional<T> DBusProxy::getResult(const std::optional<QVariantList> &args, const QString &methodName)
{
    if (!args.has_value())
    {
        return std::nullopt;
    }
    if (checkSuccess(args.value(), methodName).has_value())
    {
        return std::nullopt;
    }

    return args.value()[0].value<T>();
}

} // namespace alt
#endif // DBUSPROXY_TPP
