#pragma once

#include "BaseForm.h"

class CompactForm : public BaseForm
{
public:
    CompactForm(QWidget* parent = nullptr);
    ~CompactForm();

    void ensureVisible(const Parameter::Value::ValidationInfo* invalid, int level) override;

protected:
    void setParametersImpl(Parameter::Contexts contexts) override;

private:
    class Private;
    Private* d;
};
