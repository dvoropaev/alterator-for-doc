#include "tag.h"
#include "application.h"

#include <QString>

namespace alt
{
Tag::Tag(const toml::table &data, QString name)
    : Object(data)
{
    this->name = std::move(name);
}
} // namespace alt
