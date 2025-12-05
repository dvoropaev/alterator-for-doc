#pragma once

#include "../Page.h"

class ConfirmationPage : public Page
{
    Q_OBJECT

public:
    explicit ConfirmationPage(const std::vector<Parameter*>& parameters,
                              QAbstractItemModel&  preDiagModel,
                              QAbstractItemModel& postDiagModel,
                              QWidget *parent = nullptr);
    ~ConfirmationPage();

    bool usePage() override;
    void initializePage() override;

private:
    class Private;
    Private* d{};
};
