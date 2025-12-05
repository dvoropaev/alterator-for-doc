#include "CompositeArrayEditor.h"
#include "wrappers/CompositeArrayItem.h"
#include "ui_CompositeArrayEditor.h"

CompositeArrayEditor::CompositeArrayEditor(const BaseForm& form, Property::Value* value, QWidget *parent)
    : DetailedEditor{form, value}
    , ui(new Ui::CompositeArrayEditor)
{
    m_widget = new QWidget{parent};
    ui->setupUi(m_widget);
    connect(ui->pushButton, &QAbstractButton::clicked, this, std::bind(&CompositeArrayEditor::addNew, this, nullptr));
    checkSize();
}

CompositeArrayEditor::~CompositeArrayEditor()
{
    delete ui;
}

void CompositeArrayEditor::fill()
{
    m_children.clear();

    for ( const ValuePtr& prop : m_value->children() )
        addNew(prop.get());
}

QWidget* CompositeArrayEditor::makeVisible(const Property::Value* value)
{
    if ( m_value == value )
        return m_widget;

    auto* parent = value;
    while ( parent->parent() != m_value )
        parent = parent->parent();

    auto match = ranges::find(m_children, parent, &Editor::value);
    if ( match != m_children.cend() )
        return match->get()->makeVisible(value);

    qWarning() << "child not found";

    return m_widget;
}

void CompositeArrayEditor::addNew(Property::Value* value)
{
    if ( !value ) {
        auto newValue = m_value->property()->prototype()->defaultValue()->clone();
        value = newValue.get();
        m_value->addChild(std::move(newValue));
    }

    auto item = std::make_unique<CompositeArrayItem>(m_form, value, m_widget);
    m_widget->layout()->addWidget(item->widget());

    connect(item.get(), &Editor::changed, this, &Editor::changed);

    connect(item.get(), &CompositeArrayItem::remove, this, [this, ptr = item.get()]{
        auto positionToRemove = ranges::find(m_children, ptr, &std::unique_ptr<CompositeArrayItem>::get);
        if ( positionToRemove != m_children.end() ) {
            m_widget->layout()->removeWidget(positionToRemove->get()->widget());
            positionToRemove->get()->widget()->deleteLater();
            m_value->removeChild(std::distance(m_children.begin(), positionToRemove));
            m_children.erase(positionToRemove);
            checkSize();
        }
    });

    m_children.push_back(std::move(item));

    checkSize();
}

void CompositeArrayEditor::checkSize() {
    ui->sizeInfo->setText(tr("%n items:", nullptr, m_children.size()));

    auto [min,max] = m_value->property()->allowed().toSize();
    auto size = m_children.size();
    ui->pushButton->setDisabled( max != min && size >= static_cast<size_t>(max) );

    for ( const auto& item : m_children )
        item->refresh();

    emit changed();
}
