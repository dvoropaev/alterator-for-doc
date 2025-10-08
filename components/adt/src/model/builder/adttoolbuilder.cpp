#include "adttoolbuilder.h"
#include "model/adttest.h"
#include "model/builder/adtbuilderhelper.h"
#include "vars/adtvar.h"
#include "vars/adtvarbuilderfactory.h"

#include <QDebug>

ADTToolBuilder::ADTToolBuilder(const QString &data,
                               const QStringList &testsList,
                               const ADTTool::BusType busType,
                               const QString &dbusServiceName,
                               const QString &dbusPath,
                               const QString &dbusInterfaceName,
                               const QString &dbusInfoMethodName,
                               const QString &dbusRunMethodName,
                               const QString &dbusReportMethodName)
    : m_info(data)
    , m_testLists(testsList)
    , m_busType(busType)
    , m_dbusServiceName(dbusServiceName)
    , m_dbusPath(dbusPath)
    , m_dbusInterfaceName(dbusInterfaceName)
    , m_dbusInfoMethodName(dbusInfoMethodName)
    , m_dbusRunMethodName(dbusRunMethodName)
    , m_dbusReportMethodName(dbusReportMethodName)
{}

std::unique_ptr<ADTTool> ADTToolBuilder::buildTool()
{
    std::istringstream iStream(m_info.toStdString());

    auto tool = std::make_unique<ADTTool>();

    tool->m_bus = m_busType;

    try
    {
        auto tbl = toml::parse(iStream);

        if (!parseRoot(tbl, tool.get()))
        {
            qWarning() << "Warning: can't parse tool. Skipping..." << tool.get()->id();
            return {};
        }

        for (auto it = tbl.as_table()->begin(); it != tbl.as_table()->end(); ++it)
        {
            //Parsing tests
            if (it->second.is_table()
                && !QString::compare(QString(TOOLS_CONSTANTS.TESTS_SECTION_NAME),
                                     QString::fromStdString(std::string(it->first.str()))))
            {
                for (auto itTest = it->second.as_table()->begin(); itTest != it->second.as_table()->end(); ++itTest)
                {
                    if (!parseTestsSection(itTest->second.as_table(),
                                           tool.get(),
                                           QString(itTest->first.data()),
                                           m_busType))
                    {
                        qWarning() << "Warning: can't parse test from: " << it->first.data() << "Skipping...";
                        continue;
                    }
                }
            }

            //Parsing parameters
            if (it->second.is_table()
                && !QString::compare(QString(TOOLS_CONSTANTS.PARAMS_SECTION_NAME),
                                     QString::fromStdString(std::string(it->first.str()))))
            {
                for (auto itParams = it->second.as_table()->begin(); itParams != it->second.as_table()->end();
                     ++itParams)
                {
                    if (!parseParamsSection(itParams->second.as_table(), tool.get(), QString(itParams->first.data())))
                    {
                        qWarning() << "Warning: can't parse params from: " << tool->id() << " Skipping...";
                        continue;
                    }
                }
            }
        }
    }
    catch (const toml::parse_error &err)
    {
        qWarning() << "Warning: can't parse object info. Skipping... ";
        return {};
    }

    return tool;
}

bool ADTToolBuilder::parseRoot(const toml::table &root, ADTTool *tool)
{
    QString toolId(root[TOOLS_CONSTANTS.NAME_KEY_NAME].value_or(""));

    if (toolId.isEmpty())
    {
        qWarning() << "Warning: can't find tool id. Skipping...";
        return false;
    }

    tool->m_id          = toolId;
    tool->m_displayName = toolId;

    QString reportSuffix(root[TOOLS_CONSTANTS.REPORT_FILE_SUFFIX_KEY_NAME].value_or(""));
    if (reportSuffix.isEmpty())
    {
        qWarning() << "Warning: can't find report suffix in tool:" << tool->id() << ". Skipping...";
        return false;
    }
    tool->m_reportSuffix = reportSuffix;

    QString icon(root[TOOLS_CONSTANTS.ICON_KEY_NAME].value_or(""));
    if (icon.isEmpty())
    {
        qWarning() << "Warning: Can't find icon in tool with id: " << tool->id() << ". Use default icon.";
        tool->m_icon = TOOLS_CONSTANTS.DEFAULT_ICON;
    }
    else
        tool->m_icon = icon;

    QString type(root[TOOLS_CONSTANTS.TYPE_KEY_NAME].value_or(""));
    if (type.isEmpty() || type != "Diag")
    {
        qWarning() << "Warning: Type of tool: " << tool->id() << " is empty or not Diag. Skipping...";
        return false;
    }
    tool->m_type = type;

    QString category(root[TOOLS_CONSTANTS.CATEGORY_KEY_NAME].value_or(""));

    tool->m_dbusServiceName      = m_dbusServiceName;
    tool->m_dbusPath             = m_dbusPath;
    tool->m_dbusIface            = m_dbusInterfaceName;
    tool->m_dbusInfoMethodName   = m_dbusInfoMethodName;
    tool->m_dbusRunMethodName    = m_dbusRunMethodName;
    tool->m_dbusReportMethodName = m_dbusReportMethodName;

    if (!ADTBuilderHelper::parseToMapWithMandatoryEnField(&root,
                                                          TOOLS_CONSTANTS.DISPLAY_NAME_KEY_NAME,
                                                          tool->m_displayNameLocaleStorage,
                                                          true))
    {
        qWarning() << "Warning: can't default display name in tool:" << tool->id() << ". Skipping...";
        return false;
    }

    if (!ADTBuilderHelper::parseToMapWithMandatoryEnField(&root,
                                                          TOOLS_CONSTANTS.COMMENT_KEY_NAME,
                                                          tool->m_commentLocaleStorage,
                                                          true))
    {
        qWarning() << "Warning: can't default comment in tool:" << tool->id() << ". Skipping...";
        return false;
    }

    return true;
}

bool ADTToolBuilder::parseTestsSection(const toml::table *testSection,
                                       ADTTool *tool,
                                       const QString &testName,
                                       ADTTool::BusType busType)
{
    std::unique_ptr<ADTTest> test = std::make_unique<ADTTest>();

    if (busType == ADTTool::BusType::System)
    {
        test->m_bus = ADTTest::BusType::System;
    }
    else if (busType == ADTTool::BusType::Session)
    {
        test->m_bus = ADTTest::BusType::Session;
    }
    else
    {
        qWarning() << "Warning: unknown bus type in test with id: " << test->id() << " in tool: " << tool->id()
                   << " skipping test...";
        return false;
    }

    test->m_testId = testName;

    if (busType == ADTTool::BusType::Session)
    {
        test->m_id = testName + "session";
    }
    else if (busType == ADTTool::BusType::System)
    {
        test->m_id = testName + "system";
    }

    test->m_toolId = tool->id();

    test->m_icon = QString((*testSection)[TESTS_CONSTANTS.ICON_KEY_NAME].value_or(""));
    if (test->m_icon.isEmpty())
        test->m_icon = TESTS_CONSTANTS.DEFAULT_ICON;

    if (!ADTBuilderHelper::parseToMapWithMandatoryEnField(testSection,
                                                          TESTS_CONSTANTS.DISPLAY_NAME_KEY_NAME,
                                                          test->m_displayNameLocaleStorage,
                                                          true))
    {
        qWarning() << "Warning: can't parse display name of the test with id: " << test->id()
                   << " in tool: " << tool->id() << " skipping test...";
        return false;
    }

    test->m_displayName = test->m_displayNameLocaleStorage["en"];

    if (!ADTBuilderHelper::parseToMapWithMandatoryEnField(testSection,
                                                          TESTS_CONSTANTS.COMMENT_KEY_NAME,
                                                          test->m_commentLocaleStorage,
                                                          true))
    {
        qWarning() << "Warning: can't parse comments of the test with id: " << test->id() << " in tool: " << tool->id()
                   << " skipping test...";
        return false;
    }
    test->m_comment = test->m_commentLocaleStorage["en"];

    tool->m_tests.push_back(std::move(test));

    return true;
}

bool ADTToolBuilder::parseParamsSection(const toml::table *testSection, ADTTool *tool, const QString &varName)
{
    //get type
    auto paramType = QString((*testSection)[PARAMS_CONSTANTS.TYPE_KEY_NAME].value_or(""));
    if (paramType.isEmpty())
        paramType = QString(PARAMS_CONSTANTS.STRING_TYPE_VALUE);

    std::unique_ptr<ADTVarBuilderInterface> builder;

    auto it = STRING_TO_ADTVARBUILDER_TYPE.find(paramType);
    if (it == STRING_TO_ADTVARBUILDER_TYPE.end())
        return false;

    auto findVar{[&varName, tool](const QString &var) -> bool {
        bool flag = false;

        std::for_each(tool->m_vars.begin(),
                      tool->m_vars.end(),
                      [&flag, &var](std::unique_ptr<ADTVarInterface> &currentVar) {
                          if (var == currentVar->id())
                              flag = true;
                      });

        return flag;
    }};

    if (findVar(varName))

    {
        qWarning() << "Var " << varName << " already exist in tool " << tool->id() << " skipping..";
        return false; //Find var with same id, skipping
    }

    switch (it->second)
    {
    case ADTVarBuilderInterface::BuilderType::INT:
        return ADTVarBuilderFactory<ADTVarBuilderInterface::BuilderType::INT>::create()->build(testSection,
                                                                                               tool,
                                                                                               varName);
        break;
    case ADTVarBuilderInterface::BuilderType::STRING:
        return ADTVarBuilderFactory<ADTVarBuilderInterface::BuilderType::STRING>::create()->build(testSection,
                                                                                                  tool,
                                                                                                  varName);
        break;
    case ADTVarBuilderInterface::BuilderType::ENUM:
        return ADTVarBuilderFactory<ADTVarBuilderInterface::BuilderType::ENUM>::create()->build(testSection,
                                                                                                tool,
                                                                                                varName);
        break;
    }

    return false;
}
