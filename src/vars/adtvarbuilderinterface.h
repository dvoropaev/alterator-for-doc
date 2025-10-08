#ifndef ADTVARBUILDERINTERFACE_H
#define ADTVARBUILDERINTERFACE_H

#include "vars/adtvarsinterface.h"

#include <model/adttool.h>
#include <toml++/toml.hpp>

class ADTVarBuilderInterface
{
public:
    enum BuilderType
    {
        STRING,
        INT,
        ENUM
    };

public:
    virtual ~ADTVarBuilderInterface() = default;

    virtual bool build(const toml::table *paramSection, ADTTool *tool, const QString &varId) = 0;
};

#endif // ADTVARBUILDERINTERFACE_H
