#include "RichToolTipEventFilter.h"

#include <QEvent>
#include <QWidget>
#include <QTextDocument>

bool RichToolTipEventFilter::eventFilter(QObject* watched, QEvent* event)
{
    if ( event->type() == QEvent::ToolTipChange )
    {
        if ( auto* widget = qobject_cast<QWidget*>(watched) )
        {
            QString tooltip = widget->toolTip();

            if ( !tooltip.isEmpty() && !Qt::mightBeRichText(tooltip) )
            {
                tooltip = QStringLiteral("<qt>%1</qt>").arg(tooltip);
                widget->setToolTip(tooltip);
                return true;
            }
        }
    }
    return QObject::eventFilter(watched, event);
}
