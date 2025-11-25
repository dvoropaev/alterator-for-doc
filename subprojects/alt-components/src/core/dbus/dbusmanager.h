#ifndef DBUS_MANAGER_H
#define DBUS_MANAGER_H

#include "entity/component.h"
#include "interface/ilogger.h"

#include <chrono>
#include <string>
#include <tl/expected.hpp>
#include <unordered_set>

namespace alt
{
struct TransactionResolutionResponse
{
    std::unordered_set<std::string> onInstallation;
    std::unordered_set<std::string> onRemoval;
    std::unordered_set<std::string> onExtraRemoval;
};

struct EntityDataResponse
{
    std::unordered_set<std::string> components;
    std::unordered_set<std::string> categories;
};

using Callback = std::function<void(const std::string &chunk)>;

class DBusManager
{
public:
    struct Error;

public:
    void setLogger(const std::shared_ptr<ILogger> &logger);
    tl::expected<void, Error> setLocale(const std::string &locale);
    const Component::Package::FilterOptions &getFilterOptions();
    tl::expected<EntityDataResponse, Error> getAll();
    tl::expected<std::set<std::string>, Error> getInstalledPackages();
    tl::expected<std::set<std::string>, Error> getStatus();
    tl::expected<std::string, Error> getCurrentEdition();
    tl::expected<std::string, Error> getComponentDescription(const std::string &path);
    tl::expected<std::string, Error> getCategoryDescription(const std::string &name);
    tl::expected<TransactionResolutionResponse, Error> checkApply(std::unordered_set<std::string> install,
                                                                  const std::unordered_set<std::string> &remove);
    tl::expected<void, Error> apply(std::unordered_set<std::string> install,
                                    const std::unordered_set<std::string> &remove,
                                    const std::unordered_set<std::string> &exclude,
                                    const Callback &callback = nullptr);
    tl::expected<void, Error> updateSources(const Callback &callback = nullptr);
    tl::expected<std::chrono::system_clock::time_point, Error> lastUpdateOfSources();
    tl::expected<std::chrono::system_clock::time_point, Error> lastDistUpgrade();

public:
    DBusManager(const DBusManager &) = delete;
    DBusManager &operator=(const DBusManager &) = delete;
    DBusManager(DBusManager &&) = delete;
    DBusManager &operator=(DBusManager &&) = delete;
    DBusManager();
    ~DBusManager();

private:
    class Private;
    std::unique_ptr<Private> p;
};

struct DBusManager::Error
{
    enum Type
    {
        InvalidInterface = 401,
        Reply = 402,
        InvalidSignature = 403,
        Server = 404,
    } type;
    std::string service;
    std::string interface;
    std::string object;
    std::string method;
    std::optional<int> code;
    std::unordered_map<std::string, std::string> data;
    std::string text() const;
};

} // namespace alt

#endif // DBUS_MANAGER_H
