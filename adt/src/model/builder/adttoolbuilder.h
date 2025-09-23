#ifndef ADTTOOLBUILDER_H
#define ADTTOOLBUILDER_H

#include "adtbuilderconstants.h"
#include "model/adttool.h"

#include <memory>
#include <QString>

#include <toml++/toml.hpp>

class ADTToolBuilder
{
public:
    ADTToolBuilder(const QString &data,
                   const QStringList &testsList,
                   const ADTTool::BusType busType,
                   const QString &dbusServiceName,
                   const QString &dbusPath,
                   const QString &dbusInterfaceName,
                   const QString &dbusInfoMethodName,
                   const QString &dbusRunMethodName,
                   const QString &dbusReportMethodName);

    virtual ~ADTToolBuilder() = default;

    std::unique_ptr<ADTTool> buildTool();

private:
    bool parseRoot(const toml::table &root, ADTTool *tool);

    bool parseTestsSection(const toml::table *testSection,
                           ADTTool *tool,
                           const QString &testName,
                           ADTTool::BusType busType);
    bool parseParamsSection(const toml::table *testSection, ADTTool *tool, const QString &varName);

private:
    QString m_info;
    QStringList m_testLists;
    ADTTool::BusType m_busType;
    QString m_dbusServiceName;
    QString m_dbusPath;
    QString m_dbusInterfaceName;
    QString m_dbusInfoMethodName;
    QString m_dbusRunMethodName;
    QString m_dbusReportMethodName;
};

#endif // ADTTOOLBUILDER_H
