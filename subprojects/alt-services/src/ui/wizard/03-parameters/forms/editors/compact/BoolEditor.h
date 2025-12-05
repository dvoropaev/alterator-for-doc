#pragma once

#include "PrimitiveEditor.h"

class BoolEditor : public AbstractPrimitiveEditor
{
    Q_OBJECT
public:
    BoolEditor(const BaseForm& form, Property::Value* value, QWidget* parent);
    void load() override;

protected:
    QVariant data() override;
};
