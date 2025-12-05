#include "IntEditor.h"

#include <QSpinBox>

IntEditor::IntEditor(const BaseForm& form, Property::Value* value, QWidget* parent)
    : AbstractPrimitiveEditor{form, value}
{
    auto sb = new QSpinBox{parent};
    m_widget = sb;

    auto [min,max] = m_value->property()->allowed().toSize();

    sb->setMinimum(min);
    sb->setMaximum(max);

    connect(sb, &QSpinBox::valueChanged, this, &AbstractPrimitiveEditor::onChange);
    IntEditor::fill();
}

void IntEditor::load()
{
    auto value = m_value->get();

    if ( !value.isNull() )
        static_cast<QSpinBox*>(m_widget)->setValue(value.toInt());
}


QVariant IntEditor::data() {
    return static_cast<QSpinBox*>(m_widget)->value();
}
