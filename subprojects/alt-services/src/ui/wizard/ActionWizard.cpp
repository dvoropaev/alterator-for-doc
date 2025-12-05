#include "ActionWizard.h"

#include "controller/Controller.h"
#include "app/ServicesApp.h"
#include "data/Service.h"

#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMimeData>
#include <QDropEvent>
#include <qabstractbutton.h>

#include "01-initial/InitialPage.h"
#include "02-diag-selection/DiagSelectionPage.h"
#include "03-parameters/ParametersPage.h"
#include "04-confirmation/ConfirmationPage.h"
#include "05-progress/ProgressPage.h"

#include <range/v3/view.hpp>

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
          InitialPage* m_initialPage{};
    DiagSelectionPage* m_preDiagPage{},
                     * m_postDiagPage{};
       ParametersPage* m_parametersPage{};
     ConfirmationPage* m_confirmationPage{};
         ProgressPage* m_progressPage{};

    Action playfile;
    QSize m_defaultSize{629, 670};
};


enum Id
{
    Initial,

    PreDiag,
    PostDiag,

    Parameters,

    Confirmation,

    Progress,

    PAGE_COUNT
};

ActionWizard::ActionWizard(QWidget *parent)
    : QWizard{parent}
    , d{new Private{}}
{
    setAttribute(Qt::WA_AlwaysShowToolTips);
    setWizardStyle(QWizard::ClassicStyle);


    auto* importBtn = new QPushButton{QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpen  ), tr("&Import..."), this};
    auto* exportBtn = new QPushButton{QIcon::fromTheme(QIcon::ThemeIcon::DocumentSaveAs), tr("&Export..."), this};
    importBtn->setToolTip(tr("Import previously saved options"));
    exportBtn->setToolTip(tr("Save all the selected options for future reuse"));
    connect(importBtn, &QPushButton::clicked, this, [this]
    {
        auto fileName = QFileDialog::getOpenFileName(this,
            tr("Import saved parameters"),
            QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
            "JSON files (*.json)"
        );

        if ( auto parameters = qApp->importParameters(fileName) )
            open(parameters.value());
    });
    connect(exportBtn, &QPushButton::clicked, this, &ActionWizard::exportParameters);
    setButton(ImportButton, importBtn);
    setButton(ExportButton, exportBtn);
    setOption(QWizard::HaveCustomButton1);
    setOption(QWizard::HaveCustomButton2);

    setButtonLayout({
        ImportButton, ExportButton,
        QWizard::Stretch,
        QWizard::BackButton, QWizard::NextButton, QWizard::CommitButton, QWizard::FinishButton, QWizard::CancelButton
    });

    d->m_initialPage       = new InitialPage{this};
    d-> m_preDiagPage      = new DiagSelectionPage{DiagTool::Test::Mode:: PreDeploy, this};
    d->m_postDiagPage      = new DiagSelectionPage{DiagTool::Test::Mode::PostDeploy, this};
    d->m_parametersPage    = new ParametersPage{this};
    d->m_confirmationPage  = new ConfirmationPage{ d->m_parametersPage->parameters(),
                                             d->m_preDiagPage->model(),
                                             d->m_postDiagPage->model(),
                                             this };
    d->m_progressPage      = new ProgressPage{this};

    setPage(Id::Initial,       d-> m_initialPage      );
    setPage(Id::PreDiag,       d->  m_preDiagPage     );
    setPage(Id::PostDiag,      d-> m_postDiagPage     );
    setPage(Id::Parameters,    d-> m_parametersPage   );
    setPage(Id::Confirmation,  d-> m_confirmationPage );
    setPage(Id::Progress,      d-> m_progressPage     );

    connect(this, &QWizard::currentIdChanged, this, [this](int id)
    {
        button(ImportButton)->setHidden(id != Id::Initial);
        button(ExportButton)->setHidden(id  < Id::Confirmation );

        if ( id == Id::Progress )
        {
            button(QWizard::FinishButton)->setEnabled(false);
            button(QWizard::CancelButton)->setEnabled(false);

            bool success = true;

            d->playfile.parameters = d->playfile.service->getParameters(d->playfile.action, false);

            if ( d->playfile.action == Parameter::Context::Deploy && d->playfile.service->isForceDeployable() )
                d->playfile.parameters["force_deploy"] = d->playfile.options.force;

            success = qApp->controller()->call(d->playfile);

            // NOTE: if deploy succeeded but only post-diag failed, we can't go back anymore
            if ( d->playfile.action == Parameter::Context::Deploy )
                button(QWizard::BackButton)->setEnabled( d->playfile.service->isDeployed() || !success);

            button(QWizard::FinishButton)->setEnabled(success);
            button(QWizard::CancelButton)->setEnabled(true);
        }
    });
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


    if ( action.service->isStarted() )
        action.options.autostart = false;

    d->playfile = action;

    { // fill titles
        const auto& [windowTitle, pageTitle, pageSubtitle] = ctxnames.at(d->playfile.action)();
        setWindowTitle(windowTitle);
        page(Id::Parameters)->setTitle(pageTitle);
        page(Id::Parameters)->setSubTitle(pageSubtitle);

        page(Id::Confirmation)->setTitle(page(Id::Initial)->title());
        page(Id::Confirmation)->setSubTitle(page(Id::Initial)->subTitle());
    }


    QWizard::show();
    QWizard::restart();

    /*
     *  Set ConfirmationPage as a commit page if it is used.
     *  If not - set a previous one.
     */
    bool use = d->m_confirmationPage->usePage();
    d->m_confirmationPage->setCommitPage(use);
    for ( int i = Id::Confirmation-1; i >= Id::Initial; --i )
    {
        auto* commitpage = static_cast<Page*>(page(i));
        if ( commitpage->usePage() )
        {
            commitpage->setCommitPage(!use);
            break;
        }
    }

    if ( !d->m_initialPage->usePage() )
        next();

    d->m_parametersPage->initializePage();

    resize( currentId() == 0 && page(0)->isCommitPage() ? minimumSize() : d->m_defaultSize );
}

Action& ActionWizard::action() { return d->playfile; }

void ActionWizard::exportParameters()
{
    auto ctx = ranges::find(ctxmap | ranges::views::values, d->playfile.action).base();
    if ( ctx == ctxmap.cend() ) {
        qCritical() << "invalid context";
        return;
    }

    auto contextName = ctx->first;

    auto name = QFileDialog::getSaveFileName(this,
        tr("Export current parameters"),
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
            .append('/'+contextName+'_'+d->playfile.service->name()+".playfile.json"),
        "JSON files (*.json)"
    );

    if ( name.isEmpty() ) return;

    QFile f{name};
    if ( ! f.open(QIODevice::WriteOnly) ) return; // TODO: add error message

    QByteArray data = d->playfile.serialize();

    f.write(data);
    f.close();
}

void ActionWizard::dropEvent(QDropEvent* event)
{
    if ( auto parameters = qApp->importParameters(event->mimeData()->urls().at(0).toLocalFile()) )
        open(parameters.value());
}

int ActionWizard::nextId() const
{
    for ( int i = currentId()+1; i < PAGE_COUNT; ++i )
        if ( static_cast<Page*>(page(i))->usePage() )
            return i;

    return QWizard::nextId();
}

void ActionWizard::closeEvent(QCloseEvent* event)
{
    if ( !button(QWizard::CancelButton)->isEnabled() ) {
        event->ignore();
        return;
    }

    QWizard::closeEvent(event);
}
