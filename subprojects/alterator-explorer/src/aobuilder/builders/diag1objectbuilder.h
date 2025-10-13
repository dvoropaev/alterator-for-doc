#ifndef DIAG1OBJECTBUILDER_H
#define DIAG1OBJECTBUILDER_H

#include "baseobjectbuilder.h"
namespace ao_builder
{
class Diag1ObjectBuilder : public BaseObjectBuilder
{
public:
    Diag1ObjectBuilder() = default;
    ~Diag1ObjectBuilder() = default;

    std::unique_ptr<Object> buildObject(ObjectParserInterface *parser) override;

    Object::Type type() override;
};
} // namespace ao_builder

#endif // DIAG1OBJECTBUILDER_H
