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

EditorPtr createEditorInternal(const BaseForm& form, Property::Value* value, QWidget* parent, Parameter::Contexts contexts, bool compact = true) {
    switch ( value->property()->valueType() )
    {
#define EDITOR( ValueType, EditorClass) \
    case Property::Type:: ValueType : return std::make_unique<EditorClass>(form,value,parent);

    EDITOR( Bool,   PrimitiveEditor<QCheckBox> )
    EDITOR( String, PrimitiveEditor<KPasswordLineEdit> )
    EDITOR( Int,    PrimitiveEditor<QSpinBox>  )
#undef EDITOR

        case Property::Type::Enum:
            if ( compact )
                return std::make_unique<PrimitiveEditor<QComboBox>>(form, value, parent);
            else
                return std::make_unique<GroupEditor>(form, value, parent);

        case Property::Type::Composite:
            if ( compact )
                return {};
            else
                return std::make_unique<CompositeEditor>(form, value, parent);

        case Property::Type::Array:
            if ( compact )
                return std::make_unique<CompactArrayFace>(form, value, parent);
            else {
                if ( value->property()->prototype()->valueType() == Property::Type::Composite )
                    return std::make_unique<CompositeArrayEditor>(form, value, parent, contexts);
                else
                    return std::make_unique<ArrayEditor>(form, value, parent, contexts);
            }

        default: break;
    }
    qWarning() << "failed to create editor";
    return {};
}


EditorPtr createEditor(const BaseForm& form, Property::Value* value, QWidget* parent, Parameter::Contexts contexts, bool compact, bool array)
{
    auto editor = createEditorInternal(form, value, parent, contexts, compact);

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
        return editor;

    qWarning() << "failed to create editor wrapper";
    return {};
}
