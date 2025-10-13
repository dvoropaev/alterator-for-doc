#include "CompactArrayFace.h"

#include <QHBoxLayout>

CompactArrayFace::CompactArrayFace(Property::Value* value, QWidget* parent)
    : Editor{value}
{
    m_widget = new QWidget{parent};
    auto layout = new QHBoxLayout{};
    m_widget->setLayout(layout);

    label  = new QLabel(tr("%n items", nullptr, value->children().size()), parent);
    addBtn = new QPushButton(QIcon::fromTheme("list-add"), {}, parent);
    if ( addBtn->icon().isNull() )
        addBtn->setText(tr("Add"));

    auto [min_,max_] = value->property()->allowed().toSize();
    min = min_;
    max = max_;
    connect(addBtn, &QPushButton::clicked, [=,max=max]{
        auto copy = value->property()->prototype()->defaultValue()->clone();
        copy ->setEnabled();
        emit aboutToChange();
        value->addChild(std::move(copy));
        fill();
        emit changed();
    });

    layout->addWidget(label);
    layout->addWidget(addBtn);
    layout->addSpacerItem(new QSpacerItem{0,0, QSizePolicy::Expanding});
}

void CompactArrayFace::fill() {
    label->setText(tr("%n items", nullptr, m_value->children().size()));
    addBtn->setEnabled(m_value->children().size() < max);
}
