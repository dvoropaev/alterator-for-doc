#include "ConfirmationPage.h"
#include "ui_ConfirmationPage.h"

#include "controller/Controller.h"
#include "data/models/ResourceOwnersModel.h"
#include "data/models/ParameterModel.h"
#include <range/v3/iterator/insert_iterators.hpp>


class ConfirmationPage::Private {
public:
    inline explicit Private(const std::vector<Parameter*>& parameters)
        : m_parameters{parameters}
    {}

    Ui::ConfirmationPage ui;

    std::map<Parameter::Context, int> m_contextToIndex;

    ResourceOwnersModel m_resource_model;

    // contains all the parameters from the ParametesPage
    const std::vector<Parameter*>& m_parameters;

    // only diagnostic parameters
    std::vector<Parameter*> m_diagParameters;
    ParameterModel m_diag_parameter_model;

    // all parameters without diagnostic-only
    std::vector<Parameter*> m_ctxParameters;
    ParameterModel m_ctx_parameter_model;
};


ConfirmationPage::ConfirmationPage(const std::vector<Parameter*>& parameters,
                                   QAbstractItemModel&  preDiagModel,
                                   QAbstractItemModel& postDiagModel,
                                   QWidget *parent)
    : Page(parent)
    , d(new Private{parameters})
{
    d->ui.setupUi(this);

    d->ui. preDiagTestsView->setModel(& preDiagModel);
    d->ui.postDiagTestsView->setModel(&postDiagModel);
    d->ui. preDiagTestsView->setHeaderHidden(true);
    d->ui.postDiagTestsView->setHeaderHidden(true);

    d->ui. preDiagParametersView->setModel(&d->m_diag_parameter_model);
    d->ui.postDiagParametersView->setModel(&d->m_diag_parameter_model);

    d->m_ctx_parameter_model.setScope(Parameter::ValueScope::Edit);
    d->ui.parametersView->setModel(&d->m_ctx_parameter_model);

    d->m_resource_model.setScope(Parameter::ValueScope::Edit);
    d->ui.resourceOwnersView->setModel(&d->m_resource_model);

    connect(&d->m_ctx_parameter_model, &ParameterModel::dataChanged, &d->m_resource_model, &ResourceOwnersModel::checkConflicts);

    connect(d->ui.resourceOwnersView, &QAbstractItemView::clicked, this, [this](const QModelIndex& i){
        if ( !d->ui.ctxTabWidget->isTabVisible(0) )
            return;

        auto data = i.data(Qt::UserRole);
        if ( data.isNull() ) return;
        if ( auto* parameter = data.value<Parameter*>() ) {
            d->ui.ctxTabWidget->setCurrentIndex(0);
            int row = d->m_ctx_parameter_model.indexOf(parameter);
            d->ui.parametersView->highlight(d->m_ctx_parameter_model.index(row, 0));
        }
    });

    connect(d->ui.parametersView, &QAbstractItemView::clicked, this, [this](const QModelIndex& i){
        if ( !d->ui.ctxTabWidget->isTabVisible(1) )
            return;

        auto data = i.data(Qt::UserRole);
        if ( data.isNull() ) return;
        if ( auto* resource = data.value<Resource*>() ) {
            d->ui.ctxTabWidget->setCurrentIndex(1);
            d->ui.resourceOwnersView->highlight(d->m_resource_model.indexOf(resource));
        }
    });


#define CONTEXT_TO_TITLE(ctx, title) \
    d->m_contextToIndex[Parameter::Context:: ctx] = d->ui.stackedWidget->indexOf(d->ui. title);

    CONTEXT_TO_TITLE( Deploy,       deployPage );
    CONTEXT_TO_TITLE( Undeploy,   undeployPage );
    CONTEXT_TO_TITLE( Configure, configurePage );
    CONTEXT_TO_TITLE( Backup,       backupPage );
    CONTEXT_TO_TITLE( Restore,     restorePage );

#undef CONTEXT_TO_TITLE
}

ConfirmationPage::~ConfirmationPage() { delete d; }

bool ConfirmationPage::usePage()
{
    return true;
}

void ConfirmationPage::initializePage()
{
    QWizardPage::initializePage();

    setTitle(Controller::actionName(wizard()->action().action) + QString{" \"%0\""}.arg(wizard()->action().service->displayName()) );
    setSubTitle(tr("You may want to double-check your selection"));
    setPixmap(QWizard::LogoPixmap, Controller::actionIcon(wizard()->action().action).pixmap(32, 32));


    d->ui.scrollArea->ensureVisible(0,0);

    for ( auto* gb : findChildren<KCollapsibleGroupBox*>() )
        gb->collapse();

    for ( auto* tw : findChildren<QTabWidget*>() )
        tw->setCurrentIndex(0);


    const auto& action = wizard()->action();

    d->m_diagParameters.clear();
    d->m_ctxParameters.clear();

    for ( auto* parameter : d->m_parameters )
    {
        if ( parameter->contexts() & Parameter::Context::Diag )
            d->m_diagParameters.push_back(parameter);

        if ( parameter->contexts() & action.action )
            d->m_ctxParameters.push_back(parameter);
    }

    d->m_diag_parameter_model.setItems(d->m_diagParameters);
    d->m_ctx_parameter_model.setItems(d->m_ctxParameters);
    d->m_resource_model.setItems(action.service->resources());

    d->ui.        parametersView->header()->resizeSection(0, 300);
    d->ui. preDiagParametersView->header()->resizeSection(0, 300);
    d->ui.postDiagParametersView->header()->resizeSection(0, 300);



    { // PREDIAG/POSTDIAG STEP
        d->ui. preDiagStep->setVisible(action.options. prediag);
        d->ui.postDiagStep->setVisible(action.options.postdiag);

        d->ui. preDiagTestsView->expandAll();
        d->ui.postDiagTestsView->expandAll();

        d->ui. preDiagTabWidget->setTabVisible(1, !d->m_diagParameters.empty());
        d->ui.postDiagTabWidget->setTabVisible(1, !d->m_diagParameters.empty());
    }

    { // CONTEXTUAL STEP
        if (action.action != Parameter::Context::Diag)
            d->ui.stackedWidget->setCurrentIndex(d->m_contextToIndex.at(action.action));

        d->ui.contextStep->setVisible(action.action != Parameter::Context::Diag);
        d->ui.forceDeployMessage->setVisible(action.options.force);
        bool resourceTabNeeded = !action.service->resources().empty() &&
                                 action.modifiesResourses();

        d->ui.  deployResourceMessage->setVisible(action.action != Parameter::Context::Undeploy);
        d->ui.undeployResourceMessage->setVisible(action.action == Parameter::Context::Undeploy);

        d->ui.ctxTabWidget->setTabVisible(1, resourceTabNeeded);
        d->ui.ctxTabWidget->setTabVisible(0, !d->m_ctxParameters.empty());
        d->ui.contextDetails->setVisible(d->ui.ctxTabWidget->isTabVisible(0) || d->ui.ctxTabWidget->isTabVisible(1));
    }

    { // AUTOSTART STEP
        d->ui.startStep->setVisible(action.options.autostart);
    }

    bool diag = wizard()->action().options.prediag || wizard()->action().options.postdiag;
    const auto& tools = action.service->diagTools();

    d->m_resource_model.setItems(action.service->resources());
    for ( int r = 0; r < d->m_resource_model.rowCount(); ++r )
        d->ui.resourceOwnersView->setFirstColumnSpanned(r, {}, true);
    d->ui.resourceOwnersView->expandAll();
    d->ui.resourceOwnersView->header()->resizeSection(0, 200);
    d->ui.resourceOwnersView->header()->resizeSection(2, 200);
    d->ui.resourceOwnersView->setColumnNeverDetailed(1, true);

    d->ui.missingDiagWarning->setVisible(diag && action.service->isDiagMissing());
    d->ui.missingDiagParamsWarning->setVisible( diag && ranges::any_of( tools,
       [&,this](const std::unique_ptr<DiagTool>& tool){
           return ( action.options. prediag && action.hasTool(tool.get(), DiagTool::Test::Mode:: PreDeploy) && tool->isMissingParams() ) ||
                  ( action.options.postdiag && action.hasTool(tool.get(), DiagTool::Test::Mode::PostDeploy) && tool->isMissingParams() );
       })
    );
    d->ui.missingDiagWarning_2->setVisible(d->ui.missingDiagWarning->isVisible());
    d->ui.missingDiagParamsWarning_2->setVisible(d->ui.missingDiagParamsWarning->isVisible());

    d->ui.scrollArea->setMinimumWidth( d->ui.scrollAreaWidgetContents->sizeHint().width() );
}
