#ifndef AOB_DATA_SOURCE_INTERFACE_H
#define AOB_DATA_SOURCE_INTERFACE_H

#include <QByteArray>
#include <QString>

namespace ao_builder
{
class DataSourceInterface
{
public:
    virtual ~DataSourceInterface() = default;

    virtual bool ping() = 0;

    virtual QStringList getLocalAppPaths() = 0;
    virtual QString getLocalAppInfo(const QString &path) = 0;

    virtual QStringList getCategoriesList() = 0;
    virtual QString getCategoryInfo(const QString &path) = 0;

    virtual QStringList getLegacyObjectsPaths() = 0;
    virtual QString getLegacyObjectInfo(const QString &path) = 0;

    virtual QStringList getObjectsPath(const QString &interface) = 0;
    virtual QString getObjectInfo(const QString &path, const QString &interface) = 0;
};

} // namespace ao_builder
#endif // AOB_DATA_SOURCE_INTERFACE_H
