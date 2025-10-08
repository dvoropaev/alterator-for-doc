#pragma once

#include "EditorWrapper.h"

namespace Ui {
class CompositeArrayItem;
}

class CompositeArrayItem: public EditorWrapper
{
    Q_OBJECT
public:
    explicit CompositeArrayItem(Property::Value* value, QWidget *parent = nullptr);
    ~CompositeArrayItem();
    QWidget* makeVisible(const Property::Value::ValidationInfo* info, int level) override;

signals:
    void remove();

private:
    Ui::CompositeArrayItem *ui;
};

