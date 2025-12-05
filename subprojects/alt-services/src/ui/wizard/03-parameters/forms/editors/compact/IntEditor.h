#pragma once

#include "PrimitiveEditor.h"

class IntEditor : public AbstractPrimitiveEditor
{
    Q_OBJECT
public:
    explicit IntEditor(const BaseForm& form, Property::Value* value, QWidget* parent);

    void load() override;

protected:
    virtual QVariant data() override;
};
