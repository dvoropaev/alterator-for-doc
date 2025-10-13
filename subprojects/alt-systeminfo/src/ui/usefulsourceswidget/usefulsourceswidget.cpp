#include "usefulsourceswidget.h"

#include "ui_usefulsourceswidget.h"

#include "databuilder/databuilder.h"

namespace alt
{
UsefulSourcesWidget::UsefulSourcesWidget(QWidget *parent)
    : QWidget(parent)
    , m_ui(std::make_unique<Ui::UsefulSourcesWidget>())
{
    m_ui->setupUi(this);
    auto logo = QPixmap(":/logo/assets/altlinux_rectangle.png");
    auto size = QSize(102, 35);
    m_ui->logoLabel->setPixmap(logo.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
}

UsefulSourcesWidget::~UsefulSourcesWidget() = default;

void UsefulSourcesWidget::update()
{
    m_ui->textBrowser->setText(DataBuilder::buildUsefulSources());
}

void UsefulSourcesWidget::showEvent(QShowEvent *)
{
    update();
}
} // namespace alt
