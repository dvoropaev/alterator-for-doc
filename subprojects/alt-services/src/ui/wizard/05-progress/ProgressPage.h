#pragma once

#include "../Page.h"

class ProgressPage : public Page
{
    Q_OBJECT

public:
    explicit ProgressPage(QWidget *parent = nullptr);
    ~ProgressPage();

    bool usePage() override;

    void initializePage() override;
    bool isComplete() const override;

    QAction* exportAction() const;

private:
    class Private;
    Private* d{};
};
