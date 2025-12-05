#include "InitialPage.h"
#include "ui_InitialPage.h"

#include "controller/Controller.h"
#include "data/models/ResourceOwnersModel.h"


class InitialPage::Private {
public:
    Ui::InitialPage ui;
    ResourceOwnersModel m_current_resource_model;
};


InitialPage::InitialPage(QWidget *parent)
    : Page{parent}
    , d{new Private}
{
    d->ui.setupUi(this);

    connect(d->ui.preDiagCheckBox, &QCheckBox::toggled, this, [this](bool checked){
        wizard()->action().options.prediag = checked;
    });
    connect(d->ui.postDiagCheckBox, &QCheckBox::toggled, this, [this](bool checked){
        wizard()->action().options.postdiag = checked;
    });
    connect(d->ui.autoStartCheckBox, &QCheckBox::toggled, this, [this](bool checked){
        wizard()->action().options.autostart = checked;
    });
    connect(d->ui.forceDeployCheckBox, &QCheckBox::toggled, this, [this](bool checked){
        wizard()->action().options.force = checked;
        d->ui.otherOptionsContainer->setEnabled(!d->m_current_resource_model.hasConflicts() ||
                                                d->m_current_resource_model.conflictsResolvable() ||
                                                checked);
        d->ui.preDiagCheckBox->setDisabled(checked);
        d->ui.preDiagCheckBox->setChecked(d->ui.preDiagCheckBox->isVisible() && !checked);
        emit completeChanged();
    } );

    d->m_current_resource_model.setScope(Parameter::ValueScope::Current);
    d->ui.resourceOwnersView->setModel(&d->m_current_resource_model);
}

InitialPage::~InitialPage() { delete d; }

bool InitialPage::usePage() {
    const auto& action = wizard()->action();
    return action.modifiesResourses() ||
           action.preDiagAvailable() ||
           action.postDiagAvailable();
}

bool InitialPage::isComplete() const
{
    const auto& action = wizard()->action();
    return action.action != Parameter::Context::Deploy ||
        !d->m_current_resource_model.hasConflicts() ||
        d->m_current_resource_model.conflictsResolvable() ||
        action.options.force;
}

void InitialPage::initializePage()
{
    QWizardPage::initializePage();

    setTitle(Controller::actionName(wizard()->action().action) + QString{" \"%0\""}.arg(wizard()->action().service->displayName()) );
    setSubTitle(wizard()->action().service->comment());
    setPixmap(QWizard::LogoPixmap, Controller::actionIcon(wizard()->action().action).pixmap(32, 32));

    const auto& action = wizard()->action();
    bool deploy = action.action == Parameter::Context::Deploy;

    d->ui.   autoStartCheckBox->setHidden(action.service->isStarted() || !deploy );
    d->ui.   autoStartCheckBox->setChecked(!d->ui.autoStartCheckBox->isHidden() && wizard()->action().options.autostart);

    d->ui. forceDeployCheckBox->setVisible( deploy && action.service->isForceDeployable() );
    d->ui. forceDeployCheckBox->setChecked( !d->ui.forceDeployCheckBox->isHidden() && wizard()->action().options.force );

    d->ui.     preDiagCheckBox->setVisible( action.preDiagAvailable() );
    d->ui.     preDiagCheckBox->setChecked( wizard()->action().options.prediag );

    d->ui.    postDiagCheckBox->setVisible( action.postDiagAvailable() );
    d->ui.    postDiagCheckBox->setChecked( wizard()->action().options.postdiag );

    d->m_current_resource_model.setItems(action.service->resources());
    for ( int r = 0; r < d->m_current_resource_model.rowCount(); ++r )
        d->ui.resourceOwnersView->setFirstColumnSpanned(r, {}, true);
    d->ui.resourceOwnersView->expandAll();
    d->ui.resourceOwnersView->header()->resizeSection(0, 200);
    d->ui.resourceOwnersView->header()->resizeSection(2, 200);
    d->ui.resourceOwnersView->setColumnNeverDetailed(1, true);

    d->ui.  deployResourceMessage->setVisible(action.action == Parameter::Context::  Deploy);
    d->ui.undeployResourceMessage->setVisible(action.action == Parameter::Context::Undeploy);

    // TODO: REFACTOR: reorganize enums
    bool needCheck   = action.modifiesResourses() && action.action != Parameter::Context::Undeploy;
    bool hasConflict = needCheck   && d->m_current_resource_model.hasConflicts();
    bool resolvable  = hasConflict && d->m_current_resource_model.conflictsResolvable();

    d->ui.resourcesGroupBox->setVisible(action.modifiesResourses());

    d->ui.conflictsDetectedMessage     ->setVisible(  hasConflict );
    d->ui.conflictsOkMessage           ->setVisible( needCheck && !action.service->isDeployed() && !hasConflict );
    d->ui.conflictsResolvableMessage   ->setVisible(   resolvable );
    d->ui.conflictsUnresolvableMessage ->setVisible( hasConflict && !resolvable );

    d->ui.optionsGroupBox->setHidden( ranges::all_of(d->ui.optionsGroupBox->findChildren<QCheckBox*>(), &QCheckBox::isHidden) );
    d->ui.otherOptionsContainer->setEnabled(!hasConflict || resolvable || action.options.force);
}

