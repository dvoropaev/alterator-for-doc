#ifndef ADTBUILDERHELPER_H
#define ADTBUILDERHELPER_H

#include "adtbuilderconstants.h"

#include <toml++/toml.h>
#include <QMap>
#include <QString>

static const ADTBuilderParamConstants PARAMS_CONSTANTS;
static const ADTBuilderToolConstants TOOLS_CONSTANTS;
static const ADTBuilderTestConstants TESTS_CONSTANTS;

class ADTBuilderHelper final
{
public:
    static bool parseToMapWithMandatoryEnField(const toml::table *table,
                                               const QString &name,
                                               QMap<QString, QString> &map,
                                               bool isEnMandatory)
    {
        bool isEnExits = false;

        if (const auto section = table->get(name.toStdString()))
        {
            if (const auto sectionTable = section->as_table())
            {
                for (auto it = sectionTable->begin(); it != sectionTable->end(); it++)
                {
                    if (isEnMandatory && !QString::compare(QString("en"), QString(it->first.data())))
                    {
                        isEnExits = true;
                    }
                    map[it->first.data()] = QString::fromStdString(it->second.value<std::string>().value_or(""));
                }
            }
        }
        else
            return false;

        if (isEnMandatory && !isEnExits)
            return false;

        return true;
    }
};

#endif // ADTBUILDERHELPER_H
