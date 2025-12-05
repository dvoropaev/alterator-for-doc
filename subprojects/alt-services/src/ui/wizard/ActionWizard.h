#pragma once

#include <QWizard>
#include "data/Action.h"

class ActionWizard : public QWizard
{
    Q_OBJECT

public:
    explicit ActionWizard(QWidget *parent = nullptr);
    ~ActionWizard();

    void open(Action action);
    Action& action();

    static const inline auto ParametersImportButton = WizardButton::CustomButton1;
    static const inline auto ParametersExportButton = WizardButton::CustomButton2;
    static const inline auto LogExportButton        = WizardButton::CustomButton3;

public:
    void closeEvent(QCloseEvent* event) override;
    int nextId() const override;

public slots:
    void exportParameters();

protected:
    void dropEvent(QDropEvent* event) override;

private:
    class Private;
    Private* d{};
};
