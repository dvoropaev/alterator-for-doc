#include "DiagSelectionPage.h"
#include "ui_DiagSelectionPage.h"

class DiagSelectionPage::Private {
public:
    inline explicit Private(DiagTool::Test::Mode mode)
        : m_mode{mode}
    {}

    Ui::DiagSelectionPage ui;
    DiagTool::Test::Mode m_mode;
};

DiagSelectionPage::DiagSelectionPage(DiagTool::Test::Mode mode, QWidget* parent)
    : Page{parent}
    , d{new Private{mode}}
{
    d->ui.setupUi(this);
    d->ui.diagSelector->setMode(mode);

    setPixmap(QWizard::LogoPixmap, QIcon::fromTheme("applications-system").pixmap(32, 32));
    setTitle( d->m_mode == DiagTool::Test::Mode::PreDeploy
        ? tr("Premilinary diagnostics")
        : tr("Post-deploy diagnostics")
    );
    setSubTitle( d->m_mode == DiagTool::Test::Mode::PreDeploy
        ? tr("Run diagnostic tests to determine if deployment is currently possible")
        : tr("Run diagnostic tests to check deployed service")
    );

    connect(&d->ui.diagSelector->model(), &QAbstractItemModel::dataChanged, this, &DiagSelectionPage::completeChanged);
}

DiagSelectionPage::~DiagSelectionPage() { delete d; }

bool DiagSelectionPage::usePage()
{
    return d->m_mode == DiagTool::Test::Mode::PreDeploy
               ? wizard()->action().options.prediag
               : wizard()->action().options.postdiag;
}

void DiagSelectionPage::initializePage()
{
    QWizardPage::initializePage();

    d->ui.diagSelector->setMode(d->m_mode);
    d->ui.diagSelector->setService(wizard()->action().service, wizard()->action().options.prediagTests);
}

bool DiagSelectionPage::isComplete() const
{
    return d->ui.diagSelector->anySelected();
}

QAbstractItemModel& DiagSelectionPage::model()
{
    return d->ui.diagSelector->model();
}
