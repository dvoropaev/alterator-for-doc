#ifndef PACKAGE_REPOSITORY_H
#define PACKAGE_REPOSITORY_H

#include "model/objects/package.h"

#include <map>
#include <optional>
#include <QString>

namespace alt
{
class PackageRepository
{
public:
    static PackageRepository &instance();
    static std::optional<std::shared_ptr<Package>> get(const QString &name);
    static std::map<QString, std::shared_ptr<Package>> &getAll();

    static void add(std::shared_ptr<Package> package);

public:
    PackageRepository(const PackageRepository &) = delete;
    PackageRepository(PackageRepository &&) = delete;
    PackageRepository &operator=(const PackageRepository &) = delete;
    PackageRepository &operator=(PackageRepository &&) = delete;

private:
    PackageRepository();

private:
    std::map<QString, std::shared_ptr<Package>> packages{};
};
} // namespace alt

#endif // PACKAGE_REPOSITORY_H
