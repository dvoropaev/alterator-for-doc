#include "object.h"
#include "constants.h"

#include <QDebug>
#include <QRegularExpression>

namespace alt
{
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
            displayNameLocaleStorage.insert(language.str().data(), tr);
        }
        this->displayName = this->displayNameLocaleStorage[COMPONENT_DEFAULT_LANUAGE];
    }

    if (const toml::table *comment = data[COMPONENT_COMMENT_KEY_NAME].as_table())
    {
        for (const auto &[language, value] : *comment)
        {
            const auto tr = QString::fromStdString(*value.value<std::string>());
            this->commentLocaleStorage.insert(language.str().data(), tr);
        }
        this->comment = this->commentLocaleStorage[COMPONENT_DEFAULT_LANUAGE];
    }

    if (const toml::value<std::string> *icon = data[COMPONENT_ICON_KEY_NAME].as_string())
    {
        this->icon = QString::fromStdString(icon->value_or(""));
    }
}

Object::~Object() = default;

void Object::setLocale(const QString &locale)
{
    setFieldLocale(locale, displayNameLocaleStorage, displayName);
    setFieldLocale(locale, descriptionLocaleStorage, description);
    setFieldLocale(locale, commentLocaleStorage, comment);
}

QString Object::findLocale(const QString &locale, QMap<QString, QString> &localeStorage)
{
    QRegularExpression regex(locale);
    for (const auto &fullLocale : localeStorage.keys())
    {
        QRegularExpressionMatch match = regex.match(fullLocale);
        if (match.hasMatch())
        {
            return localeStorage[fullLocale];
        }
    }
    return {};
}

void Object::setFieldLocale(const QString &locale, QMap<QString, QString> &storage, QString &field)
{
    for (const auto &locale : QStringList{locale, locale.split('_').first()})
    {
        QString found = findLocale(locale, storage);
        if (!found.isEmpty())
        {
            field = found;
            return;
        }
        found = findLocale(locale + "_[A-Z]{2}", storage);
        if (!found.isEmpty())
        {
            field = found;
            return;
        }
    }
}
} // namespace alt
