#pragma once

#include <QString>
#include <QMap>
#include <QLocale>

using LocaleMap = QMap<QString, QString>;
using Locales = std::pair<LocaleMap, LocaleMap>;

/* Base class for named objects, parsed from toml */
class TranslatableObject {
public:
    TranslatableObject(const QString& name,
                 const Locales& locales);

    virtual void setLocale(const QLocale& locale) const;

    const QString& name()        const {return m_name;}
    const QString& displayName() const {return m_display_name;}
    const QString& comment()     const {return m_comment;}

protected:
    QString m_name;
    mutable QString m_display_name;
    mutable QString m_comment;
    LocaleMap m_display_name_storage;
    LocaleMap m_comment_storage;
};
