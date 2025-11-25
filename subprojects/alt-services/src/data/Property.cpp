#include "Property.h"
#include "Parameter.h"

#include <QRegularExpression>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QSize>

#include "app/ServicesApp.h"
#include "controller/Controller.h"

Property::Property(Property& other) noexcept
    : TranslatableObject{other}
    , m_constant        { other.m_constant  }
    , m_prototype       { other.m_prototype ? std::move(other.m_prototype) : nullptr }
    , m_required        { other.m_required  }
    , m_type            { other.m_type      }
    , m_allowed_values  { other.m_allowed_values }
    , m_value           { other.m_value->clone() }
    , m_password        { other.m_password }
    , m_array_prefix    { other.m_array_prefix }
{
    m_value->m_property = this;
    for ( auto& child : other.m_children )
        m_children.push_back(std::move(child));
    other.m_children.clear();
}

Property::Property(const QString& name, const Locales &locales, Type type,
                   bool constant, bool required, bool password, PropertyPtr prototype, TranslatableField arrayPrefix) noexcept
    : TranslatableObject{name, locales}
    , m_type           { type     }
    , m_constant       { constant }
    , m_required       { required }
    , m_value          { std::make_unique<Value>(this) }
    , m_prototype      { std::move(prototype) }
    , m_password       { password }
    , m_array_prefix   { arrayPrefix }
{
    if ( m_prototype )
        m_prototype->m_value->setEnabled();
}


// composite prototype or enum
Property::Property(Type type, std::vector<PropertyPtr>&& children) noexcept
    : TranslatableObject{{}, {}}
    , m_type     { type }
    , m_children { std::move(children) }
    , m_value    { std::make_unique<Value>(this) }
{
    for ( auto& property : m_children )
        m_value->addChild(property->defaultValue()->clone());
}

Property::Property(const QString& name, const Locales& locales, std::vector<PropertyPtr>&& children) noexcept
    : TranslatableObject{name, locales}
    , m_children{std::move(children)}
    , m_value { std::make_unique<Value>(this) }
{}

// primitive prototype
Property::Property(Type type, bool password) noexcept
    : TranslatableObject{{}, {}}
    , m_type { type }
    , m_value { std::make_unique<Value>(this) }
    , m_password {password}
{}



void Property::setLocale(const QLocale& locale) const
{
    TranslatableObject::setLocale(locale);

    if ( m_prototype )
        m_prototype->setLocale(locale);

    for ( auto& child : m_children )
        child->setLocale(locale);

    m_array_prefix.setLocale(locale);
}

std::unique_ptr<Property::Value::ValidationInfo> Property::Value::isInvalid(bool force) const
{
    using ValidationInfo = Property::Value::ValidationInfo;

    if ( m_property->isConstant() ) return {};

    if (m_enabled) switch ( m_property->m_type ) {
        case Property::Type::Array: {
            auto size = m_children.size();
            auto [min,max] = m_property->m_allowed_values.toSize();

            if ( size < min )
                return std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("List should have at least %n element", nullptr, min)} };

            if ( size > max )
                return std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("List should not have more than %n element", nullptr, max)}};

            for ( const auto& child : m_children )
                if ( auto childInfo = child->isInvalid(force) )
                    return std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("Some of the items are invalid"), std::move(childInfo)}};

            break;
        }

        case Property::Type::Enum: {
            auto currentIt = std::find_if(m_children.cbegin(), m_children.cend(),
                                          [](const auto& child){return child->isEnabled();} );

            if ( currentIt == m_children.cend() )
                return std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("One of variants should be selected")}};

            if ( auto childInfo = currentIt->get()->isInvalid(force) )
                return std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("Additional data for the selected variant is not valid"), std::move(childInfo)}};

            if ( ++currentIt == m_children.cend() )
                break;

            if ( std::any_of(currentIt, m_children.cend(), [](const auto& child){return child->isEnabled();}) )
                return std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("More than one items are selected. This is an internal error. Please re-select an item.")}};

            break;
        }

        case Property::Type::Composite:
            for ( const auto& child : m_children )
                if ( auto childInfo = child->isInvalid(force) )
                    return std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("Some of the items are invalid"), std::move(childInfo)}};
            break;


        case Property::Type::String: {
            auto string = get().toString();
            auto regexp = m_property->m_allowed_values.toString();

            if ( string.isEmpty() )
                return std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("The string should not be empty")}};

            if ( !regexp.isEmpty() && !QRegularExpression{regexp}.match(string).hasMatch() )
                return std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("Invalid input")}};

            break;
        }

        case Property::Type::Int: {
            auto [min,max] = m_property->m_allowed_values.toSize();
            auto val = get().toInt();

            if ( val < min || val > max )
                std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("The number should be between %1 and %2").arg(min).arg(max)}};

            break;
        }

        default: break;
    }

    if ( Parameter* parameter = dynamic_cast<Parameter*>(property()) ) {
        if ( !force ) {
            if ( Resource* resource = parameter->resource() ) {
                auto it = std::find_if(
                    parameter->service()->resources().cbegin(),
                    parameter->service()->resources().cend(),
                    [resource](const auto& otherResource){
                        return resource->conflicts(otherResource.get());
                    }
                );

                if ( it != parameter->service()->resources().cend() )
                {
                    return std::unique_ptr<ValidationInfo>{new ValidationInfo{this,
                        QObject::tr("Resource '%0' conflicts with '%1'")
                            .arg(resource->displayName())
                            .arg(it->get()->displayName())
                    }};
                }


                Service* otherService{};
                Resource* otherResource{};
                if ( ServicesApp::instance()->controller()->findConflict(parameter->service(), parameter->resource(),
                                                                         &otherService, &otherResource) )
                    return std::unique_ptr<ValidationInfo>{new ValidationInfo{this,
                        QObject::tr("Resource '%0' conflicts with '%1' of an already deployed service '%2'")
                            .arg(resource->displayName())
                            .arg(otherResource->displayName())
                            .arg(otherService->displayName())
                    }};
            }
        }
    }

    return {};
}

void Property::Value::resetEnabledState()
{
    m_enabled = m_property->isRequired() || m_property->isConstant();
    for ( const auto& child : m_children )
        child->resetEnabledState();
}

QString Property::Value::displayName(bool noDefaultPrefix) const {
    if ( m_parent && m_parent->property()->valueType() == Property::Type::Array ) {
        auto prefix = m_parent->property()->arrayPrefix();
        auto num = QString::number(parent()->indexOf(this)+1);

        if ( prefix.isEmpty() && !noDefaultPrefix )
            prefix = QObject::tr("Item");

        if ( !prefix.isEmpty() )
            num.prepend( prefix + ' ' );
        return num;
    }

    return m_property->displayName();
}

QJsonValue Property::Value::serialize(bool excludePasswords) const
{
    switch ( m_property->m_type ) {
        case Type::Enum:
        case Type::Composite: {
            QJsonObject object;
            for ( auto& child : m_children )
                if ( child->isEnabled() )
                    object[child->m_property->name()] = child->serialize(excludePasswords);
            return object;
        }

        case Type::Array: {
            QJsonArray array;
            for ( auto& child : m_children )
                array.append(child->serialize(excludePasswords));
            return array;
        }

        default:
            return m_property->isPassword() && excludePasswords ? "" : m_value.toJsonValue();
    }
}

bool Property::Value::fill(const QJsonValue& value, bool required)
{
    bool success = false;

    if ( value.isUndefined() || value.isNull() ) {
        setEnabled(required);
    }
    else
    {
        switch ( m_property->m_type ) {
            case Type::Enum:
            {
                QJsonObject object = value.toObject();
                QString key;
                QJsonValue value;

                if ( !object.empty() ) {
                    key   = object.begin().key();
                    value = object.begin().value();
                }

                for ( const auto& child : m_children )
                {
                    if ( child->property()->name() == key ) {
                        child->setEnabled(true);
                        success = child->fill(value, true);
                    }
                    else
                        child->setEnabled(false);
                }
            } break;

            case Type::Composite: {
                success = std::accumulate(m_children.cbegin(), m_children.cend(), true,
                    [object = value.toObject()]
                    (bool res, const auto& child){
                        return res & child->fill(object[child->m_property->name()], child->property()->isRequired());
                    }
                );
            } break;

            case Type::Array: {
                QJsonArray array = value.toArray();
                m_children.clear();
                if ( m_property->m_prototype )
                {
                    success = std::accumulate(array.cbegin(), array.cend(), true,
                        [this](bool res, const auto& value){
                            auto child = m_property->m_prototype->defaultValue()->clone();
                            bool result = res & child->fill(value, true);
                            addChild(std::move(child));
                            return result;
                        }
                    );
                }
            } break;

            default:
                set(value.toVariant());
                success = true;
        }
        m_enabled = true;
    }

    return success;
}

ValuePtr Property::Value::clone() noexcept
{
    auto copy = std::make_unique<Property::Value>(m_property);
    copy->m_parent = m_parent;
    copy->m_enabled = m_enabled;
    switch ( m_property->m_type ) {
        case Type::Array:
        case Type::Composite:
        case Type::Enum:
            for ( auto& child : m_children )
                copy->addChild(child->clone());
            break;

        default:
            copy->m_value = m_value;
            break;
    }
    return std::move(copy);
}
void Property::Value::addChild(ValuePtr &&child) {
    child->m_parent = this;
    m_children.push_back(std::move(child));
}
