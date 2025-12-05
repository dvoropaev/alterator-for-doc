#pragma once

#include "PrimitiveEditor.h"

class StringEditor : public AbstractPrimitiveEditor
{
    Q_OBJECT
public:
    StringEditor(const BaseForm& form, Property::Value* value, QWidget* parent);

    void load() override;

protected:
    virtual QVariant data() override;
};
