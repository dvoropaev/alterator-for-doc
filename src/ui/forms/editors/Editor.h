#pragma once

#include <QWidget>
#include "data/Parameter.h"

class Editor : public QObject {
    Q_OBJECT
public:
    virtual ~Editor() = default;

    Property::Value* value() const {return m_value;}
    QWidget*  widget()   const {return m_widget;  }
    virtual void fill() = 0;

signals:
    void aboutToChange();
    void changed();

protected:
    Editor(Property::Value* value) : m_value{value} {}
    Editor(Editor& other) : m_value{other.value()} {}

    Property::Value* m_value;
    QWidget* m_widget;
};

using EditorPtr = std::unique_ptr<Editor>;

EditorPtr createEditor(Property::Value* value, QWidget* parent, Parameter::Contexts context, bool compact = false, bool array = false);

