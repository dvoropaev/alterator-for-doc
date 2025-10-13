#ifndef DBUS_ERROR_H
#define DBUS_ERROR_H

#include <QString>

namespace alt
{
struct DBusError
{
    int code;
    QString text;
};
} // namespace alt

#endif // DBUS_ERROR_H
