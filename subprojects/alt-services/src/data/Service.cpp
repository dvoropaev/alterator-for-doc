#include "Service.h"

#include <QAbstractTableModel>
#include <QJsonObject>
#include <QJsonDocument>
#include <QIcon>

#include "models/ParameterStatusModel.h"
#include "models/ResourceModel.h"

class Service::Private {
public:
    Private(PtrVector<Parameter>&& parameters,
            PtrVector<Resource>&& resources,
            PtrVector<DiagTool>&& tools,
            const QString& iconName)
        : m_parameters{std::move(parameters)}
        , m_parameter_model{parameters}
        , m_resources{std::move(resources)}
        , m_resource_model{resources}
        , m_diag_tools{std::move(tools)}
        , m_icon{ QIcon::fromTheme(iconName) }
    {
        for ( auto& tool : m_diag_tools ) {
            tool->m_params_missing = !std::all_of( m_parameters.cbegin(), m_parameters.cend(), [&](const auto& serviceParam){
                if ( !serviceParam->contexts().testFlag(Parameter::Context::Diag) )
                    return true;

                auto it = std::find_if( tool->parameters().cbegin(), tool->parameters().cend(), [&](const auto& toolParameter){
                    return serviceParam->name() == toolParameter->name();
                });

                bool res = it != tool->parameters().cend();
                if ( !res )
                    qWarning() << serviceParam->name() << "had 'diag' context, but is not used by" << tool->name();

                return res;
            });
        }

    }

    void fill(QJsonObject o){
        for ( auto& param : m_parameters ) {
            auto obj = o[param->name()];
            if ( !obj.isUndefined() )
                param->currentValue()->fill(obj);
        }
    }

    bool tryFill(QJsonObject o, Parameter::Contexts ctx){
        for ( const auto& name : o.keys() ) {
            auto it = std::find_if(m_parameters.cbegin(), m_parameters.cend(), [=](const ParameterPtr& parameter){
                return  parameter->contexts() & ctx &&
                        parameter->name() == name;
            });

            if ( it == m_parameters.cend() )
                return false;

            it->get()->editValue()->fill(o.value(name));
        }
        return true;
    }

    const QIcon m_icon;
    ParameterStatusModel m_parameter_model;
    ResourceModel        m_resource_model;
    PtrVector<Property>  m_prototypes;
    PtrVector<Parameter> m_parameters;
    PtrVector<Resource>  m_resources;
    PtrVector<DiagTool>  m_diag_tools;
};


Service::Service(const QString& name,
                 const Locales& locales,
                 const QString& path,
                 const QString& iconName,
                 bool force_deployable,
                 PtrVector<Parameter>&& parameters,
                 PtrVector<Resource>&& resources,
                 PtrVector<DiagTool>&& tools, bool diagMissing)
    : TranslatableObject{name, locales}
    , d{ new Private{std::move(parameters), std::move(resources), std::move(tools), iconName} }
    , m_dbusPath{path}
    , m_force_deployable{force_deployable}
    , m_diagNotFound{diagMissing}
{}

Service::~Service() { delete d; }

ParameterModel* Service::parameterModel() { return &d->m_parameter_model; }
ResourceModel* Service::resourceModel()  { return &d->m_resource_model; }

void Service::setLocale(const QLocale& locale) const {
    TranslatableObject::setLocale(locale);

    for ( auto& obj : d->m_prototypes ) obj->setLocale(locale);
    for ( auto& obj : d->m_parameters ) obj->setLocale(locale);
    for ( auto& obj : d->m_resources  ) obj->setLocale(locale);
    for ( auto& obj : d->m_diag_tools ) obj->setLocale(locale);
}

const QIcon& Service::icon()
{
    return d->m_icon;
}

const PtrVector<Parameter>& Service::parameters()
{
    return d->m_parameters;
}

const PtrVector<DiagTool>& Service::diagTools()
{
    return d->m_diag_tools;
}


void Service::showDefault(bool how)
{
    d->m_parameter_model.showDefault(how);
}



QJsonObject Service::getParameters(Parameter::Contexts ctx, bool excludePasswords)
{
    QJsonObject parameters;

    for ( auto& parameter : d->m_parameters ) {
        if ( ! (parameter->contexts() & ctx) || ( !(parameter->required() & ctx) & !parameter->editValue()->isEnabled() ) )
            continue;

        parameters[parameter->name()] = ( parameter->isConstant()
                                          ? parameter->defaultValue()
                                          : parameter->editValue() )->serialize(excludePasswords);
    }

    return parameters;
}


void Service::setStatus(int code, const QByteArray& data)
{
    for ( const auto& param : d->m_parameters )
        param->currentValue()->resetEnabledState();

    auto parameters = QJsonDocument::fromJson(data);
    if ( parameters.isNull() )
        return; // TODO: error message

    m_deployed = parameters["deployed"].toBool(false);
    m_started = m_deployed && parameters["started"].toBool(false);

    if ( m_deployed ) {
        auto current = parameters.object();
        if ( ! current.empty() )
            d->fill(current);
    }
}

bool Service::tryFill(QJsonObject o, Parameter::Contexts ctx) { return d->tryFill(o, ctx); }

bool Service::hasConflict(Service* toDeploy, Resource** ours, Resource** theirs)
{
    if ( m_deployed || this == toDeploy )
        for ( auto& our_resource : d->m_resources )
            for ( auto& their_resource : toDeploy->d->m_resources ) {
                if ( our_resource == their_resource ) continue;
                if ( our_resource->conflicts(their_resource.get()) ){
                    *ours   = our_resource.get();
                    *theirs = their_resource.get();
                    return true;
                }
            }

    return false;
}

