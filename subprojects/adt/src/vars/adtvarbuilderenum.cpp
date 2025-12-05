#include "adtvarbuilderenum.h"
#include "adtvar.h"
#include "model/builder/adtbuilderconstants.h"
#include "model/builder/adtbuilderhelper.h"

bool ADTVarBuilderEnum::build(const toml::table *paramSection, ADTTool *tool, const QString &varId)
{
    QMap<QString, QString> displayNames;
    QMap<QString, QString> comments;
    QStringList stringValues;
    QString defaultStringValue;
    QList<int> intValues;
    int defaultIntValue = 0;

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

    //Write values to QStringList or QList<int>
    vals->for_each([&intValues, &stringValues](auto &elem) {
        if (elem.is_integer())
        {
            intValues.append(elem.value_or(0));
        }
        else if (elem.is_string())
        {
            stringValues.append(elem.value_or(""));
        }
    });
    if (intValues.isEmpty() && stringValues.isEmpty())
    {
        qWarning() << "ERROR: Can't get enum values in param: " << varId << " tool with id: " << tool->id();
        return false;
    }

    //Get and valudate default value
    bool intCheck = true;
    auto defaultValue = (*enumTable)[PARAMS_CONSTANTS.ENUM_TYPE_DEFAULT_KEY_NAME];
    if (defaultValue.is_integer())
    {
        defaultIntValue = defaultValue.value_or(0);
        bool flag = false;
        std::for_each(intValues.begin(), intValues.end(), [&flag, &intValues, &defaultIntValue](int &elem) {
            if (elem == defaultIntValue)
                flag = true;
        });

        if (!flag)
        {
            intCheck = false;
            qWarning() << "ERROR: Default value: " << defaultIntValue << "not found in values " << "in param: " << varId
                       << " tool with id: " << tool->id();
            return false;
        }
    }
    else if (defaultValue.is_string())
    {
        defaultStringValue = defaultValue.value_or("");
        if (!defaultStringValue.isEmpty())
        {
            bool flag = false;
            std::for_each(stringValues.begin(), stringValues.end(), [&flag, &stringValues, &defaultStringValue](QString &elem) {
                if (elem == defaultStringValue)
                    flag = true;
            });

            if (!flag)
            {
                qWarning() << "ERROR: Default value: " << defaultStringValue << "not found in values " << "in param: " << varId
                           << " tool with id: " << tool->id();
                return false;
            }
        }
    }
    else
    {
        qWarning() << "ERROR: Default value is absent or not int or string in param: " << varId
                   << " tool with id: " << tool->id();
        return false;
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
        if (defaultStringValue.isEmpty())
        {
            var.reset(new ADTVar(varId, stringValues, stringValues.at(0), displayNames, comments));
        }
        else
        {
            var.reset(new ADTVar(varId, stringValues, defaultStringValue, displayNames, comments));
        }
    }
    else if (!QString::compare(enum_type, PARAMS_CONSTANTS.ENUM_TYPE_INT_TYPE_VALUE)) //this is int enum
    {
        if (!intCheck)
        {
            var.reset(new ADTVar(varId, intValues, intValues.at(0), displayNames, comments));
        }
        else
        {
            var.reset(new ADTVar(varId, intValues, defaultIntValue, displayNames, comments));
        }
    }

    tool->m_vars.push_back(std::move(var));

    return true;
}
