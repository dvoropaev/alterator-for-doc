#include "adtvarbuilderenum.h"
#include "adtvar.h"
#include "model/builder/adtbuilderconstants.h"
#include "model/builder/adtbuilderhelper.h"

bool ADTVarBuilderEnum::build(const toml::table *paramSection, ADTTool *tool, const QString &varId)
{
    QMap<QString, QString> displayNames;
    QMap<QString, QString> comments;
    QStringList valuesList;
    QString defaultValue;

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
    if (type.isEmpty() || QString::compare(type, PARAMS_CONSTANTS.ENUM_TYPE_VALUE))
    {
        qWarning() << "ERROR: Type is empty or not enum in param: " << varId << " tool with id: " << tool->id();
        return false;
    }

    //get enum_values table
    const toml::table *enumTable = (*paramSection)[PARAMS_CONSTANTS.ENUM_VALUES_KEY].as_table();
    if (!enumTable)
    {
        qWarning() << "ERROR: Can't find enum_values in param: " << varId << " tool with id: " << tool->id();
        return false;
    }

    //get values from "values" key
    auto enumValsArray = (*enumTable)[PARAMS_CONSTANTS.ENUM_TYPE_VALUES_KEY_NAME];
    auto vals          = enumValsArray.as_array();
    if (vals->empty())
    {
        qWarning() << "ERROR: enum_values is empty in param: " << varId << " tool with id: " << tool->id();
        return false;
    }

    //Write values to QStringList
    vals->for_each([&valuesList](auto &elem) { valuesList.append(QString(elem.value_or(""))); });
    if (valuesList.isEmpty())
    {
        qWarning() << "ERROR: Can't get enum values in param: " << varId << " tool with id: " << tool->id();
        return false;
    }

    //Get and valudate default value
    defaultValue = (*enumTable)[PARAMS_CONSTANTS.ENUM_TYPE_DEFAULT_KEY_NAME].value_or("");
    if (!defaultValue.isEmpty())
    {
        bool flag = false;
        std::for_each(valuesList.begin(), valuesList.end(), [&flag, &valuesList, &defaultValue](QString &elem) {
            if (elem == defaultValue)
                flag = true;
        });

        if (!flag)
        {
            qWarning() << "ERROR: Default value: " << defaultValue << "not found in values " << "in param: " << varId
                       << " tool with id: " << tool->id();
            return false;
        }
    }

    //get enum type
    QString enum_type = (*enumTable)[PARAMS_CONSTANTS.ENUM_TYPE_TYPE_KEY_NAME].value_or("");
    //type is absent
    if (enum_type.isEmpty())
    {
        qWarning() << "ERROR: Type of enum is absent or not int or string in param: " << varId
                   << " tool with id: " << tool->id();
        return false;
    }

    //type is not a string or int
    if (QString::compare(enum_type, QString(PARAMS_CONSTANTS.ENUM_TYPE_STRING_TYPE_VALUE))
        && QString::compare(enum_type, PARAMS_CONSTANTS.ENUM_TYPE_INT_TYPE_VALUE))
    {
        qWarning() << "ERROR: Type of enum is wrong (not int or string) in param: " << varId
                   << " tool with id: " << tool->id();
        return false;
    }

    std::unique_ptr<ADTVar> var{nullptr};

    if (!QString::compare(enum_type, PARAMS_CONSTANTS.ENUM_TYPE_STRING_TYPE_VALUE)) //this is string enum
    {
        if (defaultValue.isEmpty())
        {
            var.reset(new ADTVar(varId, valuesList, valuesList.at(0), displayNames, comments));
        }
        else
        {
            var.reset(new ADTVar(varId, valuesList, defaultValue, displayNames, comments));
        }
    }
    else if (!QString::compare(enum_type, PARAMS_CONSTANTS.ENUM_TYPE_INT_TYPE_VALUE)) //this is int enum
    {
        QList<int> intValues;
        int defaultIntValue = 0;

        std::for_each(valuesList.begin(), valuesList.end(), [&intValues, tool, &varId](QString &val) {
            bool ok        = false;
            int currentVal = val.toInt(&ok, 10);
            if (!ok)
            {
                qWarning() << "ERROR: Value is not a number in INTEGER param: " << varId
                           << " tool with id: " << tool->id();
                return;
            }

            intValues.append(currentVal);
        });

        if (intValues.isEmpty())
        {
            qWarning() << "ERROR: Can't convert enum values to integer in param: " << varId
                       << " tool with id: " << tool->id();
            return false;
        }

        if (defaultValue.isEmpty())
        {
            bool success = false;
            int value    = valuesList.at(0).toInt(&success);
            if (!success)
            {
                qWarning() << "ERROR: Can't convert default enum value to integer in param: " << varId
                           << " tool with id: " << tool->id();

                return false;
            }

            var.reset(new ADTVar(varId, intValues, value, displayNames, comments));
        }
        else
        {
            bool success    = false;
            defaultIntValue = defaultValue.toInt(&success, 10);
            if (success)
                var.reset(new ADTVar(varId, intValues, defaultIntValue, displayNames, comments));
            else
            {
                qWarning() << "ERROR: Can't convert default enum value to integer in param: " << varId
                           << " tool with id: " << tool->id();
                return false;
            }
        }
    }

    tool->m_vars.push_back(std::move(var));

    return true;
}
