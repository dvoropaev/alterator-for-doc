#pragma once

#include "DetailedEditor.h"

class CompositeEditor : public DetailedEditor {
public:
    CompositeEditor(const BaseForm& form, Property::Value* value, QWidget* parent);
    
    void fill() override;
    
    QWidget* makeVisible(const Property::Value* value) override;

protected:
    std::vector<std::unique_ptr<DetailedEditor>> m_children;
};
