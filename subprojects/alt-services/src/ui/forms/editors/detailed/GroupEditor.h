#pragma once

#include "CompositeEditor.h"
#include <QButtonGroup>

class GroupEditor : public CompositeEditor {
public:
    using CompositeEditor::CompositeEditor;
    
    void fill() override;
    
private:
    QButtonGroup m_group;
};
