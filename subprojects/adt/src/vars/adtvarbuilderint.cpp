#include "adtvarbuilderint.h"
#include "adtvar.h"
#include "model/builder/adtbuilderconstants.h"
#include "model/builder/adtbuilderhelper.h"

#include <QDebug>

bool ADTVarBuilderInt::build(const toml::table *paramSection, ADTTool *tool, const QString &varId)
{
    QMap<QString, QString> displayNames;
    QMap<QString, QString> comments;
    bool isMandatory = false;
    int value        = 0;

    //Check param id
    if (varId.isEmpty())
    {
        qWarning() << "ERROR: Param id is empty in tool with id: " << tool->id();
        return false;
    }

    //parse display names
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
    if (type.isEmpty() || QString::compare(type, PARAMS_CONSTANTS.INT_TYPE_VALUE))
    {
        qWarning() << "ERROR: Type is empty or not int in param: " << varId << " tool with id: " << tool->id();
        return false;
    }

    //value
    auto val = (*paramSection)[PARAMS_CONSTANTS.VALUE_KEY_NAME];
    if (val.is_integer())
    {
        value = val.value_or(0);
    }
    else
    {
        qWarning() << "ERROR: Value is not a number in INTEGER param: " << varId << " tool with id: " << tool->id();
        return false;
    }

    tool->m_vars.push_back(
        std::move(std::unique_ptr<ADTVarInterface>(new ADTVar(varId, value, displayNames, comments))));

    return true;
}
