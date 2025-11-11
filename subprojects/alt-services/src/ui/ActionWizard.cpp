#include "ActionWizard.h"
#include "ui_ActionWizard.h"

#include "../controller/Controller.h"
#include "../data/Service.h"

#include <QMessageBox>

#include <QJsonObject>
#include <QJsonDocument>
#include <QScrollBar>
#include <QPushButton>
#include <QToolTip>

#include <QMenuBar>
#include <QActionGroup>

#include <QFile>
#include <QFileDialog>
#include <QStandardPaths>
#include <QJsonArray>
#include <QMimeData>
#include <QDropEvent>
#include <QStyledItemDelegate>

#include "forms/CompactForm.h"
#include "forms/DetailedForm.h"

#include "app/ServicesApp.h"

#include "data/models/ParameterModel.h"
#include "data/models/ResourceOwnersModel.h"

class ActionWizard::Private {
public:
    Ui::ActionWizard ui;

    Service* m_service{nullptr};

    std::vector<Parameter*> m_parameters;
    std::unique_ptr<BaseForm> m_form;
    ParameterModel m_parameter_model;
    ResourceOwnersModel m_current_resource_model;
    ResourceOwnersModel m_edited_resource_model;

    Parameter::Context m_ctx{};

    QAction* lowerUnrequired{nullptr};
    QAction*  compactMode{nullptr};
    QAction* detailedMode{nullptr};
    QSize m_defaultSize;

    std::unique_ptr<Property::Value::ValidationInfo> m_invalid{};

    inline bool hasPreDiag() {
        return Parameter::Contexts{Parameter::Context::Deploy | Parameter::Context::Diag}.testFlag(m_ctx)
               && !ui.forceDeployCheckBox->isChecked()
               && m_service->hasPreDiag();
    }

    inline bool hasPostDiag() {
        return (m_ctx == Parameter::Context::Diag
                    ? m_service->isDeployed()
                    : m_ctx == Parameter::Context::Deploy)
               && m_service->hasPostDiag();
    }

    inline bool preDiag() {
        return hasPreDiag() && ui.preDiagCheckBox->isChecked();
    }

    inline bool postDiag() {
        return hasPostDiag() && ui.postDiagCheckBox->isChecked();
    }
};

ActionWizard::ActionWizard(QWidget *parent)
    : QWizard{parent}
    , d{new Private{}}
{
    d->ui.setupUi(this);
    d->m_defaultSize = size();

    setAttribute(Qt::WA_AlwaysShowToolTips);
    d->ui.tabWidget->setUpdatesEnabled(true);

    { // MENU
        auto menuBar = new QMenuBar{this};
        d->ui.parametersPage->layout()->setMenuBar(menuBar);

        auto* file = menuBar->addMenu(tr("&File"));
        {
            auto exportAction = file->addAction(tr("&Export..."));
            auto importAction = file->addAction(tr("&Import..."));

            importAction->setIcon(QIcon::fromTheme("document-open"));
            exportAction->setIcon(QIcon::fromTheme("document-save"));

            connect( exportAction, &QAction::triggered, this, &ActionWizard::exportParameters );
            connect( importAction, &QAction::triggered, this, [this]{
                auto fileName = QFileDialog::getOpenFileName(this,
                                                             tr("Import saved parameters"),
                                                             QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
                                                             "JSON files (*.json)"
                                                             );

                if ( auto parameters = qApp->importParameters(fileName) )
                    readParameters(parameters.value());
            } );
        }

        auto* view = menuBar->addMenu(tr("&View"));
        {
            d->lowerUnrequired = view->addAction(tr("Put non-required parameters at the end"));
            d->lowerUnrequired->setCheckable(true);
            d->lowerUnrequired->setChecked(qApp->settings()->lowerUnrequired());
            connect(d->lowerUnrequired, &QAction::toggled, this, &ActionWizard::fillCurrentParameters);

            auto mode = view->addSection(tr("Mode"));

            d->compactMode  = view->addAction(tr("&Table"));
            d->detailedMode = view->addAction(tr("&List"));

            auto group = new QActionGroup{mode};
            group->setExclusive(true);
            d->compactMode->setCheckable(true);
            d->detailedMode->setCheckable(true);
            d->compactMode->setActionGroup(group);
            d->detailedMode->setActionGroup(group);

            ( qApp->settings()->editorTableMode()
                 ? d->detailedMode
                 : d->compactMode )->setChecked(true);


            connect(d->detailedMode, &QAction::changed, this, [=]{
                qApp->settings()->set_editorTableMode(d->detailedMode->isChecked());
                onModeChanged();
            });

            onModeChanged();

            view->addActions(qApp->controller()->tableActions());
        }
    }

    { // icons
        d->ui.preDiagPage->setPixmap(QWizard::LogoPixmap, QIcon::fromTheme("system-run").pixmap(32, 32));
        d->ui.postDiagPage->setPixmap(QWizard::LogoPixmap, d->ui.preDiagPage->pixmap(QWizard::LogoPixmap));
        d->ui.parametersPage->setPixmap(QWizard::LogoPixmap, QIcon::fromTheme("preferences-system").pixmap(32, 32));
    }

    d->ui. invalidParameterWarning->hide();
    d->ui.        autoStartMessage->hide();
    d->ui.      forceDeployMessage->hide();


    { // view/model
        d->m_parameter_model.setScope(Parameter::ValueScope::Edit);
        d->ui.parametersView->setModel(&d->m_parameter_model);

        d->m_current_resource_model.setScope(Parameter::ValueScope::Current);
        d->m_edited_resource_model.setScope(Parameter::ValueScope::Edit);

        d->ui.initialResourceOwnersView->setModel(&d->m_current_resource_model);
        d->ui.confirmResourceOwnersView->setModel(&d->m_edited_resource_model);

        connect(d->ui.confirmResourceOwnersView, &QAbstractItemView::clicked, this, [this](const QModelIndex& i){
            if ( d->ui.tabWidget->indexOf( d->ui.parametersTab ) == -1 )
                return;

            auto data = i.data(Qt::UserRole);
            if ( data.isNull() ) return;
            if ( auto* parameter = data.value<Parameter*>() ) {
                d->ui.tabWidget->setCurrentWidget(d->ui.parametersTab);
                int row = d->m_parameter_model.indexOf(parameter);
                d->ui.parametersView->highlight(d->m_parameter_model.index(row, 0));
            }
        });

        connect(d->ui.parametersView, &QAbstractItemView::clicked, this, [this](const QModelIndex& i){
            if ( d->ui.tabWidget->indexOf( d->ui.resourcesTab ) == -1 )
                return;

            auto data = i.data(Qt::UserRole);
            if ( data.isNull() ) return;
            if ( auto* resource = data.value<Resource*>() ) {
                d->ui.tabWidget->setCurrentWidget(d->ui.resourcesTab);
                d->ui.confirmResourceOwnersView->highlight(d->m_edited_resource_model.indexOf(resource));
            }
        });


        d->ui. preDiagView->setModel(d->ui. preDiagSelector->model());
        d->ui.postDiagView->setModel(d->ui.postDiagSelector->model());
    }

    connect(qApp->controller(), &Controller::stdout, this, [this](const QString& text)
    {
        d->ui.textBrowser->append(QString{"<p>%1</p>"}.arg(text));
    });

    connect(qApp->controller(), &Controller::stderr, this, [this](const QString& text)
    {
        d->ui.textBrowser->append(QString{"<p style=\"color: red;\">%1</p>"}.arg(text));
    });

    connect(d->ui. preDiagCheckBox, &QCheckBox::toggled, this, &ActionWizard::fillCurrentParameters);
    connect(d->ui.postDiagCheckBox, &QCheckBox::toggled, this, &ActionWizard::fillCurrentParameters);

    connect(d->ui. preDiagSelector->model(), &QAbstractItemModel::dataChanged, this, &ActionWizard::validateCurrentPage);
    connect(d->ui.postDiagSelector->model(), &QAbstractItemModel::dataChanged, this, &ActionWizard::validateCurrentPage);

    connect(d->ui. preDiagSelector->model(), &QAbstractItemModel::dataChanged, this, &ActionWizard::fillCurrentParameters);
    connect(d->ui.postDiagSelector->model(), &QAbstractItemModel::dataChanged, this, &ActionWizard::fillCurrentParameters);
}

ActionWizard::~ActionWizard() { delete d; }


void ActionWizard::open(Parameter::Context context, Service* service)
{
    d->m_service = service;
    d->m_ctx = context;
    d->m_parameters.clear();

    connect(&d->m_parameter_model, &ParameterModel::dataChanged, &d->m_edited_resource_model, &ResourceOwnersModel::parametersChanged);

    { // Conflicts view
        d->m_current_resource_model.setItems(service->resources());
        d->m_edited_resource_model.setItems(service->resources());

        for ( int r = 0; r < d->m_current_resource_model.rowCount(); ++r )
            d->ui.initialResourceOwnersView->setFirstColumnSpanned(r, {}, true);
        d->ui.initialResourceOwnersView->expandAll();
        d->ui.initialResourceOwnersView->header()->resizeSection(0, 200);
        d->ui.initialResourceOwnersView->header()->resizeSection(2, 200);
        d->ui.initialResourceOwnersView->setColumnNeverDetailed(1, true);

        for ( int r = 0; r < d->m_edited_resource_model.rowCount(); ++r )
            d->ui.confirmResourceOwnersView->setFirstColumnSpanned(r, {}, true);
        d->ui.confirmResourceOwnersView->expandAll();
        d->ui.confirmResourceOwnersView->header()->resizeSection(0, 200);
        d->ui.confirmResourceOwnersView->header()->resizeSection(2, 200);
        d->ui.confirmResourceOwnersView->setColumnNeverDetailed(1, true);

        d->ui.  deployResourceMessage->setVisible(context == Parameter::Context::  Deploy);
        d->ui.undeployResourceMessage->setVisible(context == Parameter::Context::Undeploy);

        bool needCheck   = context == Parameter::Context::Deploy && !service->isDeployed() && service->resources().size();
        bool hasConflict = needCheck   && d->m_current_resource_model.hasConflicts();
        bool resolvable  = hasConflict && d->m_current_resource_model.conflictsResolvable();

        d->ui.resourcesGroupBox->setVisible(needCheck || (service->resources().size() && context == Parameter::Context::Undeploy) );

        d->ui.conflictsDetectedMessage     ->setVisible(  hasConflict );
        d->ui.conflictsOkMessage           ->setVisible( needCheck && !hasConflict );
        d->ui.conflictsResolvableMessage   ->setVisible(   resolvable );
        d->ui.conflictsUnresolvableMessage ->setVisible( hasConflict && !resolvable );

        d->ui.optionsGroupBox->setVisible( context == Parameter::Context::Deploy );
        d->ui.otherOptionsContainer->setEnabled(!hasConflict || resolvable || d->m_service->forceDeploy());
    }

    switch (context) {
        case Parameter::Context::Diag: break;

        case Parameter::Context::Deploy:
            if ( d->m_service->isDeployed() && !d->m_service->isForceDeployable() ) {
                QMessageBox::critical( this,
                    tr("Cannot apply parameters"),
                    tr("Service is already deployed and it does not support force deployment.")
                );
                return;
            }
            break;

        default:
            if ( !d->m_service->isDeployed() ) {
                QMessageBox::critical( this,
                    tr("Cannot apply parameters"),
                    tr("\"%1\" is impossible because service is not deployed.")
                        .arg(Controller::actionName(context))
                );
                return;
            }
            break;
    }

    // reset forceDeploy state
    d->m_service->setForceDeploy(false);

    {
        d->ui. preDiagSelector->setService(d->m_service);
        d->ui.postDiagSelector->setService(d->m_service);

        d->ui. preDiagSelector->setMode(DiagTool::Test:: PreDeploy);
        d->ui.postDiagSelector->setMode(DiagTool::Test::PostDeploy);

        d->ui.parametersView->header()->resizeSection(0, 300);
        fillCurrentParameters();

        d->ui. preDiagView->expandAll();
        d->ui.postDiagView->expandAll();
    }

    { // fill titles
        d->ui.initialPage->setTitle(Controller::actionName(context) + QString{" \"%0\""}.arg(d->m_service->displayName()) );
        d->ui.initialPage->setSubTitle(d->m_service->comment());


        static const std::map<Parameter::Context, std::tuple<QString, QString, QString>> ctxnames {
            { Parameter::Context::Deploy    , { tr("Deployment wizard"),     tr("Deployment parameters"),     tr("Set deployment configuration") } },
            { Parameter::Context::Undeploy  , { tr("Undeployment wizard"),   tr("Undeployment parameters"),   tr("Set undeployment options")     } },
            { Parameter::Context::Backup    , { tr("Backup wizard"),         tr("Backup parameters"),         tr("Set backup options")           } },
            { Parameter::Context::Restore   , { tr("Restore wizard"),        tr("Restore parameters"),        tr("Set restore options")          } },
            { Parameter::Context::Configure , { tr("Configuration wizard"),  tr("Configuration parameters"),  tr("Set service configuration")    } },
            { Parameter::Context::Diag      , { tr("Diagnostic wizard"),     tr("Diagnostic parameters"),     tr("Set diagnostic options")       } }
        };

        const auto& [windowTitle, pageTitle, pageSubtitle] = ctxnames.at(d->m_ctx);
        setWindowTitle(windowTitle);
        d->ui.parametersPage->setTitle(pageTitle);
        d->ui.parametersPage->setSubTitle(pageSubtitle);

        d->ui.finishPage->setTitle(d->ui.initialPage->title());

        auto actionPixmap = Controller::actionIcon(d->m_ctx).pixmap(32, 32);
        d->ui.initialPage->setPixmap(QWizard::LogoPixmap, actionPixmap);
        d->ui. finishPage->setPixmap(QWizard::LogoPixmap, actionPixmap);
    }

    d->ui.   autoStartCheckBox->setChecked(false);
    d->ui.   autoStartCheckBox->setHidden(d->m_service->isStarted() || context != Parameter::Context::Deploy);
    d->ui. forceDeployCheckBox->setChecked(false);
    d->ui. forceDeployCheckBox->setVisible( (d->m_ctx == Parameter::Context::Deploy) && d->m_service->isForceDeployable() );
    d->ui.     preDiagCheckBox->setChecked(true);
    d->ui.    postDiagCheckBox->setChecked(true);
    d->ui.     preDiagCheckBox->setHidden(d->m_ctx == Parameter::Context::Diag);
    d->ui.    postDiagCheckBox->setHidden(d->m_ctx == Parameter::Context::Diag);


    QWizard::show();

    setCurrentId(0);
    int next = nextId();

    bool startPageNeeded = d->ui.forceDeployCheckBox->isVisible() ||
                           d->ui.  autoStartCheckBox->isVisible() ||
                           ( context == Parameter::Context::Deploy && d->m_current_resource_model.hasConflicts() ) ||
                           ( context == Parameter::Context::Undeploy && d->m_service->resources().size() );

    bool doubleCheckNeeded = d->ui.tabWidget->count() && (next < 4 && next != -1);
    bool finishPageNeeded = startPageNeeded && doubleCheckNeeded;
    d->ui.finishPageCheckBox->setChecked( finishPageNeeded );
    d->ui.finishPageCheckBox->setVisible( finishPageNeeded );

    setStartId( !startPageNeeded && next < 4 ? next : 0);
    restart();

    if ( currentId() == 0 && currentPage()->isFinalPage() ) {
        resize(minimumSize());
    } else resize(d->m_defaultSize);
}

void ActionWizard::validateParameters()
{
    if ( auto invalidPtr = d->m_form->findInvalidValue() ) {

        int level = 0;
        QStringList path;
        QString* message = &invalidPtr->message;

        auto invalid = invalidPtr.get();

        while ( invalid ) {
            message = &invalid->message;
            path.append(
                QString{R"(<a href="%1">%2</a>)"}
                    .arg( level++ )
                    .arg( invalid->value->displayName() )
            );
            invalid = invalid->childInfo.get();
        }

        d->ui.invalidParameterWarning->setText(
            QString{"%1:\n%2"}
                .arg( path.join(" / ") )
                .arg( *message )
        );

        d->m_invalid = std::move(invalidPtr);
    } else
        d->m_invalid = {};

    if (d->m_invalid)
        d->ui.invalidParameterWarning->animatedShow();
    else
        d->ui.invalidParameterWarning->animatedHide();

    if ( d->m_service ) {
        d->ui.      missingDiagWarning->setVisible(( d->preDiag() || d->postDiag() ) && d->m_service->isDiagMissing());
        d->ui.missingDiagParamsWarning->setVisible(( d->preDiag() || d->postDiag() ) &&
            std::any_of(d->m_service->diagTools().cbegin(), d->m_service->diagTools().cend(), [this](const std::unique_ptr<DiagTool>& tool){
                return ( d-> preDiag() ? tool->anySelected(DiagTool::Test:: PreDeploy) && tool->isMissingParams() : false ) ||
                       ( d->postDiag() ? tool->anySelected(DiagTool::Test::PostDeploy) && tool->isMissingParams() : false );
            })
        );
    }

    validateCurrentPage();
}

void ActionWizard::fillCurrentParameters()
{
    Parameter::Contexts ctx{d->m_ctx};

    if ( d->m_ctx == Parameter::Context::Deploy && (d->preDiag() || d->postDiag()) )
        ctx.setFlag(Parameter::Context::Diag);

    auto oldParameters = d->m_parameters;
    d->m_parameters.clear();
    bool useCurrent = (d->m_ctx == Parameter::Context::Configure) || d->m_service->isDeployed();
    bool firstFill  = oldParameters.empty();


    std::vector<Parameter*> optionalParameters;
    for ( const auto& param : d->m_service->parameters() ) {
        if ( !param->contexts().testAnyFlags(ctx) || param->isConstant() )
            continue;

        if ( firstFill || std::find(oldParameters.cbegin(), oldParameters.cend(), param.get()) != oldParameters.cend() )
            param->fillFromValue(useCurrent);

        param->value(Parameter::ValueScope::Edit)->setEnabled( param->required() & ctx || (useCurrent && param->value(Parameter::ValueScope::Current)->isEnabled()) );

        ( !d->lowerUnrequired->isChecked() || param->required().testAnyFlags(ctx)
                ? &d->m_parameters
                : &optionalParameters
        )->push_back(param.get());
    }

    std::copy(optionalParameters.begin(), optionalParameters.end(), std::back_inserter(d->m_parameters));

    d->m_form->setParameters(d->m_parameters, ctx);
    d->m_parameter_model.setItems(d->m_parameters);
    validateParameters();

    d->m_form->setHidden(d->m_parameters.empty());

    // QTabWidget::setTabVisible behaves strangly. TODO?
    if ( !d->preDiag() )
        d->ui.tabWidget->removeTab(d->ui.tabWidget->indexOf(d->ui.preDiagTab));
    else {
        int i = d->ui.tabWidget->insertTab(0, d->ui.preDiagTab, tr("Premilinary diagnostics"));
        d->ui.tabWidget->setTabIcon(i, QIcon::fromTheme("system-run"));
    }

    if ( !d->postDiag() )
        d->ui.tabWidget->removeTab(d->ui.tabWidget->indexOf(d->ui.postDiagTab));
    else {
        int i = d->ui.tabWidget->insertTab(1, d->ui.postDiagTab, tr("Post-deploy diagnostics"));
        d->ui.tabWidget->setTabIcon(i, QIcon::fromTheme("system-run"));
    }

    if ( d->m_parameters.empty() )
        d->ui.tabWidget->removeTab(d->ui.tabWidget->indexOf(d->ui.parametersTab));
    else {
        int i = d->ui.tabWidget->insertTab(2, d->ui.parametersTab, tr("Parameters"));
        d->ui.tabWidget->setTabIcon(i, QIcon::fromTheme("preferences-system"));
    }

    if ( d->m_service->resources().empty() )
        d->ui.tabWidget->removeTab(d->ui.tabWidget->indexOf(d->ui.resourcesTab));
    else {
        int i = d->ui.tabWidget->insertTab(3, d->ui.resourcesTab, tr("Resources"));
        d->ui.tabWidget->setTabIcon(i, QIcon::fromTheme("changes-prevent"));
    }

    if ( int count = d->ui.tabWidget->count() ) {
        d->ui.tabWidget->show();
        for ( int i = 0; i < count; ++i )
            if ( d->ui.tabWidget->widget(i) ) {
                d->ui.tabWidget->setCurrentIndex(i);
                break;
            }
    } else {
        d->ui.tabWidget->hide();
    }
}

void ActionWizard::onModeChanged()
{
    if ( d->compactMode->isChecked() )
        d->m_form = std::make_unique<CompactForm>(this);
    else
        d->m_form = std::make_unique<DetailedForm>(this);


    d->ui.formContainer->addWidget(d->m_form.get());
    d->m_form->setParameters(d->m_parameters, d->m_ctx);
    d->m_form->setHidden(d->m_parameters.empty());

    connect(d->m_form.get(), &BaseForm::changed, this, &ActionWizard::validateParameters);
    validateParameters();
}

static const std::map<QString, Parameter::Contexts> ctxmap {
    { "configure" , Parameter::Context::Configure },
    { "deploy"    , Parameter::Context::Deploy    },
    { "undeploy"  , Parameter::Context::Undeploy  },
    { "diag"      , Parameter::Context::Diag      },
  //{ "status"    , Parameter::Context::Status    },
    { "backup"    , Parameter::Context::Backup    },
    { "restore"   , Parameter::Context::Restore   },

    { "diag+deploy", Parameter::Context::Deploy | Parameter::Context::Diag },
};

void ActionWizard::readParameters(const ServicesApp::ParsedParameters& params)
{
    if ( isVisible() && params.service != d->m_service ) {
        QMessageBox::critical(this, tr("Incompatible parameters"),
            tr("This file contains parameters for \"%1\", while \"%2\" expected.")
              .arg(params.service->name())
              .arg(d->m_service->name())
        );

        return;
    }

    if ( isVisible() && !(params.contexts & d->m_ctx) ) {
        QMessageBox::critical(this, tr("Incompatible parameters"),
            tr("This file contains parameters for \"%1\", while \"%2\" expected.")
                .arg(Controller::actionName(d->m_ctx))
                .arg(Controller::actionName(
                    params.contexts.testFlags(Parameter::Context::Deploy | Parameter::Context::Diag)
                        ? Parameter::Context::Deploy
                        : (Parameter::Context)params.contexts.toInt()
                    )
                )
        );
        return;
    }

    if ( isHidden() )
        open( params.contexts.testFlags(Parameter::Context::Deploy | Parameter::Context::Diag)
                ? Parameter::Context::Deploy
                : (Parameter::Context)params.contexts.toInt()
            , params.service );

    if ( params.contexts.testFlag(Parameter::Context::Deploy) ) {
        bool diag = params.contexts.testFlag(Parameter::Context::Diag);

        if ( diag )
            d->ui.forceDeployCheckBox->setChecked(false);
        d->ui.preDiagCheckBox->setChecked(diag);
        d->ui.postDiagCheckBox->setChecked(diag);

        fillCurrentParameters();
    }


    if ( !d->m_service->tryFill( params.data, params.contexts ) ) {
        QMessageBox::critical(this, tr("Invalid data"), tr("Provided parameters are invalid."));

        fillCurrentParameters();
    }
    onModeChanged();
}

void ActionWizard::exportParameters()
{
    auto it = std::find_if(ctxmap.cbegin(), ctxmap.cend(), [this](const auto& pair){ return pair.second == d->m_ctx; });
    if ( it == ctxmap.cend() ) {
        qCritical() << "invalid context";
        return;
    }

    auto contextName = it->first;

    auto name = QFileDialog::getSaveFileName(this,
        tr("Export current parameters"),
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
            .append('/'+contextName+'_'+d->m_service->name()+".json"),
        "JSON files (*.json)"
    );

    if ( name.isEmpty() ) return;

    QFile f{name};
    if ( ! f.open(QIODevice::WriteOnly) ) return;

    QByteArray data = QJsonDocument{{
        { "service",    d->m_service->name() },
        { "context",    contextName },
        { "parameters", d->m_service->getParameters(d->m_ctx, true) },
    }}.toJson(QJsonDocument::Indented);

    f.write(data);
    f.close();
}

void ActionWizard::dropEvent(QDropEvent* event)
{
    if ( auto parameters = ServicesApp::instance()->importParameters(event->mimeData()->urls().at(0).toLocalFile()) )
        readParameters(parameters.value());
}

void ActionWizard::on_invalidParameterWarning_linkActivated(const QString& link)
{
    int level = link.toInt();
    d->m_form->ensureVisible(d->m_invalid.get(), level);
}

void ActionWizard::on_invalidParameterWarning_linkHovered(const QString& link)
{
    int level = link.toInt();

    auto invalid = d->m_invalid.get();
    while ( level ) {
        invalid = invalid->childInfo.get();
        --level;
    }

    QToolTip::showText(QCursor::pos(), invalid->message);
}

void ActionWizard::on_forceDeployCheckBox_toggled(bool checked)
{
    d->m_service->setForceDeploy(checked);

    // because is affects pre-diag;
    fillCurrentParameters();

    d->ui.otherOptionsContainer->setEnabled(!d->m_current_resource_model.hasConflicts() ||
                                            d->m_current_resource_model.conflictsResolvable() ||
                                            d->m_service->forceDeploy());
    validateCurrentPage();
}

bool ActionWizard::validateCurrentPage()
{
    bool valid = true;

    if ( currentPage() == d->ui.initialPage )
        valid = d->m_ctx != Parameter::Context::Deploy ||
                !d->m_current_resource_model.hasConflicts() ||
                 d->m_current_resource_model.conflictsResolvable() ||
                 d->m_service->forceDeploy();
    if ( currentPage() == d->ui.parametersPage )
        valid = !d->m_invalid;
    else if ( currentPage() == d->ui.preDiagPage )
        valid = !d->ui. preDiagCheckBox->isChecked() || d->ui. preDiagSelector->anySelected();
    else if ( currentPage() == d->ui.postDiagPage )
        valid = !d->ui.postDiagCheckBox->isChecked() || d->ui.postDiagSelector->anySelected();

    button( nextId() == -1 ? WizardButton::FinishButton : WizardButton::NextButton )
        ->setDisabled( !valid );

    return valid;
}

int ActionWizard::nextId() const
{
    static const std::array<std::function<bool()>, 5> checks {
        std::bind(&Private::hasPreDiag,  d),
        std::bind(&Private::hasPostDiag, d),
        std::bind(std::not_fn(&std::vector<Parameter*>::empty), &d->m_parameters),
        std::bind(&QCheckBox::isChecked, d->ui.finishPageCheckBox),
        []{ return true; },
    };

    for ( int i = currentId(); i < checks.size(); ++i )
        if ( checks[i]() )
            return i+1;

    return -1;
}

void ActionWizard::on_finishPageCheckBox_clicked(bool checked)
{
    currentPage()->setCommitPage( nextId() == -1 );
}

void ActionWizard::on_ActionWizard_currentIdChanged(int id)
{
    if ( id == -1 ) return;

    validateCurrentPage();

    page(id)->setCommitPage( nextId() == 5 );
    if ( nextId() == 5 ) {
        d->ui.textBrowser->clear();
    }

    if ( id == 5 ) {
        button(QWizard::FinishButton)->setEnabled(false);
        button(QWizard::CancelButton)->setEnabled(false);
        d->ui.progressBar->show();

        bool success = true;

        switch ( d->m_ctx ) {
            case Parameter::Context::Deploy:
                success =
                    ( !d->preDiag() || qApp->controller()->diag(d->m_service, false) ) &&
                    qApp->controller()->call(d->m_service, d->m_ctx);

                button(QWizard::BackButton)->setEnabled(!success);

                success = success && ( !d->postDiag() || qApp->controller()->diag(d->m_service, true) );
            break;

            case Parameter::Context::Diag:
                success = qApp->controller()->diag(d->m_service, d->m_service->isDeployed());
                button(QWizard::BackButton)->setEnabled(!success);
            break;

            default:
                success = qApp->controller()->call(d->m_service, d->m_ctx);
        }

        if ( success && !d->m_service->isStarted() && d->ui.autoStartCheckBox->isChecked() )
            qApp->controller()->start(d->m_service);


        d->ui.progressBar->hide();
        button(QWizard::FinishButton)->setEnabled( success);
        button(QWizard::CancelButton)->setEnabled(!success);
    }
}

void ActionWizard::done(int result)
{
    if ( result == QDialog::Rejected ||
         currentId() == 5 ||
         !currentPage()->isFinalPage() ||
         qApp->controller()->call(d->m_service, d->m_ctx)
        )
        QWizard::done(result);
}

void ActionWizard::closeEvent(QCloseEvent* event)
{
    if ( d->ui.progressBar->isVisible() ) {
        event->ignore();
        return;
    }

    QWizard::closeEvent(event);
}
