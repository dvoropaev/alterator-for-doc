#include "EditorTitle.h"
#include "ui_EditorTitle.h"

#include "data/Parameter.h"

EditorTitle::EditorTitle(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EditorTitle)
{
    ui->setupUi(this);
}

EditorTitle::~EditorTitle()
{
    delete ui;
}

void EditorTitle::setEditor(Editor* editor, Parameter::Contexts contexts)
{
    m_editor = editor;

    auto* property = editor->value()->property();
    bool inGroup = false;

    if ( auto* parent = editor->value()->parent() )
        inGroup = parent->property()->valueType() == Property::Type::Enum;


    ui->nameLabel->setText(property->displayName());
    ui->commentLabel->setText(property->comment());

    // preserve same left padding
    ui->widget->resize(ui->checkBox->width(), ui->widget->height());

    auto* param = dynamic_cast<Parameter*>(property);
    bool required = property->isRequired() || (param && (param->required() & contexts));
    ui->radioButton->setVisible(inGroup);
    ui->checkBox->setHidden( inGroup || required );

    connect(ui->checkBox,    &QCheckBox   ::toggled, this, &EditorTitle::onChecked);
    connect(ui->radioButton, &QRadioButton::toggled, this, &EditorTitle::onChecked);

    if ( !ui->checkBox->isHidden() )
        ui->checkBox->setChecked(m_editor->value()->isEnabled());
    else if ( !ui->radioButton->isHidden() )
        ui->radioButton->setChecked(m_editor->value()->isEnabled());

    onChecked(m_editor->value()->isEnabled() || required);

    if ( m_editor->value()->property()->isConstant() )
        m_editor->widget()->hide();
}

QRadioButton* EditorTitle::radio()
{
    return ui->radioButton;
}

void EditorTitle::onChecked(bool checked)
{
    m_editor->value()  ->setEnabled(checked);
    m_editor->widget() ->setEnabled(checked);
    ui->nameLabel      ->setEnabled(checked);
    ui->commentLabel   ->setEnabled(checked);

    m_editor->changed();
}

#include <QMouseEvent>
void EditorTitle::mousePressEvent(QMouseEvent* event)
{
    if ( ui->nameLabel->geometry().contains(event->pos()) ) {
        if ( ui->checkBox->isVisible() )
            ui->checkBox->toggle();
        else if ( ui->radioButton->isVisible() )
            ui->radioButton->toggle();
    }

    QWidget::mousePressEvent(event);
}
