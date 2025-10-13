#pragma once

#include "EditorWrapper.h"

namespace Ui {
class InlineEditor;
}

class InlineEditor : public EditorWrapper
{
public:
    explicit InlineEditor(EditorPtr child, QWidget *parent = nullptr, Parameter::Contexts contexts = {});
    ~InlineEditor();

    EditorTitle* title() override;
    QWidget* makeVisible(const Property::Value::ValidationInfo* info, int level) override;

private:
    Ui::InlineEditor *ui;
};
