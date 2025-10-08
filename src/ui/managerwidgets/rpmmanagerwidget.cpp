#include "rpmmanagerwidget.h"

#include "model/packagessortfilterproxymodel.h"
#include "ui_basemanagerwidget.h"

RpmManagerWidget::RpmManagerWidget(QWidget *parent)
    : BaseManagerWidget(parent)
    , m_controller(nullptr)
    , m_installPushButton(std::make_unique<QPushButton>())
    , m_removePushButton(std::make_unique<QPushButton>())
    , m_filesPushButton(std::make_unique<QPushButton>())
    , m_infoPushButton(std::make_unique<QPushButton>())
    , m_groupFilterWidget(std::make_unique<FilterWidget>())
    , m_archFilterWidget(std::make_unique<FilterWidget>())
{
    retranslateChildWidgets();
    setButtonsEnabled(false);
    m_ui->buttonsHorizontalLayout->addWidget(m_installPushButton.get());
    m_ui->buttonsHorizontalLayout->addWidget(m_removePushButton.get());
    m_ui->buttonsHorizontalLayout->addWidget(m_filesPushButton.get());
    m_ui->buttonsHorizontalLayout->addWidget(m_infoPushButton.get());

    auto header = getHorizontalHeader();
    QObject::connect(header, &QHeaderView::sectionCountChanged, this, &RpmManagerWidget::onSectionCountChanged);

    QObject::connect(m_removePushButton.get(),
                     &QPushButton::clicked,
                     this,
                     &RpmManagerWidget::onRemovePackagesRequested);
    QObject::connect(m_infoPushButton.get(),
                     &QPushButton::clicked,
                     this,
                     &RpmManagerWidget::onInfoAboutPackagesRequested);
    QObject::connect(m_filesPushButton.get(),
                     &QPushButton::clicked,
                     this,
                     &RpmManagerWidget::onFilesOfPackagesRequested);
    QObject::connect(m_groupFilterWidget.get(),
                     &FilterWidget::filterChanged,
                     this,
                     &RpmManagerWidget::onFilterGroupChanged);
    QObject::connect(m_archFilterWidget.get(),
                     &FilterWidget::filterChanged,
                     this,
                     &RpmManagerWidget::onFilterArchChanged);

    m_ui->filtersContainerHorizontalLayout->addWidget(m_groupFilterWidget.get());
    m_ui->filtersContainerHorizontalLayout->addWidget(m_archFilterWidget.get());
}

RpmManagerWidget::~RpmManagerWidget() = default;

void RpmManagerWidget::connect(RpmController *controller)
{
    setModel(controller->getModel());

    QObject::connect(m_installPushButton.get(),
                     &QPushButton::clicked,
                     controller,
                     &RpmController::onInstallPackageRequested);
    QObject::connect(this,
                     &RpmManagerWidget::requestRemovePackages,
                     controller,
                     &RpmController::onRemovePackagesRequested);
    QObject::connect(this,
                     &RpmManagerWidget::requestInfoForPackages,
                     controller,
                     &RpmController::onInfoAboutPackagesRequested);
    QObject::connect(this,
                     &RpmManagerWidget::requestFilesForPackages,
                     controller,
                     &RpmController::onFilesOfPackagesRequested);
    QObject::connect(getSelectionModel(),
                     &QItemSelectionModel::selectionChanged,
                     this,
                     &RpmManagerWidget::onSelectionChanged);

    QObject::connect(controller, &RpmController::progressChanged, this, &RpmManagerWidget::onProgressChanged);
    QObject::connect(controller, &RpmController::wait, this, &RpmManagerWidget::onWait);
    QObject::connect(controller, &RpmController::packagesListChanged, this, &RpmManagerWidget::onPackagesListChanged);

    m_groupFilterWidget->setOptions(controller->groupsOfAllPackages());
    m_archFilterWidget->setOptions(controller->archesOfAllPackages());

    m_controller = controller;
}

void RpmManagerWidget::setButtonsEnabled(bool isEnabled)
{
    m_filesPushButton->setEnabled(isEnabled);
    m_removePushButton->setEnabled(isEnabled);
    m_infoPushButton->setEnabled(isEnabled);
}

void RpmManagerWidget::onSectionCountChanged(int oldCount, int newCount)
{
    std::ignore = std::tie(oldCount, newCount);
    getHorizontalHeader()->hideSection(columnPackage);
}

void RpmManagerWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        retranslateChildWidgets();
    }

    BaseManagerWidget::changeEvent(event);
    m_groupFilterWidget->changeEvent(event);
    m_archFilterWidget->changeEvent(event);
}

void RpmManagerWidget::onRemovePackagesRequested()
{
    emit requestRemovePackages(getSelection());
}

void RpmManagerWidget::onFilesOfPackagesRequested()
{
    emit requestFilesForPackages(getSelection());
}

void RpmManagerWidget::onInfoAboutPackagesRequested()
{
    emit requestInfoForPackages(getSelection());
}

void RpmManagerWidget::onPackagesListChanged()
{
    m_groupFilterWidget->setOptions(m_controller->groupsOfAllPackages());
    m_archFilterWidget->setOptions(m_controller->archesOfAllPackages());
}

void RpmManagerWidget::onFilterGroupChanged(const QString &text)
{
    onFilterChanged(&PackagesFilterProxyModel::setFilterGroup, m_groupFilterWidget->all(text) ? "" : text);
}

void RpmManagerWidget::onFilterArchChanged(const QString &text)
{
    onFilterChanged(&PackagesFilterProxyModel::setFilterArch, m_archFilterWidget->all(text) ? "" : text);
}

void RpmManagerWidget::retranslateChildWidgets()
{
    m_installPushButton->setText(tr("Install from File"));
    m_removePushButton->setText(tr("Remove"));
    m_filesPushButton->setText(tr("Files"));
    m_infoPushButton->setText(tr("Info"));
    m_groupFilterWidget->setName(tr("Group"));
    m_archFilterWidget->setName(tr("Arch"));
}
