#include "CompositeArrayEditor.h"
#include "wrappers/CompositeArrayItem.h"
#include "ui_CompositeArrayEditor.h"

CompositeArrayEditor::CompositeArrayEditor(Property::Value* value, QWidget *parent, int context)
    : DetailedEditor{value}
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

QWidget* CompositeArrayEditor::makeVisible(const Property::Value::ValidationInfo* info, int level)
{
    if ( level == 0 )
        return m_widget;

    if ( auto* childInfo = info->childInfo.get() ) {
        for ( const auto& child : m_children )
            if ( child->value() == childInfo->value )
                return child->makeVisible(childInfo, level -1);

        qWarning() << "child not found";
    } else
        qWarning() << "child info not found";

    return m_widget;
}

void CompositeArrayEditor::addNew(Property::Value* value)
{
    if ( !value ) {
        auto newValue = m_value->property()->prototype()->defaultValue()->clone();
        value = newValue.get();
        m_value->addChild(std::move(newValue));
    }

    auto item = std::make_unique<CompositeArrayItem>(value, m_widget);
    m_widget->layout()->addWidget(item->widget());

    connect(item.get(), &Editor::changed, this, &Editor::changed);

    connect(item.get(), &CompositeArrayItem::remove, this, [this, ptr = item.get()]{
        auto it = std::find_if(m_children.begin(), m_children.end(), [ptr](auto& item){return item.get() == ptr;});
        if ( it != m_children.end() ) {
            m_widget->layout()->removeWidget(it->get()->widget());
            it->get()->widget()->deleteLater();
            m_value->removeChild(std::distance(m_children.begin(), it));
            m_children.erase(it);
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
    ui->pushButton->setDisabled( size >= max );

    emit changed();
}
