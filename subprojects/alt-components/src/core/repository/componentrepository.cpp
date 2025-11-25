#include "componentrepository.h"

#include "dbus/dbusmanager.h"
#include "entity/component.h"
#include <functional>

namespace alt
{
class ComponentRepository::Private
{
public:
    static Error map(const DBusManager::Error &error);

public:
    std::map<std::string, Component> components{};
    std::map<std::string, Category> categories{};
    std::shared_ptr<ILogger> logger;
    std::shared_ptr<DBusManager> source;
    std::shared_ptr<PackageRepository> packages;
};

template<typename T, typename... Args>
T parse(const std::shared_ptr<ILogger> &logger, const std::string &info, const Args &...args)
{
    const auto parse_result = toml::parse(info);
    if (parse_result.failed())
    {
        if (logger)
        {
            logger->write(ILogger::Level::Warning,
                          "Failed to parse component:" + std::string(parse_result.error().description()));
        }
        return {};
    }
    return T(parse_result.table(), args...);
}

ComponentRepository::ComponentRepository(const std::shared_ptr<DBusManager> &source,
                                         const std::shared_ptr<PackageRepository> &packages)
    : p(std::make_unique<Private>())
{
    p->source = source;
    p->packages = packages;
}

ComponentRepository::~ComponentRepository() = default;

void ComponentRepository::setLogger(const std::shared_ptr<ILogger> &logger)
{
    p->logger = logger;
}

template<>
std::optional<std::reference_wrapper<Component>> ComponentRepository::get<Component>(const std::string &name)
{
    auto &components = p->components;
    auto result = components.find(name);
    if (result == components.end())
    {
        return std::nullopt;
    }

    return std::make_optional(std::ref(result->second));
}

template<>
std::optional<std::reference_wrapper<Category>> ComponentRepository::get<Category>(const std::string &name)
{
    auto &categories = p->categories;
    auto result = categories.find(name);
    if (result == categories.end())
    {
        return std::nullopt;
    }

    return std::make_optional(std::ref(result->second));
}

template<>
std::map<std::string, Component> &ComponentRepository::getAll()
{
    return p->components;
}

template<>
std::map<std::string, Category> &ComponentRepository::getAll()
{
    return p->categories;
}

tl::expected<std::string, ComponentRepository::Error> ComponentRepository::getDescription(const Component &component)
{
    return p->source->getComponentDescription(component.dbusPath).map_error(static_cast<decltype(&Private::map)>(p->map));
}

void ComponentRepository::update()
{
    p->components.clear();
    p->categories.clear();
    const auto data = p->source->getAll();
    const auto notInstalledPackagesResult = p->source->getStatus();
    if (!data)
    {
        return;
    }
    if (!notInstalledPackagesResult)
    {
        return;
    }

    const auto &notInstalledPackages = notInstalledPackagesResult.value();
    const auto filterOptions = p->source->getFilterOptions();
    for (const auto &entry : data->components)
    {
        auto component = parse<Component>(p->logger, entry);
        // NOTE(sheriffkorov): if object is valid
        if (!component.type.empty())
        {
            bool installed = true;
            for (auto it = component.packages.begin(); it != component.packages.end();)
            {
                auto pkg = *it;
                if (!pkg.matchFilters(filterOptions))
                {
                    it = component.packages.erase(it);
                    continue;
                }

                if (pkg.kernelModule)
                {
                    pkg.name += "-" + p->source->getFilterOptions().kflavour;
                }
                pkg.installed = !notInstalledPackages.contains(pkg.name);
                installed &= pkg.installed;
                ++it;
            }
            if (component.packages.empty())
            {
                installed = false;
            }
            component.setState(installed ? ComponentState::installed : ComponentState::not_installed);
            p->components.emplace(component.name, std::move(component));
        }
    }

    for (const auto &entry : data->categories)
    {
        auto category = parse<Category>(p->logger, entry);
        // NOTE(sheriffkorov): if object is valid
        if (!category.type.empty())
        {
            p->categories.emplace(category.name, std::move(category));
        }
    }
}

ComponentRepository::Error ComponentRepository::Private::map(const DBusManager::Error &error)
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
