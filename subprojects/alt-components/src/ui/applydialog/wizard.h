#ifndef WIZARD_H
#define WIZARD_H

#include "dbus/dbuserror.h"
#include "ui/applydialog/wizardpage.h"
#include "ui/statusbar/applystatusbar.h"
#include <QDialog>

namespace alt
{
namespace Ui
{
class Wizard;
} // namespace Ui

class Wizard : public QDialog
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
    explicit Wizard(QWidget *parent = nullptr);
    ~Wizard() override;

public:
    QPushButton *button(WizardButton button);
    ApplyStatusBar *statusBar();

    void setNextPage();
    void setPreviousPage();
    void setPage(int index);
    WizardPage *currentPage();

public slots:
    void changeEvent(QEvent *event) override;

protected:
    void advancePage(int offset);
    virtual void onSuccess();
    virtual void onFailure(const DBusError &error);

protected:
    std::unique_ptr<Ui::Wizard> ui;
};
} // namespace alt
#endif // WIZARD_H
