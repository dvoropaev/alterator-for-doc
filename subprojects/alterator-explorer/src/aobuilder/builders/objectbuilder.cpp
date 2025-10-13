#include "objectbuilder.h"
#include "constants.h"

#include <memory>

namespace ao_builder
{
std::unique_ptr<Object> ObjectBuilder::buildObject(ObjectParserInterface *parser)
{
    auto result = std::make_unique<Object>();

    if (!buildBase(parser, result.get(), KeyProvider::tomlKeys))
    {
        return nullptr;
    }

    if (!buildFieldWithLocale(parser,
                              key::alterator::DISPLAY_NAME,
                              result->m_displayName,
                              result->m_displayNameLocaleStorage))
    {
        return nullptr;
    }

    QString categoryId = parser->getValue(key::alterator::CATEGORY);
    result->m_categoryId = categoryId;

    QString icon = parser->getValue(KeyProvider::tomlKeys.key<ICON>());
    result->m_icon = icon;

    result->m_isLegacy = false;

    result->m_override = parser->getValue(key::alterator::OVERRIDE);

    return result;
}

Object::Type ObjectBuilder::type()
{
    return Object::OBJECT;
}
} // namespace ao_builder
