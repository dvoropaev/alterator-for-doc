#ifndef COMPONENTOBJECT_H
#define COMPONENTOBJECT_H

#include "model/objects/object.h"
#include "model/objects/package.h"

#include <set>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>

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
    Component();
    Component(const toml::table &data, const QSet<QString> &notInstalledPackages);
    Component(const Component &component);
    Component(Component &&) = default;
    Component &operator=(const Component &object) = default;
    Component &operator=(Component &&object) = default;
    ~Component() override;

public:
    std::set<std::shared_ptr<Package>> packages{};
    std::set<QString> tags{};

    ComponentState state{};

    bool hasRelationToEdition = false;

public:
    void setState(ComponentState state);
};
} // namespace alt

Q_DECLARE_METATYPE(alt::Component *);

#endif // COMPONENTOBJECT_H
