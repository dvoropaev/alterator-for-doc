#ifndef AOB_LOCAL_APPLICATION_OBJECT_BUILDER_H
#define AOB_LOCAL_APPLICATION_OBJECT_BUILDER_H

#include "baseobjectbuilder.h"

namespace ao_builder
{
class LocalApplicationObjectBuilder : public BaseObjectBuilder
{
public:
    LocalApplicationObjectBuilder() = default;
    ~LocalApplicationObjectBuilder() override = default;

    std::unique_ptr<Object> buildObject(ObjectParserInterface *parser) override;

    Object::Type type() override;
};

} // namespace ao_builder

#endif // AOB_LOCAL_APPLICATION_OBJECT_BUILDER_H
