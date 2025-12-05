#include "StringEditor.h"

#include <QLineEdit>
#include <QRegularExpressionValidator>

StringEditor::StringEditor(const BaseForm& form, Property::Value* value, QWidget* parent)
    : AbstractPrimitiveEditor{form, value}
{
    auto lineEdit = new QLineEdit{parent};
    m_widget = lineEdit;

    lineEdit->setValidator( new QRegularExpressionValidator{QRegularExpression{m_value->property()->allowed().toString()}, lineEdit} );
    lineEdit->setClearButtonEnabled(true);
    connect(lineEdit, &QLineEdit::textChanged, this, &AbstractPrimitiveEditor::onChange);

    StringEditor::load();
}

void StringEditor::load()
{
    static_cast<QLineEdit*>(m_widget)->setText(m_value->get().toString());
}

QVariant StringEditor::data()
{
    return static_cast<QLineEdit*>(m_widget)->text();
}
