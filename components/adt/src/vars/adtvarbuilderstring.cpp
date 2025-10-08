#include "adtvarbuilderstring.h"
#include "adtvar.h"
#include "model/builder/adtbuilderhelper.h"

bool ADTVarBuilderString::build(const toml::table *paramSection, ADTTool *tool, const QString &varId)
{
    QMap<QString, QString> displayNames;
    QMap<QString, QString> comments;
    bool isMandatory = false;
    QString value;

    if (varId.isEmpty())
    {
        qWarning() << "ERROR: Param id is empty in tool with id:" << tool->id();
        return false;
    }

    if (!ADTBuilderHelper::parseToMapWithMandatoryEnField(paramSection,
                                                          PARAMS_CONSTANTS.DISPLAY_NAME_KEY_NAME,
                                                          displayNames,
                                                          true))
    {
        qWarning() << "ERROR: Can't find display name in param: " << varId << " tool with id: " << tool->id();
        return false;
    }

    //parse comments
    if (!ADTBuilderHelper::parseToMapWithMandatoryEnField(paramSection,
                                                          PARAMS_CONSTANTS.COMMENT_KEY_NAME,
                                                          comments,
                                                          true))
    {
        qWarning() << "ERROR: Can't find comment in param: " << varId << " tool with id: " << tool->id();
        return false;
    }

    //type
    QString type = QString((*paramSection)[PARAMS_CONSTANTS.TYPE_KEY_NAME].value_or(""));
    if (!type.isEmpty() && QString::compare(type, PARAMS_CONSTANTS.STRING_TYPE_VALUE))
    {
        qWarning() << "ERROR: Type is not empty or not string in param: " << varId << " tool with id: " << tool->id();
        return false;
    }

    //value
    QString val = QString((*paramSection)[PARAMS_CONSTANTS.VALUE_KEY_NAME].value_or(""));
    if (!val.isEmpty()) //value is specified
    {
        value = val;
    }

    tool->m_vars.push_back(
        std::move(std::unique_ptr<ADTVarInterface>(new ADTVar(varId, value, displayNames, comments))));

    return true;
}
