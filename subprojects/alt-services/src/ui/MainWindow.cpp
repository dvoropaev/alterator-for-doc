#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "ServiceWidget.h"
#include "AboutDialog.h"
#include "ActionWizard.h"

#include "controller/Controller.h"

#include <QDropEvent>
#include <QMimeData>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>

#include <QActionGroup>

class MainWindow::Private {
    std::map<QAction*, Parameter::Context> m_actionToContext;

public:
    Private(MainWindow* w)
        : ui(new Ui::MainWindow)
        , self{w}
        , m_about{w}
        , m_wizard{w}
    {}

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

    ~Private(){delete ui;}

    void importParameters(const QString& filename) {
        if ( auto playfile = ServicesApp::instance()->importParameters(filename) )
        {
            qApp->controller()->selectByPath(playfile->service->dbusPath());
            m_wizard.readParameters(playfile.value());
        }
    }

    MainWindow* self{};
    Ui::MainWindow *ui;
    Service* m_currentService{nullptr};
    ServiceWidget* m_currentWidget{nullptr};
    QSortFilterProxyModel m_filter_model;
    AboutDialog m_about;
    ActionWizard m_wizard;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d{ new Private{this} }
{
    d->ui->setupUi(this);
    d->ui->centralwidget->setEnabled(false);
    d->ui->progressBar->hide();
    connect(d->ui->treeView, &QAbstractItemView::clicked, d->ui->treeView, &QAbstractItemView::activated);

    {
        d->ui->actionRefreshCurrent ->setIcon(QIcon::fromTheme("view-refresh"));
        d->ui->actionRefreshAll     ->setIcon(QIcon::fromTheme("view-refresh"));
        d->ui->actionImport         ->setIcon(QIcon::fromTheme("document-open"));
        d->ui->actionQuit           ->setIcon(QIcon::fromTheme("application-exit"));
        d->ui->actionStart          ->setIcon(QIcon::fromTheme("media-playback-start"));
        d->ui->actionStop           ->setIcon(QIcon::fromTheme("media-playback-stop"));
    }

    d->setActionContext(d->ui->actionDiag      , Parameter::Context::Diag      );
    d->setActionContext(d->ui->actionDeploy    , Parameter::Context::Deploy    );
    d->setActionContext(d->ui->actionConfigure , Parameter::Context::Configure );
    d->setActionContext(d->ui->actionUndeploy  , Parameter::Context::Undeploy  );
    d->setActionContext(d->ui->actionBackup    , Parameter::Context::Backup    );
    d->setActionContext(d->ui->actionRestore   , Parameter::Context::Restore   );

    d->ui->menuView->addSection(tr("ToolBar"));
    d->ui->toolBar->toggleViewAction()->setText(tr("Enable ToolBar"));
    d->ui->menuView->addAction(d->ui->toolBar->toggleViewAction());

    QMenu* toolIcon = d->ui->menuView->addMenu(tr("Buttons style"));
    auto group = new QActionGroup{toolIcon};

    group->setExclusive(true);

    auto savedStyle = ServicesApp::instance()->settings()->toolButtonStyle();

    d->ui->toolBar->setToolButtonStyle(savedStyle);

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
        d->ui->toolBar->setToolButtonStyle(style);
        ServicesApp::instance()->settings()->set_toolButtonStyle(style);
    });

    d->ui->actionRefreshCurrent->setDisabled(true);

    d->ui->treeView->insertAction(0, d->ui->actionRefreshAll);
    d->ui->treeView->header()->setStretchLastSection(false);

    connect(d->ui->actionQuit,  &QAction::triggered, QApplication::instance(), &QApplication::quit);
    connect(d->ui->actionAbout, &QAction::triggered, &d->m_about, &QDialog::open);

    restoreState(ServicesApp::instance()->settings()->mainWindowState());


    d->m_filter_model.setSourceModel(qApp->controller()->model());
    d->ui->treeView->setModel(&d->m_filter_model);
    d->ui->treeView->setColumnNeverDetailed(1, true);

    connect( qApp->controller(), &Controller::beginRefresh, this, &MainWindow::onRefreshStart );
    connect( qApp->controller(), &Controller::endRefresh,   this, &MainWindow::onRefreshEnd   );
    connect( qApp->controller(), &Controller::select, this, [this](int row){
        auto index = d->m_filter_model.mapFromSource(d->m_filter_model.sourceModel()->index(row,0));
        d->ui->treeView->selectionModel()->select(index, QItemSelectionModel::SelectionFlag::Rows | QItemSelectionModel::SelectionFlag::ClearAndSelect );
        on_treeView_activated(index);
    });

    connect(d->ui->actionRefreshAll, &QAction::triggered, qApp->controller(), &Controller::refresh);

    connect(qApp->controller()->model(), &QAbstractItemModel::modelAboutToBeReset, this, [this]{
        d->m_currentService = nullptr;
        d->m_currentWidget  = nullptr;
        d->ui->stackedWidget->setCurrentIndex(0);
        d->ui->actionRefreshCurrent->setEnabled(false);

        while ( d->ui->stackedWidget->count() > 1 ) {
            auto* widget = d->ui->stackedWidget->widget(1);
            d->ui->stackedWidget->removeWidget(widget);
            delete widget;
        }
    });

    connect(qApp->controller()->model(), &QAbstractItemModel::rowsInserted, this, [=](const QModelIndex& parent, int first, int last){
        for ( int i = first; i <= last; ++i ) {
            auto index = qApp->controller()->model()->index(i, 0, parent);

            if ( auto* service = qApp->controller()->getByIndex(index) )
                d->ui->stackedWidget->insertWidget( index.row()+1, new ServiceWidget{ service, d->ui->stackedWidget } );
        }
    });

    connect(qApp->controller()->model(), &QAbstractItemModel::dataChanged, this, [=](const QModelIndex& tl, const QModelIndex& br, QList<int> roles){
        for ( int row = tl.row(); row <= br.row(); ++row )
            ((ServiceWidget*)d->ui->stackedWidget->widget(row+1))->onStatusChanged();
    });
}

MainWindow::~MainWindow() { delete d; }

void MainWindow::onRefreshStart()
{
    setCursor(QCursor{Qt::WaitCursor});
    d->ui->centralwidget->setEnabled(false);
    d->ui->progressBar->show();
    d->ui->actionRefreshAll->setDisabled(true);
}

void MainWindow::onRefreshEnd()
{
    d->m_filter_model.invalidate();
    unsetCursor();
    d->ui->centralwidget->setEnabled(true);
    d->ui->progressBar->hide();
    d->ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    d->ui->treeView->header()->setSectionResizeMode(1, QHeaderView::ResizeMode::Fixed);
    d->ui->treeView->header()->resizeSection(1, iconSize().width());
    d->ui->actionRefreshAll->setDisabled(false);

    if ( d->m_currentWidget )
        d->m_currentWidget->onStatusChanged();

    onStatusChanged();
}

void MainWindow::onStatusChanged()
{
    d->ui->actionRefreshCurrent ->setEnabled(d->m_currentService);

#define ENABLE(action, condition) \
    action->setEnabled(d->m_currentService && (condition))

    ENABLE( d->ui->actionDiag      , !d->m_currentService->diagTools().empty() );
    ENABLE( d->ui->actionDeploy    , !d->m_currentService->isDeployed() || d->m_currentService->isForceDeployable() );
    ENABLE( d->ui->actionConfigure ,  d->m_currentService->isDeployed() );
    ENABLE( d->ui->actionUndeploy  ,  d->m_currentService->isDeployed() );
    ENABLE( d->ui->actionBackup    ,  d->m_currentService->isDeployed() );
    ENABLE( d->ui->actionRestore   ,  d->m_currentService->isDeployed() );
    ENABLE( d->ui->actionStart     ,  d->m_currentService->isDeployed() && !d->m_currentService->isStarted() );
    ENABLE( d->ui->actionStop      ,  d->m_currentService->isDeployed() &&  d->m_currentService->isStarted() );

#undef ENABLE
}


void MainWindow::on_treeView_activated(const QModelIndex &index)
{
    auto srcIndex = d->m_filter_model.mapToSource(index);
    d->ui->stackedWidget->setCurrentIndex(srcIndex.row()+1);
    d->m_currentService = qApp->controller()->getByIndex(srcIndex);
    d->m_currentWidget  = (ServiceWidget*)d->ui->stackedWidget->currentWidget();
    onStatusChanged();
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
    ServicesApp::instance()->settings()->set_mainWindowState(saveState());
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
    d->ui->actionRefreshAll->setDisabled(true);
    d->ui->actionRefreshCurrent->setDisabled(true);
    d->m_currentWidget->onStatusChanged();
    onStatusChanged();
    d->ui->actionRefreshAll->setDisabled(false);
    d->ui->actionRefreshCurrent->setDisabled(false);
}

void MainWindow::on_actionStart_triggered()
{
    qApp->controller()->start(d->m_currentService);
    d->m_currentWidget->onStatusChanged();
    onStatusChanged();
}

void MainWindow::on_actionStop_triggered()
{
    qApp->controller()->stop(d->m_currentService);
    d->m_currentWidget->onStatusChanged();
    onStatusChanged();
}

void MainWindow::onContextActionTriggered()
{
    if ( QAction* source = qobject_cast<QAction*>(QObject::sender()) ) try {
        auto ctx = d->actionToContext(source);
        d->m_wizard.open(ctx, d->m_currentService);
        return;
    } catch(std::out_of_range&) {}


    qCritical() << "Internal error: The triggered action does not have a context";
}
