#include "LogWidget.h"

#include <QBoxLayout>
#include <QTreeWidget>
#include <QApplication>
#include <QScrollBar>
#include <QTextBrowser>
#include <QTextCursor>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QStandardPaths>
#include <QFileDialog>
#include <QDomDocument>

#include "CustomTreeView.h"
#include "SearchBar.h"
#include "ItemModelSearcher.h"
#include "app/ServicesApp.h"
#include "controller/Controller.h"

class LogSearcher : public ItemModelSearcher {
    Q_OBJECT

    CustomTreeView& m_tree;
    QString m_search;
    int m_currentPosition{0};

public:
    LogSearcher(CustomTreeView& target)
        : ItemModelSearcher{target.model()}
        , m_tree{target}
    {}

protected:
    bool nextLogMatch(bool next)
    {
        auto index = currentIndex();
        if ( QTextBrowser* browser = qobject_cast<QTextBrowser*>(m_tree.indexWidget(index)) )
        {
            QTextCursor cursor = browser->textCursor();
            cursor.clearSelection();
            browser->setTextCursor(cursor);

            m_currentPosition += next ? 1 : -m_search.size();

            if ( m_currentPosition < 0 )
                return false;

            cursor.setPosition(m_currentPosition);
            cursor = browser->document()->find(m_search, cursor,
                                               next ? QTextDocument::FindFlags{}
                                                    : QTextDocument::FindBackward);

            m_currentPosition = cursor.position();

            return m_currentPosition >= 0;
        }
        return false;
    }

    void move(bool next)
    {   
        auto index = currentIndex();

        bool needMove = !index.isValid() || !index.data(Qt::UserRole).toBool() || !nextLogMatch(next);

        if ( needMove )
        {
            index = next ? nextIndex() : prevIndex();
            m_currentPosition = next ? 0 : currentIndex().data().toString().size();
        }

        m_tree.setCurrentIndex(index);

        if ( index.data(Qt::UserRole).toBool() )
        {
            if ( QTextBrowser* browser = qobject_cast<QTextBrowser*>(m_tree.indexWidget(index)) )
            {
                QTextCursor cursor = browser->textCursor();

                cursor.setPosition(m_currentPosition);
                cursor.select(QTextCursor::WordUnderCursor);
                browser->setTextCursor(cursor);

                auto globalPoint = browser->mapToGlobal( browser->cursorRect(cursor).center() );
                globalPoint += QPoint{0, m_tree.verticalScrollBar()->value()};
                int offset = m_tree.viewport()->mapFromGlobal(globalPoint).y();

                m_tree.verticalScrollBar()->setValue(
                    qBound( 0,
                        offset - m_tree.verticalScrollBar()->pageStep()/2,
                        m_tree.verticalScrollBar()->maximum()
                    )
                );
            }
        }
        else
        {
            m_tree.highlight(index);
        }
    }

public slots:
    int search(const QString& text) override
    {
        m_search = text;
        m_currentPosition = 0;
        int rowcount = ItemModelSearcher::search(text);
        auto& filter = ItemModelSearcher::filterModel();

        int size = 0;
        for ( int row = 0; row < rowcount; ++row )
        {
            QModelIndex index = mapToSource(filter.index(row, 0));
            size += index.data(Qt::UserRole).toBool()
                ? index.data().toString().count(text, Qt::CaseInsensitive) : 1;
        }

        return size;
    }

    void prev() override { move(false); }
    void next() override { move(true ); }

};
#include "LogWidget.moc"

class AutoScroller {
    CustomTreeView& m_treeWidget;
    bool m_needScroll{};
public:
    AutoScroller(CustomTreeView& w)
        : m_treeWidget{w}
        , m_needScroll{ m_treeWidget.verticalScrollBar()->value() ==
                        m_treeWidget.verticalScrollBar()->maximum() }
    {}

    ~AutoScroller() {
        if ( m_needScroll )
            m_treeWidget.scrollToBottom();
    }
};

class LogWidget::Private {
public:
    QStandardItem* m_lastEntryItem{};
    QStandardItem* m_lastTextItem{};
    QTextBrowser* m_lastText{};
    CustomTreeView* m_tree{};
    QStandardItemModel m_model;

    QString m_filename{"log"};

    inline auto withScroll() { return AutoScroller(*m_tree); }

    void appendToLabel(const QString& msg)
    {
        if ( !m_lastText )
        {
            m_lastTextItem = new QStandardItem{};
            if ( m_lastEntryItem )
                m_lastEntryItem->appendRow(m_lastTextItem);
            else
                m_model.appendRow(m_lastTextItem);

            m_lastTextItem->setData(true, Qt::UserRole);

            m_lastText = new QTextBrowser;
            m_lastText->setContextMenuPolicy(Qt::NoContextMenu);
            m_lastText->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
            m_lastText->setLineWrapMode(QTextBrowser::LineWrapMode::WidgetWidth);
            m_lastText->setTextInteractionFlags(Qt::TextSelectableByMouse);
            m_tree->setIndexWidget(m_lastTextItem->index(), m_lastText);
            if ( m_lastEntryItem )
                m_tree->expand(m_lastEntryItem->index());
        }

        m_lastText->append(msg);
        m_lastTextItem->setData(m_lastTextItem->data(Qt::DisplayRole).toString() + msg, Qt::DisplayRole);
        m_lastText->setMinimumHeight(m_lastText->document()->size().height());
        m_tree->doItemsLayout();
    }


    void serialize(QDomDocument& doc, QDomElement& element, const QModelIndex& parent = {})
    {
        for ( int i = 0; i < m_model.rowCount(parent); ++i )
        {
            auto index = m_model.index(i, 0, parent);

            if ( !index.data(Qt::UserRole).toBool() )
            {
                auto entry = doc.createElement("entry");

                auto entryTitle = doc.createElement("h3");
                auto color = index.data(Qt::ForegroundRole);
                if ( color.isValid() )
                    entryTitle.setAttribute("style", "color: "+color.value<QBrush>().color().name());
                entryTitle.appendChild(doc.createTextNode(index.data().toString()));
                entry.appendChild(entryTitle);

                auto entryContents = doc.createElement("contents");
                serialize(doc, entryContents, index);
                entry.appendChild(entryContents);

                element.appendChild(entry);
            }
            else
            {
                auto text = doc.createElement("text");
                QDomDocument log;
                auto result = log.setContent(QString{"<!DOCTYPE html><text>%1</text>"}.arg(index.data().toString()));
                for ( auto msg = log.documentElement().firstChild(); !msg.isNull(); msg = msg.nextSibling() )
                    text.appendChild(msg.cloneNode());
                element.appendChild(text);
            }
        }
    }
};

LogWidget::LogWidget(QWidget *parent)
    : QWidget(parent)
    , d{ new Private }
{
    auto layout = new QBoxLayout{QBoxLayout::TopToBottom, this};
    setLayout(layout);

    auto searchBar = new SearchBar{this};
    searchBar->hide();

    d->m_tree = new CustomTreeView{this};
    d->m_tree->setModel(&d->m_model);
    d->m_tree->setItemDelegateForColumn(0, new QStyledItemDelegate{d->m_tree});

    for ( auto* action : d->m_tree->actions() )
        d->m_tree->removeAction(action);

    d->m_tree->setSelectionMode(QAbstractItemView::NoSelection);
    d->m_tree->setAnimated(true);
    d->m_tree->setHeaderHidden(true);
    d->m_tree->setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
    d->m_tree->verticalScrollBar()->setSingleStep(d->m_tree->horizontalScrollBar()->singleStep());
    d->m_tree->verticalScrollBar()->setPageStep(d->m_tree->horizontalScrollBar()->pageStep());

    searchBar->setAdapter(new LogSearcher{*d->m_tree});

    layout->addWidget(searchBar);
    layout->addWidget(d->m_tree);

    auto searchAction = new QAction{QIcon::fromTheme(QIcon::ThemeIcon::EditFind), tr("&Find..."), this};
    searchAction->setShortcut(QKeySequence::Find);
    d->m_tree->addAction(searchAction);

    connect(searchAction, &QAction::triggered, searchBar, &QWidget::show);

    auto* exportAction = new QAction{QIcon::fromTheme(QIcon::ThemeIcon::DocumentSaveAs), tr("Export journal...")};

    connect(qApp->controller(), &Controller::beginRefresh, this, [=]{exportAction->setDisabled(true );});
    connect(qApp->controller(), &Controller::endRefresh,   this, [=]{exportAction->setDisabled(false);});

    d->m_tree->addAction(exportAction);
    connect(exportAction, &QAction::triggered, this, [this]{
        auto dst = QFileDialog::getSaveFileName(this,
            tr("Chose where to save journal"),
            QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                .append('/').append(d->m_filename).append(".html"),
            tr("HTML files (*.html)")
        );

        if ( dst.isEmpty() ) return;

        QFile f{dst};
        if ( !f.open(QFile::WriteOnly) ) return;

        f.resize(0);
        QDomDocument doc{"html"};
        {
            auto html = doc.createElement("html");
            {
                auto head = doc.createElement("head");
                {
                    auto style = doc.createElement("style");
                    style.appendChild(doc.createTextNode(
R"(
body * { display: block; }
entry {
    margin-left: 10px;
    contents {
        border-left: 1px solid black;
        text {
            margin-left: 10px;
            font-family: monospace;
        }
    }
}
)"
                    ));
                    head.appendChild(style);
                }
                html.appendChild(head);
            }

            {
                auto root = doc.createElement("body");
                d->serialize(doc, root);
                html.appendChild(root);
            }

            doc.appendChild(html);
        }

        QTextStream s{&f};
        doc.save(s, 2);
        s.flush();
        f.close();
    });

}

LogWidget::~LogWidget() { delete d; }

void LogWidget::beginEntry(const QString& msg)
{
    auto scroller = d->withScroll();

    QStandardItem* item = new QStandardItem{msg};
    if ( d->m_lastEntryItem )
    {
        d->m_lastEntryItem->appendRow(item);
        d->m_tree->setExpanded(d->m_lastEntryItem->index(), true);
    }
    else
    {
        d->m_model.appendRow(item);
    }

    item->setBackground(QApplication::palette().button());
    d->m_lastEntryItem = item;
    d->m_lastText = nullptr;
    d->m_lastTextItem = nullptr;
}

void LogWidget::endEntry(bool success)
{
    auto scroller = d->withScroll();

    if ( d->m_lastEntryItem )
    {
        d->m_lastEntryItem->setIcon(QIcon::fromTheme(success ? "dialog-ok" : "window-close"));

        if ( !success )
            d->m_lastEntryItem->setForeground(QColor{255,0,0});

        d->m_lastEntryItem = d->m_lastEntryItem->parent();
        d->m_lastText = nullptr;
        d->m_lastTextItem = nullptr;
    }
    else
        qCritical() << "LogWidget: unbalanced LogEntry::Begin / LogEntry::End";
}

void LogWidget::message(const QString& msg)
{
    auto scroller = d->withScroll();
    d->appendToLabel(msg.isEmpty() ? "<br/>" : QString{"<div>%2</div>"}.arg(msg) );
}

void LogWidget::error(const QString& msg)
{
    auto scroller = d->withScroll();
    d->appendToLabel(msg.isEmpty() ? "<br/>" : QString{"<div style=\"color: red;\">%1</div>"}.arg(msg));
}


void LogWidget::clear()
{
    d->m_model.clear();
    d->m_lastText = nullptr;
    d->m_lastEntryItem  = nullptr;
    d->m_lastTextItem = nullptr;
}

void LogWidget::setExportFileName(const QString& name)
{
    d->m_filename = name;
}
