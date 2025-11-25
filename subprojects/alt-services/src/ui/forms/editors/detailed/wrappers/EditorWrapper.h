#pragma once

#include "../DetailedEditor.h"

class EditorTitle;

class EditorWrapper : public DetailedEditor {
public:
    void fill() override { m_editor->fill(); }
    
    virtual EditorTitle* title() {return nullptr;}
    inline Editor* editor() { return m_editor.get(); }
    
protected:
    using DetailedEditor::DetailedEditor;
    inline EditorWrapper(EditorPtr&& child)
        : DetailedEditor{child->form(), child->value()}
        , m_editor{std::move(child)}
    {
        connect(m_editor.get(), &Editor::changed, this, &Editor::changed);
    }
    
    EditorPtr m_editor;
};
