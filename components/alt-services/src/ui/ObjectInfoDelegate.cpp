#include "ObjectInfoDelegate.h"

#include <QTextDocument>
#include <QPainter>
#include <QApplication>

#include "app/ServicesApp.h"

static const int text_alignment = Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap;

ObjectInfoDelegate::ObjectInfoDelegate(QObject* parent)
    : QStyledItemDelegate{parent}
{
    connect(ServicesApp::instance()->settings(), &AppSettings::tablesDetailedChanged, this, &ObjectInfoDelegate::setDetailed);
    setDetailed();
}

QSize ObjectInfoDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if ( !m_detailed || index.model()->span(index).width() > 1 )
        return QStyledItemDelegate::sizeHint(option, index);

    auto copy = option;
    initStyleOption(&copy, index);
    QFont boldFont = option.font;
    boldFont.setBold(true);

    QFont italicFont = option.font;
    italicFont.setItalic(true);

    QFontMetrics boldFm{boldFont};
    QFontMetrics italicFm{italicFont};

    // QT always passes option with empty rect
    int indentation = QApplication::style()->pixelMetric(QStyle::PM_TreeViewIndentation, &option);
    int level = 1;
    for ( QModelIndex i = index; i.parent().isValid(); i = i.parent() )
        ++level;

    copy.rect.setWidth( m_column_size - indentation*level );
    copy.rect.setHeight(INT_MAX);
    QRect initialRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemText, &copy);

    QTextDocument doc{};
    doc.setHtml(
        QString{R"(<div style="font-weight: bold;">%0</div><div style="font-style: italic;">%1</div><br>)"}
            .arg(index.data().toString()).arg(index.data(Qt::ToolTipRole).toString())
        );
    doc.setPageSize(initialRect.size());

    return doc.size().toSize();
}

void ObjectInfoDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if ( !m_detailed || index.model()->span(index).width() > 1 )
        return QStyledItemDelegate::paint(painter, option, index);

    auto tooltip = index.data(Qt::ToolTipRole).toString();

    QStyleOptionViewItem copy = option;
    initStyleOption(&copy, index);

    auto text = copy.text;
    copy.text.clear();

    if ( index.data(Qt::CheckStateRole).isValid() && !index.flags().testFlag(Qt::ItemIsUserCheckable) )
        copy.palette.setCurrentColorGroup( QPalette::Disabled );

    copy.widget->style()->drawControl(QStyle::CE_ItemViewItem, &copy, painter);

    QTextDocument doc;

    QColor textColor;
    auto fg = index.data(Qt::ForegroundRole);
    if ( fg.isValid() )
        textColor = fg.value<QBrush>().color();
    else
        textColor = QApplication::palette().color(copy.palette.currentColorGroup(), QPalette::Text);

    doc.setDefaultStyleSheet( "* {color: "+textColor.name()+ ";}");
    doc.setHtml(
        QString{R"(<div style="font-weight: bold;">%0</div><div style="font-style: italic;">%1</div>)"}
            .arg(index.data().toString())
            .arg(index.data(Qt::ToolTipRole).toString())
    );

    QRect textRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemText, &copy);
    doc.setPageSize(textRect.size());

    painter->save();
    {
        painter->translate(textRect.topLeft());
        doc.drawContents(painter, textRect.translated(-textRect.topLeft()));
    }
    painter->restore();
}

void ObjectInfoDelegate::setColumnSize(int size)
{
    m_column_size = size;
    emit sizeHintChanged({});
}

void ObjectInfoDelegate::setDetailed()
{
    m_detailed = ServicesApp::instance()->settings()->tablesDetailed();
    emit sizeHintChanged({});
}
