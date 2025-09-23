#ifndef ADTVARBUILDERFACTORY_H
#define ADTVARBUILDERFACTORY_H

#include "adtvarbuilderenum.h"
#include "adtvarbuilderint.h"
#include "adtvarbuilderstring.h"
#include "model/builder/adtbuilderconstants.h"
#include "vars/adtvarsinterface.h"

template<ADTVarBuilderInterface::BuilderType type>
class ADTVarBuilderFactory final
{
public:
    static std::unique_ptr<ADTVarBuilderInterface> create() { return nullptr; }
};

template<>
class ADTVarBuilderFactory<ADTVarBuilderInterface::BuilderType::INT> final
{
public:
    static std::unique_ptr<ADTVarBuilderInterface> create() { return std::make_unique<ADTVarBuilderInt>(); }
};

template<>
class ADTVarBuilderFactory<ADTVarBuilderInterface::BuilderType::STRING> final
{
public:
    static std::unique_ptr<ADTVarBuilderInterface> create() { return std::make_unique<ADTVarBuilderString>(); }
};

template<>
class ADTVarBuilderFactory<ADTVarBuilderInterface::BuilderType::ENUM> final
{
public:
    static std::unique_ptr<ADTVarBuilderInterface> create() { return std::make_unique<ADTVarBuilderEnum>(); }
};

#endif // ADTVARBUILDERFACTORY_H
