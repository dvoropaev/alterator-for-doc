#include "category.h"
#include "model/objects/object.h"

namespace alt
{
Category::Category(const toml::table &data)
    : Object(data)
{}
} // namespace alt
