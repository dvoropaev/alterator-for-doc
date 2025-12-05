#pragma once

#include <QWidget>
#include "EditorWrapper.h"

namespace Ui {
class CollapsibleEditor;
}

class CollapsibleEditor : public EditorWrapper
{
public:
    explicit CollapsibleEditor(EditorPtr&& child, QWidget *parent = nullptr, Parameter::Contexts contexts = {});
    ~CollapsibleEditor();

    void fill() override;
    EditorTitle* title() override;

    QWidget* makeVisible(const Property::Value* value) override;


private:
    Ui::CollapsibleEditor *ui;
};

