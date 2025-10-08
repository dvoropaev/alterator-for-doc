#pragma once

#include "DetailedEditor.h"

class CompositeEditor : public DetailedEditor {
public:
    CompositeEditor(Property::Value* value, QWidget* parent);
    
    void fill() override;
    
    QWidget* makeVisible(const Property::Value::ValidationInfo* info, int level) override;

protected:
    std::vector<std::unique_ptr<DetailedEditor>> m_children;
};
