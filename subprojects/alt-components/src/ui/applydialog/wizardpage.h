#ifndef WIZARD_PAGE_H
#define WIZARD_PAGE_H

#include "dbus/dbuserror.h"
#include <memory>
#include <QVBoxLayout>
#include <QWidget>

namespace alt
{
namespace Ui
{
class WizardPage;
} // namespace Ui

class Wizard;
class WizardPage : public QWidget
{
    Q_OBJECT

public:
    virtual void initialize();
    void setTitle(const QString &text);
    void setSubTitle(const QString &text);
    void setWizard(Wizard *wizard);

public slots:
    void onError(const DBusError &error);

public:
    explicit WizardPage(QWidget *parent = nullptr);
    ~WizardPage() override;

public:
    WizardPage(const WizardPage &) = delete;
    WizardPage(WizardPage &&) = delete;
    WizardPage &operator=(const WizardPage &) = delete;
    WizardPage &operator=(WizardPage &&) = delete;

protected:
    QVBoxLayout *layout();

protected:
    std::unique_ptr<Ui::WizardPage> ui;
    Wizard *wizard;
};
} // namespace alt
#endif // WIZARD_PAGE_H
