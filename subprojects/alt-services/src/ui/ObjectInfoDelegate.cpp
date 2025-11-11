#include "ObjectInfoDelegate.h"

#include <QTextDocument>
#include <QPainter>
#include <QApplication>

#include "app/ServicesApp.h"

static const int text_alignment = Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap;

class ObjectInfoDelegate::Private {
public:
    bool m_neverDetailed{false};
    bool m_detailed{true};
    bool m_multiline{false};
    int m_column_size{0};

    std::unique_ptr<QTextDocument> prepareDocument(const QModelIndex& index, QRect initialRect, QStyleOptionViewItem option, bool elide){
        auto doc = std::make_unique<QTextDocument>();

        QColor textColor;
        auto fg = index.data(Qt::ForegroundRole);
        textColor = option.palette.brush(option.state.testFlag(QStyle::State_Selected)
                                             ? QPalette::HighlightedText
                                             : QPalette::Text).color();

        doc->setDefaultStyleSheet( "* {color: "+textColor.name()+";}");

        QTextOption textoption;
        textoption.setWrapMode(m_multiline ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap);
        doc->setDefaultTextOption(textoption);


        QString name = index.data().toString();
        QString comment = index.data(Qt::ToolTipRole).toString();

        if ( elide && !m_multiline ) {
            QFont   boldFont;
            boldFont.setBold(true);
            QFont italicFont;
            italicFont.setItalic(true);

            QFontMetrics   boldFm{boldFont};
            QFontMetrics italicFm{italicFont};

            name = boldFm.elidedText(name, Qt::TextElideMode::ElideRight, initialRect.width());
            comment = boldFm.elidedText(comment, Qt::TextElideMode::ElideRight, initialRect.width());
        }

        doc->setHtml(
            QString{R"(<div style="font-weight: bold;">%0</div><div style="font-style: italic;">%1</div>)"}
                .arg(name).arg(comment)
            );
        doc->setPageSize(initialRect.size());
        return std::move(doc);
    }
};

ObjectInfoDelegate::ObjectInfoDelegate(QObject* parent)
    : QStyledItemDelegate{parent}
    , d{new Private}
{
    connect(ServicesApp::instance()->settings(), &AppSettings::tablesDetailedChanged, this, &ObjectInfoDelegate::setDetailed);
    connect(ServicesApp::instance()->settings(), &AppSettings::tablesDetailedMultilineChanged, this, &ObjectInfoDelegate::setMultiline);
    setDetailed();
    setMultiline();
}

ObjectInfoDelegate::~ObjectInfoDelegate() { delete d; }

QSize ObjectInfoDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if ( d->m_neverDetailed || !d->m_detailed || index.model()->span(index).width() > 1 )
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

    copy.rect.setWidth( d->m_column_size - indentation*level );
    copy.rect.setHeight(INT_MAX);
    QRect initialRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemText, &copy);

    auto doc = d->prepareDocument(index, initialRect, copy, false);

    return doc->size().toSize();
}

void ObjectInfoDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem copy = option;
    initStyleOption(&copy, index);

    // make non-interactable checkboxed look disabled
    if ( index.data(Qt::CheckStateRole).isValid() && !index.flags().testFlag(Qt::ItemIsUserCheckable) )
    {
        // preserve text colors
#define REPLACE(role) copy.palette.setBrush(QPalette::Disabled, role, copy.palette.brush(copy.palette.currentColorGroup(), role))

        REPLACE(QPalette::Text);
        REPLACE(QPalette::HighlightedText);

#undef REPLACE
        copy.palette.setCurrentColorGroup(QPalette::Disabled);
    }


    if ( d->m_neverDetailed || !d->m_detailed || index.model()->span(index).width() > 1 )
        copy.widget->style()->drawControl(QStyle::CE_ItemViewItem, &copy, painter);
    else {

        { // draw background with icon and checkbox
            auto text = copy.text;
            copy.text.clear();
            copy.widget->style()->drawControl(QStyle::CE_ItemViewItem, &copy, painter);
            copy.text = text;
        }

        QRect textRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemText, &copy);

        auto doc = d->prepareDocument(index, textRect, copy, true);

        painter->save();
        {
            painter->translate(textRect.topLeft());
            doc->drawContents(painter, textRect.translated(-textRect.topLeft()));
        }
        painter->restore();
    }
}

void ObjectInfoDelegate::setColumnSize(int size)
{
    d->m_column_size = size;
    if (d->m_detailed && d->m_multiline)
        emit sizeHintChanged({});
}

void ObjectInfoDelegate::setNeverDetailed(bool how)
{
    d->m_neverDetailed = how;
    emit sizeHintChanged({});
}

void ObjectInfoDelegate::setDetailed()
{
    d->m_detailed = ServicesApp::instance()->settings()->tablesDetailed();
    emit sizeHintChanged({});
}

void ObjectInfoDelegate::setMultiline()
{
    d->m_multiline = ServicesApp::instance()->settings()->tablesDetailedMultiline();
    emit sizeHintChanged({});
}
