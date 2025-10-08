#pragma once

#include "BaseForm.h"

class DetailedForm : public BaseForm
{
public:
    DetailedForm(QWidget* parent = nullptr);
    ~DetailedForm();

    void ensureVisible(const Parameter::Value::ValidationInfo* invalid, int level) override;

protected:
    void setParametersImpl(Parameter::Contexts conexts) override;

private:
    class Private;
    Private* d;
};

