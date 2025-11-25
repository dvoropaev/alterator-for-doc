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
#include <QFontDatabase>
#include <QTextBrowser>

#include "forms/CompactForm.h"
#include "forms/DetailedForm.h"

#include "app/ServicesApp.h"

#include "data/models/ParameterModel.h"
#include "data/models/ResourceOwnersModel.h"

#include "ParameterSearcher.h"
#include "SearchBar.h"

static const std::map<Parameter::Context, std::function<std::tuple<QString, QString, QString>()>> ctxnames {
    { Parameter::Context::Deploy    , []{ return std::tuple{ QObject::tr("Deployment wizard"),    QObject::tr("Deployment parameters"),     QObject::tr("Set deployment configuration") }; } },
    { Parameter::Context::Undeploy  , []{ return std::tuple{ QObject::tr("Undeployment wizard"),  QObject::tr("Undeployment parameters"),   QObject::tr("Set undeployment options")     }; } },
    { Parameter::Context::Backup    , []{ return std::tuple{ QObject::tr("Backup wizard"),        QObject::tr("Backup parameters"),         QObject::tr("Set backup options")           }; } },
    { Parameter::Context::Restore   , []{ return std::tuple{ QObject::tr("Restore wizard"),       QObject::tr("Restore parameters"),        QObject::tr("Set restore options")          }; } },
    { Parameter::Context::Configure , []{ return std::tuple{ QObject::tr("Configuration wizard"), QObject::tr("Configuration parameters"),  QObject::tr("Set service configuration")    }; } },
    { Parameter::Context::Diag      , []{ return std::tuple{ QObject::tr("Diagnostic wizard"),    QObject::tr("Diagnostic parameters"),     QObject::tr("Set diagnostic options")       }; } }
};

class ActionWizard::Private {
public:
    Ui::ActionWizard ui;

    Action playfile;

    std::vector<Parameter*> m_parameters;
    std::unique_ptr<BaseForm> m_form;
    ParameterModel m_parameter_model;
    ResourceOwnersModel m_current_resource_model;
    ResourceOwnersModel m_edited_resource_model;

    QAction* lowerUnrequired{nullptr};
    QAction*  compactMode{nullptr};
    QAction* detailedMode{nullptr};

    QAction* search{nullptr};

    QSize m_defaultSize;

    std::unique_ptr<Property::Value::ValidationInfo> m_invalid{};

    inline bool hasPreDiag() {
        return Parameter::Contexts{Parameter::Context::Deploy | Parameter::Context::Diag}.testFlag(playfile.action)
               && !playfile.options.force
               && !playfile.service->isDeployed()
               && playfile.service->hasPreDiag();
    }

    inline bool hasPostDiag() {
        return (playfile.action == Parameter::Context::Diag
                    ? playfile.service->isDeployed()
                    : Parameter::Contexts{Parameter::Context::Deploy |
                                          Parameter::Context::Configure}.testFlag(playfile.action))
               && playfile.service->hasPostDiag();
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
                    open(parameters.value());
            } );
        }

        auto* edit = menuBar->addMenu(tr("&Edit"));
        {
            d->search = edit->addAction(tr("&Find..."));
            d->search->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::EditFind));
            d->search->setShortcut(QKeySequence::Find);

            d->ui.searchBar->hide();

            connect(d->search, &QAction::triggered, d->ui.searchBar, &QWidget::show);
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

        connect(&d->m_parameter_model, &ParameterModel::dataChanged, &d->m_edited_resource_model, &ResourceOwnersModel::parametersChanged);

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


        d->ui. preDiagSelector->setMode(DiagTool::Test::Mode:: PreDeploy);
        d->ui.postDiagSelector->setMode(DiagTool::Test::Mode::PostDeploy);


        d->ui. preDiagView->setModel(d->ui. preDiagSelector->model());
        d->ui.postDiagView->setModel(d->ui.postDiagSelector->model());
    }

    connect(qApp->controller(), &Controller::actionBegin, d->ui.logWidget, &LogWidget::beginEntry);
    connect(qApp->controller(), &Controller::actionEnd,   d->ui.logWidget, &LogWidget::endEntry);
    connect(qApp->controller(), &Controller::stdout,      d->ui.logWidget, &LogWidget::message);
    connect(qApp->controller(), &Controller::stderr,      d->ui.logWidget, &LogWidget::error);


    connect(d->ui. preDiagCheckBox, &QCheckBox::toggled, this, [this]{fillCurrentParameters();});
    connect(d->ui.postDiagCheckBox, &QCheckBox::toggled, this, [this]{fillCurrentParameters();});

    connect(d->ui. preDiagSelector->model(), &QAbstractItemModel::dataChanged, this, &ActionWizard::validateCurrentPage);
    connect(d->ui.postDiagSelector->model(), &QAbstractItemModel::dataChanged, this, &ActionWizard::validateCurrentPage);

    connect(d->ui. preDiagSelector->model(), &QAbstractItemModel::dataChanged, this, [this]{fillCurrentParameters();});
    connect(d->ui.postDiagSelector->model(), &QAbstractItemModel::dataChanged, this, [this]{fillCurrentParameters();});
}

ActionWizard::~ActionWizard() { delete d; }



static const std::map<QString, Parameter::Contexts> ctxmap {
    { "configure" , Parameter::Context::Configure },
    { "deploy"    , Parameter::Context::Deploy    },
    { "undeploy"  , Parameter::Context::Undeploy  },
    { "diag"      , Parameter::Context::Diag      },
    { "backup"    , Parameter::Context::Backup    },
    { "restore"   , Parameter::Context::Restore   },
};

void ActionWizard::open(Action action)
{
    if ( isVisible() )
    {
        if ( action.service != d->playfile.service || action.action != d->playfile.action ) {
            auto key = QMessageBox::critical(this, tr("Warning"),
                tr("This file contains %1 for \"%2\", but the wizard contains %3 for \"%4\".")
                    .arg(std::get<1>(ctxnames.at(action.action)()).toLower())
                    .arg(action.service->name())
                    .arg(std::get<1>(ctxnames.at(d->playfile.action)()).toLower())
                    .arg(d->playfile.service->name())
                .append('\n').append(tr("Do you want to continue importing?")),

                QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::Abort
            );

            if ( key == QMessageBox::StandardButton::Abort )
                return;
        }
    }

    switch (action.action) {
        case Parameter::Context::Deploy:
            if ( action.service->isDeployed() && !action.service->isForceDeployable() ) {
                QMessageBox::critical( this,
                    tr("Cannot apply parameters"),
                    tr("Service is already deployed and it does not support force deployment.")
                );
                return;
            }
            break;

        case Parameter::Context::Backup:
        case Parameter::Context::Restore:
        case Parameter::Context::Configure:
        case Parameter::Context::Undeploy:
            if ( !action.service->isDeployed() ) {
                QMessageBox::critical( this,
                    tr("Cannot apply parameters"),
                    tr("\"%1\" is impossible because service is not deployed.")
                        .arg(Controller::actionName(d->playfile.action))
                );
                return;
            }
            break;

        default: break;
    }


    d->m_parameters.clear();
    if ( action.service->isStarted() )
        action.options.autostart = false;

    d->playfile = action;

    fillCurrentParameters(true);
    onModeChanged();

    d->ui.  autoStartCheckBox->setChecked( action.options.autostart );
    d->ui.forceDeployCheckBox->setChecked( action.options.force );
    d->ui.    preDiagCheckBox->setChecked( action.options.prediag );
    d->ui.   postDiagCheckBox->setChecked( action.options.postdiag );

    d->ui.   autoStartCheckBox->setHidden(d->playfile.service->isStarted() || d->playfile.action != Parameter::Context::Deploy);
    d->ui. forceDeployCheckBox->setVisible( (d->playfile.action == Parameter::Context::Deploy) && d->playfile.service->isForceDeployable() );
    d->ui.     preDiagCheckBox->setChecked(d->playfile.options. prediag);
    d->ui.    postDiagCheckBox->setChecked(d->playfile.options.postdiag);
    d->ui.     preDiagCheckBox->setHidden(d->playfile.action == Parameter::Context::Diag);
    d->ui.    postDiagCheckBox->setHidden(d->playfile.action == Parameter::Context::Diag);

    { // Conflicts view
        d->m_current_resource_model.setItems(d->playfile.service->resources());
        d->m_edited_resource_model.setItems(d->playfile.service->resources());

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

        d->ui.  deployResourceMessage->setVisible(d->playfile.action == Parameter::Context::  Deploy);
        d->ui.undeployResourceMessage->setVisible(d->playfile.action == Parameter::Context::Undeploy);

        bool needCheck   = d->playfile.action == Parameter::Context::Deploy && !d->playfile.service->isDeployed() && d->playfile.service->resources().size();
        bool hasConflict = needCheck   && d->m_current_resource_model.hasConflicts();
        bool resolvable  = hasConflict && d->m_current_resource_model.conflictsResolvable();

        d->ui.resourcesGroupBox->setVisible(needCheck || (d->playfile.service->resources().size() && d->playfile.action == Parameter::Context::Undeploy) );

        d->ui.conflictsDetectedMessage     ->setVisible(  hasConflict );
        d->ui.conflictsOkMessage           ->setVisible( needCheck && !hasConflict );
        d->ui.conflictsResolvableMessage   ->setVisible(   resolvable );
        d->ui.conflictsUnresolvableMessage ->setVisible( hasConflict && !resolvable );

        d->ui.optionsGroupBox->setVisible( d->playfile.action == Parameter::Context::Deploy );
        d->ui.otherOptionsContainer->setEnabled(!hasConflict || resolvable || d->playfile.options.force);
    }


    {
        d->ui. preDiagSelector->setService(d->playfile.service, d->playfile.options. prediagTests);
        d->ui.postDiagSelector->setService(d->playfile.service, d->playfile.options.postdiagTests);

        d->ui.parametersView->header()->resizeSection(0, 300);

        d->ui. preDiagView->expandAll();
        d->ui.postDiagView->expandAll();
    }

    { // fill titles
        d->ui.initialPage->setTitle(Controller::actionName(d->playfile.action) + QString{" \"%0\""}.arg(d->playfile.service->displayName()) );
        d->ui.initialPage->setSubTitle(d->playfile.service->comment());

        const auto& [windowTitle, pageTitle, pageSubtitle] = ctxnames.at(d->playfile.action)();
        setWindowTitle(windowTitle);
        d->ui.parametersPage->setTitle(pageTitle);
        d->ui.parametersPage->setSubTitle(pageSubtitle);

        d->ui.finishPage->setTitle(d->ui.initialPage->title());

        auto actionPixmap = Controller::actionIcon(d->playfile.action).pixmap(32, 32);
        d->ui.initialPage->setPixmap(QWizard::LogoPixmap, actionPixmap);
        d->ui. finishPage->setPixmap(QWizard::LogoPixmap, actionPixmap);
    }


    QWizard::show();

    setCurrentId(0);
    int next = nextId();

    bool startPageNeeded = d->ui.forceDeployCheckBox->isVisible() ||
                           d->ui.  autoStartCheckBox->isVisible() ||
                           ( d->playfile.action == Parameter::Context::Deploy && d->m_current_resource_model.hasConflicts() ) ||
                           ( d->playfile.action == Parameter::Context::Undeploy && d->playfile.service->resources().size() );

    bool doubleCheckNeeded = d->ui.tabWidget->count() && (next < 4 && next != -1);
    bool finishPageNeeded = startPageNeeded && doubleCheckNeeded;
    d->ui.finishPageCheckBox->setChecked( finishPageNeeded );
    d->ui.finishPageCheckBox->setVisible( finishPageNeeded );

    setStartId( !startPageNeeded && next < 4 ? next : 0);
    restart();

    if ( currentId() == 0 && currentPage()->isFinalPage() ) {
        resize(minimumSize());
    } else resize(d->m_defaultSize);


    auto it = std::find_if(ctxmap.cbegin(), ctxmap.cend(), [this](const auto& pair){ return pair.second == d->playfile.action; });
    if ( it == ctxmap.cend() ) {
        qCritical() << "invalid context";
        return;
    }
    auto contextName = it->first;

    d->ui.logWidget->setExportFileName(contextName+'_'+d->playfile.service->name());
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

    if ( d->playfile.service ) {
        d->ui.      missingDiagWarning->setVisible(( d->playfile.options.prediag || d->playfile.options.postdiag ) && d->playfile.service->isDiagMissing());
        d->ui.missingDiagParamsWarning->setVisible(( d->playfile.options.prediag || d->playfile.options.postdiag ) &&
            std::any_of(d->playfile.service->diagTools().cbegin(), d->playfile.service->diagTools().cend(), [this](const std::unique_ptr<DiagTool>& tool){
                return ( d->playfile.options. prediag ? d->playfile.hasTool(tool.get(), DiagTool::Test::Mode:: PreDeploy) && tool->isMissingParams() : false ) ||
                       ( d->playfile.options.postdiag ? d->playfile.hasTool(tool.get(), DiagTool::Test::Mode::PostDeploy) && tool->isMissingParams() : false );
            })
        );
    }

    validateCurrentPage();
}

void ActionWizard::fillCurrentParameters(bool firstFill)
{
    d->playfile.options. prediag = d->ui. preDiagCheckBox->isChecked();
    d->playfile.options.postdiag = d->ui.postDiagCheckBox->isChecked();

    Parameter::Contexts ctx{d->playfile.action};

    // for deploy and configure
    if ( d->playfile.options.prediag || d->playfile.options.postdiag )
        ctx.setFlag(Parameter::Context::Diag);

    auto oldParameters = d->m_parameters;
    d->m_parameters.clear();
    bool useCurrent = d->playfile.service->isDeployed();

    bool fillFromPlayfile = !d->playfile.parameters.empty();


    if ( oldParameters.empty() && fillFromPlayfile )
    {
        if ( !d->playfile.service->tryFill( d->playfile.parameters, ctx) )
        {
            QMessageBox mb{this};
            mb.setIcon(QMessageBox::Icon::Warning);
            mb.setWindowTitle(tr("Invalid data"));

            mb.setStandardButtons(QMessageBox::StandardButton::RestoreDefaults |
                                  QMessageBox::StandardButton::Ignore);
            mb.setText(
                tr("Some of the required parameters are missing or invalid.")
                    .append('\n').append(tr("Press \"%1\" to use default parameters instead.")
                                .arg(mb.button(QMessageBox::StandardButton::RestoreDefaults)->text().remove('&')))
                    .append('\n').append(tr("Press \"%1\" to load them anyway.")
                                .arg(mb.button(QMessageBox::StandardButton::Ignore)->text().remove('&')))
                    .append('\n').append(tr("You may still need to enter some parameters manually."))
                );
            mb.exec();

            fillFromPlayfile = mb.clickedButton() != mb.button(QMessageBox::StandardButton::RestoreDefaults);
        }
    }

    std::vector<Parameter*> optionalParameters;
    for ( const auto& param : d->playfile.service->parameters() ) {
        if ( !param->contexts().testAnyFlags(ctx) || param->isConstant() )
            continue;

        if ( !fillFromPlayfile || ( !firstFill && std::find(oldParameters.cbegin(), oldParameters.cend(), param.get()) == oldParameters.cend() ) )
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
    if ( !d->playfile.options.prediag )
        d->ui.tabWidget->removeTab(d->ui.tabWidget->indexOf(d->ui.preDiagTab));
    else {
        int i = d->ui.tabWidget->insertTab(0, d->ui.preDiagTab, tr("Premilinary diagnostics"));
        d->ui.tabWidget->setTabIcon(i, QIcon::fromTheme("system-run"));
    }

    if ( !d->playfile.options.postdiag )
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

    if ( d->playfile.service->resources().empty() )
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
        d->m_form = std::make_unique<CompactForm>(d->playfile, this);
    else
        d->m_form = std::make_unique<DetailedForm>(d->playfile, this);


    d->ui.formContainer->addWidget(d->m_form.get());
    d->m_form->setParameters(d->m_parameters, d->playfile.action);
    d->m_form->setHidden(d->m_parameters.empty());

    connect(d->m_form.get(), &BaseForm::changed, this, &ActionWizard::validateParameters);
    d->ui.searchBar->setAdapter(d->m_form->searchAdapter());

    validateParameters();
}

void ActionWizard::exportParameters()
{
    if ( d->m_form->findInvalidValue() )
    {
        auto button = QMessageBox::warning(this, tr("Incomplete parameter set"),
            tr("Some of the required parameters are incorrect or missing. Do you want to continue exporting?"),
            QMessageBox::Cancel | QMessageBox::Yes
        );

        if ( button == QMessageBox::Cancel )
            return;
    }

    auto it = std::find_if(ctxmap.cbegin(), ctxmap.cend(), [this](const auto& pair){ return pair.second == d->playfile.action; });
    if ( it == ctxmap.cend() ) {
        qCritical() << "invalid context";
        return;
    }

    auto contextName = it->first;

    auto name = QFileDialog::getSaveFileName(this,
        tr("Export current parameters"),
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
            .append('/'+contextName+'_'+d->playfile.service->name()+".playfile.json"),
        "JSON files (*.json)"
    );

    if ( name.isEmpty() ) return;

    QFile f{name};
    if ( ! f.open(QIODevice::WriteOnly) ) return;

    QByteArray data = d->playfile.serialize();

    f.write(data);
    f.close();
}

void ActionWizard::dropEvent(QDropEvent* event)
{
    if ( auto parameters = qApp->importParameters(event->mimeData()->urls().at(0).toLocalFile()) )
        open(parameters.value());
}

void ActionWizard::on_invalidParameterWarning_linkActivated(const QString& link)
{
    int level = link.toInt();
    if ( d->m_invalid )
    {
        auto* info = d->m_invalid.get();

        while ( level && info->childInfo )
        {
            info = info->childInfo.get();
            --level;
        }

        d->m_form->ensureVisible(info->value);
    }
}

void ActionWizard::on_invalidParameterWarning_linkHovered(const QString& link)
{
    int level = link.toInt();

    auto* invalid = d->m_invalid.get();
    while ( invalid && level ) {
        invalid = invalid->childInfo.get();
        --level;
    }

    if ( invalid )
        QToolTip::showText(QCursor::pos(), invalid->message);
}

void ActionWizard::on_forceDeployCheckBox_toggled(bool checked)
{
    d->playfile.options.force = checked;

    // because is affects pre-diag;
    fillCurrentParameters();

    d->ui.otherOptionsContainer->setEnabled(!d->m_current_resource_model.hasConflicts() ||
                                            d->m_current_resource_model.conflictsResolvable() ||
                                            d->playfile.options.force);
    validateCurrentPage();
}

void ActionWizard::on_autoStartCheckBox_toggled(bool checked)
{
    d->playfile.options.autostart = checked;
}

bool ActionWizard::validateCurrentPage()
{
    bool valid = true;

    if ( currentPage() == d->ui.initialPage )
        valid = d->playfile.action != Parameter::Context::Deploy ||
                !d->m_current_resource_model.hasConflicts() ||
                 d->m_current_resource_model.conflictsResolvable() ||
                 d->playfile.options.force;
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
        d->ui.logWidget->clear();
    }

    if ( id == 5 ) {
        button(QWizard::FinishButton)->setEnabled(false);
        button(QWizard::CancelButton)->setEnabled(false);
        d->ui.progressBar->show();

        bool success = true;

        d->playfile.parameters = d->playfile.service->getParameters(d->playfile.action, false);
        if ( d->playfile.action == Parameter::Context::Deploy )
            d->playfile.parameters["force_deploy"] = d->playfile.options.force;
        success = qApp->controller()->call(d->playfile);

        // NOTE: if deploy succeeded but only post-diag failed, we can't go back anymore
        if ( d->playfile.action == Parameter::Context::Deploy )
            button(QWizard::BackButton)->setEnabled( d->playfile.service->isDeployed() || !success);


        d->ui.progressBar->hide();
        button(QWizard::FinishButton)->setEnabled( success);
        button(QWizard::CancelButton)->setEnabled(!success);
    }
}

void ActionWizard::closeEvent(QCloseEvent* event)
{
    if ( d->ui.progressBar->isVisible() ) {
        event->ignore();
        return;
    }

    QWizard::closeEvent(event);
}
