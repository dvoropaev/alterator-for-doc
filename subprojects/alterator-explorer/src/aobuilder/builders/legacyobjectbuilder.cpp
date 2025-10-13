#include "legacyobjectbuilder.h"
#include "../objects/legacyobject.h"
#include "constants.h"
#include <memory>
#include <qdebug.h>

namespace ao_builder
{
LegacyObjectBuilder::LegacyObjectBuilder(Object::Type t)
    : m_type{t}
{}

Object::Type LegacyObjectBuilder::type()
{
    return m_type;
}

std::unique_ptr<Object> LegacyObjectBuilder::buildObject(ObjectParserInterface *parser)
{
    auto result = std::make_unique<LegacyObject>();

    if (!buildBase(parser, result.get(), KeyProvider::iniKeys, key::desktop::SECTION_NAME))
    {
        return nullptr;
    }

    if (!buildFieldWithLocale(parser,
                              KeyProvider::iniKeys.key<NAME>(),
                              result->m_displayName,
                              result->m_displayNameLocaleStorage,
                              key::desktop::SECTION_NAME))
    {
        return nullptr;
    }

    QString categoryId = parser->getValue(KeyProvider::iniKeys.key<CATEGORIES>(), key::desktop::SECTION_NAME);
    result->m_categoryId = categoryId;

    QString icon = parser->getValue(KeyProvider::iniKeys.key<ICON>(), key::desktop::SECTION_NAME);
    result->m_icon = icon;

    QString x_Alterator_URI = parser->getValue(key::desktop::X_ALTERATOR_URI, key::desktop::SECTION_NAME);
    result->m_x_Alterator_URI = x_Alterator_URI;

    QString x_Alterator_Weight = parser->getValue(key::desktop::X_ALTERATOR_WEIGHT, key::desktop::SECTION_NAME);
    if (!x_Alterator_Weight.isEmpty())
        result->m_weight = x_Alterator_Weight.toInt();

    QString x_Alterator_Help = parser->getValue(key::desktop::X_ALTERATOR_HELP, key::desktop::SECTION_NAME);
    result->m_x_Alterator_Help = x_Alterator_Help;

    QString x_Alterator_UI = parser->getValue(key::desktop::X_ALTERATOR_UI, key::desktop::SECTION_NAME);
    result->m_x_Alterator_UI = x_Alterator_UI;

    QString terminal = parser->getValue(key::desktop::TERMINAL, key::desktop::SECTION_NAME);
    if (terminal.toLower() == QString("true"))
    {
        result->m_isTerminal = true;
    }

    QString x_Alterator_Internal_Name = parser->getValue(key::desktop::X_ALTERATOR_INTERNAL, key::desktop::SECTION_NAME);

    result->m_x_Alterator_Internal_Name = x_Alterator_Internal_Name;

    result->m_isLegacy = true;

    return result;
}
} // namespace ao_builder
