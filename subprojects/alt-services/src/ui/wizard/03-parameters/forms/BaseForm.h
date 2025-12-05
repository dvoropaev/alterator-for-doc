#pragma once

#include <QWidget>

#include "data/Parameter.h"
#include "data/Action.h"
#include "ui/misc/SearchAdapter.h"

class BaseForm : public QWidget {
    Q_OBJECT
protected:
    inline BaseForm(const Action& action, QWidget* parent = nullptr)
        : QWidget{parent}
        , m_action{action}
    {};
public:
    inline void setParameters(const std::vector<Parameter*>& params, Parameter::Contexts conexts)
    {
        m_parameters = params;
        m_contexts = conexts;
        setParametersImpl(conexts);
    }

    virtual void ensureVisible(const Parameter::Value* value) = 0;
    virtual SearchAdapter* searchAdapter() = 0;

    inline const Action& action() const { return m_action; };
    inline const auto& parameters() const { return m_parameters; }
    inline auto contexts() const { return m_contexts; }

signals:
    void changed();

protected:
    friend class Editor;
    virtual void setParametersImpl(Parameter::Contexts conexts) = 0;
    const Action& m_action;
    Parameter::Contexts m_contexts;
    std::vector<Parameter*> m_parameters;
};
