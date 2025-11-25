#ifndef UPDATING_SOURCES_WIZARD_H
#define UPDATING_SOURCES_WIZARD_H

#include "wizard.h"

namespace alt
{
class ProgressWizardPage;

class UpdatingSourcesWizard : public Wizard
{
    Q_OBJECT

public:
    explicit UpdatingSourcesWizard(QWidget *parent = nullptr);
    ~UpdatingSourcesWizard() override;

public slots:
    void changeEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void updateRequested();

private:
    void onSuccess() override;
    void onFailure(const QString &error) override;

private:
    std::unique_ptr<ProgressWizardPage> progressPage;
};
} // namespace alt
#endif // UPDATING_SOURCES_WIZARD_H
