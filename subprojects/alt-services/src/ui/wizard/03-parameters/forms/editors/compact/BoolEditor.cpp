#include "BoolEditor.h"

#include <QCheckBox>

BoolEditor::BoolEditor(const BaseForm& form, Property::Value* value, QWidget* parent)
    : AbstractPrimitiveEditor{form, value}
{
    auto cb = new QCheckBox{parent};
    m_widget = cb;

    connect(cb, &QCheckBox::checkStateChanged, this, &AbstractPrimitiveEditor::onChange);
    connect(cb, &QCheckBox::toggled, this, [cb](bool checked){ cb->setText( checked ? tr("Yes") : tr("No") ); });

    BoolEditor::load();
}

void BoolEditor::load()
{
    auto value = m_value->get();

    auto* cb = static_cast<QCheckBox*>(m_widget);
    cb->toggle();
    cb->setChecked(value.toBool());
}

QVariant BoolEditor::data()
{
    return static_cast<QCheckBox*>(m_widget)->isChecked();
}
