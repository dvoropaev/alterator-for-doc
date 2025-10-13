#include "TranslatableObject.h"

void TranslatableField::setLocale(const QLocale& locale) const
{
    auto name = locale.name().split('_').front();
    m_current = m_storage.value(name, m_storage.value("en", ""));
}

TranslatableObject::TranslatableObject(const QString& name, const Locales& locales)
    : m_name{name}
    , m_display_name{locales.first}
    , m_comment{locales.second}
{}

void TranslatableObject::setLocale(const QLocale& locale) const {
    m_display_name.setLocale(locale);
    m_comment.setLocale(locale);
}
