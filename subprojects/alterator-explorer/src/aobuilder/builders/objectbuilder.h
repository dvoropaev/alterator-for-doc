#ifndef AOB_OBJECT_BUILDER_H
#define AOB_OBJECT_BUILDER_H

#include "baseobjectbuilder.h"

namespace ao_builder
{
class ObjectBuilder : public BaseObjectBuilder
{
public:
    ObjectBuilder() = default;
    ~ObjectBuilder() override = default;

    std::unique_ptr<Object> buildObject(ObjectParserInterface *parser) override;

    Object::Type type() override;
};

} // namespace ao_builder

#endif // AOB_OBJECT_BUILDER_H
