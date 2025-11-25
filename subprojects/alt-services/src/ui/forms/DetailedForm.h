#pragma once

#include "BaseForm.h"

class DetailedForm : public BaseForm
{
public:
    DetailedForm(const Action& action, QWidget* parent = nullptr);
    ~DetailedForm();

    void ensureVisible(const Parameter::Value* value) override;
    SearchAdapter* searchAdapter() override;

protected:
    void setParametersImpl(Parameter::Contexts conexts) override;

private:
    class Private;
    Private* d;
};

