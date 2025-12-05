#pragma once

#include "../Page.h"

class ParametersPage : public Page
{
    Q_OBJECT

public:
    explicit ParametersPage(QWidget* parent = nullptr);
    ~ParametersPage();

    bool usePage() override;

    void initializePage() override;
    bool isComplete() const override;

    const std::vector<Parameter*>& parameters();

private slots:
    void setForm();
    void fillParameters();
    void validateParameters();
    void on_invalidParameterWarning_linkActivated(const QString& link);
    void on_invalidParameterWarning_linkHovered(const QString& link);

private:
    class Private;
    Private* d{};

};
