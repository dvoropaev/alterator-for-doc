#include "ProgressPage.h"
#include "ui_ProgressPage.h"

#include "app/ServicesApp.h"
#include "controller/Controller.h"

class ProgressPage::Private {
public:
    Ui::ProgressPage ui;

    bool m_skip_all{false};

    QAction* m_ignoreAction{};
    QAction* m_ignoreAllAction{};
    QAction* m_abortAction{};

    QEventLoop m_loop;

    class DiagErrorHandler : public Controller::DiagErrorHandler
    {
        Private& m_self;
    public:
        DiagErrorHandler(Private& self)
            : m_self{self}
        {}

        virtual Controller::Result handleError(DiagTool::Test* test, DiagTool::Test::Mode mode) override
        {
            if ( m_self.m_skip_all )
                return Controller::Result::ErrorIgnored;

            m_self.ui.diagErrorMessage->setText(
                QObject::tr("Warning: test \"%0\" failed!")
                    .arg(test->displayName())
            );
            m_self.ui.diagErrorMessage->animatedShow();
            return m_self.m_loop.exec()
                ? Controller::Result::ErrorIgnored
                : Controller::Result::Error;
        }
    } m_handler{*this};
};

ProgressPage::ProgressPage(QWidget *parent)
    : Page(parent)
    , d{new Private}
{
    d->ui.setupUi(this);

    qApp->controller()->installDiagErrorHandler(&d->m_handler);

    connect(qApp->controller(), &Controller::operationBegin, this, [this]
    {
        d->ui.progressBar->show();
        emit completeChanged();
    });

    connect(qApp->controller(), &Controller::operationEnd, this, [this]( Controller::Result r, const Action& action )
    {
        d->ui.resultMessage->setMessageType(
            r == Controller::Result::Error
                ? KMessageWidget::Error
                : KMessageWidget::Positive
        );

        d->ui.resultMessage->setIcon(QIcon::fromTheme(
            r == Controller::Result::Error
                ? "dialog-error"
                : "dialog-ok"
        ));

        d->ui.resultMessage->setText(
            r == Controller::Result::Error
                ? tr("Errors occured during operation")
                : tr("Operation performed successfully")
        );

        d->ui.resultMessage->animatedShow();
        d->ui.progressBar->hide();
        emit completeChanged();
    });

    connect(qApp->controller(), &Controller::stepBegin, d->ui.logWidget, &LogWidget::beginEntry);
    connect(qApp->controller(), &Controller::stepEnd,   d->ui.logWidget, &LogWidget::endEntry);
    connect(qApp->controller(), &Controller::stdout,      d->ui.logWidget, &LogWidget::message);
    connect(qApp->controller(), &Controller::stderr,      d->ui.logWidget, &LogWidget::error);


    d->m_ignoreAction      = new QAction{tr("Ignore"),     d->ui.diagErrorMessage};
    d->m_ignoreAllAction   = new QAction{tr("Ignore all"), d->ui.diagErrorMessage};
    d->m_abortAction       = new QAction{tr("Abort"),      d->ui.diagErrorMessage};

    d->m_ignoreAction    ->setToolTip(tr("Ignore this error and continue"));
    d->m_ignoreAllAction ->setToolTip(tr("Ignore all errors and continue (do not show this message again)"));

    connect(d->m_ignoreAction,      &QAction::triggered, this, [this]
    {
        d->m_loop.exit(1);
        d->ui.diagErrorMessage->animatedHide();
    });

    connect(d->m_ignoreAllAction,   &QAction::triggered, this, [this]
    {
        d->m_skip_all = true;
        d->m_loop.exit(1);
        d->ui.diagErrorMessage->animatedHide();
    });

    connect(d->m_abortAction, &QAction::triggered, this, [this]
    {
        d->m_loop.exit(0);
        d->ui.diagErrorMessage->animatedHide();
    });

    d->ui.diagErrorMessage->addAction(d->m_ignoreAction     );
    d->ui.diagErrorMessage->addAction(d->m_ignoreAllAction  );
    d->ui.diagErrorMessage->addAction(d->m_abortAction      );
}

ProgressPage::~ProgressPage() { delete d; }

bool ProgressPage::usePage()
{
    return true;
}

static const std::map<Parameter::Contexts, QString> ctxmap {
    { Parameter::Context::Configure, "configure"},
    { Parameter::Context::Deploy   , "deploy"   },
    { Parameter::Context::Undeploy , "undeploy" },
    { Parameter::Context::Diag     , "diag"     },
    { Parameter::Context::Backup   , "backup"   },
    { Parameter::Context::Restore  , "restore"  },
};

void ProgressPage::initializePage()
{
    QWizardPage::initializePage();

    d->m_skip_all = false;

    d->ui.diagErrorMessage->hide();
    d->ui.resultMessage->hide();
    d->ui.logWidget->setExportFileName(ctxmap.at(wizard()->action().action)+'_'+wizard()->action().service->name());
    d->ui.logWidget->clear();
}

bool ProgressPage::isComplete() const
{
    return d->ui.progressBar->isVisible();
}

QAction* ProgressPage::exportAction() const
{
    return d->ui.logWidget->exportAction();
}
