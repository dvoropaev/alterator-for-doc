#pragma once

#include "PrimitiveEditor.h"
#include <KPasswordLineEdit>

class PasswordEditor : public AbstractPrimitiveEditor
{
    Q_OBJECT
public:
    PasswordEditor(const BaseForm& form, Property::Value* value, QWidget* parent);

    void load() override;

protected:
    QVariant data() override;

private:
    KPasswordLineEdit* m_password{};
    KPasswordLineEdit* m_confirmation{};
};
