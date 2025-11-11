#pragma once

#include <QWidget>

#include "data/Parameter.h"

class BaseForm : public QWidget {
    Q_OBJECT
protected:
    inline BaseForm(QWidget* parent) : QWidget{parent}{};
public:
    inline void setParameters(const std::vector<Parameter*>& params, Parameter::Contexts conext){
        m_parameters = params;
        setParametersImpl(conext);
    }

    inline std::unique_ptr<Property::Value::ValidationInfo> findInvalidValue() {
        for ( auto* param : m_parameters )
            if ( auto info = param->value(Parameter::ValueScope::Edit)->isInvalid() )
                return info;

        return {};
    }

    virtual void ensureVisible(const Parameter::Value::ValidationInfo* invalid, int level) = 0;

signals:
    void changed();

protected:
    virtual void setParametersImpl(Parameter::Contexts conext) = 0;

    std::vector<Parameter*> m_parameters;
};
