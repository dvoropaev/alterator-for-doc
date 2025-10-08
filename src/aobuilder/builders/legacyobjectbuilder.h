#ifndef AOB_LEGACY_OBJECT_BUILDER_H
#define AOB_LEGACY_OBJECT_BUILDER_H

#include "baseobjectbuilder.h"

namespace ao_builder
{
class LegacyObjectBuilder : public BaseObjectBuilder
{
public:
    LegacyObjectBuilder(Object::Type t);
    ~LegacyObjectBuilder() override = default;

    std::unique_ptr<Object> buildObject(ObjectParserInterface *parser) override;
    Object::Type type() override;

private:
    Object::Type m_type;
};

} // namespace ao_builder
#endif // AOB_LEGACY_OBJECT_BUILDER_H
