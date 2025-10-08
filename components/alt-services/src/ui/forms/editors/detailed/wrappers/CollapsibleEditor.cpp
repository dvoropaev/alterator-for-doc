#include "CollapsibleEditor.h"
#include "ui_CollapsibleEditor.h"

CollapsibleEditor::CollapsibleEditor(EditorPtr&& child, QWidget *parent, Parameter::Contexts contexts)
    : EditorWrapper{std::move(child)}
    , ui(new Ui::CollapsibleEditor)
{
    m_widget = new QWidget{parent};
    ui->setupUi(m_widget);

    ui->title->setEditor(m_editor.get(), contexts);
    ui->indicator->setEditor(m_editor.get());

    connect(ui->collapseBtn, &QAbstractButton::clicked, ui->content, &QWidget::setVisible);
    ui->content->setVisible(m_value->property()->valueType() == Property::Type::Enum);
    ui->collapseBtn->setHidden(m_value->property()->valueType() == Property::Type::Enum);

    ui->content->layout()->addWidget(m_editor->widget());
}

CollapsibleEditor::~CollapsibleEditor()
{
    delete ui;
}

void CollapsibleEditor::fill()
{
    EditorWrapper::fill();
    if ( !m_editor->widget()->layout()->count() )
        ui->collapseBtn->hide();
}

EditorTitle* CollapsibleEditor::title()
{
    return ui->title;
}

QWidget* CollapsibleEditor::makeVisible(const Property::Value::ValidationInfo* info, int level)
{
    ui->collapseBtn->setChecked(true);
    ui->content->setVisible(true);

    if ( level == 0 )
        return ui->title;

    return static_cast<DetailedEditor*>(m_editor.get())->makeVisible(info, level);
}
