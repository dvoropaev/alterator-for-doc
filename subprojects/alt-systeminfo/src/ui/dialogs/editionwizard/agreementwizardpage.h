#ifndef AGREEMENTWIZARDPAGE_H
#define AGREEMENTWIZARDPAGE_H

#include "controller/editioncontroller.h"

#include <QWizardPage>

#include <memory>

namespace alt
{
namespace Ui
{
class AgreementWizardPage;
} // namespace Ui

class AgreementWizardPage : public QWizardPage
{
    Q_OBJECT

public:
    AgreementWizardPage(QWidget *parent = nullptr);
    ~AgreementWizardPage();

public:
    void setController(EditionController *controller);

private:
    bool isComplete() const override;
    void initializePage() override;

private:
    AgreementWizardPage(const AgreementWizardPage &) = delete;
    AgreementWizardPage(AgreementWizardPage &&) = delete;
    AgreementWizardPage &operator=(const AgreementWizardPage &) = delete;
    AgreementWizardPage &operator=(AgreementWizardPage &&) = delete;

private:
    std::unique_ptr<Ui::AgreementWizardPage> m_ui;
    EditionController *m_controller;
};
} // namespace alt
#endif // AGREEMENTWIZARDPAGE_H
