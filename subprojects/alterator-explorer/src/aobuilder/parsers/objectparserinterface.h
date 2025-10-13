#ifndef AOB_OBJECT_PARSER_INTERFACE_H
#define AOB_OBJECT_PARSER_INTERFACE_H

#include <QMap>
#include <QString>
#include <QVariant>

namespace ao_builder
{
class ObjectParserInterface
{
public:
    using Nodes = QVariantMap;

public:
    virtual ~ObjectParserInterface() = default;

    virtual bool parse(const QString &data) = 0;

    virtual const Nodes &getSections() = 0;
    virtual QString getValue(const QString &key, const QString &section = {}) = 0;
};

} // namespace ao_builder

#endif
