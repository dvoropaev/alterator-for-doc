
#include "ParameterStatusModel.h"

void ParameterStatusModel::showDefault(bool how) {
    beginResetModel();
    m_show_default = how;
    endResetModel();
}

Property::Value* ParameterStatusModel::getValue(Parameter* p) const {
    return m_show_default
            ? p->defaultValue()
            : p->currentValue();
}
