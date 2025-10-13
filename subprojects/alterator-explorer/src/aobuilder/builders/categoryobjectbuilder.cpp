#include "categoryobjectbuilder.h"
#include "constants.h"

#include <objects/category.h>

namespace ao_builder
{
std::unique_ptr<Object> CategoryObjectBuilder::buildObject(ObjectParserInterface *parser)
{
    auto result = std::make_unique<Category>();

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

    if (!buildFieldWithLocale(parser,
                              KeyProvider::tomlKeys.key<COMMENT>(),
                              result.get()->m_comment,
                              result.get()->m_commentLocaleStorage))
    {
        return nullptr;
    }

    QString icon = parser->getValue(KeyProvider::tomlKeys.key<ICON>());
    if (icon.isEmpty())
    {
        return nullptr;
    }
    result->m_icon = icon;

    return result;
}

Object::Type CategoryObjectBuilder::type()
{
    return Object::CATEGORY;
}
} // namespace ao_builder
