#include "datasource/dbusdatasource.h"
#include "../constants.h"

#include <optional>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDebug>
#include <QStringList>
#include <qlist.h>

struct AlteratorExecutorReply
{
    int status                 = 0;
    QStringList stdout_strings = {};
    QStringList stderr_strings = {};
};

constexpr int NO_TIMEOUT  = -1;
constexpr int DAY_TIMEOUT = 24 * 60 * 60 * 1000;

DBusDataSource::DBusDataSource(QString serviceName, QString path, QString ifaceName, QString methodName)
    : m_connection(QDBusConnection::systemBus())
    , m_serviceName(serviceName)
    , m_alteratorPath(path)
    , m_alteratorIfaceName(ifaceName)
    , m_alteratorMethodName(methodName)
{
    QStringList rpmObjects = objectByPath(RPM_INTERFACE_NAME);

    if (!rpmObjects.isEmpty())
    {
        m_rpmPath = rpmObjects.at(0);
    }

    QStringList aptObjects = objectByPath(APT_INTERFACE_NAME);

    if (!aptObjects.isEmpty())
    {
        m_aptPath = aptObjects.at(0);
    }

    QStringList repoObjects = objectByPath(REPO_INTERFACE_NAME);

    if (!repoObjects.isEmpty())
    {
        m_repoPath = repoObjects.at(0);
    }

    QString signalPostfix = QDBusConnection::systemBus().baseService();
    signalPostfix.replace(':', '_');
    signalPostfix.replace('.', '_');

    QString installOutSignal = APT_INSTALL_STDOUT_SIGNAL_NAME + signalPostfix;
    QString installErrSignal = APT_INSTALL_STDERR_SIGNAL_NAME + signalPostfix;

    if (!(m_connection.connect(
              m_serviceName, m_aptPath, APT_INTERFACE_NAME, installOutSignal, this, SLOT(onAptInstalledAsync(QString)))
          && m_connection.connect(m_serviceName,
                                  m_aptPath,
                                  APT_INTERFACE_NAME,
                                  installErrSignal,
                                  this,
                                  SLOT(onAptInstalledAsync(QString)))))
    {
        qWarning() << QString("%1 is not connected").arg(APT_INSTALL_STDOUT_SIGNAL_NAME);
    }

    QString updateOutSignal = APT_UPDATE_STDOUT_SIGNAL_NAME + signalPostfix;
    QString updateErrSignal = APT_UPDATE_STDERR_SIGNAL_NAME + signalPostfix;

    if (!(m_connection.connect(
              m_serviceName, m_aptPath, APT_INTERFACE_NAME, updateOutSignal, this, SLOT(onAptUpdatedAsync(QString)))
          && m_connection.connect(
              m_serviceName, m_aptPath, APT_INTERFACE_NAME, updateErrSignal, this, SLOT(onAptUpdatedAsync(QString)))))
    {
        qWarning() << QString("%1 is not connected").arg(APT_UPDATE_STDOUT_SIGNAL_NAME);
    }

    QString distUpgradeOutSignal = APT_DIST_UPGRADE_STDOUT_SIGNAL_NAME + signalPostfix;
    QString distUpgradeErrSignal = APT_DIST_UPGRADE_STDERR_SIGNAL_NAME + signalPostfix;

    if (!(m_connection.connect(m_serviceName,
                               m_aptPath,
                               APT_INTERFACE_NAME,
                               distUpgradeOutSignal,
                               this,
                               SLOT(onAptDistUpgradedAsync(QString)))
          && m_connection.connect(m_serviceName,
                                  m_aptPath,
                                  APT_INTERFACE_NAME,
                                  distUpgradeErrSignal,
                                  this,
                                  SLOT(onAptDistUpgradedAsync(QString)))))
    {
        qWarning() << QString("%1 is not connected").arg(APT_DIST_UPGRADE_STDOUT_SIGNAL_NAME);
    }
}

DBusDataSource::~DBusDataSource() {}

QStringList DBusDataSource::getObjectsPathsByIface(QString iface)
{
    return objectByPath(iface);
}

int DBusDataSource::rpmList(QStringList &result)
{
    result.clear();
    if (m_rpmPath.isEmpty())
    {
        result << "Path to RPM object is empty, RPM functionality is unavailable";
        qWarning() << result;
        return 1;
    }

    auto reply = dbusCall(m_rpmPath, RPM_INTERFACE_NAME, NO_TIMEOUT, RPM_LIST_METHOD_NAME);
    if (reply.status)
    {
        result = reply.stderr_strings;
    }
    else
    {
        result = reply.stdout_strings;
    }

    return reply.status;
}

int DBusDataSource::rpmInstall(QStringList &result, const QString package)
{
    result.clear();
    if (m_rpmPath.isEmpty())
    {
        result << "Path to RPM object is empty, RPM functionality is unavailable";
        qWarning() << result;
        return 1;
    }

    auto reply = dbusCall(m_rpmPath, RPM_INTERFACE_NAME, NO_TIMEOUT, RPM_INSTALL_METHOD_NAME, package);
    if (reply.status)
    {
        result = reply.stderr_strings;
    }
    else
    {
        result = reply.stdout_strings;
    }

    return reply.status;
}

int DBusDataSource::rpmRemove(QStringList &result, const QString package)
{
    result.clear();
    if (m_rpmPath.isEmpty())
    {
        result << "Path to RPM object is empty, RPM functionality is unavailable";
        qWarning() << result;
        return 1;
    }

    auto reply = dbusCall(m_rpmPath, RPM_INTERFACE_NAME, NO_TIMEOUT, RPM_REMOVE_METHOD_NAME, package);
    if (reply.status)
    {
        result = reply.stderr_strings;
    }
    else
    {
        result = reply.stdout_strings;
    }

    return reply.status;
}

int DBusDataSource::rpmInfo(QStringList &result, const QString package)
{
    result.clear();
    if (m_rpmPath.isEmpty())
    {
        result << "Path to RPM object is empty, RPM functionality is unavailable";
        qWarning() << result;
        return 1;
    }

    auto reply = dbusCall(m_rpmPath, RPM_INTERFACE_NAME, NO_TIMEOUT, RPM_INFO_METHOD_NAME, package);
    if (reply.status)
    {
        result = reply.stderr_strings;
        return reply.status;
    }

    auto info = QString("");
    for (QString &line : reply.stdout_strings)
    {
        info.append(line + "\n");
    }

    result << info;
    return 0;
}

int DBusDataSource::rpmFiles(QStringList &result, const QString package)
{
    result.clear();
    if (m_rpmPath.isEmpty())
    {
        result << "Path to RPM object is empty, RPM functionality is unavailable";
        qWarning() << result;
        return 1;
    }

    auto reply = dbusCall(m_rpmPath, RPM_INTERFACE_NAME, NO_TIMEOUT, RPM_FILES_METHOD_NAME, package);
    if (reply.status)
    {
        result = reply.stderr_strings;
    }
    else
    {
        result = reply.stdout_strings;
    }

    return reply.status;
}

int DBusDataSource::aptCheckApply(QStringList &stdout_result, QStringList &stderr_result, const QString package)
{
    stdout_result.clear();
    stderr_result.clear();
    if (m_rpmPath.isEmpty())
    {
        stderr_result << "Path to APT object is empty, APT functionality is unavailable";
        qWarning() << stderr_result;
        return 1;
    }

    auto reply    = dbusCall(m_aptPath, APT_INTERFACE_NAME, DAY_TIMEOUT, APT_CHECK_APPLY_METHOD_NAME, package);
    stdout_result = reply.stdout_strings;
    stderr_result = reply.stderr_strings;

    return reply.status;
}

int DBusDataSource::aptCheckDistUpgrade(QStringList &stdout_result, QStringList &stderr_result)
{
    stdout_result.clear();
    stderr_result.clear();
    if (m_rpmPath.isEmpty())
    {
        stderr_result << "Path to APT object is empty, APT functionality is unavailable";
        qWarning() << stderr_result;
        return 1;
    }

    auto reply    = dbusCall(m_aptPath, APT_INTERFACE_NAME, DAY_TIMEOUT, APT_CHECK_DIST_UPGRADE_METHOD_NAME);
    stdout_result = reply.stdout_strings;
    stderr_result = reply.stderr_strings;

    return reply.status;
}

int DBusDataSource::aptInstall(QStringList &result, const QString package)
{
    result.clear();
    if (m_rpmPath.isEmpty())
    {
        result << "Path to APT object is empty, APT functionality is unavailable";
        qWarning() << result;
        return 1;
    }

    auto reply = dbusCall(m_aptPath, APT_INTERFACE_NAME, DAY_TIMEOUT, APT_INSTALL_METHOD_NAME, package);
    if (reply.status)
    {
        result = reply.stderr_strings;
    }
    else
    {
        result = reply.stdout_strings;
    }

    return reply.status;
}

void DBusDataSource::onAptInstalledAsync(QString message)
{
    emit onAptInstallAsync(message);
}

void DBusDataSource::onAptUpdatedAsync(QString message)
{
    emit onAptUpdateAsync(message);
}

void DBusDataSource::onAptDistUpgradedAsync(QString message)
{
    emit onAptDistUpgradeAsync(message);
}

int DBusDataSource::aptApplyAsync(QStringList &result, const QString exclude_packages, const QString packages)
{
    QString path      = m_aptPath;
    QString interface = APT_INTERFACE_NAME;
    QString method    = APT_APPLY_ASYNC_METHOD_NAME;

    QDBusInterface iface(m_serviceName, path, interface, m_connection);
    if (!iface.isValid())
    {
        result << QString("Interface \"%1\" on DBus is invalid, corresponding functionality is unavailable")
                      .arg(interface);
        qWarning() << result;
        return 2;
    }
    iface.setTimeout(DAY_TIMEOUT);

    QDBusMessage reply = iface.call(QDBus::CallMode::BlockWithGui, method, exclude_packages, packages);
    if (reply.type() == QDBusMessage::ErrorMessage)
    {
        result << QString("Method \"%1\" failed with error message \"%2\"").arg(method).arg(reply.errorMessage());
        qWarning() << result;
        return 3;
    }

    if (reply.type() != QDBusMessage::QDBusMessage::ReplyMessage)
    {
        result << QString("Reply from method \"%1\" is invalid").arg(method);
        qWarning() << result;
        return 4;
    }

    return reply.arguments().last().toInt();
}

int DBusDataSource::aptDistUpgradeAsync(QStringList &result)
{
    auto reply = dbusCall(m_aptPath, APT_INTERFACE_NAME, DAY_TIMEOUT, APT_DIST_UPGRADE_ASYNC_METHOD_NAME);
    result     = reply.stderr_strings;
    return reply.status;
}

int DBusDataSource::aptReinstall(QStringList &result, const QString package)
{
    result.clear();
    if (m_rpmPath.isEmpty())
    {
        result << "Path to RPM object is empty, RPM functionality is unavailable";
        qWarning() << result;
        return 1;
    }

    auto reply = dbusCall(m_aptPath, APT_INTERFACE_NAME, DAY_TIMEOUT, APT_REINSTALL_METHOD_NAME, package);
    if (reply.status)
    {
        result = reply.stderr_strings;
    }
    else
    {
        result = reply.stdout_strings;
    }

    return reply.status;
}

int DBusDataSource::aptRemove(QStringList &result, const QString package)
{
    result.clear();
    if (m_rpmPath.isEmpty())
    {
        result << "Path to RPM object is empty, RPM functionality is unavailable";
        qWarning() << result;
        return 1;
    }

    auto reply = dbusCall(m_aptPath, APT_INTERFACE_NAME, DAY_TIMEOUT, APT_REMOVE_METHOD_NAME, package);
    if (reply.status)
    {
        result = reply.stderr_strings;
    }
    else
    {
        result = reply.stdout_strings;
    }

    return reply.status;
}

int DBusDataSource::aptUpdate(QStringList &result)
{
    result.clear();
    if (m_rpmPath.isEmpty())
    {
        result << "Path to RPM object is empty, RPM functionality is unavailable";
        qWarning() << result;
        return 1;
    }

    auto reply = dbusCall(m_aptPath, APT_INTERFACE_NAME, NO_TIMEOUT, APT_UPDATE_METHOD_NAME);
    if (reply.status)
    {
        result = reply.stderr_strings;
    }
    else
    {
        result = reply.stdout_strings;
    }

    return reply.status;
}

int DBusDataSource::aptUpdateAsync(QStringList &result)
{
    auto reply = dbusCall(m_aptPath, APT_INTERFACE_NAME, NO_TIMEOUT, APT_UPDATE_ASYNC_METHOD_NAME);
    result     = reply.stderr_strings;
    return reply.status;
}

int DBusDataSource::aptListAllPackages(QStringList &result)
{
    result.clear();
    if (m_rpmPath.isEmpty())
    {
        result.append("Path to RPM object is empty, RPM functionality is unavailable");
        qWarning() << result;
        return 1;
    }

    auto reply = dbusCall(m_aptPath, APT_INTERFACE_NAME, NO_TIMEOUT, APT_LIST_ALL_PACKAGES_METHOD_NAME);
    if (reply.status)
    {
        result = reply.stderr_strings;
    }
    else
    {
        result = reply.stdout_strings;
    }

    return reply.status;
}

int DBusDataSource::aptLastUpdate(QStringList &result)
{
    result.clear();
    if (m_rpmPath.isEmpty())
    {
        result << "Path to RPM object is empty, RPM functionality is unavailable";
        qWarning() << result;
        return 1;
    }

    auto reply = dbusCall(m_aptPath, APT_INTERFACE_NAME, NO_TIMEOUT, APT_LAST_UPDATE_METHOD_NAME);
    if (reply.status)
    {
        result = reply.stderr_strings;
    }
    else
    {
        result = reply.stdout_strings;
    }

    return reply.status;
}

QStringList DBusDataSource::objectByPath(QString iface)
{
    QStringList result;

    QDBusInterface interface(m_serviceName, m_alteratorPath, m_alteratorIfaceName, m_connection);

    if (!interface.isValid())
    {
        return result;
    }

    const QDBusReply<QList<QDBusObjectPath>> reply = interface.call(m_alteratorMethodName, iface);

    if (!reply.isValid())
    {
        return result;
    }

    for (const QDBusObjectPath &path : reply.value())
    {
        result.append(path.path());
    }

    return result;
}

int DBusDataSource::repoAdd(QStringList &result, const QString repo)
{
    result.clear();
    if (m_rpmPath.isEmpty())
    {
        result << "Path to RPM object is empty, RPM functionality is unavailable";
        qWarning() << result;
        return 1;
    }

    auto reply = dbusCall(m_repoPath, REPO_INTERFACE_NAME, NO_TIMEOUT, REPO_ADD_METHOD_NAME, repo);
    if (reply.status)
    {
        result = reply.stderr_strings;
    }
    else
    {
        result = reply.stdout_strings;
    }

    return reply.status;
}

int DBusDataSource::repoList(QStringList &result)
{
    result.clear();
    if (m_rpmPath.isEmpty())
    {
        result << "Path to RPM object is empty, RPM functionality is unavailable";
        qWarning() << result;
        return 1;
    }

    auto reply = dbusCall(m_repoPath, REPO_INTERFACE_NAME, NO_TIMEOUT, REPO_LIST_METHOD_NAME);
    if (reply.status)
    {
        result = reply.stderr_strings;
    }
    else
    {
        result = reply.stdout_strings;
    }

    return reply.status;
}

int DBusDataSource::repoRemove(QStringList &result, const QString repo)
{
    result.clear();
    if (m_rpmPath.isEmpty())
    {
        result << "Path to RPM object is empty, RPM functionality is unavailable";
        qWarning() << result;
        return 1;
    }

    auto reply = dbusCall(m_repoPath, REPO_INTERFACE_NAME, NO_TIMEOUT, REPO_REMOVE_METHOD_NAME, repo);
    if (reply.status)
    {
        result = reply.stderr_strings;
    }
    else
    {
        result = reply.stdout_strings;
    }

    return reply.status;
}

AlteratorExecutorReply DBusDataSource::dbusCall(
    const QString &path, const QString &interface, int timeout, const QString &method, const QString &arg)
{
    AlteratorExecutorReply result;

    QDBusInterface iface(m_serviceName, path, interface, m_connection);
    if (!iface.isValid())
    {
        result.stderr_strings << QString(
                                     "Interface \"%1\" on DBus is invalid, corresponding functionality is unavailable")
                                     .arg(interface);
        qWarning() << result.stderr_strings;
        result.status = 2;
        return result;
    }
    iface.setTimeout(timeout);

    QDBusMessage reply;
    if (!arg.isNull())
    {
        reply = iface.call(QDBus::CallMode::BlockWithGui, method, arg);
        if (method == "UpdateAsync")
            emit closeWaitDialog();
    }
    else
    {
        reply = iface.call(QDBus::CallMode::BlockWithGui, method);
        if (method == "UpdateAsync")
            emit closeWaitDialog();
    }

    if (reply.type() == QDBusMessage::ErrorMessage)
    {
        result.stderr_strings << QString("Method \"%1\" failed with error message \"%2\"")
                                     .arg(method)
                                     .arg(reply.errorMessage());
        qWarning() << result.stderr_strings;
        result.status = 3;
        return result;
    }

    if (reply.type() != QDBusMessage::QDBusMessage::ReplyMessage)
    {
        result.stderr_strings << QString("Reply from method \"%1\" is invalid").arg(method);
        qWarning() << result.stderr_strings;
        return result;
    }

    if (reply.arguments().count() == 3)
    {
        result.stdout_strings = reply.arguments().at(0).toStringList();
        result.stderr_strings = reply.arguments().at(1).toStringList();
    }
    else if (reply.arguments().count() == 2)
    {
        result.stdout_strings = reply.arguments().at(0).toStringList();
        result.stderr_strings = result.stdout_strings;
    }

    result.status = reply.arguments().last().toInt();

    return result;
}
