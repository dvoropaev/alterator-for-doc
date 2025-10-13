#ifndef TRANSACTION_WIZARD_H
#define TRANSACTION_WIZARD_H

#include "ui/applydialog/wizard.h"

namespace alt
{
class RequestWizardPage;
class ResolveWizardPage;
class ProgressWizardPage;

class TransactionWizard : public Wizard
{
    Q_OBJECT

public:
    explicit TransactionWizard(QWidget *parent = nullptr);
    ~TransactionWizard() override;

public slots:
    void changeEvent(QEvent *event) override;
    void applyRequested();

private:
    void onSuccess() override;
    void onFailure(const DBusError &error) override;

private:
    std::unique_ptr<RequestWizardPage> requestPage;
    std::unique_ptr<ResolveWizardPage> resolvePage;
    std::unique_ptr<ProgressWizardPage> progressPage;
};
} // namespace alt
#endif // TRANSACTION_WIZARD_H
