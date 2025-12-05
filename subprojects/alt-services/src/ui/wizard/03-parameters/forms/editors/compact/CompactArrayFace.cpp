#include "CompactArrayFace.h"

#include <QHBoxLayout>

CompactArrayFace::CompactArrayFace(const BaseForm& form, Property::Value* value, QWidget* parent)
    : Editor{form, value}
{
    m_widget = new QWidget{parent};
    auto layout = new QHBoxLayout{m_widget};
    m_widget->setLayout(layout);

    addBtn = new QPushButton(QIcon::fromTheme("list-add"), {}, m_widget);
    label  = new QLabel(tr("%n items", nullptr, value->children().size()), m_widget);
    if ( addBtn->icon().isNull() )
        addBtn->setText(tr("Add"));

    auto [min,max] = value->property()->allowed().toSize();
    m_min = min;
    m_max = max;
    connect(addBtn, &QPushButton::clicked, [=]{
        auto copy = value->property()->prototype()->defaultValue()->clone();
        copy ->setEnabled();
        emit aboutToChange();
        value->addChild(std::move(copy));
        fill();
        emit changed();
    });

    layout->addWidget(addBtn);
    layout->addWidget(label);
    addBtn->setFocusPolicy(Qt::StrongFocus);
    layout->addSpacerItem(new QSpacerItem{0,0, QSizePolicy::Expanding});
}

void CompactArrayFace::fill() {
    label->setText(tr("%n items", nullptr, m_value->children().size()));
    addBtn->setEnabled(m_value->children().size() < m_max);
}
