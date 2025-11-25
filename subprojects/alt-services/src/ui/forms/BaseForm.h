#pragma once

#include <QWidget>

#include "data/Parameter.h"
#include "data/Action.h"
#include "ui/SearchAdapter.h"

class BaseForm : public QWidget {
    Q_OBJECT
protected:
    inline BaseForm(const Action& action, QWidget* parent = nullptr)
        : QWidget{parent}
        , m_action{action}
    {};
public:
    inline void setParameters(const std::vector<Parameter*>& params, Parameter::Contexts conext){
        m_parameters = params;
        setParametersImpl(conext);
    }

    inline std::unique_ptr<Property::Value::ValidationInfo> findInvalidValue() {
        for ( auto* param : m_parameters )
            if ( auto info = param->value(Parameter::ValueScope::Edit)->isInvalid(m_action.options.force) )
                return info;

        return {};
    }

    virtual void ensureVisible(const Parameter::Value* value) = 0;
    virtual SearchAdapter* searchAdapter() = 0;

    inline const Action& action() const { return m_action; };
    inline const auto& parameters() const { return m_parameters; }

signals:
    void changed();

protected:
    friend class Editor;
    virtual void setParametersImpl(Parameter::Contexts conext) = 0;
    const Action& m_action;
    std::vector<Parameter*> m_parameters;
};
