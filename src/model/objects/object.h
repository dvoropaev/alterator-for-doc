#ifndef OBJECT_H
#define OBJECT_H

#define TOML_EXCEPTIONS 0
#include <toml++/toml.h>
#include <QMap>
#include <QObject>
#include <QString>

namespace alt
{
class Object
{
public:
    Object() = default;
    Object(const toml::table &data);
    Object(const Object &) = default;
    Object(Object &&) = default;
    Object &operator=(const Object &object) = default;
    Object &operator=(Object &&object) = default;

    virtual ~Object() = 0;
    virtual void setLocale(const QString &locale);

public:
    QString name{};
    QString type{};
    QString category{}; // TODO(cherniginma): make it optional
    QString displayName{};
    QString description{};
    QString comment{};
    QString dbusPath{};
    QString icon{};

    bool isDraft = false;

    QMap<QString, QString> displayNameLocaleStorage{};
    QMap<QString, QString> descriptionLocaleStorage{};
    QMap<QString, QString> commentLocaleStorage{};

protected:
    static QString findLocale(const QString &locale, QMap<QString, QString> &localeStorage);
    static void setFieldLocale(const QString &locale, QMap<QString, QString> &storage, QString &field);
};
} // namespace alt

Q_DECLARE_METATYPE(alt::Object *);

#endif // OBJECT_H
