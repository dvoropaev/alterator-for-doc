#include "ValidationIndicator.h"
#include "../../Editor.h"

ValidationIndicator::ValidationIndicator(QWidget* parent)
    : KMessageWidget{parent}
{
    setMessageType(KMessageWidget::MessageType::Error);
    setCloseButtonVisible(false);
    setPosition(KMessageWidget::Footer);
    setIcon(QIcon::fromTheme("dialog-error"));
}

void ValidationIndicator::setEditor(Editor *e)
{
    m_editor = e;
    connect(m_editor, &Editor::changed, this, &ValidationIndicator::validate);
    validate();
}

void ValidationIndicator::validate()
{
    /*
     *  NOTE: changed() may be emitted multiple times
     *  per editing event, but animations are shown immediately for some reason
     */

    if ( auto info = m_editor->value()->isInvalid(m_editor->form().action().options.force ) )
    {
        setText(info->message);
        //animatedShow();
        show();
    } else
        hide();
        //animatedHide();
}
