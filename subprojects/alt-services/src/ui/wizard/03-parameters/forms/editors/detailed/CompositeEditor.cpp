#include "CompositeEditor.h"

#include <QVBoxLayout>

CompositeEditor::CompositeEditor(const BaseForm& form, Property::Value* value, QWidget* parent)
    : DetailedEditor{form, value}
{
    m_widget = new QWidget{parent};
    auto layout = new QVBoxLayout{};
    m_widget->setLayout(layout);
}

void CompositeEditor::fill() {
    m_children.clear();

    for ( auto& child : m_value->children() ) {
        if ( child->property()->isConstant() ) continue;

        std::unique_ptr<DetailedEditor> editor = std::unique_ptr<DetailedEditor>(static_cast<DetailedEditor*>(createEditor(m_form, child.get(), m_widget, {}).release()));
        editor->fill();
        connect(editor.get(), &Editor::changed, this, &Editor::changed);
        m_widget->layout()->addWidget(editor->widget());
        m_children.push_back(std::move(editor));
    }
}

QWidget* CompositeEditor::makeVisible(const Property::Value* value)
{
    if ( value == m_value )
        return m_widget;

    auto* parent = value;
    while ( parent->parent() != m_value )
        parent = parent->parent();

    auto match = ranges::find(m_children, parent, &DetailedEditor::value);
    if ( match != m_children.cend() )
        return match->get()->makeVisible(value);

    qWarning() << "child not found";

    return m_widget;
}
