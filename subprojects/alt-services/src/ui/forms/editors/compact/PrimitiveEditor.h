#pragma once

#include "../Editor.h"

class AbstractPrimitiveEditor : public Editor {
    Q_OBJECT
protected:
    inline AbstractPrimitiveEditor(Property::Value* value)
        : Editor{value}
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
    inline PrimitiveEditor(Property::Value* value, QWidget* parent)
        : AbstractPrimitiveEditor{value}
    {
        init(parent);
        PrimitiveEditor<W>::fill();
    }
    
    void load() override;
    
private:
    void init(QWidget* parent);
    virtual QVariant data() override;
};
