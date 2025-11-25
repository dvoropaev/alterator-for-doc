#include "component.h"
#include "builder.h"
#include "constants.h"
#include "dbus/constants.h"
#include "object.h"

#include <algorithm>
#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/range/algorithm/find.hpp>

namespace alt
{
Component::Component() = default;

Component::Component(const toml::table &data)
    : Object(data)
{
    if (const toml::table *packages = data[entity::COMPONENT_PACKAGES_KEY_NAME].as_table())
    {
        for (const auto &[pkg_name, pkg_data] : *packages)
        {
            this->packages.emplace(*pkg_data.as_table(), std::string(pkg_name.str()));
        };
    }

    if (const toml::array *tags = data[entity::COMPONENT_TAGS_KEY_NAME].as_array())
    {
        tags->for_each([this](const toml::value<std::string> &el) {
            if (el.is_value())
            {
                this->tags.insert(*el);
            }
        });
    }

    auto object_name = this->name;
    std::replace(object_name.begin(), object_name.end(), '-', '_');
    this->dbusPath = std::string(dbus::ALTERATOR_MANAGER_PATH) + "/component_" + object_name;
}

Component::Component(const Component &component)
    : Object(component)
    , packages(component.packages)
{}

Component::~Component() = default;

void Component::setState(ComponentState state)
{
    this->state = state;
}

Component::Package::Package(const toml::table &data, std::string name)
    : name(std::move(name))
    , meta(buildField<bool>(data, entity::PACKAGE_META_KEY).value_or(false))
    , kernelModule(buildField<bool>(data, entity::PACKAGE_KERNEL_MODULE_KEY).value_or(false))
    , imageIgnore(buildField<bool>(data, entity::PACKAGE_IMAGE_IGNORE_KEY).value_or(false))
    , language(buildField<std::string>(data, entity::PACKAGE_LANGUAGE_KEY))
{
    if (const toml::array *desktops_list = data[entity::PACKAGE_DESKTOP_KEY].as_array())
    {
        desktops_list->for_each([this](const toml::value<std::string> &el) {
            if (el.is_value())
            {
                this->desktops.push_back(*el);
            }
        });
    }

    if (const toml::array *pkg_arch = data[entity::PACKAGE_ARCH_KEY].as_array())
    {
        pkg_arch->for_each([this](const toml::value<std::string> &el) {
            if (el.is_value())
            {
                this->arch.push_back(*el);
            }
        });
    }

    if (const toml::array *pkg_exclude_arch = data[entity::PACKAGE_EXCLUDE_ARCH_KEY].as_array())
    {
        pkg_exclude_arch->for_each([this](const toml::value<std::string> &el) {
            if (el.is_value())
            {
                this->excludeArch.push_back(*el);
            }
        });
    }
}

std::string Component::Package::getPackageName() const
{
    return name;
}

bool Component::Package::matchFilters(const FilterOptions &options) const
{
    return matchArch(options.arch) && matchDesktopEnvironment(options.desktops) && matchLanguage(options.language);
}

bool Component::Package::matchArch(const std::string &current_arch) const
{
    if (arch.empty() && excludeArch.empty())
    {
        return true;
    }

    if (!arch.empty() && boost::range::find(arch, current_arch) != arch.end())
    {
        return true;
    }

    if (!excludeArch.empty() && boost::range::find(excludeArch, current_arch) != excludeArch.end())
    {
        return false;
    }

    return false;
}

bool Component::Package::matchDesktopEnvironment(const std::set<std::string> &current_desktops) const
{
    if (this->desktops.empty())
    {
        return true;
    }

    return boost::algorithm::any_of(this->desktops, [&current_desktops](const std::string &desktop) {
        return current_desktops.contains(desktop);
    });
}

bool Component::Package::matchLanguage(const std::string &current_language) const
{
    return !this->language.has_value() || this->language.value() == current_language;
}

} // namespace alt
