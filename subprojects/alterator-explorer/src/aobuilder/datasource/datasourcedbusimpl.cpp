#include "datasourcedbusimpl.h"
#include "constants.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

#include <utility>
#include <QDebug>

#define DBUS_TIMEOUT 3000

namespace ao_builder
{
class DataSourseDbusImplPrivate
{
public:
    DataSourseDbusImplPrivate(const QString &serviceName)
        : m_serviceName(std::move(serviceName))
        , m_dbusConnection(QDBusConnection::systemBus())
    {}

    QString m_serviceName;
    QDBusConnection m_dbusConnection;
};

DataSourceDBusImpl::DataSourceDBusImpl(const QString &serviceName)
    : d(new DataSourseDbusImplPrivate(serviceName))
{}

DataSourceDBusImpl::~DataSourceDBusImpl()
{
    delete d;
}

bool DataSourceDBusImpl::ping()
{
    auto message = QDBusMessage::createMethodCall(d->m_serviceName,
                                                  dbus::path::ALTERATOR,
                                                  "org.freedesktop.DBus.Peer",
                                                  "Ping");

    QDBusReply<void> reply = d->m_dbusConnection.call(message, QDBus::BlockWithGui, DBUS_TIMEOUT);

    return reply.isValid();
}

QStringList DataSourceDBusImpl::getLocalAppPaths()
{
    return getPathsByInterface(dbus::interface::LOCAL_APP);
}

QString DataSourceDBusImpl::getLocalAppInfo(const QString &path)
{
    return getObjectInfo(dbus::interface::LOCAL_APP, path, dbus::method::INFO);
}

QStringList DataSourceDBusImpl::getCategoriesList()
{
    const QStringList mainCatObjectList = getPathsByInterface(dbus::interface::CATEGORY);

    if (mainCatObjectList.empty())
    {
        return {};
    }

    const QString &mainCatObjectPath = mainCatObjectList.first();

    return getObjectsList(dbus::method::LIST, mainCatObjectPath, dbus::interface::CATEGORY);
}

QString DataSourceDBusImpl::getCategoryInfo(const QString &path)
{
    return getObjectInfoByName(dbus::interface::CATEGORY, dbus::path::GLOBAL, path, dbus::method::INFO);
}

QStringList DataSourceDBusImpl::getLegacyObjectsPaths()
{
    return getPathsByInterface(dbus::interface::LEGACY_OBJECT);
}

QString DataSourceDBusImpl::getLegacyObjectInfo(const QString &path)
{
    return getObjectInfo(dbus::interface::LEGACY_OBJECT, path, dbus::method::INFO);
}

QStringList DataSourceDBusImpl::getObjectsPath(const QString &interface)
{
    return getPathsByInterface(interface);
}

QString DataSourceDBusImpl::getObjectInfo(const QString &path, const QString &interface)
{
    return getObjectInfo(interface, path, dbus::method::INFO);
}

QStringList DataSourceDBusImpl::getPathsByInterface(const QString &ifaceName)
{
    auto message = QDBusMessage::createMethodCall(d->m_serviceName,
                                                  dbus::path::ALTERATOR,
                                                  dbus::interface::MANAGER,
                                                  dbus::method::GET_OBJECTS);

    message.setArguments({ifaceName});
    QDBusReply<QList<QDBusObjectPath>> reply = d->m_dbusConnection.call(message, QDBus::BlockWithGui, DBUS_TIMEOUT);

    if (!reply.isValid())
    {
        return {};
    }

    QStringList result{};

    for (const QDBusObjectPath &path : reply.value())
    {
        result.append(path.path());
    }

    return result;
}

QStringList DataSourceDBusImpl::getObjectsList(const QString &listMethodName,
                                               const QString &path,
                                               const QString &interfaceName)
{
    auto message = QDBusMessage::createMethodCall(d->m_serviceName, path, interfaceName, listMethodName);

    QDBusReply<QStringList> reply = d->m_dbusConnection.call(message, QDBus::BlockWithGui, DBUS_TIMEOUT);
    if (!reply.isValid())
    {
        return {};
    }

    return reply.value();
}

QByteArray DataSourceDBusImpl::getObjectInfo(const QString &ifaceName, const QString &path, const QString &methodName)
{
    auto message = QDBusMessage::createMethodCall(d->m_serviceName, path, ifaceName, methodName);

    QDBusReply<QByteArray> reply = d->m_dbusConnection.call(message, QDBus::BlockWithGui, DBUS_TIMEOUT);

    if (!reply.isValid())
    {
        qWarning() << "Invalid reply from object" << path << "with interface" << ifaceName << "and method name"
                   << methodName << ":" << reply.error().message();
        return {};
    }

    return reply.value();
}

QByteArray DataSourceDBusImpl::getObjectInfoByName(const QString &ifaceName,
                                                   const QString &path,
                                                   const QString &objectName,
                                                   const QString &methodName)
{
    const QString param = QString("%1%2%3").arg("\"").arg(objectName).arg("\"");

    auto message = QDBusMessage::createMethodCall(d->m_serviceName, path, ifaceName, methodName);

    message.setArguments({param});

    const QDBusReply<QByteArray> reply = d->m_dbusConnection.call(message, QDBus::BlockWithGui, DBUS_TIMEOUT);

    if (!reply.isValid())
    {
        return {};
    }

    return reply.value();
}

} // namespace ao_builder
