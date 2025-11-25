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

private slots:
    void validateParameters();
    bool validateCurrentPage() override;
    void fillCurrentParameters(bool firstFill = false);
    void onModeChanged();
    void exportParameters();
    void on_invalidParameterWarning_linkActivated(const QString &link);
    void on_invalidParameterWarning_linkHovered(const QString &link);
    void on_autoStartCheckBox_toggled(bool checked);
    void on_forceDeployCheckBox_toggled(bool checked);
    void on_finishPageCheckBox_clicked(bool checked);
    void on_ActionWizard_currentIdChanged(int id);

    // QObject interface
public:
    void closeEvent(QCloseEvent* event) override;

    // QWizard interface
public:
    int nextId() const override;

    // QWidget interface
protected:
    void dropEvent(QDropEvent* event) override;

private:
    class Private;
    Private* d;

};
