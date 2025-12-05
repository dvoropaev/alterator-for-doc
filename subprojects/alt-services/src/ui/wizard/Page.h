#pragma once

#include <QWizardPage>
#include "ActionWizard.h"

class Page : public QWizardPage {
    Q_OBJECT
protected:
    using QWizardPage::QWizardPage;

    inline
    ActionWizard* wizard() const
    { return static_cast<ActionWizard*>(QWizardPage::wizard()); }

public:
    virtual bool usePage() = 0;
};
