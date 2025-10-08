#pragma once

#include <QString>
#include <QMap>
#include <QLocale>
#include <QCoreApplication>

using LocaleMap = QMap<QString, QString>;

class TranslatableField {
public:
    inline TranslatableField(const LocaleMap& locales = {})
        : m_storage{locales}
    {}

    void setLocale(const QLocale& locale) const;
    inline operator const QString&() const { return m_current; }

private:
    mutable QString m_current;
    LocaleMap m_storage;
};


using Locales = std::pair<TranslatableField, TranslatableField>;

/* Base class for named objects, parsed from toml */
class TranslatableObject {
public:
    TranslatableObject(const QString& name,
                 const Locales& locales);

    virtual void setLocale(const QLocale& locale) const;

    inline const QString& name()        const {return m_name;}
    inline const QString& displayName() const {return m_display_name;}
    inline const QString& comment()     const {return m_comment;}

protected:
    QString m_name;
    TranslatableField m_display_name;
    TranslatableField m_comment;
};
