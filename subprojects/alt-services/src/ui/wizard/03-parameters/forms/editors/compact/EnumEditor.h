#pragma once

#include "PrimitiveEditor.h"

class EnumEditor : public AbstractPrimitiveEditor
{
    Q_OBJECT
public:
    EnumEditor(const BaseForm& form, Property::Value* value, QWidget* parent);
    void load() override;

protected:
    QVariant data() override;
};
