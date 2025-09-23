#ifndef ADTVARBUILDERINT_H
#define ADTVARBUILDERINT_H

#include "adtvarbuilderinterface.h"

class ADTVarBuilderInt : public ADTVarBuilderInterface
{
public:
    ADTVarBuilderInt()          = default;
    virtual ~ADTVarBuilderInt() = default;

    virtual bool build(const toml::table *paramSection, ADTTool *tool, const QString &varId) override;
};

#endif // ADTVARBUILDERINT_H
