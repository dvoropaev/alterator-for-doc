#include "ProgressPage.h"
#include "ui_ProgressPage.h"

#include "app/ServicesApp.h"
#include "controller/Controller.h"

ProgressPage::ProgressPage(QWidget *parent)
    : Page(parent)
    , ui(new Ui::ProgressPage)
{
    ui->setupUi(this);

    connect(qApp->controller(), &Controller::actionBegin, this, [this]{ui->progressBar->show(); emit completeChanged();});
    connect(qApp->controller(), &Controller::actionEnd,   this, [this]{ui->progressBar->hide(); emit completeChanged();});
    connect(qApp->controller(), &Controller::actionBegin, ui->logWidget, &LogWidget::beginEntry);
    connect(qApp->controller(), &Controller::actionEnd,   ui->logWidget, &LogWidget::endEntry);
    connect(qApp->controller(), &Controller::stdout,      ui->logWidget, &LogWidget::message);
    connect(qApp->controller(), &Controller::stderr,      ui->logWidget, &LogWidget::error);
}

ProgressPage::~ProgressPage() { delete ui; }

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

    ui->logWidget->setExportFileName(ctxmap.at(wizard()->action().action)+'_'+wizard()->action().service->name());
    ui->logWidget->clear();
}

bool ProgressPage::isComplete() const
{
    return ui->progressBar->isVisible();
}

QAction* ProgressPage::exportAction() const
{
    return ui->logWidget->exportAction();
}
