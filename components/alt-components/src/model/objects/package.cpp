#include "package.h"

#include "constants.h"
#include "dbus/dbusproxy.h"
#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/range/algorithm/find.hpp>

namespace alt
{
Package::Package(const toml::table &data, std::string name, bool installed)
    : name(std::move(name))
    , installed(installed)
{
    this->meta = data[PACKAGE_META_KEY].value_or<bool>(false);
    this->kernelModule = data[PACKAGE_KERNEL_MODULE_KEY].value_or<bool>(false);
    this->imageIgnore = data[PACKAGE_IMAGE_IGNORE_KEY].value_or<bool>(false);

    this->language = data[PACKAGE_LANGUAGE_KEY].value<std::string>();
    if (const toml::array *desktops_list = data[PACKAGE_DESKTOP_KEY].as_array())
    {
        desktops_list->for_each([this](const toml::value<std::string> &el) {
            if (el.is_value())
            {
                this->desktops.push_back(*el);
            }
        });
    }

    if (const toml::array *pkg_arch = data[PACKAGE_ARCH_KEY].as_array())
    {
        pkg_arch->for_each([this](const toml::value<std::string> &el) {
            if (el.is_value())
            {
                this->arch.push_back(*el);
            }
        });
    }

    if (const toml::array *pkg_exclude_arch = data[PACKAGE_EXCLUDE_ARCH_KEY].as_array())
    {
        pkg_exclude_arch->for_each([this](const toml::value<std::string> &el) {
            if (el.is_value())
            {
                this->excludeArch.push_back(*el);
            }
        });
    }
}

QString Package::getPackageName() const
{
    QString resultingPackageName = QString::fromStdString(this->name);

    const QString kernelName = DBusProxy::get().getFilterOptions().kflavour;
    if (this->kernelModule)
    {
        resultingPackageName += "-" + kernelName;
    }

    return resultingPackageName;
}

bool Package::matchFilters(const FilterOptions &options) const
{
    return matchArch(options.arch) && matchDesktopEnvironment(options.desktops) && matchLanguage(options.language);
}

bool Package::matchArch(const QString &current_arch) const
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

bool Package::matchDesktopEnvironment(const QSet<QString> &current_desktops) const
{
    if (this->desktops.empty())
    {
        return true;
    }

    return boost::algorithm::any_of(this->desktops, [&current_desktops](const std::string &desktop) {
        return current_desktops.contains(QString::fromStdString(desktop));
    });
}

bool Package::matchLanguage(const QString &current_language) const
{
    return !this->language.has_value() || this->language.value() == current_language;
}
} // namespace alt
