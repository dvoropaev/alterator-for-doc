#ifndef ADTVARBUILDERSTRING_H
#define ADTVARBUILDERSTRING_H

#include "adtvarbuilderinterface.h"

class ADTVarBuilderString : public ADTVarBuilderInterface
{
public:
    ADTVarBuilderString()          = default;
    virtual ~ADTVarBuilderString() = default;

    bool build(const toml::table *paramSection, ADTTool *tool, const QString &varId) override;
};

#endif // ADTVARBUILDERSTRING_H
