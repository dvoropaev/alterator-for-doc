#ifndef TAG_H
#define TAG_H

#include "object.h"

namespace alt
{
class Tag : public Object
{
public:
    Tag() = default;
    Tag(const toml::table &data);
    Tag(const Tag &) = default;
    Tag(Tag &&) = default;
    Tag &operator=(const Tag &object) = default;
    Tag &operator=(Tag &&object) = default;
};
} // namespace alt

#endif // TAG_H
