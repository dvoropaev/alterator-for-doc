#pragma once

#include <QWizard>
#include "data/Service.h"
#include "app/ServicesApp.h"

class Controller;

class ActionWizard : public QWizard
{
    Q_OBJECT

public:
    explicit ActionWizard(Controller* controller, QWidget *parent = nullptr);
    ~ActionWizard();
    void initMenu();

    void open(Parameter::Context context, Service* service);
    Parameter::Context context() const;
    Service* service() const;
    bool     preDiag() const;
    bool    postDiag() const;
    bool   autoStart() const;
    bool forceDeploy() const;

public slots:
    void readParameters(const ServicesApp::ParsedParameters& parameters);

private slots:
    void validateParameters();
    bool validateCurrentPage() override;
    void fillCurrentParameters();
    void onModeChanged();
    void exportParameters();
    void on_invalidParameterWarningText_linkActivated(const QString &link);
    void on_invalidParameterWarningText_linkHovered(const QString &link);
    void on_forceDeployCheckBox_toggled(bool checked);
    void on_finishPageCheckBox_clicked(bool checked);
    void on_ActionWizard_currentIdChanged(int id);

    // QDialog interface
public slots:
    void done(int) override;

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
