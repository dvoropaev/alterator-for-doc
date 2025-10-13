#ifndef ADTVARBUILDERENUM_H
#define ADTVARBUILDERENUM_H

#include "adtvarbuilderinterface.h"

class ADTVarBuilderEnum : public ADTVarBuilderInterface
{
public:
    ADTVarBuilderEnum()          = default;
    virtual ~ADTVarBuilderEnum() = default;

    bool build(const toml::table *paramSection, ADTTool *tool, const QString &varId) override;
};

#endif // ADTVARBUILDERENUM_H
