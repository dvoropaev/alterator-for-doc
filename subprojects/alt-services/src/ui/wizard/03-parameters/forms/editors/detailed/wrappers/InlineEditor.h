#pragma once

#include "EditorWrapper.h"

namespace Ui {
class InlineEditor;
}

class InlineEditor : public EditorWrapper
{
public:
    explicit InlineEditor(EditorPtr child, QWidget *parent = nullptr);
    ~InlineEditor();

    EditorTitle* title() override;
    QWidget* makeVisible(const Property::Value* value) override;

private:
    Ui::InlineEditor *ui;
};
