#ifndef AOB_ALTERATOR_OBJECT_BUILDER_INTERFACE_H
#define AOB_ALTERATOR_OBJECT_BUILDER_INTERFACE_H

#include <memory>
#include <set>
#include <QString>

#include "../objects/category.h"
#include "../objects/localappobject.h"
#include "../objects/object.h"

namespace ao_builder
{
class AOBuilderInterface
{
public:
    virtual ~AOBuilderInterface() = default;

public:
    virtual std::vector<std::unique_ptr<LocalAppObject>> buildLocalApps() = 0;
    virtual std::vector<std::unique_ptr<Category>> buildCategories() = 0;
    virtual std::vector<std::unique_ptr<Object>> buildObjects(std::set<QString>) = 0;
    virtual std::unique_ptr<Object> buildObject(const QString &info) = 0;
};
} // namespace ao_builder

#endif // AOB_ALTERATOR_OBJECT_BUILDER_INTERFACE_H
