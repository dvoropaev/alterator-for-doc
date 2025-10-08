#include "Property.h"

#include <QRegularExpression>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QSize>

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

std::unique_ptr<Property::Value::ValidationInfo> Property::Value::isInvalid() const
{
    using ValidationInfo = Property::Value::ValidationInfo;

    if ( m_property->isConstant() || !m_enabled ) return {};

    switch ( m_property->m_type ) {
        case Property::Type::Array: {
            auto size = m_children.size();
            auto [min,max] = m_property->m_allowed_values.toSize();

            if ( size < min )
                return std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("List should have at least %n element", nullptr, min)} };

            if ( size > max )
                return std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("List should not have more than %n element", nullptr, max)}};

            for ( const auto& child : m_children )
                if ( auto childInfo = child->isInvalid() )
                    return std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("Some of the items are invalid"), std::move(childInfo)}};

            return {};
        }

        case Property::Type::Enum: {
            auto currentIt = std::find_if(m_children.cbegin(), m_children.cend(),
                                          [](const auto& child){return child->isEnabled();} );

            if ( currentIt == m_children.cend() )
                return std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("One of variants should be selected")}};

            if ( auto childInfo = currentIt->get()->isInvalid() )
                return std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("Additional data for the selected variant is not valid"), std::move(childInfo)}};

            if ( ++currentIt == m_children.cend() )
                return {};

            if ( std::any_of(currentIt, m_children.cend(), [](const auto& child){return child->isEnabled();}) )
                return std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("More than one items are selected. This is an internal error. Please re-select an item.")}};

            return {};
        }

        case Property::Type::Composite:
            for ( const auto& child : m_children )
                if ( auto childInfo = child->isInvalid() )
                    return std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("Some of the items are invalid"), std::move(childInfo)}};
            return {};


        case Property::Type::String: {
            auto string = get().toString();
            auto regexp = m_property->m_allowed_values.toString();

            if ( string.isEmpty() )
                return std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("The string should not be empty")}};

            if ( !regexp.isEmpty() && !QRegularExpression{regexp}.match(string).hasMatch() )
                return std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("Invalid input")}};

            return {};
        }

        case Property::Type::Int: {
            auto [min,max] = m_property->m_allowed_values.toSize();
            auto val = get().toInt();

            if ( val < min || val > max )
                std::unique_ptr<ValidationInfo>{new ValidationInfo{this, QObject::tr("The number should be between %1 and %2").arg(min).arg(max)}};

            return {};
        }

        default: return {};
    }
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

void Property::Value::fill(const QJsonValue& value)
{
    if ( value.isUndefined() || value.isNull() ) {
        setEnabled(false);
        return;
    }

    switch ( m_property->m_type ) {
        case Type::Enum:
            for ( const auto& child : m_children )
                child->setEnabled(false);
        case Type::Composite: {
            QJsonObject object = value.toObject();

            for ( const auto& child : m_children )
                child->fill(object[child->m_property->name()]);

        } break;

        case Type::Array: {
            QJsonArray array = value.toArray();
            m_children.clear();
            if ( m_property->m_prototype )
                for ( const auto& item : array ) {
                    auto child = m_property->m_prototype->defaultValue()->clone();
                    child->fill(item);
                    addChild(std::move(child));
                }
        } break;

        default: set(value.toVariant());
    }
    m_enabled = true;
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
