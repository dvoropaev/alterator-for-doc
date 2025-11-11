#pragma once

#include <QtGlobal>

class DBusDataSource
{
public:
    DBusDataSource();
    ~DBusDataSource();

    QStringList listFacilities();
    QStringList getAllInfo();

    bool getValue(const QString& path, QString& result, QString& messages);
    bool setValue(const QString& path, const QString& value, QString& messages);
    bool listStates(const QString& path, QStringList& result, QString& messages);

private:
    class Private;
    Private* d;
};

