#ifndef DBUSPROXY_H
#define DBUSPROXY_H

#include "model/objects/package.h"

#include <optional>
#include <set>
#include <QDBusInterface>
#include <QDBusReply>
#include <QString>

namespace alt
{
struct PackageTransactionInfo
{
    int exitCode;
    std::set<QString> onInstallation;
    std::set<QString> onRemoval;
    std::set<QString> onExtraRemoval;
    QStringList error;
};

class DBusProxy : public QObject
{
    Q_OBJECT
public:
    static DBusProxy &get()
    {
        static DBusProxy instance;
        return instance;
    }

    static QDBusConnection &rawConnection() { return get().connection; };

public:
    const FilterOptions &getFilterOptions();
    QString getCurrentEditionInfo();

    std::optional<QStringList> getBatchComponentsInfo();
    std::optional<QSet<QString>> getBatchComponentsStatus();
    std::optional<QStringList> getBatchCategoriesInfo();

    QString getComponentDescription(const QString &path);
    QString getCategoryDescription(const QString &category);

    PackageTransactionInfo getActualPackageListInTransaction(std::set<QString> packagesOnInstallation,
                                                             const std::set<QString> &packagesOnRemoval);

    std::optional<QDate> getDateResult(const QString &methodName);
    void resetManagerLocale();

public:
    DBusProxy(const DBusProxy &) = delete;
    DBusProxy &operator=(const DBusProxy &) = delete;
    DBusProxy(DBusProxy &&) = delete;
    DBusProxy &operator=(DBusProxy &&) = delete;

private:
    DBusProxy() = default;
    ~DBusProxy() override = default;

    QString getCurrentArch();
    QString getCurrentKernelFlavour();
    QSet<QString> getCurrentDesktopEnvironments();
    QString getCurrentLanguage();

public:
    template<typename T>
    std::optional<T> getResult(const std::optional<QVariantList> &args, const QString &methodName);

    template<class... Args>
    std::optional<QVariantList> callDBus(QDBusInterface *iface, QString method, Args... args);

    template<typename T>
    std::optional<T> getResultAndError(const QVariantList &args, const QString &methodName);

    std::optional<QString> checkSuccess(const QVariantList &args, const QString &methodName);

signals:
    void errorOccured(const QtMsgType &level, const QString &message);

private:
    QDBusConnection connection = QDBusConnection::systemBus();
};
} // namespace alt

#include "dbusproxy.tpp"

#endif // DBUSPROXY_H
