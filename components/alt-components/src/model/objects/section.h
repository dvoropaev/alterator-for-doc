#ifndef SECTION_H
#define SECTION_H

#include "model/objects/object.h"

#include <QObject>
#include <QSet>

namespace alt
{
class Section : public Object
{
public:
    Section() = default;
    Section(const toml::table &data, QString name);
    Section(const toml::table &data, QString name, unsigned sort_weight);
    Section(const Section &) = default;
    Section(Section &&) = default;
    Section &operator=(const Section &object) = default;
    Section &operator=(Section &&object) = default;

    ~Section() override = default;

public:
    QSet<QString> components;
    unsigned sort_weight = 0;

private:
    static unsigned weight_count;
};
} // namespace alt

Q_DECLARE_METATYPE(alt::Section *);

#endif // SECTION_H
