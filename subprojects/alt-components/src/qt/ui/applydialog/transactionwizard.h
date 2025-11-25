#ifndef TRANSACTION_WIZARD_H
#define TRANSACTION_WIZARD_H

#include "repository/componentrepository.h"
#include "service/transactionservice.h"
#include "wizard.h"

namespace alt
{
class RequestWizardPage;
class ResolveWizardPage;
class ProgressWizardPage;

class TransactionWizard : public Wizard
{
    Q_OBJECT;

public:
    TransactionWizard(const std::shared_ptr<TransactionService> &service,
                      const std::shared_ptr<ComponentRepository> &components,
                      QWidget *parent = nullptr);
    ~TransactionWizard() override;

public slots:
    void changeEvent(QEvent *event) override;
    void applyRequested();

private:
    void onSuccess() override;
    void onFailure(const QString &error) override;

private:
    std::shared_ptr<TransactionService> service;
    std::shared_ptr<ComponentRepository> componentRepository;
    std::unique_ptr<RequestWizardPage> requestPage;
    std::unique_ptr<ResolveWizardPage> resolvePage;
    std::unique_ptr<ProgressWizardPage> progressPage;
};
} // namespace alt
#endif // TRANSACTION_WIZARD_H
