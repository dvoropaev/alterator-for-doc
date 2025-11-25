#include "tag.h"

namespace alt
{
Tag::Tag(const toml::table &data)
    : Object(data)
{}
} // namespace alt
