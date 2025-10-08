#ifndef ADTVARMODELBUILDFACTORY_H
#define ADTVARMODELBUILDFACTORY_H

#include "adttoolvarsmodelrowbuilders.h"
#include "adtvarmodelbuilderinterface.h"

template<ADTVarModelBuilderInterface::VarBuilderType type>
class ADTVarModelBuildFactory
{
public:
    static std::unique_ptr<ADTVarModelBuilderInterface> create() { return nullptr; }
};

template<>
class ADTVarModelBuildFactory<ADTVarModelBuilderInterface::VarBuilderType::INT>
{
public:
    static std::unique_ptr<ADTVarModelBuilderInterface> create()
    {
        return std::make_unique<ADTToolVarModelRowBuilderInt>();
    }
};

template<>
class ADTVarModelBuildFactory<ADTVarModelBuilderInterface::VarBuilderType::STRING>
{
public:
    static std::unique_ptr<ADTVarModelBuilderInterface> create()
    {
        return std::make_unique<ADTToolVarModelRowBuilderString>();
    }
};

template<>
class ADTVarModelBuildFactory<ADTVarModelBuilderInterface::VarBuilderType::ENUM_INT>
{
public:
    static std::unique_ptr<ADTVarModelBuilderInterface> create()
    {
        return std::make_unique<ADTToolVarModelRowBuilderEnumInt>();
    }
};

template<>
class ADTVarModelBuildFactory<ADTVarModelBuilderInterface::VarBuilderType::ENUM_STRING>
{
public:
    static std::unique_ptr<ADTVarModelBuilderInterface> create()
    {
        return std::make_unique<ADTToolVarModelRowBuilderEnumString>();
    }
};

#endif // ADTVARMODELBUILDFACTORY_H
