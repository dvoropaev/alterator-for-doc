#include "repomanagerwidget.h"
#include "ui_basemanagerwidget.h"

RepoManagerWidget::RepoManagerWidget(QWidget *parent)
    : BaseManagerWidget(parent)
    , m_addPushButton(std::make_unique<QPushButton>())
    , m_removePushButton(std::make_unique<QPushButton>())
    , m_addShortcut(std::make_unique<QShortcut>(QKeySequence(Qt::CTRL | Qt::Key_N), this))
    , m_rmShortcut(std::make_unique<QShortcut>(QKeySequence(Qt::CTRL | Qt::Key_D), this))
{
    retranslateButtons();
    setButtonsEnabled(false);
    m_ui->buttonsHorizontalLayout->addWidget(m_addPushButton.get());
    m_ui->buttonsHorizontalLayout->addWidget(m_removePushButton.get());
    auto header = getHorizontalHeader();
    QObject::connect(header, &QHeaderView::sectionCountChanged, this, &RepoManagerWidget::onSectionCountChanged);
    QObject::connect(m_rmShortcut.get(), &QShortcut::activated, this, &RepoManagerWidget::onRemoveRepoRequested);
    QObject::connect(m_removePushButton.get(), &QPushButton::clicked, this, &RepoManagerWidget::onRemoveRepoRequested);

    m_ui->filterLabel->hide();
}

RepoManagerWidget::~RepoManagerWidget() = default;

void RepoManagerWidget::connect(RepoController *controller)
{
    setModel(controller->getModel());

    QObject::connect(m_addShortcut.get(), &QShortcut::activated, controller, &RepoController::onAddRepoRequested);
    QObject::connect(m_addPushButton.get(), &QPushButton::clicked, controller, &RepoController::onAddRepoRequested);
    QObject::connect(this, &RepoManagerWidget::requestRemoveRepos, controller, &RepoController::onRemoveRepoRequested);
    QObject::connect(getSelectionModel(),
                     &QItemSelectionModel::selectionChanged,
                     this,
                     &RepoManagerWidget::onSelectionChanged);

    QObject::connect(controller, &RepoController::progressChanged, this, &RepoManagerWidget::onProgressChanged);
    QObject::connect(controller, &RepoController::wait, this, &RepoManagerWidget::onWait);
}

void RepoManagerWidget::onSectionCountChanged(int oldCount, int newCount)
{
    std::ignore = std::tie(oldCount, newCount);
    getHorizontalHeader()->hideSection(columnRepo);
}

void RepoManagerWidget::onRemoveRepoRequested()
{
    emit requestRemoveRepos(getSelection());
}

void RepoManagerWidget::setButtonsEnabled(bool isEnable)
{
    m_removePushButton->setEnabled(isEnable);
}

void RepoManagerWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        retranslateButtons();
    }

    BaseManagerWidget::changeEvent(event);
}

void RepoManagerWidget::retranslateButtons()
{
    m_addPushButton->setText(tr("Add a repo"));
    m_removePushButton->setText(tr("Remove repo"));
}
