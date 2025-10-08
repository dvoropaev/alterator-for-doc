#ifndef AOB_CATEGORY_H
#define AOB_CATEGORY_H

#include "object.h"

namespace ao_builder
{
class Category : public Object
{
public:
    Category() = default;
    ~Category() override = default;

    void setLocale(const QString &locale) override;

public:
    QString m_comment{};

    QMap<QString, QString> m_commentLocaleStorage{};
};
} // namespace ao_builder

#endif // AB_CATEGORY_H
