#ifndef REQUEST_WIZARDPAGE_H
#define REQUEST_WIZARDPAGE_H

#include "repository/componentrepository.h"
#include "service/transactionservice.h"
#include "transactiontabwidget.h"
#include "wizardpage.h"

#include <memory>

namespace alt
{
class RequestWizardPage : public WizardPage
{
    Q_OBJECT

public:
    void initialize() override;

public:
    RequestWizardPage(const std::shared_ptr<TransactionService> &service,
                      const std::shared_ptr<ComponentRepository> &components,
                      QWidget *parent = nullptr);
    ~RequestWizardPage() override;

public:
    RequestWizardPage(const RequestWizardPage &) = delete;
    RequestWizardPage(RequestWizardPage &&) = delete;
    RequestWizardPage &operator=(const RequestWizardPage &) = delete;
    RequestWizardPage &operator=(RequestWizardPage &&) = delete;

private:
    std::shared_ptr<TransactionService> service;
    std::shared_ptr<ComponentRepository> componentRepository;
    std::unique_ptr<TransactionTabWidget> centralWidget;
};
} // namespace alt
#endif // REQUEST_WIZARDPAGE_H
