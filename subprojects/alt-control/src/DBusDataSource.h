#pragma once

#include <QtGlobal>

class DBusDataSource
{
public:
    DBusDataSource();
    ~DBusDataSource();

    QByteArray getFacilitiesJson();
    // QStringList listFacilities();
    // QStringList listValues(const QString& facility);
    // QStringList getHelp(const QString& facility);
    // QString     getSummary(const QString& facility);
    // QString getValue(const QString& facility);
    bool setValue(const QString& facility, const QString& value, QString& messages);

private:
    class Private;
    Private* d;
};

