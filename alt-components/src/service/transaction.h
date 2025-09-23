#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "dbus/dbuserror.h"
#include "dbus/dbusproxy.h"
#include "model/objects/component.h"

#include <functional>
#include <map>
#include <set>
#include <tl/expected.hpp>
#include <QString>

namespace alt
{
class Transaction
{
public:
    enum Status
    {
        New,
        Resolving,
        Denied,
        Allowed,
        Processing,
        Failed,
        Completed,
    };
    enum Action : uchar
    {
        Nothing,
        Install,
        Remove,
    };
    struct SystemModuleStatus
    {
        bool isManuallyInstalled = false;
        bool isBase = false;
        std::set<QString> relation;
    };

public:
    bool add(std::reference_wrapper<Component> component);
    void discard(std::reference_wrapper<Component> component);
    [[nodiscard]] tl::expected<std::reference_wrapper<Transaction>, DBusError> resolve();
    tl::expected<void, DBusError> run();
    [[nodiscard]] Status status() const;
    [[nodiscard]] SystemModuleStatus status(const Component &component) const;
    [[nodiscard]] SystemModuleStatus status(const Package &package) const;
    [[nodiscard]] Action action(const Component &component) const;
    [[nodiscard]] Action action(const Package &package) const;
    [[nodiscard]] std::map<QString, std::reference_wrapper<Component>> components() const;
    [[nodiscard]] std::map<QString, std::shared_ptr<Package>> packages() const;

private:
    [[nodiscard]] bool checkOnInstallation(const Component &component) const;
    [[nodiscard]] bool checkOnRemoval(const Component &component) const;
    void refreshPackageLists();
    void addPackages(std::reference_wrapper<Component> component);
    [[nodiscard]] tl::expected<std::reference_wrapper<Transaction>, DBusError> resolveComponents();
    [[nodiscard]] tl::expected<std::reference_wrapper<Transaction>, DBusError> resolvePackages();
    tl::expected<std::map<QString, std::shared_ptr<Package>>, DBusError> packagesToExclude();
    [[nodiscard]] std::map<QString, std::shared_ptr<Package>> slice(Action action) const;
    void setStatus(Status status);

private:
    static Status checkPermission(const Transaction &transaction);
    static Transaction extractResolvedPackages(PackageTransactionInfo &&info);
    static void defineBaseComponentRelation(Transaction &transaction);
    static std::set<QString> getRelatedComponents(const QString &package);
    template<typename T>
    static std::set<QString> toSet(const std::map<QString, T> &packages);

private:
    std::map<QString, std::reference_wrapper<Component>> m_components;
    std::map<QString, std::shared_ptr<Package>> m_packages;
    std::map<QString, SystemModuleStatus> componentStatuses;
    std::map<QString, SystemModuleStatus> packageStatuses;
    std::map<QString, Action> componentActions;
    std::map<QString, Action> packageActions;

    std::unique_ptr<Transaction> cacheResolved;
    Status m_status;
};
} // namespace alt

#include "transaction.tpp"

#endif // TRANSACTION_H
