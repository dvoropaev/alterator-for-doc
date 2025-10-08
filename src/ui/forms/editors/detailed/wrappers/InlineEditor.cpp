#include "InlineEditor.h"
#include "ui_InlineEditor.h"


InlineEditor::InlineEditor(EditorPtr child, QWidget *parent, Parameter::Contexts contexts)
    : EditorWrapper{std::move(child)}
    , ui(new Ui::InlineEditor)
{
    m_widget = new QWidget{parent};
    ui->setupUi(m_widget);

    ui->content->layout()->addWidget(m_editor->widget());
    ui->title->setEditor(m_editor.get(), contexts);
    ui->indicator->setEditor(m_editor.get());
}

InlineEditor::~InlineEditor()
{
    delete ui;
}

EditorTitle* InlineEditor::title()
{
    return ui->title;
}

QWidget* InlineEditor::makeVisible(const Property::Value::ValidationInfo* info, int level)
{
    return ui->title;
}
