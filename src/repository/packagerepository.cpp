#include "packagerepository.h"

namespace alt
{
PackageRepository::PackageRepository() = default;

PackageRepository &PackageRepository::instance()
{
    static PackageRepository instance{};
    return instance;
}

std::optional<std::shared_ptr<Package>> PackageRepository::get(const QString &name)
{
    auto &packages = instance().packages;
    auto result = packages.find(name);
    if (result == packages.end())
    {
        return std::nullopt;
    }

    return std::make_optional(result->second);
}

void PackageRepository::add(std::shared_ptr<Package> package)
{
    instance().packages[package->getPackageName()] = package;
}
} // namespace alt
