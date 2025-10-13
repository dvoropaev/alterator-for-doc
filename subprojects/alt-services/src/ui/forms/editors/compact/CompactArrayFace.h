#pragma once

#include "../Editor.h"

#include <QPushButton>
#include <QLabel>

class CompactArrayFace : public Editor {
    Q_OBJECT
public:
    CompactArrayFace(Property::Value* value, QWidget* parent);
    
    void fill() override;
    
private:
    QLabel* label;
    QPushButton* addBtn;
    int min, max;
};
