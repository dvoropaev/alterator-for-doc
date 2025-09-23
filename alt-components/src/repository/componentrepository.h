#ifndef COMPONENT_REPOSITORY_H
#define COMPONENT_REPOSITORY_H

#include "model/objects/category.h"
#include "model/objects/component.h"

#include <functional>
#include <map>
#include <optional>
#include <QString>

namespace alt
{
class ComponentRepository
{
public:
    static ComponentRepository &instance();
    template<typename T>
    static std::optional<std::reference_wrapper<T>> get(const QString &name);
    template<typename T>
    static std::map<QString, T> &getAll();
    static void update();

public:
    ComponentRepository(const ComponentRepository &) = delete;
    ComponentRepository(ComponentRepository &&) = delete;
    ComponentRepository &operator=(const ComponentRepository &) = delete;
    ComponentRepository &operator=(ComponentRepository &&) = delete;

private:
    static std::map<QString, Component> pullComponents();
    static std::map<QString, Category> pullCategories();

private:
    ComponentRepository();

private:
    std::map<QString, Component> components{};
    std::map<QString, Category> categories{};
};
} // namespace alt

#endif // COMPONENT_REPOSITORY_H
