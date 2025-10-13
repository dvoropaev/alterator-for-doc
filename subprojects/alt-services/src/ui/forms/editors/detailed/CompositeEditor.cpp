#include "CompositeEditor.h"

#include <QVBoxLayout>

CompositeEditor::CompositeEditor(Property::Value* value, QWidget* parent)
    : DetailedEditor{value}
{
    m_widget = new QWidget{parent};
    auto layout = new QVBoxLayout{};
    m_widget->setLayout(layout);
}

void CompositeEditor::fill() {
    m_children.clear();

    for ( auto& child : m_value->children() ) {
        if ( child->property()->isConstant() ) continue;

        std::unique_ptr<DetailedEditor> editor = std::unique_ptr<DetailedEditor>(static_cast<DetailedEditor*>(createEditor(child.get(), m_widget, {}).release()));
        editor->fill();
        connect(editor.get(), &Editor::changed, this, &Editor::changed);
        m_widget->layout()->addWidget(editor->widget());
        m_children.push_back(std::move(editor));
    }
}

QWidget* CompositeEditor::makeVisible(const Property::Value::ValidationInfo* info, int level)
{
    if ( level == 0 )
        return m_widget;

    if ( auto* childInfo = info->childInfo.get() ) {
        for ( const auto& child : m_children )
            if ( child->value() == childInfo->value )
                return child->makeVisible(childInfo, level -1);

        qWarning() << "child not found";
    } else
        qWarning() << "child info not found";

    return m_widget;
}
