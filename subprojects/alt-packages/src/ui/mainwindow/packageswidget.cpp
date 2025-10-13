#include "packageswidget.h"

#include "constants.h"
#include "ui_packageswidget.h"

#include <QPushButton>
#include <QStandardItem>

PackagesWidget::PackagesWidget(QWidget *parent)
    : QWidget(parent)
    , m_ui(std::make_unique<Ui::PackagesWidget>())
{
    m_ui->setupUi(this);
    QObject::connect(m_ui->managersTabWidget, &QTabWidget::currentChanged, this, &PackagesWidget::onManagerChanged);
}

PackagesWidget::~PackagesWidget() = default;

void PackagesWidget::connect(AptController *controller)
{
    m_ui->aptManagerWidget->connect(controller);
    QObject::connect(this, &PackagesWidget::aptSelected, controller, &AptController::onSelected);
}

void PackagesWidget::connect(RpmController *controller)
{
    m_ui->rpmManagerWidget->connect(controller);
}

void PackagesWidget::connect(RepoController *controller)
{
    m_ui->repoManagerWidget->connect(controller);
}

void PackagesWidget::setActive(const QString &objectPath)
{
    if (objectPath == APT_OBJECT_PATH)
    {
        m_ui->managersTabWidget->setCurrentWidget(m_ui->aptTab);
        emit aptSelected();
    }
    else if (objectPath == RPM_OBJECT_PATH)
    {
        m_ui->managersTabWidget->setCurrentWidget(m_ui->rpmTab);
    }
    else if (objectPath == REPO_OBJECT_PATH)
    {
        m_ui->managersTabWidget->setCurrentWidget(m_ui->repoTab);
    }
}

void PackagesWidget::onManagerChanged(int index)
{
    if (m_ui->managersTabWidget->widget(index) == m_ui->aptTab)
    {
        emit aptSelected();
    }
}

void PackagesWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        m_ui->retranslateUi(this);
    }

    QWidget::changeEvent(event);
}
