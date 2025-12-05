#include "GroupEditor.h"

#include "misc/EditorTitle.h"
#include "wrappers/EditorWrapper.h"

void GroupEditor::fill() {
    CompositeEditor::fill();

    for ( auto& child : m_children ) {
        if ( auto wrapper = dynamic_cast<EditorWrapper*>(child.get()) ) {
            if ( auto title = wrapper->title() ) {
                m_group.addButton(title->radio());
            } else qWarning() << "null title";
        } else
            qWarning() << "editor is not of type wrapper";
    }
    if ( m_children.size() && !m_group.checkedButton() )
        m_group.buttons().front()->toggle();

    connect(&m_group, &QButtonGroup::buttonToggled, this, &Editor::changed);

}
