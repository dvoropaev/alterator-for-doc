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
    ((KPasswordLineEdit*)m_widget)->setPassword(m_value->get().toString());
}
template<> QVariant PrimitiveEditor<KPasswordLineEdit>::data(){
    return ((KPasswordLineEdit*)m_widget)->password();
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
    auto cb = (QComboBox*)m_widget;
    int i = -1;
    if ( m_value->property()->valueType() == Property::Type::Enum ) {
        auto it = std::find_if(m_value->children().cbegin(),
                               m_value->children().cend(),
                               [](const auto& child){ return child->isEnabled(); });
        if ( it != m_value->children().cend() )
            i = cb->findData( it->get()->property()->name() );
    } else {
        i = cb->findData(m_value->get().toString());
    }

    cb->setCurrentIndex( i >= 0 ? i : 0 );
}
template<> QVariant PrimitiveEditor<QComboBox>::data(){
    return ((QComboBox*)m_widget)->currentData();
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
        ((QSpinBox*)m_widget)->setValue(value.toInt());

}
template<> QVariant PrimitiveEditor<QSpinBox>::data(){
    return ((QSpinBox*)m_widget)->value();
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

    auto* cb = ((QCheckBox*)m_widget);
    cb->toggle();
    cb->setChecked(value.toBool());
}
template<> QVariant PrimitiveEditor<QCheckBox>::data(){
    return ((QCheckBox*)m_widget)->isChecked();
}
