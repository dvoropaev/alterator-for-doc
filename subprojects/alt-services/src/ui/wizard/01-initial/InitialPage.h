#pragma once

#include "../Page.h"

class InitialPage : public Page
{
    Q_OBJECT

public:
    explicit InitialPage(QWidget *parent = nullptr);
    ~InitialPage();

public:
    bool usePage() override;

    bool isComplete() const override;
    void initializePage() override;

private:
    class Private;
    Private* d{};
};
