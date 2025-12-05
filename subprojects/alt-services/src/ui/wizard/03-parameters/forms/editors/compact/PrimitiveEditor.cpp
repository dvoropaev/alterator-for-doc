#include "PrimitiveEditor.h"

#include <KPasswordLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>


void AbstractPrimitiveEditor::fill() {
    load();
    onChange();
}

void AbstractPrimitiveEditor::onChange() {
    emit aboutToChange();
    m_value->set(data());
    emit changed();
}



template<> void PrimitiveEditor<KPasswordLineEdit>::init(QWidget* parent)
{
    auto passwordEdit = new KPasswordLineEdit{parent};
    m_widget = passwordEdit;

    passwordEdit->lineEdit()
        ->setValidator( new QRegularExpressionValidator{QRegularExpression{m_value->property()->allowed().toString()}, passwordEdit} );

    passwordEdit->setClearButtonEnabled(true);

    passwordEdit->setEchoMode( m_value->property()->isPassword()
        ? QLineEdit::Password
        : QLineEdit::Normal
    );

    passwordEdit->setRevealPasswordMode( m_value->property()->isPassword()
        ? KPassword::RevealMode::Always
        : KPassword::RevealMode::Never
    );

    connect(passwordEdit, &KPasswordLineEdit::passwordChanged, this, &AbstractPrimitiveEditor::onChange);
}
template<> void PrimitiveEditor<KPasswordLineEdit>::load(){
    static_cast<KPasswordLineEdit*>(m_widget)->setPassword(m_value->get().toString());
}
template<> QVariant PrimitiveEditor<KPasswordLineEdit>::data(){
    return static_cast<KPasswordLineEdit*>(m_widget)->password();
}


template<> void PrimitiveEditor<QComboBox>::init(QWidget* parent)
{
    auto cb = new QComboBox{parent};
    m_widget = cb;

    for ( const auto& child : m_value->children() )
        cb->addItem( child->property()->displayName(), child->property()->name() );

    connect( cb, &QComboBox::currentIndexChanged, this, [this](const int i){
        emit aboutToChange();

        for ( int row = 0; row < m_value->children().size(); ++row )
            m_value->children().at(row)->setEnabled(row == i);

        emit changed();
    });

}
template<> void PrimitiveEditor<QComboBox>::load() {
    auto cb = static_cast<QComboBox*>(m_widget);

    auto activeValue = ranges::find_if(m_value->children(), &Property::Value::isEnabled);

    // Set first one as fallback
    if ( activeValue == m_value->children().cend() ) {
        activeValue = m_value->children().cbegin();
        activeValue->get()->setEnabled();
    }

    cb->setCurrentIndex(std::max( 0, cb->findData(activeValue->get()->property()->name()) ));
}
template<> QVariant PrimitiveEditor<QComboBox>::data(){
    return static_cast<QComboBox*>(m_widget)->currentData();
}


template<> void PrimitiveEditor<QSpinBox>::init(QWidget* parent)
{
    auto sb = new QSpinBox{parent};
    m_widget = sb;

    auto [min,max] = m_value->property()->allowed().toSize();

    sb->setMinimum(min);
    sb->setMaximum(max);

    connect(sb, &QSpinBox::valueChanged, this, &AbstractPrimitiveEditor::onChange);
}
template<> void PrimitiveEditor<QSpinBox>::load(){
    auto value = m_value->get();

    if ( !value.isNull() )
        static_cast<QSpinBox*>(m_widget)->setValue(value.toInt());

}
template<> QVariant PrimitiveEditor<QSpinBox>::data(){
    return static_cast<QSpinBox*>(m_widget)->value();
}


template<> void PrimitiveEditor<QCheckBox>::init(QWidget* parent)
{
    auto cb = new QCheckBox{parent};
    m_widget = cb;

    connect(cb, &QCheckBox::checkStateChanged, this, &AbstractPrimitiveEditor::onChange);
    connect(cb, &QCheckBox::toggled, this, [cb](bool checked){ cb->setText( checked ? QObject::tr("Yes") : QObject::tr("No") ); });
}
template<> void PrimitiveEditor<QCheckBox>::load(){
    auto value = m_value->get();

    auto* cb = static_cast<QCheckBox*>(m_widget);
    cb->toggle();
    cb->setChecked(value.toBool());
}
template<> QVariant PrimitiveEditor<QCheckBox>::data(){
    return static_cast<QCheckBox*>(m_widget)->isChecked();
}
