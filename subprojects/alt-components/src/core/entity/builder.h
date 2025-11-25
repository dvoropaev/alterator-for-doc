#ifndef BUILDER_H
#define BUILDER_H

#define TOML_EXCEPTIONS 0
#include "constants.h"
#include <map>
#include <string>
#include <toml++/toml.h>

namespace alt
{
template<typename T>
std::optional<T> buildField(const toml::table &data, const std::string &name)
{
    if (const auto *node = data.get(name))
    {
        return node->value<T>();
    }
    return std::nullopt;
}

template<typename T>
std::map<std::string, T> buildMap(const toml::table &data, const std::string &name)
{
    std::map<std::string, T> map;
    if (const toml::table *table = data[name].as_table())
    {
        for (const auto &[key, value] : *table)
        {
            map[std::string(key.str())] = *value.value<T>();
        }
    }
    return map;
}
} // namespace alt

#endif // BUILDER_H
