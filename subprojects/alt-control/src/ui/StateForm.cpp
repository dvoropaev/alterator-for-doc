#include "StateForm.h"
#include "ui_StateForm.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QSpacerItem>

class StateForm::Private {
public:
    inline Private(Facility::State* state) : m_state{state} {}
    Ui::StateForm ui;
    Facility::State* m_state;
    QWidget* m_widget{nullptr};
};

StateForm::StateForm(Facility::State* state, QWidget *parent)
    : QWidget(parent)
    , d(new Private{state})
{
    d->ui.setupUi(this);

    d->ui.info->setObject(state);
    connect(d->ui.info, &ObjectInfoWidget::clicked,
            [this]{ d->ui.radioButton->setChecked(true);});

    switch ( state->type() ) {
        case Facility::State::Type::String: {
            auto* lineEdit = new QLineEdit{this};
            d->m_widget = lineEdit;

            auto pattern = d->m_state->allowedValues().toString();
            if ( !pattern.isEmpty() ) {
                auto* validator = new QRegularExpressionValidator{lineEdit};
                validator->setRegularExpression(QRegularExpression{pattern});
                lineEdit->setValidator(validator);
            }

            lineEdit->setText( d->m_state->value().toString() );
            connect(lineEdit, &QLineEdit::textChanged, this, &StateForm::changed);
            d->ui.verticalLayout_2->addWidget(lineEdit);
            lineEdit->setEnabled(false);

            connect(d->ui.radioButton, &QRadioButton::toggled, lineEdit, &QWidget::setEnabled);
            break;
        }
        case Facility::State::Type::Integer: {
            auto* spinBox = new QSpinBox{this};
            d->m_widget = spinBox;

            auto [min,max] = d->m_state->allowedValues().toSize();
            spinBox->setMinimum(min);
            spinBox->setMaximum(max);

            spinBox->setValue( d->m_state->value().toInt() );
            connect(spinBox, &QSpinBox::textChanged, this, &StateForm::changed);
            d->ui.verticalLayout_2->addWidget(spinBox);
            spinBox->setEnabled(false);

            connect(d->ui.radioButton, &QRadioButton::toggled, spinBox, &QWidget::setEnabled);
            break;
        }
        default: break;
    }

}

StateForm::~StateForm() { delete d; }

QRadioButton* StateForm::radio() { return d->ui.radioButton; }

Facility::State* StateForm::state() { return d->m_state; }

QVariant StateForm::value()
{
    switch ( d->m_state->type() ) {
        case Facility::State::Type::String:
            if ( auto* lineEdit = qobject_cast<QLineEdit*>(d->m_widget) )
                return lineEdit->text();
            break;

        case Facility::State::Type::Integer:
            if ( auto* spinBox = qobject_cast<QSpinBox*>(d->m_widget) )
                return spinBox->value();
            break;

        default:
            return d->m_state->name();
    }

    return {};
}

bool StateForm::isChanged()
{
    switch ( d->m_state->type() ) {
        case Facility::State::Type::String:
        case Facility::State::Type::Integer:
            return value() != d->m_state->value();

        default:
            return false;
    }
}
