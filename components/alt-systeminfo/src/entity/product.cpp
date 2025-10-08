#include "product.h"

namespace alt
{
const std::optional<QString> &Product::displayName() const &
{
    return m_displayName;
}

const std::optional<Branch> &Product::branch() const &
{
    return m_branch;
}

const std::optional<QString> &Product::license() const &
{
    return m_edition.has_value() ? m_edition.value().m_license : m_license;
}

const std::optional<Edition> &Product::edition() const &
{
    return m_edition;
}

const std::optional<QByteArray> &Product::logo() const &
{
    return m_edition.has_value() && m_edition.value().m_logo.has_value() ? m_edition->m_logo : m_logo;
}

void Product::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
}

void Product::setBranch(const Branch &branch)
{
    m_branch = branch;
}

void Product::setLicense(const QString &license)
{
    m_license = license;
}

void Product::setEdition(const std::optional<Edition> &edition)
{
    m_edition = edition;
}

void Product::setLogo(const QByteArray &logo)
{
    m_logo = logo;
}
} // namespace alt
