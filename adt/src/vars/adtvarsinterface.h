#ifndef ADTVARSINTERFACE_H
#define ADTVARSINTERFACE_H

#include <QList>
#include <QString>
#include <QVariant>

class ADTVarInterface
{
public:
    enum ADTVarType
    {
        NONE,
        INT,
        STRING,
        ENUM_INT,
        ENUM_STRING
    };

public:
    virtual ~ADTVarInterface() = default;

    //int
    virtual bool set(const int value)  = 0;
    virtual bool get(int *value) const = 0;

    //QString
    virtual bool set(const QString &string) = 0;
    virtual bool get(QString *string) const = 0;

    //Enum
    virtual bool hasDefault() const                           = 0;
    virtual bool getDefault(int *value) const                 = 0;
    virtual bool getDefault(QString *value) const             = 0;
    virtual bool getEnumValues(QList<QVariant> *values) const = 0;

    //Common
    virtual QString id() const                    = 0;
    virtual ADTVarType getType() const            = 0;
    virtual const QString getDisplayName() const  = 0;
    virtual const QString getComment() const      = 0;
    virtual void translate(const QString &locale) = 0;
};

#endif // ADTVARSINTERFACE_H
