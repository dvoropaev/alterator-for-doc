#pragma once

#include "DetailedEditor.h"

namespace Ui {
class ArrayEditor;
}

class ArrayEditor : public DetailedEditor
{
    Q_OBJECT
public:
    explicit ArrayEditor(const BaseForm& form, Property::Value* value, QWidget *parent, Parameter::Contexts contexts);
    ~ArrayEditor();

    void fill() override {checkSize();}
    QWidget* makeVisible(const Property::Value* value) override;

private:
    bool checkSize();

    class Private;
    Private* d;
};

