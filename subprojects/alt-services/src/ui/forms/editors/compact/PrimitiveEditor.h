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
private:
    virtual QVariant data() = 0;
};

template<class W, typename = std::enable_if<std::is_base_of<QWidget, W>::value>>
class PrimitiveEditor : public AbstractPrimitiveEditor {
public:
    inline PrimitiveEditor(const BaseForm& form, Property::Value* value, QWidget* parent)
        : AbstractPrimitiveEditor{form, value}
    {
        init(parent);
        PrimitiveEditor<W>::fill();
    }
    
    void load() override;
    
private:
    void init(QWidget* parent);
    virtual QVariant data() override;
};
