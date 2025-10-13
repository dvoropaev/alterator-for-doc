#pragma once

#include "ParameterModel.h"

class ParameterStatusModel : public ParameterModel {
public:
    using ParameterModel::ParameterModel;
    
    void showDefault(bool how);
    
protected:
    Property::Value* getValue(Parameter* p) const override;
    
    bool m_show_default{true};
};
