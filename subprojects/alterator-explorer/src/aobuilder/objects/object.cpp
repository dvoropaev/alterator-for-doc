#include <QRegularExpression>

#include "object.h"

namespace ao_builder
{
void Object::setLocale(const QString &locale)
{
    auto nameIt = m_displayNameLocaleStorage.find(locale);
    if (nameIt != m_displayNameLocaleStorage.end())
    {
        m_displayName = *nameIt;
        return;
    }

    m_displayName = findLocale(locale, m_displayNameLocaleStorage);
}

QString Object::findLocale(const QString &locale, const QMap<QString, QString> &localeStorage)
{
    QRegularExpression regex(locale + "_[A-Z]{2}");
    for (auto &fullLocale : localeStorage.keys())
    {
        QRegularExpressionMatch match = regex.match(fullLocale);
        if (match.hasMatch())
        {
            return localeStorage[fullLocale];
        }
    }
    return localeStorage["en"];
}

} // namespace ao_builder
