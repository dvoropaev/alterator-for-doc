#include "SearchBar.h"
#include "ui_SearchBar.h"

class SearchBar::Private {
public:
    Ui::SearchBar ui;
    SearchAdapter* m_adapter{};
    int m_results{0};
    int m_currentMatch{0};

    void updateButtons()
    {
        ui.prevButton->setEnabled( m_currentMatch > 0 );
        ui.nextButton->setEnabled( m_currentMatch < (m_results - 1)  );
    }
};

SearchBar::SearchBar(QWidget *parent)
    : QWidget{parent}
    , d{new Private}
{
    d->ui.setupUi(this);
    d->ui.lineEdit->addAction(QIcon::fromTheme(QIcon::ThemeIcon::EditFind), QLineEdit::LeadingPosition)
        ->setDisabled(true);

    d->ui.prevButton->setShortcut(QKeySequence::FindPrevious);
    d->ui.nextButton->setShortcut(QKeySequence::FindNext);
}

SearchBar::~SearchBar() { delete d; }

void SearchBar::setAdapter(SearchAdapter* adapter)
{
    d->m_adapter = adapter;
    d->m_currentMatch = 0;
    d->m_results = 0;

    d->ui.prevButton->setDisabled(true);
    d->ui.nextButton->setDisabled(true);
}

#define ADAPTER_CHECK() \
if ( !d->m_adapter ) \
{ qWarning() << "SearchAdapter is not set"; return; }

void SearchBar::on_lineEdit_textChanged(const QString &text)
{
    ADAPTER_CHECK();

    d->m_currentMatch = 0;
    d->m_results = text.isEmpty() ? 0 : d->m_adapter->search(text);

    if ( d->m_results )
        d->m_adapter->next();

    d->updateButtons();
}

void SearchBar::on_prevButton_clicked()
{
    ADAPTER_CHECK();

    d->m_adapter->prev();
    d->m_currentMatch--;

    d->updateButtons();
}

void SearchBar::on_nextButton_clicked()
{
    ADAPTER_CHECK();

    d->m_adapter->next();
    d->m_currentMatch++;

    d->updateButtons();
}
