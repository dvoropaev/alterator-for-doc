#include "DetailedForm.h"

#include "data/Parameter.h"
#include "editors/Editor.h"
#include "editors/detailed/DetailedEditor.h"
#include <QScrollArea>
#include <QVBoxLayout>
#include <QScrollBar>

#include "ui_DetailedForm.h"

class DetailedForm::Private {
public:
    Private()
        : ui{new Ui::DetailedForm}
    {}

    ~Private(){delete ui;}

    PtrVector<DetailedEditor> m_editors;
    Ui::DetailedForm* ui;
};

DetailedForm::DetailedForm(QWidget* parent)
    : BaseForm{parent}
    , d{ new Private{} }
{
    d->ui->setupUi(this);
}

DetailedForm::~DetailedForm() { delete d; }

#include <QPropertyAnimation>
#include <QGraphicsColorizeEffect>
void DetailedForm::ensureVisible(const Parameter::Value::ValidationInfo* invalid, int level)
{
    auto it = std::find_if(d->m_editors.cbegin(), d->m_editors.cend(),
                           [=](const auto& editor){return editor->value() == invalid->value;});
    if ( it != d->m_editors.cend() ) {
        auto* widget = it->get()->makeVisible(invalid, level);
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

void DetailedForm::setParametersImpl(Parameter::Contexts contexts)
{
    d->m_editors.clear();
    while ( auto* item = d->ui->content->layout()->takeAt(0) ) {
        item->widget()->deleteLater();
        delete item;
    }


    for ( auto* param : m_parameters ) {

        auto editor = createEditor( param->editValue(), d->ui->content, contexts );
        editor->fill();

        d->ui->content->layout()->addWidget(editor->widget());
        connect(editor.get(), &Editor::changed, this, &DetailedForm::changed);

        d->m_editors.push_back(std::unique_ptr<DetailedEditor>(static_cast<DetailedEditor*>(editor.release())));
    }

    d->ui->scrollArea->ensureVisible(0,0);
    d->ui->scrollArea->setMinimumWidth( d->ui->content->sizeHint().width() );
    d->ui->scrollArea->setMinimumHeight( qMin(d->ui->content->sizeHint().height(), d->ui->content->sizeHint().width()/2) );
}
