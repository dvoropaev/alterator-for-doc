#include "ValidationIndicator.h"
#include "ui_ValidationIndicator.h"
#include "../../Editor.h"

ValidationIndicator::ValidationIndicator(QWidget* parent)
    : QWidget{parent}
    , ui { new Ui::ValidationIndicator }
{
    ui->setupUi(this);
    ui->iconLabel->setPixmap(QIcon::fromTheme("dialog-warning").pixmap({24,24}).scaled(24,24));
}

ValidationIndicator::~ValidationIndicator() { delete ui; }

void ValidationIndicator::setEditor(Editor *e)
{
    m_editor = e;
    connect(m_editor, &Editor::changed, this, &ValidationIndicator::validate);
    validate();
}

void ValidationIndicator::validate(){
    if ( auto info = m_editor->value()->isInvalid() ) {
        ui->textLabel->setText(info->message);
        show();
    } else {
        hide();
    }
}
