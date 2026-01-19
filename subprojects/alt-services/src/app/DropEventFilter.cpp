#include "DropEventFilter.h"

#include <QEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QWidget>

bool DropEventFilter::eventFilter(QObject* watched, QEvent* event)
{
    if ( auto* widget = qobject_cast<QWidget*>(watched) )
    {
        if ( widget->acceptDrops() )
        {
            switch ( event->type() )
            {
                case QEvent::Type::DragEnter:
                case QEvent::Type::Drop:
                {
                    // NOTE: QDropEvent is also a base class for QDragEnterEvent
                    auto* dropEvent = static_cast<QDropEvent*>(event);
                    QList<QUrl> urls = dropEvent->mimeData()->urls();
                    if ( urls.size() == 1 &&
                         urls.at(0).isLocalFile() &&
                         urls.at(0).fileName().toLower().endsWith(QStringLiteral(".json"))
                        )
                    {
                        dropEvent->acceptProposedAction();
                        dropEvent->accept();
                    }
                } break;

                default:
                    break;
            }
        }
    }

    return QObject::eventFilter(watched, event);
}
