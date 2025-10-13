#include "categorywidget.h"
#include "flowlayout.h"
#include "pushbutton.h"
#include "ui_categorywidget.h"

#include <QAction>
#include <QDebug>
#include <QMenu>
#include <QMouseEvent>
#include <QPushButton>
#include <QSpacerItem>
#include <QStandardItemModel>

namespace ab
{

CategoryWidget::CategoryWidget(MainWindow *w, model::Model *m, ao_builder::Category *cat, QWidget *parent)
    : QWidget{parent}
    , ui(new Ui::CategoryWidget)
    , category(cat)
    , window(w)
    , model(m)
    , layout(new FlowLayout(0, 0, 0))
{
    ui->setupUi(this);

    setPixmap();
    ui->iconLabel->setMinimumSize(48,48);

    ui->titleLabel->setText(cat->m_displayName);
    ui->titleLabel->setMinimumSize(ui->titleLabel->sizeHint());

    ui->descriptionLabel->setText(cat->m_comment);

    ui->headerWidget->setMinimumWidth(ui->headerWidget->sizeHint().width());

    layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    ui->modulesWidget->setLayout(layout);

    for (auto &obj : m->getObjects(cat))
        addObject(obj);
}

CategoryWidget::~CategoryWidget()
{
    delete ui;
}

void CategoryWidget::addObject(ao_builder::Object *object)
{
    const auto legacyObject = dynamic_cast<ao_builder::LegacyObject *>(object);
    auto moduleButton = std::make_unique<PushButton>(window);
    moduleButton->setObject(object);
    layout->addWidget(moduleButton.release());
}

int CategoryWidget::getWeight()
{
    return category->m_weight;
}

bool CategoryWidget::isEmpty()
{
    return layout->isEmpty();
}

bool CategoryWidget::event(QEvent* event)
{
    if ( event->type() == QEvent::ApplicationPaletteChange ) {
        for ( auto* btn : findChildren<PushButton*>() )
            qApp->postEvent(btn, event->clone());
        setPixmap();
        event->accept();
        return true;
    } else
        return QWidget::event(event);
}

void CategoryWidget::setPixmap()
{
    auto icon = QIcon::fromTheme(category->m_icon, QIcon("/usr/share/alterator/design/images/" + category->m_icon + ".png"));
    QPixmap iconMap = icon.pixmap(48, 48);

    ui->iconLabel->setPixmap(iconMap);
}
} // namespace ab
