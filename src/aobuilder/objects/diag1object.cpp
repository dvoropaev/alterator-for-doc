#include "diag1object.h"

#include <QRegularExpression>

namespace ao_builder
{
void Diag1Object::setLocale(const QString &locale)
{
    Object::setLocale(locale);
    auto commentIt = m_commentLocaleStorage.find(locale);
    if (commentIt != m_commentLocaleStorage.end())
    {
        m_comment = *commentIt;
    }
    else
    {
        m_comment = findLocale(locale, m_commentLocaleStorage);
    }
}
} // namespace ao_builder
