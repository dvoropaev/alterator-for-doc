#ifndef DBUSDATASOURCE_H
#define DBUSDATASOURCE_H

#include "datasource/datasourceinterface.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QStringList>
#include <qlist.h>

class DBusDataSourcePrivate;
struct AlteratorExecutorReply;

class DBusDataSource : public QObject, public DataSourceInterface
{
    Q_OBJECT
    Q_INTERFACES(DataSourceInterface)
public:
    DBusDataSource(QString serviceName, QString path, QString ifaceName, QString methodName);
    ~DBusDataSource();

    QStringList getObjectsPathsByIface(QString iface) override;

    int rpmList(QStringList &result) override;
    int rpmInstall(QStringList &result, const QString package) override;
    int rpmRemove(QStringList &result, const QString package) override;
    int rpmInfo(QStringList &result, const QString package) override;
    int rpmFiles(QStringList &result, const QString package) override;

    int aptCheckApply(QStringList &stdout_result, QStringList &stderr_result, const QString package) override;
    int aptCheckDistUpgrade(QStringList &stdout_result, QStringList &stderr_result) override;
    int aptApplyAsync(QStringList &result, const QString exclude_packages, const QString packages) override;
    int aptDistUpgradeAsync(QStringList &result) override;
    int aptInstall(QStringList &result, const QString package) override;
    int aptReinstall(QStringList &result, const QString package) override;
    int aptRemove(QStringList &result, const QString package) override;
    int aptUpdate(QStringList &result) override;
    int aptUpdateAsync(QStringList &result) override;
    int aptListAllPackages(QStringList &result) override;
    int aptLastUpdate(QStringList &result) override;

    int repoAdd(QStringList &result, const QString repo) override;
    int repoList(QStringList &result) override;
    int repoRemove(QStringList &result, const QString repo) override;

signals:
    void onAptInstallAsync(QString message);
    void onAptUpdateAsync(QString message);
    void onAptDistUpgradeAsync(QString message);
    void closeWaitDialog();

private slots:
    void onAptInstalledAsync(QString message);
    void onAptUpdatedAsync(QString message);
    void onAptDistUpgradedAsync(QString message);

private:
    QStringList objectByPath(QString iface);

    AlteratorExecutorReply dbusCall(const QString &path,
                                    const QString &interface,
                                    const QString &method,
                                    const QString &arg = QString());

private:
    QDBusConnection m_connection;
    QString m_serviceName{};
    QString m_alteratorPath{};
    QString m_alteratorIfaceName{};
    QString m_alteratorMethodName{};

    QString m_aptPath{};
    QString m_rpmPath{};
    QString m_repoPath{};
};

#endif //DBUSDATASOURCE_H
