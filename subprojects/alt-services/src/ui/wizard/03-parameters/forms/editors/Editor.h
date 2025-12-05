#pragma once

#include <QWidget>
#include "data/Parameter.h"
#include "../BaseForm.h"

class Editor : public QObject {
    Q_OBJECT
public:
    virtual ~Editor() = default;

    Property::Value* value() const {return m_value;}
    QWidget*  widget()       const {return m_widget;}
    virtual void fill() = 0;
    inline const BaseForm& form() const { return m_form; }

signals:
    void aboutToChange();
    void changed();

protected:
    Editor(const BaseForm& form, Property::Value* value)
        : m_value{value}
        , m_form{form}
    {}

    Editor(Editor& other)
        : m_value{other.value()}
        , m_form{other.m_form}
    {}

    Property::Value* m_value;
    QWidget* m_widget;

    const BaseForm& m_form;
};

using EditorPtr = std::unique_ptr<Editor>;

EditorPtr createEditor(const BaseForm& form, Property::Value* value, QWidget* parent, Parameter::Contexts context, bool compact = false, bool array = false);

