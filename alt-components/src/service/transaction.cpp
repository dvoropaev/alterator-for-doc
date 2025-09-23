#include "transaction.h"

#include "constants.h"
#include "dbus/dbusproxy.h"
#include "model/model.h"
#include "model/objects/component.h"
#include "repository/componentrepository.h"
#include "repository/packagerepository.h"
#include "transactionservice.h"
#include <algorithm>
#include <boost/range/algorithm/transform.hpp>
#include <functional>
#include <tl/expected.hpp>

namespace alt
{
bool Transaction::add(std::reference_wrapper<Component> component)
{
    setStatus(Status::New);
    cacheResolved.reset();

    m_components.insert_or_assign(component.get().name, component);
    if (component.get().state == ComponentState::installed)
    {
        componentActions[component.get().name] = Action::Remove;
    }
    else if (component.get().state == ComponentState::not_installed)
    {
        componentActions[component.get().name] = Action::Install;
    }

    addPackages(component);

    return true;
}

void Transaction::discard(std::reference_wrapper<Component> component)
{
    if (!m_components.contains(component.get().name))
    {
        return;
    }

    setStatus(Status::New);
    cacheResolved.reset();
    const auto &name = component.get().name;
    m_components.erase(name);
    componentActions.erase(name);
    componentStatuses.erase(name);
    refreshPackageLists();
}

tl::expected<std::reference_wrapper<Transaction>, DBusError> Transaction::resolve()
{
    if (cacheResolved && (status() == Status::Allowed || status() == Status::Denied))
    {
        setStatus(checkPermission(*cacheResolved));
        return std::ref(*cacheResolved);
    }

    setStatus(Status::Resolving);
    auto result = resolveComponents();
    if (!result.has_value())
    {
        setStatus(Status::Denied);
    }
    else
    {
        setStatus(checkPermission(result.value()));
    }

    return result;
}

tl::expected<void, DBusError> Transaction::run()
{
    QDBusInterface iface(ALTERATOR_MANAGER_SERVICE_NAME, APT1_PATH, APT1_INTERFACE_NAME, DBusProxy::rawConnection());
    constexpr auto dayInSeconds = 24 * 60 * 60 * 1000;
    iface.setTimeout(dayInSeconds);

    QStringList applyList;
    for (const auto &[name, action] : packageActions)
    {
        applyList << (action == Action::Install ? name : QString("%1-").arg(name));
    }
    QString applyQuery = applyList.join(' ');

    const auto excludePackages = packagesToExclude();
    if (!excludePackages.has_value())
    {
        setStatus(Transaction::Status::Failed);
        return tl::make_unexpected(excludePackages.error());
    }

    setStatus(Transaction::Status::Processing);
    QStringList excludeList;
    for (const auto &[name, _] : excludePackages.value())
    {
        excludeList << name;
    }

    const QString excludeQuery = QString("\"%1\"").arg(excludeList.join(' '));
    const auto dbusResult = DBusProxy::get().callDBus(&iface, APT1_APPLY_METHOD_NAME, excludeQuery, applyQuery);
    if (!dbusResult.has_value())
    {
        setStatus(Status::Failed);
        return tl::make_unexpected(DBusError{.code = 101, .text = ""});
    }

    auto errorMessage = DBusProxy::get().checkSuccess(dbusResult.value(), APT1_APPLY_METHOD_NAME);
    if (errorMessage.has_value())
    {
        setStatus(Status::Failed);
        return tl::make_unexpected(DBusError{.code = dbusResult->first().toInt(), .text = errorMessage.value()});
    }

    setStatus(Status::Completed);
    return {};
}

Transaction::Status Transaction::status() const
{
    return m_status;
}

Transaction::Action Transaction::action(const Component &component) const
{
    auto iter = componentActions.find(component.name);
    return iter != componentActions.end() ? iter->second : Action::Nothing;
}

Transaction::Action Transaction::action(const Package &package) const
{
    auto iter = packageActions.find(package.getPackageName());
    return iter != packageActions.end() ? iter->second : Action::Nothing;
}

Transaction::SystemModuleStatus Transaction::status(const Component &component) const
{
    auto iter = componentStatuses.find(component.name);
    return iter != componentStatuses.end() ? iter->second : SystemModuleStatus{};
}

Transaction::SystemModuleStatus Transaction::status(const Package &package) const
{
    auto iter = packageStatuses.find(package.getPackageName());
    return iter != packageStatuses.end() ? iter->second : SystemModuleStatus{};
}

[[nodiscard]] std::map<QString, std::reference_wrapper<Component>> Transaction::components() const
{
    return m_components;
}

[[nodiscard]] std::map<QString, std::shared_ptr<Package>> Transaction::packages() const
{
    return m_packages;
}

void Transaction::refreshPackageLists()
{
    m_packages.clear();
    packageActions.clear();
    packageStatuses.clear();
    for (const auto &[name, component] : m_components)
    {
        addPackages(component);
    }
}

void Transaction::addPackages(std::reference_wrapper<Component> component)
{
    if (component.get().state == ComponentState::installed)
    {
        for (const auto &pkg : component.get().packages)
        {
            if (pkg->installed)
            {
                m_packages.insert_or_assign(pkg->getPackageName(), pkg);
                packageActions[pkg->getPackageName()] = Action::Remove;
            }
        }
    }
    else if (component.get().state == ComponentState::not_installed)
    {
        for (const auto &pkg : component.get().packages)
        {
            if (!pkg->installed)
            {
                m_packages.insert_or_assign(pkg->getPackageName(), pkg);
                packageActions[pkg->getPackageName()] = Action::Install;
            }
        }
    }
}

tl::expected<std::reference_wrapper<Transaction>, DBusError> Transaction::resolveComponents()
{
    auto result = resolvePackages();
    if (!result.has_value())
    {
        return result;
    }

    auto sections = Model::current_edition ? Model::current_edition->sections : QList<Section>();
    auto iter = std::find_if(sections.begin(), sections.end(), [](const Section &section) {
        return section.name == "base";
    });
    auto baseComponents = iter != sections.end() ? iter->components : QSet<QString>();

    auto transaction = *result;
    for (auto &[name, component] : ComponentRepository::getAll<Component>())
    {
        if (transaction.get().checkOnRemoval(component))
        {
            transaction.get().m_components.insert_or_assign(name, std::ref(component));
            transaction.get().componentActions[name] = Action::Remove;
            transaction.get().componentStatuses[name] = {
                .isManuallyInstalled = false,
                .isBase = baseComponents.contains(component.name),
                .relation = {},
            };
        }
        else if (transaction.get().checkOnInstallation(component))
        {
            transaction.get().m_components.insert_or_assign(name, std::ref(component));
            transaction.get().componentActions[name] = Action::Install;
        }
    }

    return std::ref(*this->cacheResolved);
}

tl::expected<std::reference_wrapper<Transaction>, DBusError> Transaction::resolvePackages()
{
    auto result = DBusProxy::get().getActualPackageListInTransaction(toSet(slice(Action::Install)),
                                                                     toSet(slice(Action::Remove)));
    if (result.exitCode != 0)
    {
        return tl::make_unexpected(DBusError{.code = result.exitCode, .text = result.error.join("\n")});
    }

    Transaction resolved = extractResolvedPackages(std::move(result));
    defineBaseComponentRelation(resolved);
    this->cacheResolved = std::make_unique<Transaction>(std::move(resolved));
    return std::ref(*this->cacheResolved);
}

Transaction Transaction::extractResolvedPackages(PackageTransactionInfo &&info)
{
    Transaction resolved;
    for (const auto &name : info.onInstallation)
    {
        auto pkg = PackageRepository::get(name);
        if (pkg.has_value())
        {
            resolved.m_packages[name] = pkg.value();
            resolved.packageActions[name] = Action::Install;
        }
    }
    for (const auto &name : info.onExtraRemoval)
    {
        auto pkg = PackageRepository::get(name);
        if (pkg.has_value())
        {
            resolved.m_packages[name] = pkg.value();
            resolved.packageStatuses[name] = {
                .isManuallyInstalled = true,
                .relation = {},
            };
            resolved.packageActions[name] = Action::Remove;
        }
    }
    for (const auto &name : info.onRemoval)
    {
        auto pkg = PackageRepository::get(name);
        auto rmPkgs = resolved.slice(Action::Remove);
        if (pkg.has_value() && rmPkgs.find(name) == rmPkgs.end())
        {
            resolved.m_packages[name] = pkg.value();
            resolved.packageActions[name] = Action::Remove;
        }
    }

    return resolved;
}

void Transaction::defineBaseComponentRelation(Transaction &transaction)
{
    QList<Section> empty;
    const auto &sections = Model::current_edition ? Model::current_edition->sections : empty;
    auto sectionIter = std::find_if(sections.begin(), sections.end(), [](const Section &section) {
        return section.name == "base";
    });

    if (sectionIter != sections.end())
    {
        const auto &sectionBase = *sectionIter;
        for (auto &[name, pkg] : transaction.slice(Action::Remove))
        {
            auto &status = transaction.packageStatuses[name];
            auto componentNames = getRelatedComponents(name);
            auto baseComponentNames = std::set<QString>{};
            for (const auto &name : componentNames)
            {
                if (sectionBase.components.contains(name))
                {
                    baseComponentNames.insert(name);
                }
            }
            if (!baseComponentNames.empty())
            {
                status.isBase = true;
                status.relation = std::move(baseComponentNames);
            }
        }
    }
}

bool Transaction::checkOnInstallation(const Component &component) const
{
    if (component.state == ComponentState::installed || component.packages.empty())
    {
        return false;
    }

    auto remove = slice(Action::Remove);
    auto install = slice(Action::Install);
    return std::all_of(component.packages.begin(), component.packages.end(), [&](const std::shared_ptr<Package> &pkg) {
        return (pkg->installed && !remove.contains(pkg->getPackageName()))
               || (!pkg->installed && install.contains(pkg->getPackageName()));
    });
}

bool Transaction::checkOnRemoval(const Component &component) const
{
    if (component.state != ComponentState::installed)
    {
        return false;
    }

    auto remove = slice(Action::Remove);
    return std::any_of(component.packages.begin(), component.packages.end(), [&](const std::shared_ptr<Package> &pkg) {
        return remove.contains(pkg->getPackageName());
    });
}

tl::expected<std::map<QString, std::shared_ptr<Package>>, DBusError> Transaction::packagesToExclude()
{
    std::map<QString, std::shared_ptr<Package>> packages;
    // NOTE(sheriffkorov): it is crutch due to weird interface apt
    for (const auto &[name, pkg] : m_packages)
    {
        packages[name] = pkg;
        break;
    }

    if (TransactionService::safeMode().test(TransactionService::SafeMode::Manually))
    {
        return packages;
    }

    auto resolved = resolve();
    if (status() == Status::Denied)
    {
        return tl::make_unexpected(resolved.error());
    }

    for (const auto &[name, pkg] : resolved->get().m_packages)
    {
        auto status = resolved->get().packageStatuses[name];
        if (status.isManuallyInstalled)
        {
            packages[name] = pkg;
        }
    }
    return packages;
}

std::set<QString> Transaction::getRelatedComponents(const QString &package)
{
    std::set<QString> result;
    for (const auto &[name, component] : ComponentRepository::getAll<Component>())
    {
        auto pkg = PackageRepository::get(package);
        if (!pkg.has_value())
        {
            continue;
        }

        if (component.packages.find(*pkg) != component.packages.end())
        {
            result.insert(component.name);
        }
    }

    return result;
}

void Transaction::setStatus(Status status)
{
    m_status = status;
    if (cacheResolved)
    {
        cacheResolved->setStatus(status);
    }
}

std::map<QString, std::shared_ptr<Package>> Transaction::slice(Action action) const
{
    std::map<QString, std::shared_ptr<Package>> result;
    for (const auto &[name, curAction] : packageActions)
    {
        if (curAction == action)
        {
            result[name] = m_packages.at(name);
        }
    }
    return result;
}

Transaction::Status Transaction::checkPermission(const Transaction &transaction)
{
    auto safeMode = TransactionService::safeMode();
    bool any = std::any_of(transaction.packageStatuses.begin(),
                           transaction.packageStatuses.end(),
                           [safeMode](const std::pair<QString, SystemModuleStatus> &pair) {
                               const auto &[name, status] = pair;
                               return (status.isBase && safeMode.test(TransactionService::SafeMode::Base))
                                      || (status.isManuallyInstalled
                                          && safeMode.test(TransactionService::SafeMode::Manually));
                           });

    return any ? Status::Denied : Status::Allowed;
}
} // namespace alt
