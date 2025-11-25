#include "transactionservice.h"
#include "repository/componentrepository.h"
#include "repository/editionrepository.h"
#include "repository/packagerepository.h"
#include "transaction.h"

#include <boost/range/algorithm/transform.hpp>
#include <chrono>
#include <functional>

namespace alt
{
class TransactionService::Private
{
public:
    bool checkOnInstallation(const Transaction &transaction, const Component &component) const;
    bool checkOnRemoval(const Transaction &transaction, const Component &component) const;
    tl::expected<std::reference_wrapper<Transaction>, Error> resolveComponents(Transaction &transaction);
    tl::expected<std::reference_wrapper<Transaction>, Error> resolvePackages(Transaction &transaction);
    void defineBaseComponentRelation(Transaction &transaction);
    std::set<std::string> getRelatedComponents(const std::string &package);
    Transaction::Status checkPermission(const Transaction &transaction);
    void refreshPackageLists(Transaction &transaction);
    void addPackages(Transaction &transaction, std::reference_wrapper<Component> component);
    Transaction extractResolvedPackages(PackageRepository::SimulationResult &&response);
    static Error map(const PackageRepository::Error &error);

public:
    std::shared_ptr<PackageRepository> packageRepository;
    std::shared_ptr<ComponentRepository> componentRepository;
    std::shared_ptr<EditionRepository> editionRepository;

    std::map<Transaction::TimePoint, Transaction> transactions;
    std::map<Transaction::TimePoint, Transaction> resolved;
    SafeMode safeMode = SafeMode{.value = SafeMode::Target::All};
};

void TransactionService::SafeMode::set(Target variant)
{
    value = static_cast<Target>(variant | value);
}

void TransactionService::SafeMode::reset(Target variant)
{
    value = static_cast<Target>(~variant & value);
}

bool TransactionService::SafeMode::test(Target variant) const
{
    return value & variant;
}

TransactionService::SafeMode::operator TransactionService::SafeMode::Target() const
{
    return value;
}

TransactionService::TransactionService(const std::shared_ptr<PackageRepository> &packages,
                                       const std::shared_ptr<ComponentRepository> &components,
                                       const std::shared_ptr<EditionRepository> &editions)
    : p(std::make_unique<TransactionService::Private>())
{
    p->packageRepository = packages;
    p->componentRepository = components;
    p->editionRepository = editions;
}

TransactionService::~TransactionService() = default;

Transaction &TransactionService::create()
{
    Transaction newTransaction;
    newTransaction.wasCreated = std::chrono::system_clock::now();
    p->transactions.insert({newTransaction.wasCreated, std::move(newTransaction)});
    return current();
}

Transaction &TransactionService::current()
{
    if (p->transactions.empty())
    {
        return create();
    }
    auto end = p->transactions.end();
    return (--end)->second;
}

TransactionService::SafeMode TransactionService::safeMode()
{
    return p->safeMode;
}

void TransactionService::setSafeMode(SafeMode mode)
{
    p->safeMode = mode;
}

bool TransactionService::add(Transaction &transaction, std::reference_wrapper<Component> component)
{
    if (transaction.m_components.contains(component.get().name))
    {
        return false;
    }

    transaction.setStatus(Transaction::Status::New);
    p->resolved.erase(transaction.wasCreated);

    transaction.m_components.insert_or_assign(component.get().name, component);
    if (component.get().state == ComponentState::installed)
    {
        transaction.componentActions[component.get().name] = Transaction::Action::Remove;
    }
    else if (component.get().state == ComponentState::not_installed)
    {
        transaction.componentActions[component.get().name] = Transaction::Action::Install;
    }

    p->addPackages(transaction, component);

    return true;
}

void TransactionService::discard(Transaction &transaction, std::reference_wrapper<Component> component)
{
    if (!transaction.m_components.contains(component.get().name))
    {
        return;
    }

    transaction.setStatus(Transaction::Status::New);
    p->resolved.erase(transaction.wasCreated);

    const auto &name = component.get().name;
    transaction.m_components.erase(name);
    transaction.componentActions.erase(name);
    transaction.componentStatuses.erase(name);
    p->refreshPackageLists(transaction);
}

tl::expected<std::reference_wrapper<Transaction>, TransactionService::Error> TransactionService::resolve(
    Transaction &transaction)
{
    using Result = decltype(resolve(transaction));
    if (transaction.status() == Transaction::Status::Processing)
    {
        return tl::unexpected(Error{
            .type = Error::Type::AlreadyInProgress,
            .details = "The transaction is already being processed!",
        });
    }
    else if (transaction.status() == Transaction::Status::Resolving)
    {
        return tl::unexpected(Error{
            .type = Error::Type::NotResolved,
            .details = "The transaction has not been resolved yet!",
        });
    }
    else if (transaction.components().empty())
    {
        return tl::unexpected(Error{
            .type = Error::Type::InvalidRequest,
            .details = "The transaction contains no components!",
        });
    }

    auto found = p->resolved.find(transaction.wasCreated);
    if (found != p->resolved.end()
        && (transaction.status() == Transaction::Status::Allowed || transaction.status() == Transaction::Status::Denied))
    {
        auto permission = p->checkPermission(found->second);
        transaction.setStatus(permission);
        found->second.setStatus(permission);
        return std::ref(found->second);
    }

    transaction.setStatus(Transaction::Status::Resolving);
    return p->resolveComponents(transaction)
        .and_then([&transaction, this](std::reference_wrapper<Transaction> resolved) -> Result {
            transaction.setStatus(p->checkPermission(resolved));
            resolved.get().setStatus(transaction.status());
            return resolved;
        })
        .or_else([&transaction, this](const Error &error) -> Result {
            transaction.setStatus(Transaction::Status::Denied);
            auto resolvedIter = p->resolved.find(transaction.wasCreated);
            if (resolvedIter != p->resolved.end())
            {
                resolvedIter->second.setStatus(Transaction::Status::Denied);
            }
            return tl::unexpected(error);
        });
}

tl::expected<void, TransactionService::Error> TransactionService::run(Transaction &transaction, const Callback &callback)
{
    if (transaction.status() == Transaction::Status::Processing)
    {
        return tl::unexpected(Error{
            .type = Error::Type::AlreadyInProgress,
            .details = "The transaction is already being processed!",
        });
    }
    else if (transaction.status() == Transaction::Status::Resolving || transaction.status() == Transaction::Status::New)
    {
        return tl::unexpected(Error{
            .type = Error::Type::NotResolved,
            .details = "The transaction has not been resolved yet!",
        });
    }
    else if (transaction.status() == Transaction::Status::Denied)
    {
        return tl::unexpected(Error{
            .type = Error::Type::Denied,
            .details = "The transaction is denied!",
        });
    }
    else if (transaction.components().empty())
    {
        return tl::unexpected(Error{
            .type = Error::Type::InvalidRequest,
            .details = "The transaction contains no components!",
        });
    }

    transaction.setStatus(Transaction::Status::Processing);
    std::set<std::string> install;
    std::set<std::string> remove;
    for (const auto &[pkg, action] : transaction.packageActions)
    {
        (action == Transaction::Action::Install ? install : remove).insert(pkg);
    }

    return p->packageRepository
        ->apply(install,
                remove,
                !TransactionService::safeMode().test(TransactionService::SafeMode::Target::Manually),
                callback)
        .map([&transaction]() { transaction.setStatus(Transaction::Status::Completed); })
        .map_error([&transaction, this](const PackageRepository::Error &error) {
            transaction.setStatus(Transaction::Status::Failed);
            return p->map(error);
        });
}

tl::expected<std::reference_wrapper<Transaction>, TransactionService::Error>
TransactionService::Private::resolveComponents(Transaction &transaction)
{
    using Result = decltype(resolveComponents(transaction));
    return resolvePackages(transaction).and_then([this](std::reference_wrapper<Transaction> resolved) -> Result {
        const auto currentEdition = editionRepository->current();
        auto sections = currentEdition ? currentEdition->get().sections : std::vector<Section>();
        auto iter = std::find_if(sections.begin(), sections.end(), [](const Section &section) {
            return section.name == "base";
        });
        auto baseComponents = iter != sections.end() ? iter->components : std::unordered_set<std::string>();

        for (auto &[name, component] : componentRepository->getAll<Component>())
        {
            if (checkOnRemoval(resolved.get(), component))
            {
                resolved.get().m_components.insert_or_assign(name, std::ref(component));
                resolved.get().componentActions[name] = Transaction::Action::Remove;
                resolved.get().componentStatuses[name] = {
                    .isManuallyInstalled = false,
                    .isBase = baseComponents.contains(component.name),
                    .relation = {},
                };
            }
            else if (checkOnInstallation(resolved.get(), component))
            {
                resolved.get().m_components.insert_or_assign(name, std::ref(component));
                resolved.get().componentActions[name] = Transaction::Action::Install;
            }
        }

        return resolved;
    });
}

tl::expected<std::reference_wrapper<Transaction>, TransactionService::Error> TransactionService::Private::resolvePackages(
    Transaction &transaction)
{
    std::set<std::string> install;
    std::set<std::string> remove;
    for (const auto &[pkg, action] : transaction.packageActions)
    {
        (action == Transaction::Action::Install ? install : remove).insert(pkg);
    }

    return packageRepository->simulateApply(install, remove)
        .map([&transaction, this](PackageRepository::SimulationResult &&response) {
            resolved[transaction.wasCreated] = extractResolvedPackages(std::move(response));
            return std::ref(resolved[transaction.wasCreated]);
        })
        .map_error(static_cast<decltype(&Private::map)>(Private::map));
}

Transaction TransactionService::Private::extractResolvedPackages(PackageRepository::SimulationResult &&response)
{
    Transaction resolved;
    for (const auto &name : response.install)
    {
        resolved.m_packages.insert(name);
        resolved.packageActions[name] = Transaction::Action::Install;
    }
    for (const auto &name : response.critical_remove)
    {
        resolved.m_packages.insert(name);
        resolved.packageStatuses[name] = {
            .isManuallyInstalled = true,
            .relation = {},
        };
        resolved.packageActions[name] = Transaction::Action::Remove;
    }
    for (const auto &name : response.remove)
    {
        resolved.m_packages.insert(name);
        resolved.packageActions[name] = Transaction::Action::Remove;
    }
    defineBaseComponentRelation(resolved);

    return resolved;
}

void TransactionService::Private::defineBaseComponentRelation(Transaction &transaction)
{
    std::vector<Section> empty;
    const auto currentEdition = editionRepository->current();
    const auto &sections = currentEdition ? currentEdition->get().sections : empty;
    auto sectionIter = std::find_if(sections.begin(), sections.end(), [](const Section &section) {
        return section.name == "base";
    });

    if (sectionIter != sections.end())
    {
        const auto &sectionBase = *sectionIter;
        for (auto &[name, status] : transaction.packageStatuses)
        {
            auto componentNames = getRelatedComponents(name);
            auto baseComponentNames = std::set<std::string>{};
            std::copy_if(componentNames.begin(),
                         componentNames.end(),
                         std::inserter(baseComponentNames, baseComponentNames.begin()),
                         [&sectionBase](const std::string &name) { return sectionBase.components.contains(name); });
            if (!baseComponentNames.empty())
            {
                status.isBase = true;
                status.relation = std::move(baseComponentNames);
            }
        }
    }
}

std::set<std::string> TransactionService::Private::getRelatedComponents(const std::string &package)
{
    std::set<std::string> result;

    for (const auto &[name, component] : componentRepository->getAll<Component>())
    {
        if (std::find_if(component.packages.begin(),
                         component.packages.end(),
                         [&package](const Component::Package &pkg) { return package == pkg.name; })
            != component.packages.end())
        {
            result.insert(component.name);
        }
    }

    return result;
}

void TransactionService::Private::addPackages(Transaction &transaction, std::reference_wrapper<Component> component)
{
    if (component.get().state == ComponentState::installed)
    {
        for (const auto &pkg : component.get().packages)
        {
            if (pkg.installed)
            {
                transaction.m_packages.insert(pkg.getPackageName());
                transaction.packageActions[pkg.getPackageName()] = Transaction::Action::Remove;
            }
        }
    }
    else if (component.get().state == ComponentState::not_installed)
    {
        for (const auto &pkg : component.get().packages)
        {
            if (!pkg.installed)
            {
                transaction.m_packages.insert(pkg.getPackageName());
                transaction.packageActions[pkg.getPackageName()] = Transaction::Action::Install;
            }
        }
    }
}

void TransactionService::Private::refreshPackageLists(Transaction &transaction)
{
    transaction.m_packages.clear();
    transaction.packageActions.clear();
    transaction.packageStatuses.clear();
    for (const auto &[name, component] : transaction.m_components)
    {
        addPackages(transaction, component);
    }
}

Transaction::Status TransactionService::Private::checkPermission(const Transaction &transaction)
{
    bool any = std::any_of(transaction.packageStatuses.begin(),
                           transaction.packageStatuses.end(),
                           [this](const std::pair<std::string, Transaction::SystemModuleStatus> &pair) {
                               const auto &[name, status] = pair;
                               return (status.isBase && safeMode.test(TransactionService::SafeMode::Base))
                                      || (status.isManuallyInstalled
                                          && safeMode.test(TransactionService::SafeMode::Manually));
                           });
    return any ? Transaction::Status::Denied : Transaction::Status::Allowed;
}

bool TransactionService::Private::checkOnInstallation(const Transaction &transaction, const Component &component) const
{
    if (component.state == ComponentState::installed || component.packages.empty())
    {
        return false;
    }

    return std::all_of(component.packages.begin(),
                       component.packages.end(),
                       [&transaction](const Component::Package &pkg) {
                           auto found = transaction.packageActions.find(pkg.getPackageName());
                           return (pkg.installed
                                   && !(found != transaction.packageActions.end()
                                        && found->second == Transaction::Action::Remove))
                                  || (!pkg.installed
                                      && (found != transaction.packageActions.end()
                                          && found->second == Transaction::Action::Install));
                       });
}

bool TransactionService::Private::checkOnRemoval(const Transaction &transaction, const Component &component) const
{
    if (component.state != ComponentState::installed)
    {
        return false;
    }

    return std::any_of(component.packages.begin(),
                       component.packages.end(),
                       [&transaction](const Component::Package &pkg) {
                           auto found = transaction.packageActions.find(pkg.getPackageName());
                           return found != transaction.packageActions.end()
                                  && found->second == Transaction::Action::Remove;
                       });
}

TransactionService::Error TransactionService::Private::map(const PackageRepository::Error &error)
{
    Error::Type err;
    switch (error.type)
    {
    case PackageRepository::Error::Type::OperationFailed:
        err = Error::Type::ServerOperationFailed;
        break;
    case PackageRepository::Error::Type::SourceUnavailable:
        err = Error::Type::SourceUnavailable;
    default:
        break;
    }
    return Error{
        .type = err,
        .details = error.details,
    };
}

} // namespace alt
