#include "serviceunregisteredwidget.h"
#include "ui_serviceunregistered.h"

#include <QPixmap>
#include <QStyle>

ServiceUnregisteredWidget::ServiceUnregisteredWidget(QWidget *parent)
    : ui(new Ui::ServiceUnregisteredWidget)
    , m_animationTimer(new QTimer())
    , m_counter(0)
    , m_buttonPressed(ButtonPressed::none)
{
    ui->setupUi(this);

    QIcon icon  = style()->standardIcon(QStyle::SP_MessageBoxCritical);
    QPixmap pix = icon.pixmap(30, 30);
    ui->iconLabel->setPixmap(icon.pixmap(30, 30));

    connect(m_animationTimer, &QTimer::timeout, this, &ServiceUnregisteredWidget::animationTick);
}

ServiceUnregisteredWidget::~ServiceUnregisteredWidget()
{
    m_animationTimer->stop();

    delete m_animationTimer;
    delete ui;
}

void ServiceUnregisteredWidget::show()
{
    m_buttonPressed = ButtonPressed::none;
    QWidget::show();
}

void ServiceUnregisteredWidget::startAnimation()
{
    m_animationTimer->setInterval(INTERVAL);
    m_animationTimer->start();
}

ServiceUnregisteredWidget::ButtonPressed ServiceUnregisteredWidget::getPressedButton()
{
    return m_buttonPressed;
}

void ServiceUnregisteredWidget::animationTick()
{
    if (m_counter < NUMBER_OF_CYCLES)
    {
        ui->textLabel->setText(ui->textLabel->text().append("."));
        m_counter++;
    }
    else
    {
        QString truncated = ui->textLabel->text();
        truncated.truncate(ui->textLabel->text().size() - NUMBER_OF_CYCLES);
        ui->textLabel->setText(truncated);
        m_counter = 0;
    }
}

void ServiceUnregisteredWidget::on_closeAndExitButton_clicked()
{
    m_buttonPressed = ButtonPressed::closeAndExit;
    emit closeAndExit();
}

void ServiceUnregisteredWidget::on_closeButton_clicked()
{
    m_buttonPressed = ButtonPressed::close;
    emit closeAll();
}
