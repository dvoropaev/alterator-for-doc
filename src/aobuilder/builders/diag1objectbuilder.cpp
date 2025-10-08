#include "diag1objectbuilder.h"
#include "constants.h"

#include <objects/diag1object.h>

namespace ao_builder
{
std::unique_ptr<ao_builder::Object> ao_builder::Diag1ObjectBuilder::buildObject(ObjectParserInterface *parser)
{
    auto result = std::make_unique<Diag1Object>();

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
                              result->m_comment,
                              result->m_commentLocaleStorage))
    {
        return nullptr;
    }

    QString categoryId = parser->getValue(key::alterator::CATEGORY);
    result->m_categoryId = categoryId;

    QString icon = parser->getValue(KeyProvider::tomlKeys.key<ICON>());
    if (icon.isEmpty())
    {
        return nullptr;
    }
    result->m_icon = icon;

    return result;
}

Object::Type Diag1ObjectBuilder::type()
{
    return Object::DIAG1;
}
} // namespace ao_builder
