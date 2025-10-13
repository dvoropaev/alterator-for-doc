#ifndef DIAG1OBJECT_H
#define DIAG1OBJECT_H

#include "object.h"

namespace ao_builder
{
class Diag1Object : public Object
{
public:
    Diag1Object() = default;
    ~Diag1Object() override = default;

    void setLocale(const QString &locale) override;

public:
    QString m_comment{};

    QMap<QString, QString> m_commentLocaleStorage{};
};

} // namespace ao_builder

#endif // DIAG1OBJECT_H
