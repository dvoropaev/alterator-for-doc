#ifndef EDITION_H
#define EDITION_H

#include "model/objects/object.h"
#include "model/objects/section.h"
#include "model/objects/tag.h"

#include <QString>
#include <QStringList>

namespace alt
{
class Edition : public Object
{
public:
    Edition() = default;
    explicit Edition(const toml::table &data);
    Edition(const Edition &) = default;
    Edition(Edition &&) = default;
    Edition &operator=(const Edition &object) = default;
    Edition &operator=(Edition &&object) = default;

public:
    QString license;
    QStringList arches;
    QString desktopEnvironment;
    QString defaultKflavour;
    QString defaultLanguage;
    QList<Section> sections;
    QList<Tag> tags;
};
} // namespace alt

Q_DECLARE_METATYPE(alt::Edition *);

#endif // EDITION_H
