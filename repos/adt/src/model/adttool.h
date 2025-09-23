#ifndef ADTTOOLIMPL_H
#define ADTTOOLIMPL_H

#include "adttest.h"

#include <memory>
#include <QDBusConnection>
#include <QMap>
#include <QStandardItemModel>
#include <QString>

#include "vars/adtvarsinterface.h"

class ADTTool
{
    friend class ADTToolBuilder;
    friend class ADTModelBuilderDbus;
    friend class ADTVarBuilderInt;
    friend class ADTVarBuilderString;
    friend class ADTVarBuilderEnum;

public:
    enum BusType
    {
        None,
        System,
        Session,
        All
    };

    ADTTool();
    virtual ~ADTTool() = default;

    QString id();
    QString type() const;
    BusType bus() const;
    QString category() const;
    QString icon() const;
    QString displayName() const;
    QString comment() const;
    QString reportSuffix(BusType bus) const;
    QString dbusPath() const;
    QString dbusIface() const;
    QString dbusServiceName() const;
    QString runMethodName() const;
    QString reportMethodName() const;
    QString infoMethodName() const;

    //Vars
    QStringList getVarNames() const;
    ADTVarInterface *getVar(QString &varId);

    QStringList getFilteredTests();
    QStringList getTests(BusType bus = All);
    ADTTest *getTest(QString id);

    void clearTestsLogs();

    void setLocale(QString &locale);
    void setFilter(QString filter);
    QString getFilter();

    int getReport(QDBusConnection conn, QByteArray &result);

    std::unique_ptr<QStandardItemModel> getVarsModel();

private:
    QString m_id;
    BusType m_bus;
    QString m_type;
    QString m_category;
    QString m_icon;
    QString m_displayName;
    QString m_comment;
    QString m_reportSuffix;
    QString m_reportSuffixUser;

    QString m_dbusPath;
    QString m_dbusIface;
    QString m_dbusServiceName;
    QString m_dbusRunMethodName;
    QString m_dbusReportMethodName;
    QString m_dbusInfoMethodName;

    QMap<QString, QString> m_displayNameLocaleStorage;
    QMap<QString, QString> m_commentLocaleStorage;

    std::vector<std::unique_ptr<ADTTest>> m_tests;
    QString m_filter;

    std::vector<std::unique_ptr<ADTVarInterface>> m_vars;
};

#endif // ADTTOOLIMPL_H
