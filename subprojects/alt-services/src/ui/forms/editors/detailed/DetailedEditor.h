#pragma once

#include "../Editor.h"

class DetailedEditor : public Editor
{
    Q_OBJECT
protected:
    using Editor::Editor;

public:
    virtual QWidget* makeVisible(const Property::Value* value) = 0;
};

