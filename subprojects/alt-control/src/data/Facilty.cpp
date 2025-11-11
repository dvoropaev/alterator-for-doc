#include "Facilty.h"
#include <qregularexpression.h>
#include <qsize.h>

void Facility::setLocale(const QLocale& locale) const
{
    TranslatableObject::setLocale(locale);
    for ( const auto& [name,state] : m_known_states ) state->setLocale(locale);
    for ( const auto& state : m_states ) state->setLocale(locale);
}

void Facility::updateAvailableStates(const QStringList& stateNames)
{
    m_states.clear();
    m_states.reserve(stateNames.size());

    for ( const QString& stateName : stateNames ) {
        std::shared_ptr<State> state;

        try {
            state = m_known_states.at(stateName);
        } catch (...) {
            state = std::make_shared<State>(stateName);
        }

        m_states.push_back(std::move(state));
    }
}

void Facility::setStateFromValue(const QString& value) noexcept
{
    State* valueState = nullptr;

    for ( const auto& state : m_states ) {
        if ( state->type() == State::Type::State )
        {
            if ( state->name() == value ) {
                m_current = state.get();
                return;
            }
        } else
            valueState = state.get();
    }

    if ( !valueState ) {
        qCritical() << "failed to determine current state of" << m_name << "from value" << value;
        return;
    }

    switch ( valueState->type() ) {
        case State::Type::Integer: {
            bool ok = false;
            int v = value.toInt(&ok);
            if ( ok ) {
                valueState->setValue(v);
                m_current = valueState;
            }
            break;
        }

        case State::Type::String: {
            if ( value != "unknown" ) {
                valueState->setValue(value);
                m_current = valueState;
            }
            break;
        }

        default: break;
    }
}

bool Facility::State::setValue(const QVariant& value)
{
    bool result = false;

    switch (m_type) {
        case Type::String: {
            QString pattern = m_allowed.toString();
            result = pattern.isEmpty() || QRegularExpression(pattern).match(value.toString()).hasMatch();
            break;
        }

        case Type::Integer: {
            auto [min,max] = m_allowed.toSize();

            bool ok = false;
            int v = value.toInt(&ok);
            result = ok && min <= v && v <= max;
            return result;
        }
        default: break;
    }

    if ( result )
        m_value = value;
    return result;
}
