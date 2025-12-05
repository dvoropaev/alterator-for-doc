#pragma once

#include "../Editor.h"

class AbstractPrimitiveEditor : public Editor {
    Q_OBJECT
protected:
    inline AbstractPrimitiveEditor(const BaseForm& form, Property::Value* value)
        : Editor{form, value}
    {}
    
    virtual void load() = 0;
    
public:
    void fill() override;
    
public slots:
    void onChange();

protected:
    virtual QVariant data() = 0;
};
