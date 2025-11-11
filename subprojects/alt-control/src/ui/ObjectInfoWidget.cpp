#include "ObjectInfoWidget.h"
#include "ui_ObjectInfoWidget.h"

#include <QMouseEvent>

class ObjectInfoWidget::Private {
public:
    Ui::ObjectInfoWidget ui;
    bool pressedHere{false};
};

ObjectInfoWidget::ObjectInfoWidget(QWidget *parent)
    : QWidget(parent)
    , d(new Private)
{
    d->ui.setupUi(this);
}

ObjectInfoWidget::~ObjectInfoWidget() { delete d; }

void ObjectInfoWidget::setObject(TranslatableObject* obj)
{
    d->ui.    nameLabel->setText( obj->displayName() );
    d->ui. commentLabel->setText( obj->comment() );
}

void ObjectInfoWidget::mousePressEvent(QMouseEvent* event)
{
    d->pressedHere = event->button() == Qt::MouseButton::LeftButton;
    QWidget::mousePressEvent(event);
}

void ObjectInfoWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if ( d->pressedHere &&
        event->button() == Qt::MouseButton::LeftButton &&
        rect().contains(event->position().toPoint())
        )
        emit clicked();
    d->pressedHere = false;
    QWidget::mouseReleaseEvent(event);
}
