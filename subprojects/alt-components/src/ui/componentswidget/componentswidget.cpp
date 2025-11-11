#include "componentswidget.h"
#include "application.h"
#include "controller/controller.h"
#include "ui_componentswidget.h"

#include <memory>
#include <QAbstractItemModel>
#include <QClipboard>
#include <QFile>
#include <QMenu>
#include <QTextDocument>

namespace alt
{
ComponentsWidget::ComponentsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(std::make_unique<Ui::ComponentsWidget>())
    , treeContextMenu(std::make_unique<QMenu>(this))
    , packagesContextMenu(std::make_unique<QMenu>(this))
    , findShortcut(std::make_unique<QShortcut>(QKeySequence::Find, this))
{
    ui->setupUi(this);
    ui->splitter->setStretchFactor(0, 3);
    ui->splitter->setStretchFactor(1, 5);
    ui->splitter->setStretchFactor(2, 3);
    ui->componentsTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->componentsTreeView->setHeaderHidden(true);
    ui->descriptionTextBrowser->setOpenExternalLinks(true);

    // Load CSS stylesheet for description formatting
    loadDescriptionStyleSheet();

    connect(findShortcut.get(), &QShortcut::activated, [this]() {
        ui->searchEdit->setFocus();
        ui->searchEdit->selectAll();
    });

    setupTreeCustomContextMenu();
    setupPackagesContextMenu();
    setDefaultDescription();
    ui->contentListView->setFont(QFont());
}

ComponentsWidget::~ComponentsWidget() = default;

void ComponentsWidget::setDefaultDescription()
{
    const auto setDescriptionWithLocale = [this](const QString &locale) -> bool {
        const auto &resource = QString(":welcome-screen.%1.html").arg(locale);
        QFile file(resource);
        if (file.open(QIODevice::ReadOnly))
        {
            ui->descriptionTextBrowser->setHtml(file.readAll());
            return true;
        }
        return false;
    };

    const auto language = Application::getLocale().name().split("_")[0];
    setDescriptionWithLocale(language) || setDescriptionWithLocale("en");
}

void ComponentsWidget::setupTreeCustomContextMenu()
{
    auto collapseAllAction = std::make_unique<QAction>(tr("Collapse all"), this);
    connect(collapseAllAction.get(), &QAction::triggered, [&]() {
        ui->componentsTreeView->collapseAll();
        if (Controller::instance().getViewMode() == MainWindow::ViewMode::BySections
            || Controller::instance().getViewMode() == MainWindow::ViewMode::ByTags)
        {
            expandTopLevel();
        }
    });
    treeContextMenu->addAction(collapseAllAction.release());

    auto expandAllAction = std::make_unique<QAction>(tr("Expand all"), this);
    connect(expandAllAction.get(), &QAction::triggered, ui->componentsTreeView, &QTreeView::expandAll);
    treeContextMenu->addAction(expandAllAction.release());

    connect(ui->componentsTreeView,
            &QTreeView::customContextMenuRequested,
            this,
            &ComponentsWidget::treeCustomContextMenu);
}

void ComponentsWidget::setupPackagesContextMenu()
{
    auto copyAllAction = std::make_unique<QAction>(tr("Copy all"), this);
    connect(copyAllAction.get(), &QAction::triggered, this, &ComponentsWidget::onCopyAllPackages);
    packagesContextMenu->addAction(copyAllAction.release());

    connect(ui->contentListView,
            &QListView::customContextMenuRequested,
            this,
            &ComponentsWidget::packagesCustomContextMenu);
}

void ComponentsWidget::onCopyAllPackages()
{
    auto *const model = this->ui->contentListView->model();
    if (model == nullptr)
    {
        return;
    }

    QStringList packages = {};
    for (int i = 0; i < model->rowCount(); i++)
    {
        packages.append(model->data(model->index(i, 0)).toString());
    }

    Application::clipboard()->setText(packages.join(" "));
}

void ComponentsWidget::setComponentsModel(QAbstractItemModel *model)
{
    ui->componentsTreeView->setModel(model);
    ui->componentsTreeView->setSortingEnabled(true);

    connect(ui->componentsTreeView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &ComponentsWidget::onSelectionChanged);
    connect(ui->contentListView, &QAbstractItemView::clicked, [this, model](const QModelIndex &index) {
        if (index.model() == model)
        {
            ui->componentsTreeView->selectionModel()->select(index, QItemSelectionModel::SelectionFlag::ClearAndSelect);
        }
    });

    connect(ui->searchEdit, &QLineEdit::textChanged, this, &ComponentsWidget::onSearchTextChanged);
}

void ComponentsWidget::setVisibleContent(bool visible)
{
    if (visible)
        ui->gridLayoutWidget_3->show();
    else
        ui->gridLayoutWidget_3->hide();
}

void ComponentsWidget::onSelectionChanged(const QItemSelection &newSelection,
                                          const QItemSelection & /* previousSelection */)
{
    if (newSelection.isEmpty())
    {
        return;
    }

    QModelIndex index = newSelection.indexes().at(0);
    ui->descriptionTextBrowser->setHtml(Controller::instance().getDescription(index));
    const auto &[type, model] = Controller::instance().getContent(index);
    ui->contentListView->setModel(model);
    if (type == ModelItem::Type::Component)
    {
        ui->packagesLabel->setText(tr("Packages:"));
    }
    else
    {
        ui->packagesLabel->setText(tr("Content:"));
        ui->contentListView->setRootIndex(index);
    }

    for (auto it = index.parent(); it.isValid(); it = it.parent())
    {
        ui->componentsTreeView->expand(it);
    }
}

void ComponentsWidget::onSearchTextChanged(const QString &query)
{
    Controller::instance().setTextFilter(query);
}

void ComponentsWidget::updateDescription()
{
    const auto &selectedIndexes = ui->componentsTreeView->selectionModel()->selectedIndexes();
    if (selectedIndexes.size() != 0)
    {
        onSelectionChanged(ui->componentsTreeView->selectionModel()->selection(), {});
    }
    else
    {
        setDefaultDescription();
    }
}

void ComponentsWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        retranslateContextMenus();
        updateDescription();
    }

    QWidget::changeEvent(event);
}

void ComponentsWidget::treeCustomContextMenu(const QPoint &pos)
{
    treeContextMenu->popup(ui->componentsTreeView->viewport()->mapToGlobal(pos));
}

void ComponentsWidget::packagesCustomContextMenu(const QPoint &pos)
{
    packagesContextMenu->popup(ui->contentListView->viewport()->mapToGlobal(pos));
}

void ComponentsWidget::retranslateContextMenus()
{
    // TODO(chernigin): I don't believe we actually need to recreate context
    // menus for them to be translated. It's dumb. We need to find better way
    // to translate actions inside those menus at runtime.

    treeContextMenu->clear();
    packagesContextMenu->clear();

    setupTreeCustomContextMenu();
    setupPackagesContextMenu();
}

void ComponentsWidget::expandTopLevel()
{
    if (const auto *model = ui->componentsTreeView->model())
    {
        for (int i = 0; i < model->rowCount(); ++i)
        {
            ui->componentsTreeView->expand(model->index(i, 0));
        }
    }
}

void ComponentsWidget::loadDescriptionStyleSheet()
{
    QFile cssFile(":description.css");
    if (cssFile.open(QIODevice::ReadOnly))
    {
        QString styleSheet = QString::fromUtf8(cssFile.readAll());
        ui->descriptionTextBrowser->document()->setDefaultStyleSheet(styleSheet);
    }
}

} // namespace alt
