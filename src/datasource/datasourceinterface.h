#ifndef DATASOURCEINTERFACE_H
#define DATASOURCEINTERFACE_H

#include <QDBusReply>
#include <QString>
#include <QStringList>

class DataSourceInterface
{
public:
    virtual ~DataSourceInterface() = default;

    virtual QStringList getObjectsPathsByIface(QString iface) = 0;

    virtual int rpmList(QStringList &result)                           = 0;
    virtual int rpmInstall(QStringList &result, const QString package) = 0;
    virtual int rpmRemove(QStringList &result, const QString package)  = 0;
    virtual int rpmInfo(QStringList &result, const QString package)    = 0;
    virtual int rpmFiles(QStringList &result, const QString package)   = 0;

    virtual int aptCheckApply(QStringList &stdout_result, QStringList &stderr_result, const QString package) = 0;
    virtual int aptCheckDistUpgrade(QStringList &stdout_result, QStringList &stderr_result)                  = 0;
    virtual int aptInstall(QStringList &result, const QString package)                                       = 0;
    virtual int aptApplyAsync(QStringList &result, const QString exclude_packages, const QString package)    = 0;
    virtual int aptDistUpgradeAsync(QStringList &result)                                                     = 0;
    virtual int aptReinstall(QStringList &result, const QString package)                                     = 0;
    virtual int aptRemove(QStringList &result, const QString package)                                        = 0;
    virtual int aptUpdate(QStringList &result)                                                               = 0;
    virtual int aptUpdateAsync(QStringList &result)                                                          = 0;
    virtual int aptListAllPackages(QStringList &result)                                                      = 0;
    virtual int aptLastUpdate(QStringList &result)                                                           = 0;

    virtual int repoAdd(QStringList &result, const QString repo)    = 0;
    virtual int repoList(QStringList &result)                       = 0;
    virtual int repoRemove(QStringList &result, const QString repo) = 0;
};

Q_DECLARE_INTERFACE(DataSourceInterface, "DataSourceInterface")

#endif //DATASOURCEINTERFACE_H
