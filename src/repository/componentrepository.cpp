#include "componentrepository.h"

#include "dbus/dbusproxy.h"
#include "repository/packagerepository.h"
#include <functional>

namespace alt
{
template<typename T, typename... Args>
T parse(const QString &info, const Args &...args)
{
    const auto parse_result = toml::parse(info.toStdString());
    if (parse_result.failed())
    {
        qWarning() << "Failed to parse component:" << parse_result.error().description();
        return {};
    }
    return T(parse_result.table(), args...);
}

ComponentRepository::ComponentRepository() = default;

ComponentRepository &ComponentRepository::instance()
{
    static ComponentRepository instance{};
    return instance;
}

template<>
std::optional<std::reference_wrapper<Component>> ComponentRepository::get<Component>(const QString &name)
{
    auto &components = instance().components;
    auto result = components.find(name);
    if (result == components.end())
    {
        return std::nullopt;
    }

    return std::make_optional(std::ref(result->second));
}

template<>
std::optional<std::reference_wrapper<Category>> ComponentRepository::get<Category>(const QString &name)
{
    auto &categories = instance().categories;
    auto result = categories.find(name);
    if (result == categories.end())
    {
        return std::nullopt;
    }

    return std::make_optional(std::ref(result->second));
}

template<>
std::map<QString, Component> &ComponentRepository::getAll()
{
    return instance().components;
}

template<>
std::map<QString, Category> &ComponentRepository::getAll()
{
    return instance().categories;
}

void ComponentRepository::update()
{
    instance().components = pullComponents();
    instance().categories = pullCategories();
}

std::map<QString, Component> ComponentRepository::pullComponents()
{
    const auto componentsInfo = DBusProxy::get().getBatchComponentsInfo();
    if (!componentsInfo.has_value())
    {
        return {};
    }

    const auto notInstalledPackages = DBusProxy::get().getBatchComponentsStatus();
    Q_ASSERT(notInstalledPackages.has_value() && "Batch status failed");

    std::map<QString, Component> components;
    for (const auto &info : componentsInfo.value())
    {
        auto component = parse<Component>(info, notInstalledPackages.value());
        // NOTE(sheriffkorov): if object is valid
        if (!component.type.isEmpty())
        {
            for (auto pkg : component.packages)
            {
                PackageRepository::add(pkg);
            }
            components[component.name] = std::move(component);
        }
    }

    return components;
}

std::map<QString, Category> ComponentRepository::pullCategories()
{
    const auto categoriesInfo = DBusProxy::get().getBatchCategoriesInfo();
    if (!categoriesInfo.has_value())
    {
        return {};
    }

    std::map<QString, Category> categories;
    for (const auto &info : categoriesInfo.value())
    {
        auto category = parse<Category>(info);
        // NOTE(sheriffkorov): if object is valid
        if (!category.type.isEmpty())
        {
            categories[category.name] = std::move(category);
        }
    }

    return categories;
}

} // namespace alt
