#pragma once

#include "DetailedEditor.h"
#include "wrappers/CompositeArrayItem.h"

namespace Ui {
class CompositeArrayEditor;
}

class CompositeArrayEditor : public DetailedEditor
{
    Q_OBJECT
public:
    explicit CompositeArrayEditor(Property::Value* value, QWidget *parent = nullptr, int context = 0);
    ~CompositeArrayEditor();

    void fill() override;

    QWidget* makeVisible(const Property::Value::ValidationInfo* info, int level) override;

private slots:
    void addNew(Property::Value* value = nullptr);
    void checkSize();
private:
    Ui::CompositeArrayEditor *ui;
    std::vector<std::unique_ptr<CompositeArrayItem>> m_children;
};

