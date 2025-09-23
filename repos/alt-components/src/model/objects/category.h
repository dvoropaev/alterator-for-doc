#ifndef COMPONENT_CATEGORY_H
#define COMPONENT_CATEGORY_H

#include "model/objects/object.h"

#include <QObject>

namespace alt
{
class Category : public Object
{
public:
    Category() = default;
    explicit Category(const toml::table &data);
    Category(const Category &) = default;
    Category(Category &&) = default;
    Category &operator=(const Category &object) = default;
    Category &operator=(Category &&object) = default;

    ~Category() override = default;
};
} // namespace alt

Q_DECLARE_METATYPE(alt::Category *);

#endif // COMPONENT_CATEGORY_H
