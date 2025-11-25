#ifndef COMPONENT_REPOSITORY_H
#define COMPONENT_REPOSITORY_H

#include "entity/category.h"
#include "entity/component.h"
#include "interface/ilogger.h"

#include <functional>
#include <map>
#include <optional>
#include <tl/expected.hpp>

namespace alt
{
class DBusManager;
class PackageRepository;
class ComponentRepository
{
public:
    struct Error;

public:
    template<typename T>
    std::optional<std::reference_wrapper<T>> get(const std::string &name);
    template<typename T>
    std::map<std::string, T> &getAll();
    void update();
    void setLogger(const std::shared_ptr<ILogger> &logger);
    tl::expected<std::string, Error> getDescription(const Component &component);

public:
    ComponentRepository(const ComponentRepository &) = delete;
    ComponentRepository(ComponentRepository &&) = delete;
    ComponentRepository &operator=(const ComponentRepository &) = delete;
    ComponentRepository &operator=(ComponentRepository &&) = delete;
    ComponentRepository(const std::shared_ptr<DBusManager> &source, const std::shared_ptr<PackageRepository> &packages);
    ~ComponentRepository();

private:
    class Private;
    std::unique_ptr<Private> p;
};

struct ComponentRepository::Error
{
    enum Type
    {
        SourceUnavailable = 401,
        OperationFailed = 402,
    } type;
    std::string details;
};
} // namespace alt

#endif // COMPONENT_REPOSITORY_H
