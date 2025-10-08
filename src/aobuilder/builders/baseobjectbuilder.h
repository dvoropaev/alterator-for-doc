#ifndef AOB_BASE_OBJECT_BUILDER_H
#define AOB_BASE_OBJECT_BUILDER_H

#include "builders/keyprovider.h"
#include "objectbulderinterface.h"

namespace ao_builder
{
class BaseObjectBuilder : public ObjectBuilderInterface
{
public:
    BaseObjectBuilder() = default;
    ~BaseObjectBuilder() override = default;

public:
    bool buildFieldWithLocale(ObjectParserInterface *parser,
                              QString entryName,
                              QString &field,
                              QMap<QString, QString> &localeStorage,
                              QString sectionName = {});

    bool buildBase(ObjectParserInterface *parser,
                   Object *localAppObject,
                   const KeyProvider &provider,
                   const QString &sectionName = {});

    std::vector<QString> parseValuesFromKey(ObjectParserInterface *parser,
                                            QString key,
                                            QString delimiter,
                                            QString section = {});

    virtual Object::Type type() = 0;
};
} // namespace ao_builder

#endif // AOB_BASE_OBJECT_BUILDER_H
