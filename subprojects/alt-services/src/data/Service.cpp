#include "Service.h"

#include <QAbstractTableModel>
#include <QJsonObject>
#include <QJsonDocument>
#include <QIcon>

class Service::Private {
public:
    Private(PtrVector<Parameter>&& parameters,
            PtrVector<Resource>&& resources,
            PtrVector<DiagTool>&& tools,
            const QString& iconName)
        : m_parameters{std::move(parameters)}
        , m_resources{std::move(resources)}
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

    void fill(QJsonObject o, Parameter::Context ctx){
        for ( auto& param : m_parameters ) {
            auto obj = o[param->name()];
            if ( !obj.isUndefined() && !obj.isNull() )
                param->value(Parameter::ValueScope::Current)->fill(obj, param->required().testFlag(ctx));
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

            it->get()->value(Parameter::ValueScope::Edit)->fill(o.value(name), ctx);
        }
        return true;
    }

    const QIcon m_icon;
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
{
    for ( auto& parameter : d->m_parameters )
        parameter->m_service = this;

    for ( auto& resource : d->m_resources )
        resource->m_service = this;
}

Service::~Service() { delete d; }

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

const PtrVector<Resource>& Service::resources()
{
    return d->m_resources;
}

const PtrVector<DiagTool>& Service::diagTools()
{
    return d->m_diag_tools;
}


QJsonObject Service::getParameters(Parameter::Contexts ctx, bool excludePasswords)
{
    QJsonObject parameters;

    for ( auto& parameter : d->m_parameters ) {
        if ( ! (parameter->contexts() & ctx) || ( !(parameter->required() & ctx) & !parameter->value(Parameter::ValueScope::Edit)->isEnabled() ) )
            continue;

        parameters[parameter->name()] = parameter->value(parameter->isConstant() ? Parameter::ValueScope::Default : Parameter::ValueScope::Edit)->serialize(excludePasswords);
    }

    return parameters;
}


void Service::setStatus(const QByteArray& data)
{
    for ( const auto& param : d->m_parameters )
        if ( param->contexts().testFlag(Parameter::Context::Status) )
            param->value(Parameter::ValueScope::Current)->resetEnabledState();

    auto parameters = QJsonDocument::fromJson(data);
    if ( parameters.isNull() )
        return; // TODO: error message

    m_deployed = parameters["deployed"].toBool(false);
    m_started = m_deployed && parameters["started"].toBool(false);

    if ( m_deployed ) {
        auto current = parameters.object();
        if ( ! current.empty() )
            d->fill(current, Parameter::Context::Status);
    }
}

bool Service::tryFill(QJsonObject o, Parameter::Contexts ctx) { return d->tryFill(o, ctx); }

bool Service::hasPreDiag() const {
    using namespace std::placeholders;
    return !forceDeploy() && !isDeployed() &&
           std::any_of( d->m_diag_tools.cbegin(), d->m_diag_tools.cend(),
                        std::bind(&DiagTool::hasTests, _1, DiagTool::Test::Mode::PreDeploy) );
}

bool Service::hasPostDiag() const {
    using namespace std::placeholders;
    return std::any_of(d->m_diag_tools.cbegin(), d->m_diag_tools.cend(),
                       std::bind(&DiagTool::hasTests, _1, DiagTool::Test::Mode::PostDeploy));
}

bool Service::hasConflict(Service* toDeploy, Resource* theirs, Resource** ours)
{
    if ( m_deployed && this != toDeploy )
        for ( auto& our_resource : d->m_resources ) {
            if ( our_resource.get() == theirs ) continue;
            if ( our_resource->conflicts(theirs) ){
                *ours   = our_resource.get();
                return true;
            }
        }

    return false;
}

