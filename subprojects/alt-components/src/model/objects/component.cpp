#include "component.h"
#include "constants.h"
#include "dbus/dbusproxy.h"
#include "model/objects/object.h"

#include <algorithm>
#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/range/algorithm/find.hpp>
#include <iterator>
#include <sys/utsname.h>
#include <QRegularExpression>

namespace alt
{
Component::Component() {};

Component::Component(const toml::table &data, const QSet<QString> &notInstalledPackages)
    : Object(data)
{
    bool isComponentInstalled = true;
    if (const toml::table *packages = data[COMPONENT_PACKAGES_KEY_NAME].as_table())
    {
        if (packages->size() == 0)
        {
            isComponentInstalled = false;
        }

        for (const auto &[pkg_name, pkg_data] : *packages)
        {
            const auto installed = !notInstalledPackages.contains(QString(pkg_name.data()));
            const auto package = std::make_shared<Package>(*pkg_data.as_table(), std::string(pkg_name.str()), installed);
            if (package->matchFilters(DBusProxy::get().getFilterOptions()))
            {
                this->packages.insert(package);
                QString pkgName = package->getPackageName();
                package->installed = !notInstalledPackages.contains(pkgName);
                isComponentInstalled &= package->installed;
            }
        };
    }

    if (const toml::array *tags = data[COMPONENT_TAGS_KEY_NAME].as_array())
    {
        tags->for_each([this](const toml::value<std::string> &el) {
            if (el.is_value())
            {
                this->tags.insert(QString::fromStdString(*el));
            }
        });
    }

    this->state = isComponentInstalled ? ComponentState::installed : ComponentState::not_installed;

    auto object_name = QString("component_") + this->name;
    object_name.replace('-', '_');
    this->dbusPath = QString(ALTERATOR_MANAGER_PATH) + "/" + object_name;
}

Component::Component(const Component &component)
    : Object(component)
{
    this->name = component.name;
    this->type = component.type;
    this->displayNameStorage = component.displayNameStorage;
    this->commentStorage = component.commentStorage;
    this->category = component.category;
    this->icon = component.icon;
    this->dbusPath = component.dbusPath;

    std::copy(component.packages.begin(), component.packages.end(), std::inserter(this->packages, packages.begin()));
}

Component::~Component() = default;

void Component::setState(ComponentState state)
{
    this->state = state;
}
} // namespace alt
