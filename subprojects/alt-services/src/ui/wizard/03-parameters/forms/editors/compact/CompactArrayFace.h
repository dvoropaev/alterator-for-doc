#pragma once

#include "../Editor.h"

#include <QPushButton>
#include <QLabel>

class CompactArrayFace : public Editor {
    Q_OBJECT
public:
    CompactArrayFace(const BaseForm& form, Property::Value* value, QWidget* parent);
    
    void fill() override;
    
private:
    QLabel* label;
    QPushButton* addBtn;
    size_t m_min, m_max;
};
