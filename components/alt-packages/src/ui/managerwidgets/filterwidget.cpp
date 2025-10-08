#include "filterwidget.h"

#include "ui_filterwidget.h"

FilterWidget::FilterWidget(QWidget *parent)
    : QWidget(parent)
    , m_ui(std::make_unique<Ui::FilterWidget>())
{
    m_ui->setupUi(this);
    connect(m_ui->comboBox, &QComboBox::currentTextChanged, this, &FilterWidget::filterChanged);
}

FilterWidget::~FilterWidget() = default;

void FilterWidget::setName(const QString &text)
{
    m_ui->nameLabel->setText(QString("%1:").arg(text));
}

void FilterWidget::setOptions(const QStringList &options)
{
    m_ui->comboBox->clear();
    m_ui->comboBox->addItem(tr("All"));
    m_ui->comboBox->addItems(options);
}

void FilterWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        m_ui->comboBox->setItemData(0, tr("All"), Qt::DisplayRole);
    }

    QWidget::changeEvent(event);
}

void FilterWidget::onCurrentTextChanged(const QString &text)
{
    emit filterChanged(text);
}

bool FilterWidget::all(const QString &text)
{
    return text == tr("All");
}
