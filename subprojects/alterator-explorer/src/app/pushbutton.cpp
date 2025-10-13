#include "pushbutton.h"
#include "mainwindow.h"

#include <QAction>
#include <QDebug>
#include <QEvent>
#include <QMenu>
#include <QProcess>
#include <QApplication>

namespace ab
{
PushButton::PushButton(MainWindow *w, QWidget *parent)
    : window(w)
{
    setParent(parent);

    {
        setAutoRaise(true);
        setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        setColor();
    }

    connect(this, &PushButton::clicked, this->window, [this]() { this->window->onModuleClicked(this); });
}

void PushButton::setObject(ao_builder::Object *obj)
{
    this->object = obj;
    this->setText(obj->m_displayName);
    //this->setStyleSheet("padding: 3px 7px;");

    QFont font = this->font();
    font.setPointSize(11);
    this->setFont(font);
}

ao_builder::Object *PushButton::getObject()
{
    return object;
}


void PushButton::setColor()
{
    QPalette palette = qApp->palette();
    palette.setBrush(QPalette::ButtonText, palette.link());
    palette.setBrush(QPalette::WindowText, palette.link());
    setPalette(palette);
}

bool ab::PushButton::event(QEvent* event)
{
    if ( event->type() == QEvent::ApplicationPaletteChange ) {
        event->accept();
        setColor();
        return true;
    } else
        return QToolButton::event(event);
}

} // namespace ab
