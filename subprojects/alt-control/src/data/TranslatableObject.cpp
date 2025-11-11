#include "TranslatableObject.h"

TranslatableObject::TranslatableObject(const QString& name, const Locales& locales)
    : m_name{name}
    , m_display_name_storage{locales.first}
    , m_comment_storage{locales.second}
{}

void TranslatableObject::setLocale(const QLocale& locale) const {
    auto name = locale.name().split('_').front();
    m_display_name = m_display_name_storage.value(name, m_display_name_storage.value("en", ""));
    m_comment      =      m_comment_storage.value(name,      m_comment_storage.value("en", ""));
}
