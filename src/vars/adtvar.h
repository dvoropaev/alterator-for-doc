#ifndef ADTVAR_H
#define ADTVAR_H

#include "vars/adtvarsinterface.h"

#include <memory>
#include <QList>
#include <QVariant>

class ADTVar : public ADTVarInterface
{
public:
    ADTVar(const QString &id,
           int value,
           const QMap<QString, QString> &displayNames,
           const QMap<QString, QString> &comments);
    ADTVar(const QString &id,
           const QString &value,
           const QMap<QString, QString> &displayNames,
           const QMap<QString, QString> &comments);

    ADTVar(const QString &id,
           const QList<int> &value,
           const int currentValue,
           const QMap<QString, QString> &displayNames,
           const QMap<QString, QString> &comments);
    ADTVar(const QString &id,
           const QList<int> &value,
           const int currentValue,
           const int defaultValue,
           const QMap<QString, QString> &displayNames,
           const QMap<QString, QString> &comments);

    ADTVar(const QString &id,
           const QList<QString> &values,
           const QString &currentValue,
           const QMap<QString, QString> &displayNames,
           const QMap<QString, QString> &comments);
    ADTVar(const QString &id,
           const QList<QString> &values,
           const QString &currentValue,
           const QString &defaultValue,
           const QMap<QString, QString> &displayNames,
           const QMap<QString, QString> &comments);

    virtual ~ADTVar();

public:
    //int
    virtual bool set(const int value) override;
    virtual bool get(int *value) const override;

    //QString
    virtual bool set(const QString &string) override;
    virtual bool get(QString *string) const override;

    //Enum
    virtual bool hasDefault() const override;
    virtual bool getDefault(int *value) const override;
    virtual bool getDefault(QString *value) const override;
    virtual bool getEnumValues(QList<QVariant> *values) const override;

    //Common
    virtual QString id() const override;
    virtual ADTVarType getType() const override;
    virtual const QString getDisplayName() const override;
    virtual const QString getComment() const override;
    virtual void translate(const QString &locale) override;

private:
    ADTVarType m_type;

    std::unique_ptr<QVariant> m_currentValue;
    std::unique_ptr<QVariant> m_default;
    std::unique_ptr<QVariant> m_enumStorage;
    QMap<QString, QString> m_displayNames;
    QMap<QString, QString> m_comments;

    QString m_id;
    QString m_displayName;
    QString m_comment;
};

#endif // ADTVAR_H
