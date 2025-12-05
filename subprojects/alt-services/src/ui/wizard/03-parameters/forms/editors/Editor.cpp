#include "Editor.h"

#include "detailed/wrappers/CollapsibleEditor.h"
#include "detailed/wrappers/InlineEditor.h"
#include "detailed/CompositeEditor.h"
#include "detailed/GroupEditor.h"
#include "detailed/CompositeArrayEditor.h"
#include "detailed/ArrayEditor.h"

#include "compact/CompactArrayFace.h"

#include "compact/IntEditor.h"
#include "compact/StringEditor.h"
#include "compact/PasswordEditor.h"
#include "compact/EnumEditor.h"
#include "compact/BoolEditor.h"


#define Args const BaseForm&, Property::Value*, QWidget*

#define CREATOR(EditorClass) &std::make_unique<EditorClass, Args>

EditorPtr stub(Args) { return {}; }

EditorPtr createDetailedArrayEditor(const BaseForm& form, Property::Value* value, QWidget* parent)
{
    if ( value->property()->prototype()->valueType() == Property::Type::Composite )
        return std::make_unique<CompositeArrayEditor>(form, value, parent);
    else
        return std::make_unique<ArrayEditor>(form, value, parent);
}

EditorPtr createStringEditor(const BaseForm& form, Property::Value* value, QWidget* parent)
{
    if ( value->property()->isPassword() )
        return std::make_unique<PasswordEditor>(form, value, parent);
    else
        return std::make_unique<StringEditor>(form, value, parent);
}

using EditorMap = std::map<Property::Type, std::function<EditorPtr(Args)>>;
#define MAPPING( type, func ) { Property::Type:: type, func }

static const EditorMap compact_editors
{
    MAPPING( Int,       CREATOR(IntEditor)        ),
    MAPPING( Bool,      CREATOR(BoolEditor)       ),
    MAPPING( String,    &createStringEditor       ),
    MAPPING( Enum,      CREATOR(EnumEditor)       ),
    MAPPING( Array,     CREATOR(CompactArrayFace) ),
    MAPPING( Composite, &stub                     )
};

static const EditorMap detailed_editors
{
    MAPPING( Int,       CREATOR(IntEditor)         ),
    MAPPING( Bool,      CREATOR(BoolEditor)        ),
    MAPPING( String,    &createStringEditor        ),
    MAPPING( Enum,      CREATOR(GroupEditor)       ),
    MAPPING( Array,     &createDetailedArrayEditor ),
    MAPPING( Composite, CREATOR(CompositeEditor)   )
};

EditorPtr createEditor(const BaseForm& form, Property::Value* value, QWidget* parent, bool compact, bool array)
{
    EditorPtr editor;
    try {
        editor = std::invoke(
            ( compact ? compact_editors : detailed_editors )
                .at(value->property()->valueType()),
            form, value, parent
        );
    }
    catch ( std::out_of_range& )
    {
        qWarning() << "failed to create editor";
    }

    if ( !compact && !array ) switch ( value->property()->valueType() )
    {
        case Property::Type::Bool   :
        case Property::Type::String :
        case Property::Type::Int    :
            return std::make_unique<InlineEditor>(std::move(editor),parent);

        case Property::Type::Composite:
        case Property::Type::Array:
        case Property::Type::Enum:
            return std::make_unique<CollapsibleEditor>(std::move(editor),parent);

        default: break;
    } else
        return editor;

    qWarning() << "failed to create editor wrapper";
    return {};
}
