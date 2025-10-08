#ifndef PRODUCT_H
#define PRODUCT_H

#include "branch.h"
#include "edition.h"

#include <QString>

namespace alt
{
struct Product
{
public:
    const std::optional<QString> &displayName() const &;
    const std::optional<Branch> &branch() const &;
    const std::optional<QString> &license() const &;
    const std::optional<Edition> &edition() const &;
    const std::optional<QByteArray> &logo() const &;

    void setDisplayName(const QString &displayName);
    void setBranch(const Branch &branch);
    void setLicense(const QString &license);
    void setEdition(const std::optional<Edition> &edition);
    void setLogo(const QByteArray &logo);

public:
    std::optional<QString> m_displayName;
    std::optional<Branch> m_branch;
    std::optional<Edition> m_edition;
    std::optional<QString> m_license;
    std::optional<QByteArray> m_logo;
    bool m_isBasealt = false;
};
} // namespace alt

#endif
