#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "ServiceWidget.h"
#include "AboutDialog.h"

#include "controller/Controller.h"
#include "ui/ObjectInfoDelegate.h"

#include <QDropEvent>
#include <QMimeData>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>

#include <QActionGroup>

class MainWindow::Private {
public:
    Private(MainWindow* w)
        : ui(new Ui::MainWindow)
        , m_about{w}
    {}
    ~Private(){delete ui;}

    inline void handleAction(Parameter::Context ctx) {
        m_controller->prepareAction(ctx, m_currentService);
    }

    Ui::MainWindow *ui;
    Controller* m_controller{nullptr};
    Service* m_currentService{nullptr};
    ServiceWidget* m_currentWidget{nullptr};
    QSortFilterProxyModel m_filter_model;
    AboutDialog m_about;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d{ new Private{this} }
{
    d->ui->setupUi(this);
    d->ui->centralwidget->setEnabled(false);
    d->ui->progressBar->hide();
    connect(d->ui->tableView, &QAbstractItemView::clicked, d->ui->tableView, &QAbstractItemView::activated);
    d->ui->tableView->setModel(&d->m_filter_model);
    auto delegate = new ObjectInfoDelegate{d->ui->tableView};
    d->ui->tableView->setItemDelegateForColumn(0, delegate);
    connect(delegate, &QStyledItemDelegate::sizeHintChanged, d->ui->tableView, &QTableView::resizeRowsToContents);

    {
        d->ui->actionRefreshCurrent ->setIcon(QIcon::fromTheme("view-refresh"));
        d->ui->actionRefreshAll     ->setIcon(QIcon::fromTheme("view-refresh"));
        d->ui->actionImport         ->setIcon(QIcon::fromTheme("document-open"));
        d->ui->actionQuit           ->setIcon(QIcon::fromTheme("application-exit"));

        d->ui->actionDiag           ->setIcon(Controller::actionIcon(Parameter::Context::Diag      ));
        d->ui->actionDeploy         ->setIcon(Controller::actionIcon(Parameter::Context::Deploy    ));
        d->ui->actionConfigure      ->setIcon(Controller::actionIcon(Parameter::Context::Configure ));
        d->ui->actionUndeploy       ->setIcon(Controller::actionIcon(Parameter::Context::Undeploy  ));
        d->ui->actionBackup         ->setIcon(Controller::actionIcon(Parameter::Context::Backup    ));
        d->ui->actionRestore        ->setIcon(Controller::actionIcon(Parameter::Context::Restore   ));

        d->ui->actionStart          ->setIcon(QIcon::fromTheme("media-playback-start"));
        d->ui->actionStop           ->setIcon(QIcon::fromTheme("media-playback-stop"));
    }

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

    d->ui->tableView->addAction(d->ui->actionRefreshAll);

    connect(d->ui->actionQuit,  &QAction::triggered, QApplication::instance(), &QApplication::quit);
    connect(d->ui->actionAbout, &QAction::triggered, &d->m_about, &QDialog::open);

    restoreState(ServicesApp::instance()->settings()->mainWindowState());
}

MainWindow::~MainWindow() { delete d; }

void MainWindow::setControler(Controller* c)
{
    d->m_controller = c;

    d->m_controller->addTableActions(d->ui->menuView);

    d->m_filter_model.setSourceModel(c->model());

#define ACTION(action, context) \
    connect(action, &QAction::triggered, this, std::bind(&MainWindow::Private::handleAction, d, context));

    ACTION( d->ui->actionDiag      , Parameter::Context::Diag       )
    ACTION( d->ui->actionDeploy    , Parameter::Context::Deploy     )
    ACTION( d->ui->actionConfigure , Parameter::Context::Configure  )
    ACTION( d->ui->actionUndeploy  , Parameter::Context::Undeploy   )
    ACTION( d->ui->actionBackup    , Parameter::Context::Backup     )
    ACTION( d->ui->actionRestore   , Parameter::Context::Restore    )

#undef ACTION

    connect( c, &Controller::beginRefresh, this, &MainWindow::onRefreshStart );
    connect( c, &Controller::endRefresh,   this, &MainWindow::onRefreshEnd   );
    connect( c, &Controller::select, this, [this](int row){
        auto index = d->m_filter_model.mapFromSource(d->m_filter_model.sourceModel()->index(row,0));
        d->ui->tableView->clearSelection();
        d->ui->tableView->selectRow(index.row());
        on_tableView_activated(index);
    });

    connect(d->ui->actionRefreshAll, &QAction::triggered, c, &Controller::refresh);

    connect(c->model(), &QAbstractItemModel::modelAboutToBeReset, this, [this]{
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


    connect(c->model(), &QAbstractItemModel::rowsInserted, this, [=](const QModelIndex& parent, int first, int last){
        for ( int i = first; i <= last; ++i ) {
            auto index = c->model()->index(i, 0, parent);

            if ( auto* service = d->m_controller->getByIndex(index) )
                d->ui->stackedWidget->insertWidget( index.row()+1, new ServiceWidget{ c, service, d->ui->stackedWidget } );
        }
    });
}

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
    d->ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    d->ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::Fixed);
    d->ui->tableView->horizontalHeader()->resizeSection(1, iconSize().width());
    d->ui->actionRefreshAll->setDisabled(false);

    if ( d->m_currentWidget )
        d->m_currentWidget->updateStatus();

    onStatusChanged();

    d->ui->tableView->resizeRowsToContents();
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


void MainWindow::on_tableView_activated(const QModelIndex &index)
{
    auto srcIndex = d->m_filter_model.mapToSource(index);
    d->ui->stackedWidget->setCurrentIndex(srcIndex.row()+1);
    d->m_currentService = d->m_controller->getByIndex(srcIndex);
    d->m_currentWidget  = (ServiceWidget*)d->ui->stackedWidget->currentWidget();
    onStatusChanged();
}


void MainWindow::on_searchBar_textChanged(const QString &arg1)
{
    d->m_filter_model.setFilterFixedString(arg1);
}

void MainWindow::dropEvent(QDropEvent* event)
{
    if ( auto parameters = ServicesApp::instance()->importParameters(event->mimeData()->urls().at(0).toLocalFile()) )
        d->m_controller->importParameters(parameters.value());
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
    if ( auto parameters = ServicesApp::instance()->importParameters(fileName) )
        d->m_controller->importParameters(parameters.value());
}


void MainWindow::on_actionRefreshCurrent_triggered()
{
    d->ui->actionRefreshAll->setDisabled(true);
    d->ui->actionRefreshCurrent->setDisabled(true);
    d->m_currentWidget->updateStatus();
    onStatusChanged();
    d->ui->actionRefreshAll->setDisabled(false);
    d->ui->actionRefreshCurrent->setDisabled(false);
}

void MainWindow::on_actionStart_triggered()
{
    d->m_controller->start(d->m_currentService);
    d->m_currentWidget->updateStatus();
    onStatusChanged();
}

void MainWindow::on_actionStop_triggered()
{
    d->m_controller->stop(d->m_currentService);
    d->m_currentWidget->updateStatus();
    onStatusChanged();
}
