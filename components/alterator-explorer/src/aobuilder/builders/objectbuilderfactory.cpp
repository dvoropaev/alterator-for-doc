
#include "objectbuilderfactory.h"
#include "categoryobjectbuilder.h"
#include "constants.h"
#include "diag1objectbuilder.h"
#include "legacyobjectbuilder.h"
#include "localapplicationobjectbuilder.h"
#include "objectbuilder.h"

#include <memory>

namespace ao_builder
{
using BuilderConstructor = std::function<std::unique_ptr<ObjectBuilderInterface>()>;
#define BUILDER(T) static_cast<BuilderConstructor>(&std::make_unique<T>)

static const std::map<QString, std::pair<Object::Type, BuilderConstructor>>
    typemap{{type::OBJECT, {Object::OBJECT, BUILDER(ObjectBuilder)}},
            {type::APPLICATION, {Object::APPLICATION, BUILDER(LocalApplicationObjectBuilder)}},
            {type::CATEGORY, {Object::CATEGORY, BUILDER(CategoryObjectBuilder)}},
            {type::DIAG1, {Object::DIAG1, BUILDER(Diag1ObjectBuilder)}}};

#undef BUILDER

std::unique_ptr<ObjectBuilderInterface> ObjectBuilderFactory::getBuilder(ObjectParserInterface *parser)
{
    QString type_name = parser->getValue(KeyProvider::tomlKeys.key<TYPE>());
    bool legacy = type_name.isEmpty();
    if (legacy)
        type_name = parser->getValue(KeyProvider::iniKeys.key<TYPE>(), key::desktop::SECTION_NAME);

    try
    {
        auto [type, makeBuilder] = typemap.at(type_name);
        return legacy ? std::make_unique<LegacyObjectBuilder>(type) : makeBuilder();
    }
    catch (std::out_of_range &e)
    {
        qWarning() << "unknown type, falling back to object";
        return std::make_unique<ObjectBuilder>();
    };

    return {};
}

} // namespace ao_builder
