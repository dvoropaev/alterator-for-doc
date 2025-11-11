#pragma once

#include "TranslatableObject.h"

class Facility : public TranslatableObject {
public:
    class State : public TranslatableObject {
    public:
        enum class Type {
            State,
            Integer,
            String
        };

        inline State(const QString& name, const Locales& locales,
                     Type type = Type::State, const QVariant& allowedValues = {}) noexcept
            : TranslatableObject{name, locales}
            , m_type{type}
            , m_allowed{allowedValues}
        {}

        inline State(const QString& name) noexcept
            : TranslatableObject{name, Locales{{{"en", name}}, {}}}
            , m_type{Type::State}
        {
            m_display_name = name;
        }

        inline Type type() const noexcept { return m_type; }

        /*
         *  - Type::State   QVariant{} ( no value )
         *  - Type::Integer QSize      ( min, max )
         *  - Type::String  QString    ( regexp   )
         */
        inline const QVariant& allowedValues() const noexcept { return m_allowed; }

        inline const QVariant value() const noexcept { return m_type == Type::State ? m_name : m_value; }
        bool setValue(const QVariant& value);

    private:
        const Type m_type;
        const QVariant m_allowed;
        QVariant m_value;
    };

    inline Facility(const QString& name, const Locales& locales,
                    std::vector<std::unique_ptr<State>>&& knownStates) noexcept
        : TranslatableObject{name, locales}
    {
        for ( auto& state : knownStates )
            m_known_states.insert({state->name(), std::shared_ptr<State>{state.release()}});
    }

    virtual void setLocale(const QLocale&) const override;

    inline const auto& states()  const noexcept { return m_states;  }
    inline State* currentState() const noexcept { return m_current; }
    inline void setcurrentState(State* newState) noexcept { m_current = newState; }

    void updateAvailableStates(const QStringList& stateNames);
    void setStateFromValue(const QString&) noexcept;

private:
    State* m_current{};
    std::map<QString, std::shared_ptr<State>> m_known_states;
    std::vector<std::shared_ptr<State>> m_states;
};
Q_DECLARE_METATYPE(Facility*);
