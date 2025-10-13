#include "DBusDataSource.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>


class DBusDataSource::Private{
public:
    Private() : m_connection{ QDBusConnection::systemBus() } {}


    inline QDBusMessage methodCall(QString&& name, QVariantList&& args = {}){
        auto msg = QDBusMessage::createMethodCall(
            "org.altlinux.alterator",
            "/org/altlinux/alterator/control",
            "org.altlinux.alterator.control1",
            name
        );
        msg.setArguments(args);
        return m_connection.call(msg, QDBus::BlockWithGui);
    }

    QDBusConnection m_connection;
};

DBusDataSource::DBusDataSource() : d{ new Private{} } {}
DBusDataSource::~DBusDataSource() {delete d;}

QByteArray DBusDataSource::getFacilitiesJson()
{
    auto relpy = d->methodCall("ListAll");
    return relpy.arguments().at(0).toByteArray();
}

// QString DBusDataSource::getValue(const QString& facility)
// {
//     auto relpy = d->methodCall("Get", {facility});
//     return relpy.arguments().at(0).toString();
// }

bool DBusDataSource::setValue(const QString& facility, const QString& value, QString& messages)
{
    auto relpy = d->methodCall("Set", {facility,value});
    messages = relpy.arguments().at(0).toStringList().join('\n');
    return relpy.arguments().size() == 2 && relpy.arguments().at(1).toInt() == 0;
}
