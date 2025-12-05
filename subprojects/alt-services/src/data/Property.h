#pragma once

#include "TranslatableObject.h"
#include <range/v3/algorithm.hpp>

template <typename T>
using PtrVector = std::vector<std::unique_ptr<T>>;

class Property;
using PropertyPtr = std::unique_ptr<Property>;


/*
 *  Base class for a Parameter.
 *  Also may be a child property of a composite parameter.
 *  A Property contains name, constraints and a default value.
 */
class Property : public TranslatableObject
{
public:
    Property(Property& other) noexcept;

    /*
     *  Property may be of primitive or composite type.
     *  Primitive property stores its value in QVariant.
     *  Composite, Array or Enum will have child properties.
     */
    enum class Type {
        String,
        Int,
        Bool,

        Composite,
        Enum,

        /*
         *  If property is array, it has a prototype, which determines
         *  type and constraints of its items.
         */
        Array
    };

    inline Type valueType() const { return m_type; }


    Property(const QString& name, const Locales& locales, Type type,
             bool hidden, bool required, bool password = false, PropertyPtr prototype = {}, TranslatableField arrayPrefix = {}) noexcept;


    /*
     *  Composite prototype constructor.
     *  Prototype has no name, but its children do.
     */
    Property(Type type, std::vector<PropertyPtr>&& children) noexcept;

    /*
     *  Primitive prototype constructor.
     *  Prototype has no name.
     */
    Property(Type type, bool password = false) noexcept;

    /*
     *  Indicates that text must be hidden during input.
     *  Only if valueType() == Type::String,
     */
    inline bool isPassword(){ return m_password; }

    /*
     *  Constant parameters are not shown.
     *  Mostly useful for diag.
     */
    inline bool isConstant() const { return m_constant; }

    /*
     *  Some properties of a composite type may not be required.
     *  Necessity of a Parameter determined by comparing Parameter::contexts() and Parameter::required()
     */
    inline bool isRequired() const { return m_required; }

    /* For validation purposes. Depends on Type:
     *
     *   TYPE       VALUE           DESCRIPTION
     *
     * - String     QString          (regular expression)
     * - Int        QSize            (min/max)
     * - Array      QSize            (min/max children count)
     */
    inline void setAllowed(const QVariant &v) { m_allowed_values = v; }
    inline QVariant allowed() const { return m_allowed_values; }

    class Value;
    using ValuePtr = std::unique_ptr<Value>;

    inline Value* defaultValue() const { return m_value.get(); };

    /*
     *  Prototype of an array item.
     *  Important for determining value constraints.
     *  Also may be a composite property, so Value::clone() will also copy its children
     */
    inline const Property* prototype() const { return m_prototype.get(); }

    inline const QString& arrayPrefix() const { return m_array_prefix; }

    // Translatable interface
public:
    void setLocale(const QLocale& locale) const override;

protected:
    bool m_constant{false};
    bool m_required{false};
    bool m_password{false};
    Type m_type;
    TranslatableField m_array_prefix{};

    QVariant m_allowed_values;
    PropertyPtr m_prototype{nullptr};

    /*
     *  A property of Type::Composite owns its child properties.
     *  They are accessible from the children of its value ( see Property::Value::property() ).
     */
    std::vector<PropertyPtr> m_children;

    ValuePtr m_value;
};

using ValuePtr = Property::ValuePtr;


/*
 *  Value of a Property.
 *  Property has only a default value, while Paremeter (a top-level property)
 *  wiil also have current value, and a working copy, modified by an editor.
 *
 *  If there are any Properties of Type::Composite or Type::Array,
 *  the configuration of Service has a tree-like structure,
 *  where Parameters are some kind of "roots",
 *  and Values are holding the rest of the "tree".
 *
 *  Each value refers to the Property to display property name in Editor
 *  and validate against its constraints.
 *
 */
class Property::Value {
    friend class Property;
public:
    Value(Property* property) noexcept
        : m_property{property}
    {
        resetEnabledState();
    }

    inline Property* property() const { return m_property; }

    /*
     *  If Property::isRequired() is false, the value may be enabled or not.
     *  User may enable one set of properties during deployment, and other set during reconfiguration,
     *  so we track this state per single value, not per entire Property.
     */
    inline bool isEnabled() const { return m_enabled; }
    inline void setEnabled(bool enabled = true) {
        m_enabled = m_property->m_required || enabled;
    }
    void resetEnabledState();

    /*
     *  If current value does not meet Property::allowed() requirements, returns a structure, containing a validation hint.
     *  For Property::Type::Composite, Property::Type::Array and Property::Type::Enum this function is called recursively on its children.
     *  If force is true, parameter's linked resources will not be checked for conflicts.
     */
    struct ValidationInfo {
        const Value* value;
        QString message;

        std::unique_ptr<Value::ValidationInfo> childInfo;
    };
    std::unique_ptr<ValidationInfo> isInvalid(bool force) const;

    /*
     * Returns Property::displayName
     * or in-array position
     */
    QString displayName(bool noDefaultPrefix = false) const;

    /*
     *  Convert current value to JSON.
     *  For Property::Type::Composite, Property::Type::Array and Property::Type::Enum
     *  this functions called recursively on its children,
     *  returning an object or an array.
     */
    QJsonValue serialize(bool excludePasswords = false) const;

    /*
     *  Parse value from JSON
     *  For Property::Type::Composite, Property::Type::Array and Property::Type::Enum
     *  this functions called recursively on its children.
     */
    bool fill(const QJsonValue& value, bool required);


    /*
     *  Value of Int/String/Bool
     */
    inline QVariant get() const { return m_value; }
    inline void set(const QVariant &v) { m_value = v; };
    inline void setConfirmation(const QString& s) { m_passwordConfirmation = s; };

    /*
     *  Makes a copy of a value with all its children.
     *  Called from array editors on Property::prototype()->defaultValue().
     */
    ValuePtr clone() noexcept;

    /*
     *  Items of an array, enum, or a composite value.
     */
    inline const PtrVector<Value>& children() const { return m_children; }
    inline int indexOf(const Value* child) const {
        auto match = ranges::find(m_children, child, &ValuePtr::get);
        return match == m_children.end() ? -1 : std::distance(m_children.begin(), match);
    }

    void addChild(ValuePtr &&child);
    inline void removeChild(int at) { m_children.erase( m_children.begin()+at ); }

    inline Value* parent() const { return m_parent; }

private:
    Property* m_property;

    bool m_enabled{false};
    QVariant m_value;
    QString m_passwordConfirmation;

    Value* m_parent{nullptr};
    PtrVector<Value> m_children;
};
