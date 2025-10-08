#ifndef EDITION_H
#define EDITION_H

#include <optional>
#include <QByteArray>
#include <QString>

namespace alt
{
struct Edition
{
    QString m_name;
    std::optional<QString> m_displayName;
    std::optional<QString> m_desktopEnvironment;
    std::optional<QString> m_license;
    std::optional<QByteArray> m_logo;
};
} // namespace alt

#endif
