#ifndef AOB_OBJECT_BUILDER_INTERFACE_H
#define AOB_OBJECT_BUILDER_INTERFACE_H

#include <memory>

#include "objects/object.h"
#include "parsers/objectparserinterface.h"

namespace ao_builder
{
class ObjectBuilderInterface
{
public:
    virtual ~ObjectBuilderInterface() = default;

    virtual std::unique_ptr<Object> buildObject(ObjectParserInterface *parser) = 0;
};
} // namespace ao_builder

#endif // AOB_OBJECT_BUILDER_INTERFACE_H
