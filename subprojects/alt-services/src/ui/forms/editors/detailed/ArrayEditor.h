#pragma once

#include "DetailedEditor.h"

namespace Ui {
class ArrayEditor;
}

class ArrayEditor : public DetailedEditor
{
    Q_OBJECT
public:
    explicit ArrayEditor( Property::Value* value, QWidget *parent, Parameter::Contexts contexts);
    ~ArrayEditor();

    void fill() override {checkSize();}
    QWidget* makeVisible(const Property::Value::ValidationInfo* info, int level) override;

private:
    bool checkSize();

    class Private;
    Private* d;
};

