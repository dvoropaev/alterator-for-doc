#include "ControlForm.h"
#include "ui_ControlForm.h"

#include "Controller.h"

#include <QRadioButton>
#include <QRegularExpression>
#include <QMessageBox>

ControlForm::ControlForm(Controller* controller, const QString& name, const QString& summary, const QString& currentValue,
                         const QVariantMap& values, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ControlForm)
    , m_controller{controller}
{
    ui->setupUi(this);
    ui->nameLabel->setText(name);
    ui->summaryLabel->setText(summary);
    auto buttonLayout = new QGridLayout{};
    ui->scrollAreaWidgetContents->setLayout(buttonLayout);

    int row = 0;
    for ( const auto& [value, help] : values.asKeyValueRange() ){
        auto button = new QRadioButton{this};
        button->setSizePolicy(QSizePolicy{QSizePolicy::Fixed, QSizePolicy::Fixed});

        auto nameLabel = new QLabel{value, this};
        nameLabel->setBuddy(button);

        auto helpLabel = new QLabel{help.toString().remove(QRegularExpression{'^'+value+": "}), this};
        helpLabel->setBuddy(button);
        helpLabel->setWordWrap(true);
        helpLabel->setContentsMargins(helpLabel->contentsMargins() + QMargins{0,0,0,10});


        buttonLayout->addWidget(button,      row, 0);
        buttonLayout->addWidget(nameLabel,   row, 1);
        buttonLayout->addWidget(helpLabel, ++row, 1);
        ++row;

        m_group.addButton(button);
        m_values[button] = value;

        if ( currentValue != "unknown" && !values.keys().contains(currentValue) )
            qWarning() << "possibly invalid current value" << name << currentValue << values.keys();

        if ( currentValue == value ) {
            button->toggle();
            current = button;
        }
    }
    buttonLayout->addItem(new QSpacerItem(0,0, QSizePolicy::Minimum, QSizePolicy::Expanding), ++row, 1);

    connect(&m_group, &QButtonGroup::buttonClicked, this, &ControlForm::onButtonClicked);
}

ControlForm::~ControlForm()
{
    delete ui;
}

void ControlForm::onButtonClicked(QAbstractButton* b)
{
    QString error;
    if ( m_controller->setValue(ui->nameLabel->text(), m_values[b], error) )
    {
        current = b;
    }
    else
    {
        if ( current )
            current->toggle();

        QMessageBox::critical(this, tr("error"), error);
    }
}
