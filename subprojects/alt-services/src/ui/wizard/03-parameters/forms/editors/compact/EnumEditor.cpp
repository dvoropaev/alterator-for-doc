#include "EnumEditor.h"

#include <QComboBox>

EnumEditor::EnumEditor(const BaseForm& form, Property::Value* value, QWidget* parent)
    : AbstractPrimitiveEditor{form, value}
{
    auto cb = new QComboBox{parent};
    m_widget = cb;

    for ( const auto& child : m_value->children() )
        cb->addItem( child->property()->displayName(), child->property()->name() );

    connect( cb, &QComboBox::currentIndexChanged, this, [this](const int i){
        emit aboutToChange();

        for ( size_t row = 0; row < m_value->children().size(); ++row )
            m_value->children().at(row)->setEnabled( i >= 0 && row == static_cast<size_t>(i));

        emit changed();
    });

    EnumEditor::load();
}

void EnumEditor::load()
{
    auto cb = static_cast<QComboBox*>(m_widget);

    auto activeValue = ranges::find_if(m_value->children(), &Property::Value::isEnabled);

    // Set first one as fallback
    if ( activeValue == m_value->children().cend() ) {
        activeValue = m_value->children().cbegin();
        activeValue->get()->setEnabled();
    }

    cb->setCurrentIndex(std::max( 0, cb->findData(activeValue->get()->property()->name()) ));
}

QVariant EnumEditor::data()
{
    return static_cast<QComboBox*>(m_widget)->currentData();
}
