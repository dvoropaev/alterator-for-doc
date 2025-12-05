#include "DetailedForm.h"

#include "data/Parameter.h"
#include "editors/Editor.h"
#include "editors/detailed/DetailedEditor.h"
#include <QScrollArea>
#include <QVBoxLayout>
#include <QScrollBar>
#include "ui/misc/ParameterSearcher.h"
#include "ui_DetailedForm.h"

class DetailedFormSearchAdapter : public ParameterSearcher {
    Q_OBJECT

    ParameterModel m_model;
    DetailedForm* m_form{};

public:
    explicit DetailedFormSearchAdapter(DetailedForm* form)
        : m_form{form}
    {
        m_model.setScope(Parameter::ValueScope::Edit);
        ParameterSearcher::setModel(m_model);
    }

    void setParameters() {
        m_model.setItems(m_form->parameters());
    }

public slots:
    void prev() override
    {
        if ( auto* value = ParameterSearcher::prevValue() )
            m_form->ensureVisible(value);
    }

    void next() override
    {
        if ( auto* value = ParameterSearcher::nextValue() )
            m_form->ensureVisible(value);
    }
};
#include "DetailedForm.moc"

class DetailedForm::Private {
public:
    explicit Private(DetailedForm* self)
        : ui{new Ui::DetailedForm}
        , m_searchAdapter{self}
    {}

    ~Private(){delete ui;}

    PtrVector<DetailedEditor> m_editors;
    Ui::DetailedForm* ui;
    DetailedFormSearchAdapter m_searchAdapter;
};

DetailedForm::DetailedForm(const Action& action, QWidget* parent)
    : BaseForm{action, parent}
    , d{ new Private{this} }
{
    d->ui->setupUi(this);
}

DetailedForm::~DetailedForm() { delete d; }

#include <QPropertyAnimation>
#include <QGraphicsColorizeEffect>
#include <range/v3/algorithm.hpp>
void DetailedForm::ensureVisible(const Parameter::Value* value)
{
    auto* topLevelParent = value;
    while ( auto* p = topLevelParent->parent() )
        topLevelParent = p;

    auto match = ranges::find(d->m_editors, topLevelParent, &Editor::value);
    if ( match != d->m_editors.cend() ) {
        auto* widget = match->get()->makeVisible(value);
        d->ui->scrollArea->ensureWidgetVisible( widget );

        widget->setAutoFillBackground(true);

        QGraphicsColorizeEffect* effect= new QGraphicsColorizeEffect(widget);
        widget->setGraphicsEffect(effect);

        QPropertyAnimation* anim = new QPropertyAnimation(effect, "color", effect);
        anim->setDuration(500);
        anim->setStartValue(widget->palette().color(QPalette::ColorGroup::Active, QPalette::ColorRole::Accent));
        anim->setEndValue(  widget->palette().color(QPalette::ColorGroup::Active, QPalette::ColorRole::Base)  );
        anim->start();

        connect(anim, &QPropertyAnimation::finished, [=]{
            anim->deleteLater();
            effect->deleteLater();
            widget->setAutoFillBackground(false);
        });

    }
}

SearchAdapter* DetailedForm::searchAdapter() { return &d->m_searchAdapter; }

void DetailedForm::setParametersImpl(Parameter::Contexts contexts)
{
    d->m_editors.clear();
    while ( auto* item = d->ui->content->layout()->takeAt(0) ) {
        item->widget()->deleteLater();
        delete item;
    }


    for ( auto* param : m_parameters ) {

        auto editor = createEditor(*this, param->value(Parameter::ValueScope::Edit), d->ui->content );
        editor->fill();

        d->ui->content->layout()->addWidget(editor->widget());
        connect(editor.get(), &Editor::changed, this, &DetailedForm::changed);

        d->m_editors.push_back(std::unique_ptr<DetailedEditor>(static_cast<DetailedEditor*>(editor.release())));
    }

    d->ui->scrollArea->ensureVisible(0,0);
    d->ui->scrollArea->setMinimumWidth( d->ui->content->sizeHint().width() );
    d->ui->scrollArea->setMinimumHeight( qMin(d->ui->content->sizeHint().height(), d->ui->content->sizeHint().width()/2) );

    d->m_searchAdapter.setParameters();
}
