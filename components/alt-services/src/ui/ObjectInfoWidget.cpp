#include "ObjectInfoWidget.h"
#include "ui_ObjectInfoWidget.h"

ObjectInfoWidget::ObjectInfoWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ObjectInfoWidget)
{
    ui->setupUi(this);
}

ObjectInfoWidget::~ObjectInfoWidget() { delete ui; }

void ObjectInfoWidget::setObject(TranslatableObject* obj)
{
    ui->    nameLabel->setText( obj->displayName() );
    ui-> commentLabel->setText( obj->comment() );
}
