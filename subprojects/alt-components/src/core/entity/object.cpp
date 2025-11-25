#include "object.h"
#include "builder.h"
#include "constants.h"

namespace alt
{

Object::Object(const toml::table &data)
    : name(buildField<std::string>(data, entity::COMPONENT_NAME_KEY_NAME).value_or(""))
    , type(buildField<std::string>(data, entity::COMPONENT_OBJECT_TYPE_KEY_NAME).value_or(""))
    , category(buildField<std::string>(data, entity::COMPONENT_CATEGORY_KEY_NAME).value_or(""))
    , icon(buildField<std::string>(data, entity::COMPONENT_ICON_KEY_NAME).value_or(""))
    , displayNameStorage(buildMap<std::string>(data, entity::COMPONENT_DISPLAY_NAME_KEY_NAME))
    , commentStorage(buildMap<std::string>(data, entity::COMPONENT_COMMENT_KEY_NAME))
    , isDraft(buildField<bool>(data, entity::COMPONENT_DRAFT_KEY_NAME).value_or(false))
{}

Object::~Object() = default;

std::string_view Object::displayName(const std::string &lang) const
{
    return localizedString(displayNameStorage, lang);
}

std::string_view Object::comment(const std::string &lang) const
{
    return localizedString(commentStorage, lang);
}

std::string_view Object::localizedString(const std::map<std::string, std::string> &storage, const std::string &lang)
{
    auto iter = storage.find(lang);
    if (iter != storage.end())
    {
        return iter->second;
    }
    iter = storage.find(entity::COMPONENT_DEFAULT_LANUAGE);
    if (iter != storage.end())
    {
        return iter->second;
    }
    return "";
}
} // namespace alt
