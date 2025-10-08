#ifndef TAG_H
#define TAG_H

#include "model/objects/object.h"

#include <QString>

namespace alt
{
class Tag : public Object
{
public:
    Tag() = default;
    Tag(const toml::table &data, QString name);
    Tag(const Tag &) = default;
    Tag(Tag &&) = default;
    Tag &operator=(const Tag &object) = default;
    Tag &operator=(Tag &&object) = default;
};
} // namespace alt

Q_DECLARE_METATYPE(alt::Tag *);

#endif // TAG_H
