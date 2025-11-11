#include "ValidationIndicator.h"
#include "../../Editor.h"

ValidationIndicator::ValidationIndicator(QWidget* parent)
    : KMessageWidget{parent}
{
    setMessageType(KMessageWidget::MessageType::Error);
    setCloseButtonVisible(false);
    setIcon(QIcon::fromTheme("dialog-error"));
}

void ValidationIndicator::setEditor(Editor *e)
{
    m_editor = e;
    connect(m_editor, &Editor::changed, this, &ValidationIndicator::validate);
    validate();
}

void ValidationIndicator::validate(){
    if ( auto info = m_editor->value()->isInvalid() ) {
        setText(info->message);
        animatedShow();
    } else
        animatedHide();
}
