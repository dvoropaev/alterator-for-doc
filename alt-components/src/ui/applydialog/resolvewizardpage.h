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
    explicit ResolveWizardPage(QWidget *parent = nullptr);
    ~ResolveWizardPage() override;

public:
    ResolveWizardPage(const ResolveWizardPage &) = delete;
    ResolveWizardPage(ResolveWizardPage &&) = delete;
    ResolveWizardPage &operator=(const ResolveWizardPage &) = delete;
    ResolveWizardPage &operator=(ResolveWizardPage &&) = delete;

private:
    std::unique_ptr<TransactionTabWidget> centralWidget;
};
} // namespace alt
#endif // RESOLVE_WIZARDPAGE_H
