#ifndef COMPONENTOBJECT_H
#define COMPONENTOBJECT_H

#include "object.h"
#include <toml++/toml.hpp>

#include <set>

namespace alt
{
enum ComponentState : int
{
    not_installed = 0,
    partially_installed = 1,
    installed = 2
};

class Component : public Object
{
public:
    class Package;

public:
    Component();
    Component(const toml::table &data);
    Component(const Component &component);
    Component(Component &&) = default;
    Component &operator=(const Component &object) = default;
    Component &operator=(Component &&object) = default;
    ~Component() override;

public:
    std::set<Package> packages{};
    std::set<std::string> tags{};

    ComponentState state{};

public:
    void setState(ComponentState state);
};

class Component::Package
{
public:
    struct FilterOptions
    {
        std::string kflavour;
        std::string arch;
        std::set<std::string> desktops;
        std::string language;
    };

public:
    Package(const toml::table &data, std::string name);
    Package(const Package &) = default;
    Package(Package &&) = default;
    Package &operator=(const Package &object) = default;
    Package &operator=(Package &&object) = default;

    bool operator<(const Package &rhs) const { return name < rhs.name; }
    [[nodiscard]] std::string getPackageName() const;
    [[nodiscard]] bool matchFilters(const FilterOptions &options) const;

    std::string name;
    bool meta;
    bool kernelModule;
    bool imageIgnore;
    std::optional<std::string> language{};
    std::vector<std::string> desktops{};
    std::vector<std::string> arch{};
    std::vector<std::string> excludeArch{};

    bool installed{};

private:
    [[nodiscard]] bool matchArch(const std::string &current_arch) const;
    [[nodiscard]] bool matchDesktopEnvironment(const std::set<std::string> &current_desktops) const;
    [[nodiscard]] bool matchLanguage(const std::string &current_language) const;
};
} // namespace alt

#endif // COMPONENTOBJECT_H
