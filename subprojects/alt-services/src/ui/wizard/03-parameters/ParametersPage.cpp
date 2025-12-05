#include "ParametersPage.h"
#include "ui_ParametersPage.h"

#include "data/Parameter.h"
#include "forms/CompactForm.h"
#include "forms/DetailedForm.h"

#include "app/ServicesApp.h"
#include "controller/Controller.h"

#include <QMenuBar>
#include <QActionGroup>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QAbstractButton>

#include <QToolTip>

#include <range/v3/iterator.hpp>

class ParametersPage::Private {
public:
    Ui::ParametersPage ui;

    QAction* actionLowerUnrequired{};
    QAction* actionModeCompact{};
    QAction* actionModeDetailed{};

    std::unique_ptr<Property::Value::ValidationInfo> m_invalid{};

    Service* m_service{};
    std::vector<Parameter*> m_parameters;
    std::unique_ptr<BaseForm> m_form{};
};


ParametersPage::ParametersPage(QWidget *parent)
    : Page{parent}
    , d{new Private}
{
    d->ui.setupUi(this);

    setPixmap(QWizard::LogoPixmap, QIcon::fromTheme("preferences-system").pixmap(32, 32));

    { // MENU
        auto menuBar = new QMenuBar{this};
        layout()->setMenuBar(menuBar);

        {
            QMenu* edit = menuBar->addMenu(tr("&Edit"));

            QAction* actionSearch = edit->addAction(tr("&Find..."), QKeySequence::Find);
            actionSearch->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::EditFind));

            d->ui.searchBar->hide();

            connect(actionSearch, &QAction::triggered, d->ui.searchBar, &QWidget::show);
        }

        {
            QMenu* view = menuBar->addMenu(tr("&View"));

            d->actionLowerUnrequired = view->addAction(tr("Put non-required parameters at the end"));
            d->actionLowerUnrequired->setCheckable(true);
            d->actionLowerUnrequired->setChecked(qApp->settings()->lowerUnrequired());

            connect(d->actionLowerUnrequired, &QAction::toggled, this, &ParametersPage::fillParameters);

            QAction* mode = view->addSection(tr("Mode"));
            QActionGroup* group = new QActionGroup{mode};
            group->setExclusive(true);

            d->actionModeCompact  = view->addAction(tr("&Table"));
            d->actionModeDetailed = view->addAction(tr("&List"));

            d->actionModeCompact ->setCheckable(true);
            d->actionModeDetailed->setCheckable(true);

            d->actionModeCompact ->setActionGroup(group);
            d->actionModeDetailed->setActionGroup(group);

            ( qApp->settings()->editorTableMode()
                 ? d->actionModeDetailed
                 : d->actionModeCompact  )->setChecked(true);

            connect(d->actionModeDetailed, &QAction::toggled, qApp->settings(), &AppSettings::set_editorTableMode);
            connect(d->actionModeDetailed, &QAction::toggled, this, &ParametersPage::setForm);

            view->addActions(qApp->controller()->tableActions());
        }
    }
}

ParametersPage::~ParametersPage() { delete d; }

bool ParametersPage::usePage()
{
    initializePage();
    return !d->m_parameters.empty();
}

void ParametersPage::on_invalidParameterWarning_linkActivated(const QString& link)
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

void ParametersPage::on_invalidParameterWarning_linkHovered(const QString& link)
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

void ParametersPage::setForm()
{
    d->m_form.reset( d->actionModeCompact->isChecked()
        ? static_cast<BaseForm*>( new  CompactForm(wizard()->action(), this) )
        : static_cast<BaseForm*>( new DetailedForm(wizard()->action(), this) ) );

    d->ui.formContainer->addWidget(d->m_form.get());
    d->m_form->setParameters(d->m_parameters, wizard()->action().action);

    connect(d->m_form.get(), &BaseForm::changed, this, &ParametersPage::validateParameters);
    d->ui.searchBar->setAdapter(d->m_form->searchAdapter());
}

void ParametersPage::validateParameters()
{
    std::unique_ptr<Property::Value::ValidationInfo> invalidPtr;

    for ( auto* param : d->m_parameters )
        if ( (invalidPtr = param->value(Parameter::ValueScope::Edit)->isInvalid(wizard()->action().options.force)) )
            break;

    if ( invalidPtr ) {

        int level = 0;
        QStringList path;
        QString* message = &invalidPtr->message;

        auto* invalid = invalidPtr.get();

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

    std::invoke( d->m_invalid
                    ? &KMessageWidget::animatedShow
                    : &KMessageWidget::animatedHide,
                d->ui.invalidParameterWarning );

    emit completeChanged();
}

void ParametersPage::fillParameters()
{
    Parameter::Contexts ctx{wizard()->action().action};

    // for deploy and configure
    if ( wizard()->action().options.prediag || wizard()->action().options.postdiag )
        ctx.setFlag(Parameter::Context::Diag);

    auto oldParameters = d->m_parameters;
    d->m_parameters.clear();
    bool useCurrent = wizard()->action().service->isDeployed();

    bool fillFromPlayfile = !wizard()->action().parameters.empty();
    if ( oldParameters.empty() && fillFromPlayfile )
    {
        if ( !wizard()->action().service->tryFill( wizard()->action().parameters, ctx) )
        {
            QMessageBox mb{this};
            mb.setIcon(QMessageBox::Icon::Warning);
            mb.setWindowTitle(tr("Invalid data"));

            mb.setStandardButtons(QMessageBox::StandardButton::RestoreDefaults |
                                  QMessageBox::StandardButton::Ignore);
            mb.setText(
                tr("Some of the required parameters are missing or invalid.")
                    .append('\n')
                    .append(tr("Press \"%1\" to use default parameters instead.")
                        .arg(mb.button(QMessageBox::StandardButton::RestoreDefaults)->text().remove('&')))
                    .append('\n')
                    .append(tr("Press \"%1\" to load them anyway.")
                        .arg(mb.button(QMessageBox::StandardButton::Ignore)->text().remove('&')))
                    .append('\n')
                    .append(tr("You may still need to enter some parameters manually."))
                );
            mb.exec();

            fillFromPlayfile = mb.clickedButton() != mb.button(QMessageBox::StandardButton::RestoreDefaults);
        }
    }

    std::vector<Parameter*> optionalParameters;
    for ( const auto& param : wizard()->action().service->parameters() ) {
        if ( !param->contexts().testAnyFlags(ctx) || param->isConstant() )
            continue;

        if ( !fillFromPlayfile || !ranges::contains(oldParameters, param.get()) )
            param->fillFromValue(useCurrent); // FIXME: do it inside service, when status changed

        param->value(Parameter::ValueScope::Edit)->setEnabled(
            param->required().testAnyFlags(ctx) ||
            (useCurrent && param->value(Parameter::ValueScope::Current)->isEnabled())
        );

        ( !d->actionLowerUnrequired->isChecked() || param->required().testAnyFlags(ctx)
             ? &d->m_parameters
             : &optionalParameters )->push_back(param.get());
    }

    ranges::copy(optionalParameters, ranges::back_inserter(d->m_parameters));

    d->m_form->setParameters(d->m_parameters, ctx);
    validateParameters();
}

void ParametersPage::initializePage()
{
    QWizardPage::initializePage();

    if ( d->m_service != wizard()->action().service )
    {
        d->m_parameters.clear();
        d->m_service = wizard()->action().service;
    }

    d->ui.invalidParameterWarning->hide();
    setForm();
    fillParameters();
}

bool ParametersPage::isComplete() const
{
    return !d->m_invalid;
}

const std::vector<Parameter*>& ParametersPage::parameters()
{
    return d->m_parameters;
}
