#include "PrimitiveEditor.h"

void AbstractPrimitiveEditor::fill() {
    load();
    onChange();
}

void AbstractPrimitiveEditor::onChange() {
    emit aboutToChange();
    m_value->set(data());
    emit changed();
}
