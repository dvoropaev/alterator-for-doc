#include "category.h"
#include "object.h"

namespace alt
{
Category::Category(const toml::table &data)
    : Object(data)
{}
} // namespace alt
