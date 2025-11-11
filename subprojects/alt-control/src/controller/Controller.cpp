#include "Controller.h"

#include "DBusDataSource.h"
#include <QStandardItemModel>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <toml.hpp>

#define CONSTANT(k) static const char* const key_##k = #k
namespace keys
{
    CONSTANT(display_name);
    CONSTANT(comment);
    CONSTANT(states);

    namespace state {
        CONSTANT(type);
        CONSTANT(min);
        CONSTANT(max);
        CONSTANT(pattern);
    }
}
#undef CONSTANT

class Controller::Private {
public:
    inline Private() : m_model{m_facilities} {}

    DBusDataSource m_datasource;
    FacilityModel m_model;
    std::vector<std::unique_ptr<Facility>> m_facilities;
};

Controller::Controller()
    : d { new Private{} }
{}

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

std::optional<Locales> buildBase(const toml::ordered_table& t) noexcept {
    const toml::ordered_table
        * names    = nullptr,
        * comments = nullptr;

    if ( !getTomlValue(t, keys::key_display_name, names, true) ) {
        qCritical() << "skipping invalid object";
        return {};
    }

    LocaleMap display_name_storage = buildLocalizedStrings(*names),
        comment_storage;

    if ( getTomlValue(t, keys::key_comment, comments) )
        comment_storage = buildLocalizedStrings(*comments);


    return {{display_name_storage, comment_storage}};
}

void Controller::refresh(){
    emit beginRefresh();
    emit d->m_model.beginResetModel();

    d->m_facilities.clear();

    // non-sorted
    QStringList facilities = d->m_datasource.getAllInfo();
    d->m_facilities.reserve(facilities.size());

    for ( const QString& facilityInfo : facilities )
    {
        std::istringstream iStream(facilityInfo.toStdString());

        toml::ordered_table data;

        try {data = toml::parse<toml::ordered_type_config>(iStream).as_table();}
        catch (std::exception& e){
            qCritical() << "failed to parse facility"
                        << e.what();
            continue;
        }

        const std::string* facilityName;
        if ( !getTomlValue(data, "name", facilityName) ) {
            qCritical() << "failed to parse facility: name not found";
            continue;
        }


        auto facilityLocales = buildBase(data);

        if ( !facilityLocales ) {
            qWarning() << "failed to build facility" << *facilityName;
            continue;
        }


        std::vector<std::unique_ptr<Facility::State>> states;

        toml::ordered_table states_data;
        if (!getTomlValue(data, keys::key_states, states_data, true))
            qWarning() << "adding empty facility" << *facilityName;

        try {
            for ( const auto& [ stateName, state_data ] : states_data ) {
                auto stateLocales = buildBase(state_data.as_table());

                if ( !stateLocales ) {
                    qWarning() << "failed to build state" << stateName;
                    throw new std::invalid_argument("");
                }

                static const std::map<std::string, Facility::State::Type> typemap {
                    { "state",   Facility::State::Type::State   },
                    { "integer", Facility::State::Type::Integer },
                    { "string",  Facility::State::Type::String  },
                };

                const std::string* typeName;
                if ( !getTomlValue(state_data.as_table(), keys::state::key_type, typeName) ) {
                    qWarning() << "failed to get state type" << stateName;
                    throw new std::invalid_argument{""};
                }
                Facility::State::Type type = typemap.at(*typeName);

                QVariant allowedValues;

                switch (type) {
                    case Facility::State::Type::Integer: {
                        toml::ordered_value::integer_type min = INT_MIN,
                                                          max = INT_MAX;

                        getTomlValue(state_data.as_table(), keys::state::key_min, min);
                        getTomlValue(state_data.as_table(), keys::state::key_max, max);

                        allowedValues = QSize{int(min), int(max)};
                    }
                    break;

                    case Facility::State::Type::String: {
                        std::string pattern;
                        if ( getTomlValue(state_data.as_table(), keys::state::key_pattern, pattern) )
                            allowedValues = QString::fromStdString(pattern);
                    }
                    break;

                    default: break;
                }

                states.push_back(std::make_unique<Facility::State>(QString::fromStdString(stateName), stateLocales.value(), type, allowedValues));
            }
        } catch(...) {
            qWarning() << "invalid facility" << *facilityName << "adding a dummy object";
            states.clear();
        }


        auto facility = std::make_unique<Facility>(QString::fromStdString(*facilityName), facilityLocales.value(), std::move(states));
        facility->setLocale(QLocale::system());

        d->m_facilities.push_back(std::move(facility));
    }

    std::sort(d->m_facilities.begin(), d->m_facilities.end(), [](const auto& a, const auto& b){return a->name() < b->name();});

    emit d->m_model.endResetModel();
    emit endRefresh();
}

Controller::~Controller() { delete d; }

FacilityModel* Controller::facilitiesModel() { return &d->m_model; }

bool Controller::getValue(Facility* facility, QString& error)
{
    emit beginRefresh();
    QString result;
    bool success = d->m_datasource.getValue("/org/altlinux/alterator/"+ QString{facility->name()}.replace('-', '_'), result, error);
    facility->setStateFromValue(result);
    emit endRefresh();
    return success;
}

bool Controller::setValue(Facility* facility, const QString& value, QString& error)
{
    emit beginRefresh();

    bool success = d->m_datasource.setValue("/org/altlinux/alterator/"+ QString{facility->name()}.replace('-', '_'), value, error);

    if ( success )
        success = getValue(facility, error);

    emit endRefresh();
    return success;
}

bool Controller::getStates(Facility* facility, QString& error)
{
    emit beginRefresh();
    QStringList result;
    bool success = d->m_datasource.listStates("/org/altlinux/alterator/"+ QString{facility->name()}.replace('-', '_'), result, error);
    if (success)
        facility->updateAvailableStates(result);

    emit endRefresh();
    return success;
}
