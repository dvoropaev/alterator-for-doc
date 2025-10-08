#ifndef PACKAGE_OBJECT_H
#define PACKAGE_OBJECT_H

#define TOML_EXCEPTIONS 0
#include <toml++/toml.hpp>
#include <QSet>
#include <QString>

namespace alt
{
struct FilterOptions
{
    QString kflavour;
    QString arch;
    QSet<QString> desktops;
    QString language;
};

class Package
{
public:
    Package(const toml::table &data, std::string name, bool installed);
    Package(const Package &) = default;
    Package(Package &&) = default;
    Package &operator=(const Package &object) = default;
    Package &operator=(Package &&object) = default;

    bool operator<(const Package &rhs) const { return name < rhs.name; }
    [[nodiscard]] QString getPackageName() const;
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
    [[nodiscard]] bool matchArch(const QString &current_arch) const;
    [[nodiscard]] bool matchDesktopEnvironment(const QSet<QString> &current_desktops) const;
    [[nodiscard]] bool matchLanguage(const QString &current_language) const;
};
} // namespace alt

#endif // COMPONENT_OBJECT_H
