#ifndef AOB_DATA_SOURCE_DBUS_IMPL_H
#define AOB_DATA_SOURCE_DBUS_IMPL_H

#include "datasourceinterface.h"

#include <QString>

namespace ao_builder
{
class DataSourseDbusImplPrivate;

class DataSourceDBusImpl : public DataSourceInterface
{
public:
    DataSourceDBusImpl(const QString &serviceName);
    ~DataSourceDBusImpl() override;

public:
    bool ping() override;

    QStringList getLocalAppPaths() override;
    QString getLocalAppInfo(const QString &path) override;

    QStringList getCategoriesList() override;
    QString getCategoryInfo(const QString &path) override;

    QStringList getLegacyObjectsPaths() override;
    QString getLegacyObjectInfo(const QString &path) override;

    QStringList getObjectsPath(const QString &interface) override;
    QString getObjectInfo(const QString &path, const QString &interface) override;

public:
    DataSourceDBusImpl(const DataSourceDBusImpl &) = delete;
    DataSourceDBusImpl(DataSourceDBusImpl &&) = delete;
    DataSourceDBusImpl &operator=(const DataSourceDBusImpl &) = delete;
    DataSourceDBusImpl &operator=(DataSourceDBusImpl &&) = delete;

private:
    QByteArray getObjectInfo(const QString &ifaceName, const QString &path, const QString &methodName);
    QByteArray getObjectInfoByName(const QString &ifaceName,
                                   const QString &path,
                                   const QString &objectName,
                                   const QString &methodName);

    QStringList getPathsByInterface(const QString &ifaceName);
    QStringList getObjectsList(const QString &listMethodName, const QString &path, const QString &interfaceName);

private:
    DataSourseDbusImplPrivate *d;
};
} // namespace ao_builder

#endif // AOB_DATA_SOURCE_DBUS_IMPL_H
