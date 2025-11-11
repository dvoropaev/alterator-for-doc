#include "DBusDataSource.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>


class DBusDataSource::Private{
public:
    Private() : m_connection{ QDBusConnection::systemBus() } {}

    inline QDBusMessage methodCall(QString path, QString iface, QString name, QVariantList args = {}){
        auto msg = QDBusMessage::createMethodCall("org.altlinux.alterator", path, iface, name);
        msg.setArguments(args);
        return m_connection.call(msg, QDBus::BlockWithGui);
    }

    inline QDBusMessage callBatch(QString method, QVariantList args = {}){
        return methodCall("/org/altlinux/alterator/control",
                          "org.altlinux.alterator.batch_control1",
                          method, args);
    }

    inline QDBusMessage callControl(const QString& path, QString&& method, QVariantList&& args = {}){
        return methodCall(path, "org.altlinux.alterator.control1", method, args);
    }

    QDBusConnection m_connection;
};

DBusDataSource::DBusDataSource() : d{ new Private{} } {}
DBusDataSource::~DBusDataSource() {delete d;}

QStringList DBusDataSource::listFacilities()
{
    auto reply = d->callBatch("ListFacilities");
    return reply.arguments().at(0).toStringList();
}

QStringList DBusDataSource::getAllInfo()
{
    auto reply = d->callBatch("FacilitiesInfo");
    return reply.arguments().at(0).toStringList();
}


bool DBusDataSource::getValue(const QString& path, QString& result, QString& messages)
{
    auto reply = d->callControl(path, "Get");
    if ( reply.arguments().size() == 3 ) {
        result = reply.arguments().at(0).toByteArray();
        messages = reply.arguments().at(1).toStringList().join('\n');
        return reply.arguments().at(2) == 0;
    }

    messages = reply.errorMessage();
    return false;
}

bool DBusDataSource::setValue(const QString& path, const QString& value, QString& messages)
{
    auto reply = d->callControl(path, "Set", {value});
    if ( reply.arguments().size() == 2 ) {
        messages = reply.arguments().at(0).toStringList().join('\n');
        return reply.arguments().at(1).toInt() == 0;
    }

    messages = reply.errorMessage();
    return false;
}

bool DBusDataSource::listStates(const QString& path, QStringList& result, QString& messages)
{
    auto reply = d->callControl(path, "List");

    if ( reply.arguments().size() == 3 ) {
        result = reply.arguments().at(0).toStringList();
        messages = reply.arguments().at(1).toStringList().join('\n');
        return reply.arguments().at(2).toInt() == 0;
    }

    messages = reply.errorMessage();
    return false;
}
