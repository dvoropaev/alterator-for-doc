#ifndef PACKAGE_H
#define PACKAGE_H

#include <QMetaType>
#include <QString>

struct Package
{
    Package()  = default;
    ~Package() = default;

    QString m_name{};
    QString m_version{};
    QString m_release{};
    QString m_arch{};
    QString m_group{};
    bool m_installed{};
};

Q_DECLARE_METATYPE(Package)

#endif // PACKAGE_H
