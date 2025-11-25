#ifndef EDITION_H
#define EDITION_H

#include "object.h"
#include "section.h"
#include "tag.h"

namespace alt
{
class Edition : public Object
{
public:
    Edition() = default;
    explicit Edition(const toml::table &data, bool installed = false);
    Edition(const Edition &) = default;
    Edition(Edition &&) = default;
    Edition &operator=(const Edition &object) = default;
    Edition &operator=(Edition &&object) = default;

public:
    std::string license;
    std::vector<std::string> arches;
    std::string desktopEnvironment;
    std::string defaultKflavour;
    std::string defaultLanguage;
    std::vector<Section> sections;
    std::vector<Tag> tags;
    bool installed;
};
} // namespace alt

#endif // EDITION_H
