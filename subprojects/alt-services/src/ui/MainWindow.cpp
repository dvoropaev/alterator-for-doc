#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ServiceWidget.h"
#include "AboutDialog.h"
#include "wizard/ActionWizard.h"

#include "controller/Controller.h"
#include "app/DropEventFilter.h"

#include <QDropEvent>
#include <QMimeData>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>

#include <QActionGroup>

class MainWindow::Private {
public:
    explicit Private(MainWindow* w)
        : self{w}
        , m_about{w}
        , m_wizard{w}
    {}


    std::map<QAction*, Parameter::Context> m_actionToContext;

    inline Parameter::Context actionToContext(QAction* action)
    {
        return m_actionToContext.at(action);
    }

    void setActionContext(QAction* action, Parameter::Context ctx)
    {
        m_actionToContext[action] = ctx;
        action->setIcon(Controller::actionIcon(ctx));
        connect(action, &QAction::triggered, self, &MainWindow::onContextActionTriggered);
    }

    void importParameters(const QString& filename) {
        if ( auto playfile = Action::importFromFile(filename) )
        {
            qApp->controller()->selectByPath(playfile->service->dbusPath());
            m_wizard.open(playfile.value());
        }
    }

    MainWindow* self{};
    Ui::MainWindow ui;
    Service* m_currentService{nullptr};
    QSortFilterProxyModel m_filter_model;
    AboutDialog m_about;
    ActionWizard m_wizard;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d{ new Private{this} }
{
    d->ui.setupUi(this);
    installEventFilter(dropEventFilter);

    d->ui.searchBar->addAction(QIcon::fromTheme(QIcon::ThemeIcon::EditFind), QLineEdit::LeadingPosition)
        ->setDisabled(true);


    connect(d->ui.treeView, &QAbstractItemView::clicked, d->ui.treeView, &QAbstractItemView::activated);

    {
        d->ui.actionRefreshCurrent ->setIcon(QIcon::fromTheme("view-refresh"));
        d->ui.actionRefreshAll     ->setIcon(QIcon::fromTheme("view-refresh"));
        d->ui.actionImport         ->setIcon(QIcon::fromTheme("document-open"));
        d->ui.actionQuit           ->setIcon(QIcon::fromTheme("application-exit"));
        d->ui.actionStart          ->setIcon(QIcon::fromTheme("media-playback-start"));
        d->ui.actionStop           ->setIcon(QIcon::fromTheme("media-playback-stop"));
    }

    d->setActionContext(d->ui.actionDiag      , Parameter::Context::Diag      );
    d->setActionContext(d->ui.actionDeploy    , Parameter::Context::Deploy    );
    d->setActionContext(d->ui.actionConfigure , Parameter::Context::Configure );
    d->setActionContext(d->ui.actionUndeploy  , Parameter::Context::Undeploy  );
    d->setActionContext(d->ui.actionBackup    , Parameter::Context::Backup    );
    d->setActionContext(d->ui.actionRestore   , Parameter::Context::Restore   );


    { // toolbar style
        d->ui.menuView->addSection(tr("ToolBar"));
        d->ui.toolBar->toggleViewAction()->setText(tr("Enable ToolBar"));
        d->ui.menuView->addAction(d->ui.toolBar->toggleViewAction());

        QMenu* toolIcon = d->ui.menuView->addMenu(tr("Buttons style"));
        auto group = new QActionGroup{toolIcon};

        group->setExclusive(true);

        auto savedStyle = qApp->settings()->toolButtonStyle();

        d->ui.toolBar->setToolButtonStyle(savedStyle);

#define TOOLSTYLE(text, style) { \
    auto* action = toolIcon->addAction(text); \
    action->setData(style); \
    action->setActionGroup(group); \
    action->setCheckable(true); \
    action->setChecked(style == savedStyle); }

        TOOLSTYLE(tr("Icons only"),         Qt::ToolButtonIconOnly      )
        TOOLSTYLE(tr("Text only"),          Qt::ToolButtonTextOnly      )
        TOOLSTYLE(tr("Text beside icon"),   Qt::ToolButtonTextBesideIcon)
        TOOLSTYLE(tr("Text under icon"),    Qt::ToolButtonTextUnderIcon )

#undef TOOLSTYLE

        connect(group, &QActionGroup::triggered, this, [this](QAction* a){
            auto style = (Qt::ToolButtonStyle)a->data().toInt();
            d->ui.toolBar->setToolButtonStyle(style);
            qApp->settings()->set_toolButtonStyle(style);
        });
    }

    d->ui.menuView->addActions(qApp->controller()->tableActions());

    d->ui.actionRefreshCurrent->setDisabled(true);

    d->ui.treeView->insertAction(0, d->ui.actionRefreshAll);
    d->ui.treeView->header()->setStretchLastSection(false);

    connect(d->ui.actionQuit,  &QAction::triggered, QApplication::instance(), &QApplication::quit);
    connect(d->ui.actionAbout, &QAction::triggered, &d->m_about, &QDialog::open);

    restoreState(qApp->settings()->mainWindowState());


    d->m_filter_model.setSourceModel(qApp->controller()->model());
    d->ui.treeView->setModel(&d->m_filter_model);
    d->ui.treeView->setColumnNeverDetailed(1, true);

    connect( qApp->controller(), &Controller::beginRefresh, this, &MainWindow::onRefreshStart );
    connect( qApp->controller(), &Controller::endRefresh,   this, &MainWindow::onRefreshEnd   );
    connect( qApp->controller(), &Controller::select, this, [this](int row){
        auto index = d->m_filter_model.mapFromSource(d->m_filter_model.sourceModel()->index(row,0));
        d->ui.treeView->selectionModel()->select(index, QItemSelectionModel::SelectionFlag::Rows | QItemSelectionModel::SelectionFlag::ClearAndSelect );
        on_treeView_activated(index);
    });

    connect(d->ui.actionRefreshAll, &QAction::triggered, qApp->controller(), &Controller::refresh);

    connect(qApp->controller()->model(), &QAbstractItemModel::modelAboutToBeReset, this, [this]{
        d->m_currentService = nullptr;
        d->ui.stackedWidget->setCurrentWidget(d->ui.initialPage);
        d->ui.actionRefreshCurrent->setEnabled(false);
        d->ui.serviceWidget->clear();
    });

    connect(qApp->controller()->model(), &QAbstractItemModel::dataChanged, this,
    [=](const QModelIndex& tl, const QModelIndex& br, const QList<int>& roles)
    {
        if ( d->m_currentService )
            for ( int row = tl.row(); row <= br.row(); ++row )
                if ( d->m_currentService == qApp->controller()->getByIndex(qApp->controller()->model()->index(row, 0)) )
                    d->ui.serviceWidget->onStatusChanged();
    });
}

MainWindow::~MainWindow() { delete d; }

void MainWindow::onRefreshStart()
{
    setCursor(QCursor{Qt::WaitCursor});
    d->ui.progressBar->show();
    d->ui.actionRefreshAll->setDisabled(true);
    d->ui.serviceWidget->setEnabled(false);

    for ( const auto& [action,ctx] : d->m_actionToContext )
        action->setEnabled(false);

    d->ui.treeView->setEnabled(false);
}

void MainWindow::onRefreshEnd()
{
    d->m_filter_model.invalidate();
    unsetCursor();
    d->ui.serviceWidget->setEnabled(true);
    d->ui.treeView->setEnabled(true);
    d->ui.progressBar->hide();
    d->ui.treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->ui.treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    d->ui.treeView->header()->setSectionResizeMode(1, QHeaderView::ResizeMode::Fixed);
    d->ui.treeView->header()->resizeSection(1, iconSize().width());
    d->ui.actionRefreshAll->setDisabled(false);

    if ( d->ui.stackedWidget->currentWidget() == d->ui.serviceWidget )
        d->ui.serviceWidget->onStatusChanged();

    onStatusChanged();
}

void MainWindow::onStatusChanged()
{
    d->ui.actionRefreshCurrent ->setEnabled(d->m_currentService);

#define ENABLE(action, condition) \
    action->setEnabled(d->m_currentService && (condition))

    ENABLE( d->ui.actionDiag      , !d->m_currentService->diagTools().empty() );
    ENABLE( d->ui.actionDeploy    , !d->m_currentService->isDeployed() || d->m_currentService->isForceDeployable() );
    ENABLE( d->ui.actionConfigure ,  d->m_currentService->isDeployed() );
    ENABLE( d->ui.actionUndeploy  ,  d->m_currentService->isDeployed() );
    ENABLE( d->ui.actionBackup    ,  d->m_currentService->isDeployed() );
    ENABLE( d->ui.actionRestore   ,  d->m_currentService->isDeployed() );
    ENABLE( d->ui.actionStart     ,  d->m_currentService->isDeployed() && !d->m_currentService->isStarted() );
    ENABLE( d->ui.actionStop      ,  d->m_currentService->isDeployed() &&  d->m_currentService->isStarted() );

#undef ENABLE
}


void MainWindow::on_treeView_activated(const QModelIndex &index)
{
    auto srcIndex = d->m_filter_model.mapToSource(index);
    if ( auto* service = qApp->controller()->getByIndex(srcIndex) )
    {
        d->m_currentService = service;
        d->ui.stackedWidget->setCurrentWidget(d->ui.serviceWidget);
        d->ui.serviceWidget->setService(service);
        onStatusChanged();
    }
    else
        qWarning() << "internal error: cannot get service by index";
}


void MainWindow::on_searchBar_textChanged(const QString &arg1)
{
    d->m_filter_model.setFilterFixedString(arg1);
}

void MainWindow::dropEvent(QDropEvent* event)
{
    d->importParameters(event->mimeData()->urls().at(0).toLocalFile());
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    qApp->settings()->set_mainWindowState(saveState());
    QMainWindow::closeEvent(event);
}

void MainWindow::on_actionImport_triggered()
{
    auto fileName = QFileDialog::getOpenFileName(this,
        tr("Import saved parameters"),
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
        "JSON files (*.json)"
    );

    if ( !fileName.isEmpty() )
        d->importParameters(fileName);
}


void MainWindow::on_actionRefreshCurrent_triggered()
{
    d->ui.actionRefreshAll->setDisabled(true);
    d->ui.actionRefreshCurrent->setDisabled(true);
    qApp->controller()->updateStatus(d->m_currentService);
    onStatusChanged();
    d->ui.actionRefreshAll->setDisabled(false);
    d->ui.actionRefreshCurrent->setDisabled(false);
}

void MainWindow::on_actionStart_triggered()
{
    qApp->controller()->start(d->m_currentService);
    d->ui.serviceWidget->onStatusChanged();
    onStatusChanged();
}

void MainWindow::on_actionStop_triggered()
{
    qApp->controller()->stop(d->m_currentService);
    d->ui.serviceWidget->onStatusChanged();
    onStatusChanged();
}

void MainWindow::onContextActionTriggered()
{
    if ( QAction* source = qobject_cast<QAction*>(QObject::sender()) ) try {
        auto ctx = d->actionToContext(source);
        Action action;
        action.service = d->m_currentService;
        action.action = ctx;

        // NOTE: diagnostics should be enabled by default
        action.options.prediag = action.preDiagAvailable() ||
            ( ctx == Parameter::Context::Diag && !action.service->isDeployed() );

        action.options.postdiag = action.postDiagAvailable() ||
            ( ctx == Parameter::Context::Diag && action.service->isDeployed() );

        d->m_wizard.open(action);
        return;
    } catch(std::out_of_range&) {}


    qCritical() << "Internal error: The triggered action does not have a context";
}
