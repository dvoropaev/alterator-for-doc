#include "object.h"
#include "application.h"
#include "constants.h"

namespace alt
{
QHash<QLocale, QString> Object::cachedLangs = {};

Object::Object(const toml::table &data)
{
    if (const toml::value<std::string> *type = data[COMPONENT_OBJECT_TYPE_KEY_NAME].as_string())
    {
        this->type = QString::fromStdString(type->value_or(""));
    }

    if (const toml::value<std::string> *name = data[COMPONENT_NAME_KEY_NAME].as_string())
    {
        this->name = QString::fromStdString(name->value_or(""));
    }

    if (const toml::value<bool> *draft = data[COMPONENT_DRAFT_KEY_NAME].as_boolean())
    {
        this->isDraft = draft->value_or(false);
    }

    if (const toml::value<std::string> *parentCategory = data[COMPONENT_CATEGORY_KEY_NAME].as_string())
    {
        this->category = QString::fromStdString(parentCategory->value_or(""));
    }

    if (const toml::table *displayName = data[COMPONENT_DISPLAY_NAME_KEY_NAME].as_table())
    {
        for (const auto &[language, value] : *displayName)
        {
            const auto tr = QString::fromStdString(*value.value<std::string>());
            this->displayNameStorage.insert(language.str().data(), tr);
        }
    }

    if (const toml::table *comment = data[COMPONENT_COMMENT_KEY_NAME].as_table())
    {
        for (const auto &[language, value] : *comment)
        {
            const auto tr = QString::fromStdString(*value.value<std::string>());
            this->commentStorage.insert(language.str().data(), tr);
        }
    }

    if (const toml::value<std::string> *icon = data[COMPONENT_ICON_KEY_NAME].as_string())
    {
        this->icon = QString::fromStdString(icon->value_or(""));
    }
}

Object::~Object() = default;

QString Object::displayName() const
{
    return localizedString(displayNameStorage);
}

QString Object::comment() const
{
    return localizedString(commentStorage);
}

QString Object::localizedString(const QMap<QString, QString> &storage)
{
    auto &value = cachedLangs[Application::getLocale()];
    if (value.isEmpty())
    {
        value = Application::getLocale().name().split("_")[0];
    }

    auto iter = storage.find(value);
    if (iter != storage.end())
    {
        return *iter;
    }
    return storage.value(COMPONENT_DEFAULT_LANUAGE, {});
}
} // namespace alt
