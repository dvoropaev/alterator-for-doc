#ifndef REPO_H
#define REPO_H

#include <QMetaType>
#include <QString>

struct Repo
{
    Repo()  = default;
    ~Repo() = default;

    QString m_type{};
    QString m_branch{};
    QString m_url{};
    QString m_component{};
    QString m_full{};
};

Q_DECLARE_METATYPE(Repo)

#endif // REPO_H
