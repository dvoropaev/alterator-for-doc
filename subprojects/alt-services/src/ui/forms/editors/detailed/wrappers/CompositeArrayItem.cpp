#include "CompositeArrayItem.h"
#include "ui_CompositeArrayItem.h"

CompositeArrayItem::CompositeArrayItem(Property::Value* value, QWidget *parent)
    : EditorWrapper{createEditor(value, parent, {}, false, true)}
    , ui(new Ui::CompositeArrayItem)
{
    m_widget = new QWidget{parent};
    ui->setupUi(m_widget);

    EditorWrapper::fill();

    ui->indicator->setEditor(m_editor.get());
    connect(ui->collapseBtn, &QAbstractButton::clicked, ui->contentWrapper, &QWidget::setVisible);
    connect(ui->deleteButton, &QAbstractButton::clicked, this, &CompositeArrayItem::remove);
    ui->contentWrapper->hide();
    ui->content->layout()->addWidget(m_editor->widget());
}

CompositeArrayItem::~CompositeArrayItem()
{
    delete ui;
}

QWidget* CompositeArrayItem::makeVisible(const Property::Value::ValidationInfo* info, int level)
{
    ui->collapseBtn->setChecked(true);
    ui->contentWrapper->setVisible(true);

    if ( level == 0 )
        return ui->indicator;

    return static_cast<DetailedEditor*>(m_editor.get())->makeVisible(info, level);
}
