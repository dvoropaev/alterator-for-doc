#ifndef PACKAGE_REPOSITORY_H
#define PACKAGE_REPOSITORY_H

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <tl/expected.hpp>

using Callback = std::function<void(const std::string &chunk)>;

namespace alt
{
struct Package
{
    std::string name;
    bool installed = false;
};
class DBusManager;
class PackageRepository
{
public:
    struct Error;
    struct SimulationResult;

public:
    std::optional<std::reference_wrapper<Package>> get(const std::string &name);
    std::map<std::string, Package> &getAll();
    tl::expected<SimulationResult, Error> simulateApply(const std::set<std::string> &install,
                                                        const std::set<std::string> &remove);
    tl::expected<void, Error> apply(const std::set<std::string> &install,
                                    const std::set<std::string> &remove,
                                    bool force = false,
                                    const Callback &callback = nullptr);
    tl::expected<void, Error> updateSources(const Callback &callback = nullptr);
    tl::expected<std::chrono::system_clock::time_point, Error> lastUpdateOfSources();
    tl::expected<std::chrono::system_clock::time_point, Error> lastUpdateOfSystem();

public:
    PackageRepository(const PackageRepository &) = delete;
    PackageRepository(PackageRepository &&) = delete;
    PackageRepository &operator=(const PackageRepository &) = delete;
    PackageRepository &operator=(PackageRepository &&) = delete;
    explicit PackageRepository(const std::shared_ptr<DBusManager> &source);
    ~PackageRepository();

private:
    class Private;
    std::unique_ptr<Private> p;
};

struct PackageRepository::Error
{
    enum Type
    {
        SourceUnavailable = 401,
        OperationFailed = 402,
    } type;
    std::string details;
};

struct PackageRepository::SimulationResult
{
    std::set<std::string> install;
    std::set<std::string> remove;
    std::set<std::string> critical_remove;
};
} // namespace alt

#endif // PACKAGE_REPOSITORY_H
