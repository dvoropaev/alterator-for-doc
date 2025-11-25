#pragma once

#include "BaseForm.h"

class CompactForm : public BaseForm
{
public:
    CompactForm(const Action& action, QWidget* parent = nullptr);
    ~CompactForm();

    void ensureVisible(const Parameter::Value* value) override;
    SearchAdapter* searchAdapter() override;

protected:
    void setParametersImpl(Parameter::Contexts contexts) override;

private:
    class Private;
    Private* d;
};
