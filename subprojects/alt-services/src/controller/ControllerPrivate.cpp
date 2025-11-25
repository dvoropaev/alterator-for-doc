#include "controller/ControllerPrivate.h"
#include <QSize>
#include <toml.hpp>
#include <QDebug>
#include <QApplication>
#include <QMessageBox>

#define CONST(k) static const char* const key_##k = #k

namespace keys
{
    CONST(display_name);
    CONST(comment);
    CONST(type);
    CONST(properties);
    CONST(enable_force_deploy);
    CONST(diag_tools);

    namespace diag {
        CONST(bus);
        CONST(path);
    }

    namespace param
    {
        CONST(required);
        CONST(constant);
        CONST(context);
        CONST(values);
        CONST(default);
        CONST(prototype);
        CONST(password);

        CONST(min);
        CONST(max);
        CONST(pattern);

        CONST(array_type);
        CONST(array_min);
        CONST(array_max);
        CONST(array_label);
    }

    namespace resource
    {
        CONST(path);
        CONST(unit_name);
        CONST(inet_service);
        CONST(value);
        CONST(parameter);
        CONST(tcp);
        CONST(udp);
    }
}


namespace {
    toml::ordered_table serviceData;
    std::map<std::string, Parameter*> parametersIndex;
};

template<typename V>
inline bool getTomlValue(const toml::ordered_table& table, const char* key, V& value, bool warnMissing = false, bool warnInvalid = true) {
    using Type = std::decay_t<std::remove_pointer_t<V>>;

    auto it = table.find(key);

    if ( it == table.cend() ) {
        if ( warnMissing )
            qWarning() << key << "not found";
        return false;
    }

    const toml::ordered_value& val = it->second;

    if ( !val.is<Type>() ) {
        if ( warnInvalid )
            qWarning() << key << "has incorrect type";
        return false;
    }

    const Type& targetTypeVal = val.as<toml::detail::type_to_enum<Type, toml::ordered_value>::value>();

    if constexpr ( std::is_pointer<V>() )
        value = &targetTypeVal;
    else
        value = targetTypeVal;
    return true;
}

LocaleMap buildLocalizedStrings(const toml::ordered_table& t) noexcept {
    LocaleMap m;
    for ( auto& [locale, string] : t )
        if ( string.is_string() )
            m[QString::fromStdString(locale)]
                    = QString::fromStdString(string.as_string());
    return m;
}

std::optional<TranslatableField> buildTranslatable(const toml::ordered_table& t, const char* name) {
    const toml::ordered_table* locales = nullptr;
    if ( !getTomlValue(t, name, locales, false) )
        return {};

    return {buildLocalizedStrings(*locales)};
}

std::optional<Locales> buildBase(const toml::ordered_table& t) noexcept {
    auto displayName = buildTranslatable(t, keys::key_display_name);
    if ( !displayName ) {
        qWarning() << "invalid translatable object";
        return {};
    }

    auto comment = buildTranslatable(t, keys::key_comment);

    return {{ displayName.value(), comment.value_or(TranslatableField{}) }};
}

bool buildContexts(Parameter::Contexts& contexts, const toml::ordered_array& contexts_data) noexcept {
    for ( auto ctx : contexts_data ) {
        static const std::map<std::string, Parameter::Context> ctxmap {
            { "configure", Parameter::Context::Configure },
            { "deploy"   , Parameter::Context::Deploy    },
            { "undeploy" , Parameter::Context::Undeploy  },
            { "status"   , Parameter::Context::Status    },
            { "backup"   , Parameter::Context::Backup    },
            { "restore"  , Parameter::Context::Restore   },
            { "diag"     , Parameter::Context::Diag      },
        };
        if ( ctx.is_string() ) try {
            contexts.setFlag( ctxmap.at(ctx.as_string()) );
            continue;
        } catch (std::out_of_range&){}
        qWarning() << "skipping unknown context" << ctx.as_string();
    }

    return true;
}

Parameter* findOverride(const QString& paramName, const QVariant& defaultValue) noexcept {
    if ( paramName.isEmpty() ) return nullptr;

    if ( auto override_it = parametersIndex.find(paramName.toStdString());
         override_it != parametersIndex.cend() )
    {
        auto& override = override_it->second;
        if (override->defaultValue()->get().isNull())
            override->defaultValue()->set(defaultValue);
            return override;
    }

    return nullptr;
}

QVariant parseDefault(const toml::value& v, Property::Type type) {
    try {
        switch (type) {
            case Property::Type::Int:
                return QVariant::fromValue(v.as_integer());

            case Property::Type::Bool:
                return QVariant::fromValue(v.as_boolean());

            case Property::Type::Enum:
            case Property::Type::String:
                return QString::fromStdString(v.as_string());

            default: return {};
        }
    } catch (...) {
        qWarning() << "invalid 'default' value";
        return {};
    }
}

void buildDefault(const toml::value& data, Property::Value* value){
    switch ( value->property()->valueType() )
    {
        case Property::Type::Array: {
            if ( value->property()->prototype()->valueType() == Property::Type::Composite )
                return;

            if ( data.is_array() ) for ( const auto& val : data.as_array() ) {
                auto child = value->property()->prototype()->defaultValue()->clone();
                buildDefault(val, child.get());
                value->property()->defaultValue()->addChild(std::move(child));
            } else {
                qWarning() << "invalid 'default' array value for"
                           << value->property()->name();
            }
        } break;

        case Property::Type::Composite: break;

        case Property::Type::Enum: {
            if ( data.is_string() ) {
                for ( auto& child : value->children() )
                    child->setEnabled( child->property()->name() == data.as_string() );
            } else {
                qWarning() << "invalid 'default' enum value for"
                           << value->property()->name()
                           << "falling back to first one";
                for ( auto& child : value->children() )
                    child->setEnabled(false);
                value->children().front()->setEnabled(true);
            }
        }

        default:
            value->set(parseDefault(data, value->property()->valueType()));
    }
}

bool buildAllowed(QString name, const toml::ordered_table& data, Property* property) {
    switch ( property->valueType() ) {
        case Parameter::Type::String: {
            const std::string* pattern = nullptr;
            if ( getTomlValue(data, keys::param::key_pattern, pattern) )
                property->setAllowed(QString::fromStdString(*pattern));
        } break;

        case Parameter::Type::Int: {
            toml::ordered_value::integer_type min = INT_MIN,
                                              max = INT_MAX;

            getTomlValue(data, keys::param::key_min, min);
            getTomlValue(data, keys::param::key_max, max);

            property->setAllowed(QSize{int(min), int(max)});
        } break;

        case Property::Type::Array: {
            toml::ordered_value::integer_type min = 0,
                                              max = INT_MAX;

            getTomlValue(data, keys::param::key_array_min, min);
            getTomlValue(data, keys::param::key_array_max, max);

            property->setAllowed(QSize{int(min), int(max)});
        } break;

        default: break;
    }

    return true;
}


PropertyPtr buildProperty(const QString& name, const toml::ordered_table& data, Property::Type parentType = Property::Type::Composite) noexcept;

ParameterPtr buildParameter(const QString& name, const toml::ordered_table& data) noexcept;
PropertyPtr getPrototype(Property::Type type, const toml::ordered_table& parameterData, QString name, Property::Type parentType = Property::Type::Composite){
    const std::string* prototypeName = nullptr;

    if ( type == Property::Type::Composite || type == Property::Type::Enum )
    {
        if ( type == Property::Type::Composite && getTomlValue(parameterData, keys::param::key_prototype, prototypeName) ) {
            try { return std::make_unique<Property>(*parametersIndex.at(*prototypeName)); }
            catch (std::out_of_range&) {

                const toml::ordered_table* paramsData;
                if ( getTomlValue( serviceData, "parameters", paramsData, true) )
                {
                    const toml::ordered_table* prototypeData = nullptr;
                    if ( getTomlValue(*paramsData, prototypeName->c_str(), prototypeData ) )
                    {
                        if ( auto param = buildParameter(QString::fromStdString(*prototypeName), *prototypeData ) )
                        {
                            return std::make_unique<Property>(*param.get());
                        }
                    }
                }
            }
            qWarning() << "prototype" << *prototypeName << "not found for property" << name;
            return {};
        } else {

            const toml::ordered_table* properties = nullptr;
            PtrVector<Property> children;

            bool isEnum = type == Property::Type::Enum;
            bool isEnumSubvalue = parentType == Property::Type::Enum;

            if ( getTomlValue(parameterData, isEnum ? keys::param::key_values : keys::key_properties, properties, !isEnum && !isEnumSubvalue, !isEnum && !isEnumSubvalue) ) {

                for ( auto& [childName, childData] : *properties )
                    if ( auto child = buildProperty(QString::fromStdString(childName), childData.as_table(), type) )
                        children.push_back( std::move(child) );
                    else
                        return {};
            }

            if ( !children.empty() || isEnum || isEnumSubvalue )
                return std::make_unique<Property>(type, std::move(children));

            qWarning() << "failed to build properties";
            return {};
        }
    }

    auto res = std::make_unique<Property>(type);

    if ( !buildAllowed(name, parameterData, res.get()) )
        return {};
    return std::move(res);
}

PropertyPtr buildProperty(const QString& name, const toml::ordered_table& propData, Property::Type parentType) noexcept {
    auto locales = buildBase(propData);
    if ( !locales ) return {};

    const std::string* type;
    Property::Type valType;

    static const std::map<std::string, Property::Type> types {
        { "string" , Property::Type::String    },
        { "enum"   , Property::Type::Enum      },
        { "integer", Property::Type::Int       },
        { "boolean", Property::Type::Bool      },
        { "object" , Property::Type::Composite },
        { "array"  , Property::Type::Array     }
    };

    if ( getTomlValue(propData, keys::key_type, type) )
    {
        try { valType = types.at( *type ); }
        catch (std::out_of_range&) {
            qWarning() << "unknown type" << *type;
            return {};
        }

    } else {
        if ( parentType == Property::Type::Enum )
            valType = Property::Type::Composite;
        else {
            qWarning() << "failed to get type for" << name;
            return {};
        }
    }

    PropertyPtr prototypePtr = nullptr;
    TranslatableField array_prefix;

    switch ( valType )
    {
        case Property::Type::Array: {
            Property::Type arrayItemType;

            if ( !getTomlValue(propData, keys::param::key_array_type, type, true) )
                return {};

            try { arrayItemType = types.at( *type ); }
            catch (std::out_of_range&) {
                qWarning() << "unknown array type" << *type;
                return {};
            }

            if ( !(prototypePtr = getPrototype(arrayItemType, propData, name, parentType)) )
                return {};

            array_prefix = buildTranslatable(propData, keys::param::key_array_label).value_or(TranslatableField{});

            break;
        }

        case Property::Type::Composite:
        case Property::Type::Enum:
            if ( !(prototypePtr = getPrototype(valType, propData, name, parentType)) )
                return {};
            break;

        default: break;
    }

    bool constant = false;
    getTomlValue(propData, keys::param::key_constant, constant);

    bool required = false;
    getTomlValue(propData, keys::param::key_required, required, false, false);

    bool password = false;
    if ( valType == Property::Type::String )
        getTomlValue(propData, keys::param::key_password, password);


    PropertyPtr result;

    auto prototype = prototypePtr.get();
    result = std::make_unique<Property>(name, locales.value(),
                                        valType, constant,
                                        required, password, std::move(prototypePtr), array_prefix);

    if ( !buildAllowed(name, propData, result.get()) )
        return {};

    if ( valType == Property::Type::Composite || valType == Property::Type::Enum ) {
        for ( auto& child : prototype->defaultValue()->children() )
            result->defaultValue()->addChild(child->clone());
        if (valType == Property::Type::Enum)
            result->defaultValue()->children().at(0)->setEnabled(true);
    }

    try {
        buildDefault(propData.at(keys::param::key_default), result->defaultValue());
    } catch (...){}

    return std::move(result);
}

ParameterPtr buildParameter(const QString& name, const toml::ordered_table& data) noexcept {
    Parameter::Contexts contexts;
    Parameter::Contexts required;

    const toml::ordered_value::array_type* ctx = nullptr;
    if ( !getTomlValue(data, keys::param::key_context, ctx, true) ||
         !buildContexts(contexts, *ctx) )
    {
        qWarning() << "failed to build contexts for" << name;
        return {};
    }

    const toml::ordered_value::array_type* required_ctx = nullptr;
    if ( getTomlValue(data, keys::param::key_required, required_ctx) )
        buildContexts(required, *required_ctx);

    if ( auto base = buildProperty(name, data) )
        return std::make_unique<Parameter>(base, contexts, required);

    qWarning() << "failed to build parameter" << name;
    return {};
}

ResourcePtr buildResource(const QString& name, const toml::ordered_table& res_data) noexcept {
    auto locales = buildBase(res_data);
    if ( !locales )
        return {};

    const std::string* resource_type = nullptr;
    if ( !getTomlValue(res_data, keys::key_type, resource_type) )
    {
        qWarning() << "failed to get resource type" << name;
        return {};
    }

    static const std::map<std::string, std::tuple<Resource::Type, const char*, Property::Type>> keymap {
        { "file",         { Resource::Type::File, keys::resource::key_path        , Property::Type::String } },
        { "directory",    { Resource::Type::Path, keys::resource::key_path        , Property::Type::String } },
        { "systemd_unit", { Resource::Type::Unit, keys::resource::key_unit_name   , Property::Type::String } },
        { "port",         { Resource::Type::Port, keys::resource::key_inet_service, Property::Type::Int    } },
    };

    auto it = keymap.find(*resource_type);
    if ( it == keymap.cend() ) {
        qWarning() << "invalid resource type" << *resource_type;
        return {};
    }

    const auto& [type,key,valType] = it->second;
    Parameter* overridingParameter = nullptr;

    const toml::ordered_table* prop = nullptr;
    if ( !getTomlValue(res_data, key, prop, true) ) {
        qWarning() << "invalid resource" << name;
        return {};
    }


    auto value_it = prop->find(keys::resource::key_value);
    if ( value_it == prop->cend() ) {
        qWarning() << "resource property" << key << "does not have a default value";
        return {};
    }

    auto val = parseDefault(value_it->second, valType);

    const std::string* paramName = nullptr;
    if ( getTomlValue(*prop, keys::resource::key_parameter, paramName) ) {

        if ( (overridingParameter = findOverride(QString::fromStdString(*paramName), val)) )
        {
            auto allowed = overridingParameter->allowed();

            switch ( type ) {
                case Resource::Type::Port: {
                    auto [min,max] = allowed.toSize();
                    if ( min < 0     ) min = 0;
                    if ( max > 65535 ) max = 65535;
                    overridingParameter->setAllowed(QSize{min,max});
                } break;

                case Resource::Type::Path: {
                    if ( allowed.toString().isEmpty() )
                    overridingParameter->setAllowed("^\\/(.+)$");
                } break;

                case Resource::Type::File : {
                    if ( allowed.toString().isEmpty() )
                    overridingParameter->setAllowed("^\\/(.+)\\/([^\\/]+)$");
                } break;

                case Resource::Type::Unit: {
                    if ( allowed.toString().isEmpty() )
                    overridingParameter->setAllowed("^[^\\/]+\\.(service|socket|timer))$");
                } break;
            }
        }
    }

    std::unique_ptr<Resource> result;

    if ( type == Resource::Type::Port ) {
        bool tcp = false,
             udp = false;

        getTomlValue(res_data, keys::resource::key_tcp, tcp);
        getTomlValue(res_data, keys::resource::key_udp, udp);

        result = std::make_unique<Resource>(name, locales.value(),
                                          tcp,udp, val, overridingParameter);
    }
    else
        result = std::make_unique<Resource>(name, locales.value(), type, val, overridingParameter);

    if ( overridingParameter )
        overridingParameter->setResource(result.get());

    return std::move(result);
}

using TestFlags = std::map<QString, std::pair<DiagTool::Test::Modes, DiagTool::Test::Modes>>;
std::unique_ptr<DiagTool> buildDiagTool(const QString& path, bool session, const QString& data, const TestFlags& testModes) {
    std::istringstream iStream(data.toStdString());
    toml::ordered_table table;
    try {table = toml::parse<toml::ordered_type_config>(iStream).as_table();}
    catch (std::exception& e){
        qCritical() << "failed to parse diagnostic tool" << path << ":"
                    << e.what();
        return {};
    }

    const std::string* diagName = nullptr;
    auto locales = buildBase(table);

    if ( !getTomlValue(table, "name", diagName) || !locales ) {
        qWarning() << "skipping invalid diag";
        return {};
    }

    PtrVector<DiagTool::Test> tests;

    const toml::ordered_table* tests_data = nullptr;
    if ( !getTomlValue(table, "tests", tests_data) ) {
        qWarning() << "failed to build tests for" << diagName;
        return {};
    }

    for ( auto& [testName,modes] : testModes ) {
        const toml::ordered_table* testData = nullptr;
        if ( getTomlValue(*tests_data, testName.toStdString().c_str(), testData) ) {
            if ( auto testLocales = buildBase(*testData) ) {

                QString iconName;
                const std::string* icon = nullptr;
                if ( getTomlValue(*testData, "icon", icon) )
                    iconName = QString::fromStdString(*icon);
                else
                    iconName = "preferences-other";

                tests.push_back(std::make_unique<DiagTool::Test>(testName, testLocales.value(), iconName, modes.first, modes.second));
                continue;
            }
        }
        qWarning() << "invalid diag test" << testName;
    }

    PtrVector<DiagTool::Parameter> diagParameters;

    const toml::ordered_table* params_data = nullptr;
    if ( getTomlValue(table, "parameters", params_data) ) {
        for ( auto& [paramName, paramData] : *params_data ) {
            if ( paramData.is_table() ) {
                static const std::map<std::string, Property::Type> typemap {
                    {"string", Property::Type::String},
                    {"enum",   Property::Type::Enum},
                    {"int",    Property::Type::Int}
                };

                if ( auto paramLocales = buildBase(paramData.as_table()) ) {
                    const std::string* paramType = nullptr;
                    if ( !getTomlValue(paramData.as_table(), keys::key_type, paramType) ) {
                        qWarning() << "failed to get parameter type for" << paramName;
                        return {};
                    }

                    try {
                        diagParameters.push_back( std::make_unique<DiagTool::Parameter>(
                            QString::fromStdString(paramName.data()),
                            paramLocales.value(),
                            typemap.at(*paramType)
                        ));
                        continue;
                    } catch (std::out_of_range&) {
                        qWarning() << "unknown diag parameter type";
                    }
                }
            }
            qWarning() << "failed to build diag parameter" << paramName;
        }
    } else {
        qWarning() << "failed to build parameters for" << *diagName;
    }

    QString iconName;
    const std::string* icon = nullptr;
    if ( getTomlValue(table, "icon", icon) )
        iconName = QString::fromStdString(*icon);
    else
        iconName = "system-run";

    return std::make_unique<DiagTool>(QString::fromStdString(*diagName), locales.value(),
                                      path, iconName, session, std::move(tests), std::move(diagParameters));
}

Controller::Private::Private(Controller* self)
    : m_table_mode_group{self}
{}

std::unique_ptr<Service> Controller::Private::buildService(const QString& path, const QString& data) noexcept
{
    parametersIndex.clear();

    bool diagMissing = false;

    std::istringstream iStream(data.toStdString());
    try {serviceData = toml::parse<toml::ordered_type_config>(iStream).as_table();}
    catch (std::exception& e){
        qCritical() << "failed to parse service" << path << ":"
                    << e.what();
        return {};
    }

    if ( serviceData.empty() ) {
        qWarning() << "failed to parse service";
        return {};
    }

    const std::string* type = nullptr;
    if ( !getTomlValue(serviceData, keys::key_type, type) || "Service" != *type ) {
        qWarning() << "skipping non-service entry";
        return {};
    }

    const std::string* serviceName = nullptr;
    auto locales = buildBase(serviceData);
    if ( !getTomlValue(serviceData, "name", serviceName) || !locales ) {
        qWarning() << "skipping invalid service";
        return {};
    }

    PtrVector<Parameter> serviceParameners;
    if ( serviceData.contains("parameters") ) {
        const toml::ordered_table* parameters_data = nullptr;
        if ( !getTomlValue(serviceData, "parameters", parameters_data) )
            return {};

        for ( auto& [paramName, data] : *parameters_data ) {
            if ( auto param = buildParameter(QString::fromStdString(paramName), data.as_table()) ) {
                parametersIndex[paramName] = param.get();
                serviceParameners.push_back(std::move(param));
                continue;
            }
            return {};
        }
    }

    PtrVector<Resource> resources;
    if ( serviceData.contains("resources") ) {
        const toml::ordered_table* resources_data = nullptr;
        if ( !getTomlValue(serviceData, "resources", resources_data) )
            qWarning() << "failed to build resources for" << *serviceName;

        for ( auto& [resourceName, data] : *resources_data ) {
            if ( data.is_table() ) {
                if ( auto resource = buildResource(QString::fromStdString(resourceName), data.as_table()) ) {
                    resources.push_back(std::move(resource));
                    continue;
                }
            }
            return {};
        }
    }


    PtrVector<DiagTool> tools;

    if ( serviceData.contains(keys::key_diag_tools) ) {
        const toml::ordered_table* diag_data = nullptr;
        if ( !getTomlValue(serviceData, keys::key_diag_tools, diag_data) ) {
            qWarning() << "invalid diag data";
            diagMissing = true;
        } else for ( auto& [toolName, data] : *diag_data ) {
            if ( !data.is_table() ) {
                qWarning() << "failed to build diag tool" << toolName;
                diagMissing = true;
                continue;
            }

            const auto& tool_data = data.as_table();

            const std::string
                *diagBus  = nullptr,
                *diagPath = nullptr;

            if ( !getTomlValue(tool_data, keys::diag::key_bus, diagBus, true) ) {
                qWarning() << "failed to get diag object bus of" << toolName;
                diagMissing = true;
                continue;
            }

            if (!getTomlValue(tool_data, keys::diag::key_path, diagPath, true) )
            {
                qWarning() << "failed to get diag object path of" << toolName;
                diagMissing = true;
                continue;
            }

            bool isSession = *diagBus == "session";

            std::map<QString, std::pair<DiagTool::Test::Modes, DiagTool::Test::Modes>> testModes;

            QStringList  preDeploy;
            QStringList  preDeployRequired;
            QStringList postDeploy;
            QStringList postDeployRequired;

            if ( m_datasource.getDiagToolTests(QString::fromStdString(*diagPath), path, "pre",  false,  preDeploy, isSession) &&
                 m_datasource.getDiagToolTests(QString::fromStdString(*diagPath), path, "post", false, postDeploy, isSession) &&
                 m_datasource.getDiagToolTests(QString::fromStdString(*diagPath), path, "pre",  true,   preDeployRequired, isSession) &&
                 m_datasource.getDiagToolTests(QString::fromStdString(*diagPath), path, "post", true,  postDeployRequired, isSession)
                )
            {
#define FILLFLAGS(container, mode, member) \
    while ( !container.empty() ) { \
        auto& pair = testModes[container.front()]; \
        pair.member.setFlag(DiagTool::Test::Mode:: mode, true); \
        container.pop_front(); }

#define FILL_MODE( source, mode ) FILLFLAGS( source, mode, first )
#define FILL_REQUIRED( source, mode ) FILLFLAGS( source, mode, second )

                FILL_MODE     (  preDeploy,          PreDeploy );
                FILL_MODE     ( postDeploy,         PostDeploy );
                FILL_REQUIRED (  preDeployRequired,  PreDeploy );
                FILL_REQUIRED ( postDeployRequired, PostDeploy );
#undef FILL_MODE
#undef FILL_REQUIRED
#undef FILLFLAG

                if ( auto tool = buildDiagTool(QString::fromStdString(*diagPath), isSession,
                                                    m_datasource.getDiagInfo(QString::fromStdString(*diagPath), isSession),
                                                    testModes) )
                {
                    tools.push_back(std::move(tool));
                    continue;
                }
            }
            else
                qWarning() << "failed to get test list";


            qCritical() << "invalid diag tool" << toolName;
            diagMissing = true;
        }
    }

    bool forceDeploy = false;
    getTomlValue(serviceData, keys::key_enable_force_deploy, forceDeploy);

    QString iconName;
    const std::string* icon = nullptr;
    if ( getTomlValue(serviceData, "icon", icon) )
        iconName = QString::fromStdString(*icon);
    else
        iconName = "network-server";

    return std::make_unique<Service>(QString::fromStdString(*serviceName), locales.value(),
                                     path, iconName, forceDeploy,
                                     std::move(serviceParameners), std::move(resources), std::move(tools), diagMissing);
}
