#ifndef SECTION_H
#define SECTION_H

#include "object.h"
#include <unordered_set>

namespace alt
{
class Section : public Object
{
public:
    Section() = default;
    Section(const toml::table &data);
    Section(const toml::table &data, unsigned sort_weight);
    Section(const Section &) = default;
    Section(Section &&) = default;
    Section &operator=(const Section &object) = default;
    Section &operator=(Section &&object) = default;

    ~Section() override = default;

public:
    std::unordered_set<std::string> components;
    unsigned sort_weight = 0;

private:
    static unsigned weight_count;
};
} // namespace alt

#endif // SECTION_H
