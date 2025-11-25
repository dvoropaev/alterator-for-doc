#include "packagerepository.h"

#include "dbus/dbusmanager.h"
#include <algorithm>
#include <iterator>

namespace alt
{
class PackageRepository::Private
{
public:
    static Error map(const DBusManager::Error &error);

public:
    std::map<std::string, Package> packages{};
    std::shared_ptr<DBusManager> source;
};

PackageRepository::PackageRepository(const std::shared_ptr<DBusManager> &source)
    : p(std::make_unique<Private>())
{
    p->source = source;
}

PackageRepository::~PackageRepository() = default;

std::optional<std::reference_wrapper<Package>> PackageRepository::get(const std::string &name)
{
    auto &packages = p->packages;
    auto result = packages.find(name);
    if (result == packages.end())
    {
        return std::nullopt;
    }

    return std::make_optional(std::ref(result->second));
}

tl::expected<PackageRepository::SimulationResult, PackageRepository::Error> PackageRepository::simulateApply(
    const std::set<std::string> &install, const std::set<std::string> &remove)
{
    return p->source->checkApply({install.begin(), install.end()}, {remove.begin(), remove.end()})
        .map([](TransactionResolutionResponse &&response) {
            return SimulationResult{
                .install = {std::make_move_iterator(response.onInstallation.begin()),
                            std::make_move_iterator(response.onInstallation.end())},
                .remove = {std::make_move_iterator(response.onRemoval.begin()),
                           std::make_move_iterator(response.onRemoval.end())},
                .critical_remove = {std::make_move_iterator(response.onExtraRemoval.begin()),
                                    std::make_move_iterator(response.onExtraRemoval.end())},
            };
        })
        .map_error(static_cast<decltype(&Private::map)>(Private::map));
}

tl::expected<void, PackageRepository::Error> PackageRepository::apply(const std::set<std::string> &install,
                                                                      const std::set<std::string> &remove,
                                                                      bool force,
                                                                      const Callback &callback)
{
    std::set<std::string> request;
    std::set_union(install.begin(), install.end(), remove.begin(), remove.end(), std::inserter(request, request.begin()));
    auto ignore = force ? p->source->getInstalledPackages().value_or(std::move(request)) : std::move(request);
    return p->source
        ->apply({install.begin(), install.end()},
                {remove.begin(), remove.end()},
                {ignore.begin(), ignore.end()},
                callback)
        .map_error(static_cast<decltype(&Private::map)>(p->map));
}

tl::expected<void, PackageRepository::Error> PackageRepository::updateSources(const Callback &callback)
{
    return p->source->updateSources(callback).map_error(static_cast<decltype(&Private::map)>(p->map));
}

tl::expected<std::chrono::system_clock::time_point, PackageRepository::Error> PackageRepository::lastUpdateOfSources()
{
    return p->source->lastUpdateOfSources().map_error(static_cast<decltype(&Private::map)>(p->map));
}

tl::expected<std::chrono::system_clock::time_point, PackageRepository::Error> PackageRepository::lastUpdateOfSystem()
{
    return p->source->lastDistUpgrade().map_error(static_cast<decltype(&Private::map)>(p->map));
}

PackageRepository::Error PackageRepository::Private::map(const DBusManager::Error &error)
{
    Error::Type err;
    switch (error.type)
    {
    case DBusManager::Error::Type::Reply:
    case DBusManager::Error::Type::InvalidInterface:
    case DBusManager::Error::Type::InvalidSignature:
        err = Error::Type::SourceUnavailable;
        break;
    default:
        err = Error::Type::OperationFailed;
        break;
    }
    return Error{
        .type = err,
        .details = error.text(),
    };
}

} // namespace alt
