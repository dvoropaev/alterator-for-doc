#include "Editor.h"

#include "detailed/wrappers/CollapsibleEditor.h"
#include "detailed/wrappers/InlineEditor.h"
#include "detailed/CompositeEditor.h"
#include "detailed/GroupEditor.h"
#include "detailed/CompositeArrayEditor.h"
#include "detailed/ArrayEditor.h"

#include "compact/CompactArrayFace.h"
#include "compact/PrimitiveEditor.h"

#include <KPasswordLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>

EditorPtr createEditorInternal(Property::Value* value, QWidget* parent, Parameter::Contexts contexts, bool compact = true) {
    switch ( value->property()->valueType() )
    {
#define EDITOR( ValueType, EditorClass) \
    case Property::Type:: ValueType : return std::make_unique<EditorClass>(value,parent);

    EDITOR( Bool,   PrimitiveEditor<QCheckBox> )
    EDITOR( String, PrimitiveEditor<KPasswordLineEdit> )
    EDITOR( Int,    PrimitiveEditor<QSpinBox>  )
#undef EDITOR

        case Property::Type::Enum:
            if ( compact )
                return std::make_unique<PrimitiveEditor<QComboBox>>(value, parent);
            else
                return std::make_unique<GroupEditor>(value, parent);

        case Property::Type::Composite:
            if ( compact )
                return {};
            else
                return std::make_unique<CompositeEditor>(value, parent);

        case Property::Type::Array:
            if ( compact )
                return std::make_unique<CompactArrayFace>(value, parent);
            else {
                if ( value->property()->prototype()->valueType() == Property::Type::Composite )
                    return std::make_unique<CompositeArrayEditor>(value, parent, contexts);
                else
                    return std::make_unique<ArrayEditor>(value, parent, contexts);
            }

        default: break;
    }
    qWarning() << "failed to create editor";
    return {};
}


EditorPtr createEditor(Property::Value* value, QWidget* parent, Parameter::Contexts contexts, bool compact, bool array)
{
    auto editor = createEditorInternal(value, parent, contexts, compact);

    if ( !compact && !array ) switch ( value->property()->valueType() )
    {
        case Property::Type::Bool   :
        case Property::Type::String :
        case Property::Type::Int    :
            return std::make_unique<InlineEditor>(std::move(editor),parent,contexts);

        case Property::Type::Composite:
        case Property::Type::Array:
        case Property::Type::Enum:
            return std::make_unique<CollapsibleEditor>(std::move(editor),parent,contexts);

        default: break;
    } else
        return std::move(editor);

    qWarning() << "failed to create editor wrapper";
    return {};
}
