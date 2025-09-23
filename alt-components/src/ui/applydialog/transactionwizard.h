#ifndef TRANSACTION_WIZARD_H
#define TRANSACTION_WIZARD_H

#include "dbus/dbuserror.h"
#include "ui/applydialog/wizardpage.h"
#include "ui/statusbar/applystatusbar.h"
#include <QDialog>

namespace alt
{
namespace Ui
{
class TransactionWizard;
} // namespace Ui

class TransactionWizard : public QDialog
{
    Q_OBJECT

public:
    enum WizardButton
    {
        CancelButton,
        ApplyButton,
        BackButton,
        NextButton,
        FinishButton,
        RetryButton,
    };

public:
    explicit TransactionWizard(QWidget *parent = nullptr);
    ~TransactionWizard() override;

public:
    QPushButton *button(WizardButton button);
    ApplyStatusBar *statusBar();

    void setNextPage();
    void setPreviousPage();
    void setPage(int index);
    WizardPage *currentPage();

public slots:
    void applyRequested();
    void changeEvent(QEvent *event) override;

private:
    void advancePage(int offset);
    void onApplied();
    void onError(const DBusError &error);

private:
    std::unique_ptr<Ui::TransactionWizard> ui;
};
} // namespace alt
#endif // TRANSACTION_WIZARD_H
