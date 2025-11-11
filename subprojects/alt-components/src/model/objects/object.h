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
    QString displayName() const;
    QString comment() const;

public:
    Object() = default;
    Object(const toml::table &data);
    Object(const Object &) = default;
    Object(Object &&) = default;
    Object &operator=(const Object &object) = default;
    Object &operator=(Object &&object) = default;

    virtual ~Object() = 0;

public:
    QString name{};
    QString type{};
    QString category{}; // TODO(cherniginma): make it optional
    QString dbusPath{};
    QString icon{};

    QMap<QString, QString> displayNameStorage{};
    QMap<QString, QString> descriptionStorage{};
    QMap<QString, QString> commentStorage{};

    bool isDraft = false;

protected:
    static QString localizedString(const QMap<QString, QString> &storage);

private:
    static QHash<QLocale, QString> cachedLangs;
};
} // namespace alt

Q_DECLARE_METATYPE(alt::Object *);

#endif // OBJECT_H
