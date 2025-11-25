#ifndef RESOLVE_WIZARDPAGE_H
#define RESOLVE_WIZARDPAGE_H

#include "transactiontabwidget.h"
#include "wizardpage.h"

#include <memory>

namespace alt
{
class ResolveWizardPage : public WizardPage
{
    Q_OBJECT

public:
    void initialize() override;

public:
    void onResolve(const Transaction &transaction);

public:
    ResolveWizardPage(const std::shared_ptr<TransactionService> &service,
                      const std::shared_ptr<ComponentRepository> &components,
                      QWidget *parent = nullptr);
    ~ResolveWizardPage() override;

public:
    ResolveWizardPage(const ResolveWizardPage &) = delete;
    ResolveWizardPage(ResolveWizardPage &&) = delete;
    ResolveWizardPage &operator=(const ResolveWizardPage &) = delete;
    ResolveWizardPage &operator=(ResolveWizardPage &&) = delete;

private:
    std::shared_ptr<TransactionService> service;
    std::shared_ptr<ComponentRepository> componentRepository;
    std::unique_ptr<TransactionTabWidget> centralWidget;
};
} // namespace alt
#endif // RESOLVE_WIZARDPAGE_H
