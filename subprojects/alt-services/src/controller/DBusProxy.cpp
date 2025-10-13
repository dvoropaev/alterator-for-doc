#include "DBusProxy.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusReply>
#include <QDebug>

static const auto ALTERATOR     = "org.altlinux.alterator";
static const auto IFACE_SERVICE = "org.altlinux.alterator.service1";
static const auto IFACE_DIAG    = "org.altlinux.alterator.diag1";
static const auto IFACE_MANAGER = "org.altlinux.alterator.manager";
static const auto PATH_MANAGER  = "/org/altlinux/alterator";

#define DBUS_TIMEOUT 86400000

class DBusProxy::Private {
public:
    Private()
        : m_system  { QDBusConnection:: systemBus() }
        , m_session { QDBusConnection::sessionBus() }
    {}

    QDBusConnection m_system;
    QDBusConnection m_session;

    std::vector<QString> m_env;
};

DBusProxy::DBusProxy()
    : d { new Private() }
{}

DBusProxy::~DBusProxy() { delete d; }


class DBusProxy::ScopedSignalConnection {
    ScopedSignalConnection(const ScopedSignalConnection &) = delete;
    ScopedSignalConnection &operator=(const ScopedSignalConnection &) = delete;
    ScopedSignalConnection(ScopedSignalConnection &&) = delete;
    ScopedSignalConnection &operator=(ScopedSignalConnection &&) = delete;

public:
    inline ScopedSignalConnection(QDBusConnection& connection,
                                  const QString &service, const QString &path, const QString &interface,
                                  const QString &name, QObject* reciever, const char* target)
    {
        auto signame = name +
                       ( connection.baseService()
                            .replace(':', '_')
                            .replace('.', '_') );

        if ( connection.connect(service, path, interface, signame, reciever, target) )
            m_deleter = [=, &connection] {
                if ( ! connection.disconnect(service, path, interface, signame, reciever, target) )
                    qWarning() << "failed to disconnect" << signame;
            };
        else
            qWarning() << "failed to connect" << signame;
    }

    inline ~ScopedSignalConnection() { if ( m_deleter ) m_deleter(); }

private:
    std::function<void()> m_deleter;
};

inline DBusProxy::ScopedSignalPair DBusProxy::makeDiagSignals(const QString& path, bool session)
{
    QDBusConnection& connection = session ? std::ref(d->m_session) : std::ref(d->m_system);
    return std::make_pair(
        std::make_unique<ScopedSignalConnection>(connection, ALTERATOR, path, IFACE_DIAG, "diag1_stdout_signal", this, SIGNAL(stdout(QString))),
        std::make_unique<ScopedSignalConnection>(connection, ALTERATOR, path, IFACE_DIAG, "diag1_stderr_signal", this, SIGNAL(stderr(QString)))
    );
}

inline DBusProxy::ScopedSignalPair DBusProxy::makeServiceSignals(const QString& path)
{
    return std::make_pair(
        std::make_unique<ScopedSignalConnection>(d->m_system, ALTERATOR, path, IFACE_SERVICE, "service_stdout_signal", this, SIGNAL(stdout(QString))),
        std::make_unique<ScopedSignalConnection>(d->m_system, ALTERATOR, path, IFACE_SERVICE, "service_stderr_signal", this, SIGNAL(stderr(QString)))
    );
}


QStringList DBusProxy::getServicePaths()
{
    auto message = QDBusMessage::createMethodCall(
        ALTERATOR,
        PATH_MANAGER,
        IFACE_MANAGER,
        "GetObjects"
    );

    message.setArguments({IFACE_SERVICE});
    QDBusReply<QList<QDBusObjectPath>> reply = d->m_system.call(message, QDBus::BlockWithGui, DBUS_TIMEOUT);

    QStringList result;

    if ( reply.isValid() )
        for (const QDBusObjectPath &path : reply.value())
            result.append(path.path());

    else
        qCritical() << reply.error();

    return result;
}

inline auto method(QString iface, QString path, QString methodName, QList<QVariant> args = {}){
    auto msg = QDBusMessage::createMethodCall(
        ALTERATOR, path, iface, methodName
    );
    msg.setArguments(args);
    return msg;
}

inline auto serviceMethod(QString path, QString methodName, QList<QVariant> args = {}){
    return method(IFACE_SERVICE, path, methodName, args);
}

static std::pair<int, QString> parseReply(const QDBusMessage& reply)
{
    int code = -1;
    QString text;

    const auto args = reply.arguments();
    if (args.isEmpty())
        return {code, text};

    if (args.size() == 1) {
        const auto& a0 = args.at(0);
        if (a0.canConvert<int>())
            code = a0.toInt();
        else if (a0.canConvert<QByteArray>())
            text = QString::fromUtf8(a0.toByteArray());
        else if (a0.canConvert<QString>())
            text = a0.toString();
        else
            text = a0.toString();
        return {code, text};
    }

    // Prefer the last int as exit code and concatenate any string/bytearray args as text
    for (int i = 0; i < args.size(); ++i) {
        const auto& a = args.at(i);
        if (i == args.size() - 1 && a.canConvert<int>()) {
            code = a.toInt();
            continue;
        }
        if (a.canConvert<QString>()) {
            if (!text.isEmpty()) text.append('\n');
            text.append(a.toString());
        } else if (a.canConvert<QByteArray>()) {
            if (!text.isEmpty()) text.append('\n');
            text.append(QString::fromUtf8(a.toByteArray()));
        }
    }

    return {code, text};
}

QString DBusProxy::getServiceInfo(const QString& path)
{
    auto message = serviceMethod(path, "Info");

    QDBusReply<QByteArray> reply = d->m_system.call(message, QDBus::BlockWithGui, DBUS_TIMEOUT);

    if ( reply.isValid() )
        return reply.value();

    qCritical() << reply.error();
    return {};
}

QString DBusProxy::getDiagInfo(const QString& path, bool session)
{
    auto message = method(IFACE_DIAG, path, "Info");

    QDBusReply<QByteArray> reply = (session ? d->m_session : d->m_system).call(message, QDBus::BlockWithGui, DBUS_TIMEOUT);

    if ( reply.isValid() )
        return reply.value();

    qCritical() << reply.error();
    return {};
}

bool DBusProxy::getDiagToolTests(const QString& path, const QString& service, const QString& mode, bool required, QStringList& data, bool session)
{
    setEnv("service_path", service);
    setEnv("service_deploy_mode", mode);
    if ( required )
        setEnv("service_required_only", "true");

    auto message = method(IFACE_DIAG, path, "List");
    QDBusReply<QStringList> reply = (session ? d->m_session : d->m_system).call(message, QDBus::BlockWithGui, DBUS_TIMEOUT);

    if ( reply.isValid() )
        data = reply.value();

    clearEnv();

    return reply.isValid();
}

int DBusProxy::status(const QString& path, QByteArray& result)
{
    auto message = serviceMethod(path, "Status");
    auto reply = d->m_system.call(message, QDBus::BlockWithGui, DBUS_TIMEOUT);

    if ( reply.type() == QDBusMessage::ErrorMessage ) {
        emit stderr(reply.errorMessage());
        return false;
    }

    auto value = reply.arguments().at(0).toByteArray();
    int code = reply.arguments().length() == 2 ? reply.arguments().at(1).toInt() : -1;

    result = value;

    return code;
}

bool DBusProxy::deploy(const QString& path, const QString& data)
{
    auto dbusSignals = makeServiceSignals(path);

    auto message = serviceMethod(path, "Deploy", {data});
    auto reply = d->m_system.call(message, QDBus::BlockWithGui, DBUS_TIMEOUT);

    if ( reply.type() == QDBusMessage::ErrorMessage ) {
        emit stderr(reply.errorMessage());
        return false;
    }

    const auto [code, text] = parseReply(reply);
    if (!text.trimmed().isEmpty())
        (code == 0 ? emit stdout(text) : emit stderr(text));
    else if (code != 0)
        emit stderr(tr("Operation failed with code %1").arg(code));

    return code == 0;
}

bool DBusProxy::undeploy(const QString& path, const QString& data)
{
    auto dbusSignals = makeServiceSignals(path);

    auto message = serviceMethod(path, "Undeploy", {data});
    auto reply = d->m_system.call(message, QDBus::BlockWithGui, DBUS_TIMEOUT);

    if ( reply.type() == QDBusMessage::ErrorMessage ) {
        emit stderr(reply.errorMessage());
        return false;
    }

    const auto [code, text] = parseReply(reply);
    if (!text.trimmed().isEmpty())
        (code == 0 ? emit stdout(text) : emit stderr(text));
    else if (code != 0)
        emit stderr(tr("Operation failed with code %1").arg(code));

    return code == 0;
}

bool DBusProxy::configure(const QString& path, const QString& data)
{
    auto dbusSignals = makeServiceSignals(path);

    auto message = serviceMethod(path, "Configure", {data});
    auto reply = d->m_system.call(message, QDBus::BlockWithGui, DBUS_TIMEOUT);

    if ( reply.type() == QDBusMessage::ErrorMessage ) {
        emit stderr(reply.errorMessage());
        return false;
    }

    const auto [code, text] = parseReply(reply);
    if (!text.trimmed().isEmpty())
        (code == 0 ? emit stdout(text) : emit stderr(text));
    else if (code != 0)
        emit stderr(tr("Operation failed with code %1").arg(code));

    return code == 0;
}

bool DBusProxy::start(const QString& path)
{
    auto dbusSignals = makeServiceSignals(path);

    auto reply = d->m_system.call(serviceMethod(path, "Start"), QDBus::BlockWithGui, DBUS_TIMEOUT);

    if ( reply.type() == QDBusMessage::ErrorMessage ) {
        emit stderr(reply.errorMessage());
        return false;
    }

    const auto [code, text] = parseReply(reply);
    if (!text.trimmed().isEmpty())
        (code == 0 ? emit stdout(text) : emit stderr(text));
    else if (code != 0)
        emit stderr(tr("Operation failed with code %1").arg(code));

    return code == 0;
}

bool DBusProxy::stop(const QString& path)
{
    auto dbusSignals = makeServiceSignals(path);

    auto reply = d->m_system.call(serviceMethod(path, "Stop"), QDBus::BlockWithGui, DBUS_TIMEOUT);

    if ( reply.type() == QDBusMessage::ErrorMessage ) {
        emit stderr(reply.errorMessage());
        return false;
    }

    const auto [code, text] = parseReply(reply);
    if (!text.trimmed().isEmpty())
        (code == 0 ? emit stdout(text) : emit stderr(text));
    else if (code != 0)
        emit stderr(tr("Operation failed with code %1").arg(code));

    return code == 0;
}

bool DBusProxy::backup(const QString& path, const QString& data)
{
    auto dbusSignals = makeServiceSignals(path);

    auto message = serviceMethod(path, "Backup", {data});
    auto reply = d->m_system.call(message, QDBus::BlockWithGui, DBUS_TIMEOUT);

    if ( reply.type() == QDBusMessage::ErrorMessage ) {
        emit stderr(reply.errorMessage());
        return false;
    }

    const auto [code, text] = parseReply(reply);
    if (!text.trimmed().isEmpty())
        (code == 0 ? emit stdout(text) : emit stderr(text));
    else if (code != 0)
        emit stderr(tr("Operation failed with code %1").arg(code));

    return code == 0;
}

bool DBusProxy::restore(const QString& path, const QString& data)
{
    auto dbusSignals = makeServiceSignals(path);

    auto message = serviceMethod(path, "Restore", {data});
    auto reply = d->m_system.call(message, QDBus::BlockWithGui, DBUS_TIMEOUT);

    if ( reply.type() == QDBusMessage::ErrorMessage ) {
        emit stderr(reply.errorMessage());
        return false;
    }

    const auto [code, text] = parseReply(reply);
    if (!text.trimmed().isEmpty())
        (code == 0 ? emit stdout(text) : emit stderr(text));
    else if (code != 0)
        emit stderr(tr("Operation failed with code %1").arg(code));

    return code == 0;
}

void DBusProxy::setEnv(const QString& key, const QString& value)
{
    d->m_env.push_back(key);
    d->m_system .call(method(IFACE_MANAGER, PATH_MANAGER, "SetEnvValue", {key, value}), QDBus::BlockWithGui, DBUS_TIMEOUT);
    d->m_session.call(method(IFACE_MANAGER, PATH_MANAGER, "SetEnvValue", {key, value}), QDBus::BlockWithGui, DBUS_TIMEOUT);
}

void DBusProxy::clearEnv()
{
    for ( const auto& env : d->m_env ) {
        d->m_system .call(method(IFACE_MANAGER, PATH_MANAGER, "UnsetEnvValue", {env}), QDBus::BlockWithGui, DBUS_TIMEOUT);
        d->m_session.call(method(IFACE_MANAGER, PATH_MANAGER, "UnsetEnvValue", {env}), QDBus::BlockWithGui, DBUS_TIMEOUT);
    }
    d->m_env.clear();
}

bool DBusProxy::runDiag(const QString& path, const QString& test, bool session)
{
    QDBusConnection& connection = session ? std::ref(d->m_session) : std::ref(d->m_system);

    auto dbusSignals = makeDiagSignals(path, session);

    auto testReply = connection.call(method(IFACE_DIAG, path, "Run", {test}), QDBus::BlockWithGui, DBUS_TIMEOUT);

    if ( testReply.type() == QDBusMessage::ErrorMessage || testReply.arguments().at(0) != 0 ) {
        emit stderr(tr("Test failed"));
        emit stderr(testReply.errorMessage());
        return false;
    }

    return true;
}
