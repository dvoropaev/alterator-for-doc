#pragma once

#include "../Page.h"

class DiagSelectionPage : public Page
{
    Q_OBJECT

public:
    explicit DiagSelectionPage(DiagTool::Test::Mode mode, QWidget *parent = nullptr);
    ~DiagSelectionPage();

    bool usePage() override;

    void initializePage() override;
    bool isComplete() const override;

    QAbstractItemModel& model();

private:
    class Private;
    Private* d{};
};
