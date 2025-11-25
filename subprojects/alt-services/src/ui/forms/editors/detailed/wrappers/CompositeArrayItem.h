#pragma once

#include "EditorWrapper.h"

namespace Ui {
class CompositeArrayItem;
}

class CompositeArrayItem: public EditorWrapper
{
    Q_OBJECT
public:
    explicit CompositeArrayItem(const BaseForm& form, Property::Value* value, QWidget *parent = nullptr);
    ~CompositeArrayItem();
    QWidget* makeVisible(const Property::Value* value) override;

    /*
     * Re-read item name and number
     * after array modified
     */
    void refresh();

signals:
    void remove();

private:
    Ui::CompositeArrayItem *ui;
};

