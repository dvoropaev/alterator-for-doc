#pragma once

#include "Property.h"
class Resource;


/*
 *  Parameter is a top-level configuration Property of a Service.
 *  It has a set of values:
 *      - Property::defaultValue()
 *      - currentValue() - representing actual parameter value of deployed Service
 *      -    editValue() - a working copy, used by an editor
 */
class Parameter : public Property
{
public:
    /*
     *  Each parameter may take place in different contexts.
     *  Also it may be optional in some of them.
     */
    enum class Context {
        /*
         *  Initial configuration.
         *  Usually contains most configuration properties from Context::Configure
         *  and some additional parameters, affecting the deployment process.
         */
        Deploy    = 1 << 0,

        /*
         *  Current configuration.
         *  Usually contains most configuration properties from Context::Configure
         *  and some additional parameters, representing current status.
         */
        Status    = 1 << 1,

        /*
         *  Reconfiguration of a service, that is already deployed.
         *  May also contain some additional parameters, affecting the reconfiguration process.
         */
        Configure = 1 << 2,

        /*
         *  Parameters, affecting backup management.
         */
        Backup    = 1 << 3,
        Restore   = 1 << 4,

        /*
         *  Parameters for pre-deploy and post-deploy diagnostics.
         */
        Diag      = 1 << 5,

        Undeploy  = 1 << 6,
    };
    Q_DECLARE_FLAGS(Contexts, Context)

    inline Parameter(PropertyPtr& base, Contexts contexts, Contexts contexts_required)
        : Property{*base.get()}
        , m_contexts{contexts}
        , m_contexts_required{contexts_required}
        , m_current {m_value->clone()}
        , m_edit    {m_value->clone()}
    {}

    inline Contexts contexts() const { return m_contexts; }
    inline Contexts required() const { return m_contexts_required; }


    inline void fill(QJsonValue& v){ m_current->fill(v); }

    inline Value* currentValue() const { return m_current.get(); }
    inline Value*    editValue() const { return m_edit.get(); }

    inline void fillFromValue(bool current){
        m_edit = (current ? m_current : m_value)->clone();
    }

    /*
     *  Parameter may be used to override a Resource.
     */
    inline void setResource(Resource* resource) { m_resource = resource; }
    inline Resource* resource() const { return m_resource; }


protected:
    const Contexts m_contexts;
    const Contexts m_contexts_required;
    Resource* m_resource{nullptr};

    ValuePtr m_current;
    ValuePtr m_edit;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Parameter::Contexts)
Q_DECLARE_METATYPE(Parameter*)

using ParameterPtr = std::unique_ptr<Parameter>;
