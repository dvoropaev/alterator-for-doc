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
    auto &components = instance().packages;
    auto result = components.find(name);
    if (result == components.end())
    {
        return std::nullopt;
    }

    return std::make_optional(result->second);
}

void PackageRepository::add(std::shared_ptr<Package> package)
{
    instance().packages[QString::fromStdString(package->name)] = package;
}
} // namespace alt
