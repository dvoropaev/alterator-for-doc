#include "PasswordEditor.h"

#include <QRegularExpressionValidator>
#include <QVBoxLayout>

PasswordEditor::PasswordEditor(const BaseForm& form, Property::Value* value, QWidget* parent)
    : AbstractPrimitiveEditor{form, value}
{
    m_widget = new QWidget{parent};
    auto* layout = new QVBoxLayout{m_widget};
    m_widget->setLayout(layout);

    m_password = new KPasswordLineEdit{m_widget};
    m_confirmation= new KPasswordLineEdit{m_widget};

    layout->addWidget( m_password );
    layout->addWidget( m_confirmation );

    m_password->     lineEdit()->setPlaceholderText(tr("Password"));
    m_confirmation-> lineEdit()->setPlaceholderText(tr("Confirmation"));
    m_password->     setEchoMode( QLineEdit::Password );
    m_confirmation-> setEchoMode( QLineEdit::Password );
    m_password->     setRevealPasswordMode( KPassword::RevealMode::Always );
    m_confirmation-> setRevealPasswordMode( KPassword::RevealMode::Always );
    m_password->     setClearButtonEnabled(true);
    m_confirmation-> setClearButtonEnabled(true);

    connect(m_password,     &KPasswordLineEdit::passwordChanged, this, &AbstractPrimitiveEditor::onChange);
    connect(m_confirmation, &KPasswordLineEdit::passwordChanged, [this](const QString& text){
        m_value->setConfirmation(text);
        emit onChange();
    });

    PasswordEditor::load();
}

void PasswordEditor::load()
{
    m_password    ->setPassword(m_value->get().toString());
    m_confirmation->setPassword(m_value->get().toString());
}

QVariant PasswordEditor::data()
{
    return m_password->password();
}
