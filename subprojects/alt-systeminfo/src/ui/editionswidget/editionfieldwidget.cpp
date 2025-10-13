#include "editionfieldwidget.h"

#include "ui_editionfieldwidget.h"

namespace alt
{
EditionFieldWidget::EditionFieldWidget(QWidget *parent)
    : QWidget(parent)
    , m_ui(std::make_unique<Ui::EditionFieldWidget>())
{
    m_ui->setupUi(this);
    connect(m_ui->editToolButton, &QToolButton::clicked, [this]() { emit requestEdit(); });
}

void EditionFieldWidget::setText(const QString &text)
{
    m_ui->fieldValueLabel->setText(text);
}

EditionFieldWidget::~EditionFieldWidget() = default;
} // namespace alt
