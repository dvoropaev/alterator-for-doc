#ifndef MODEL_OBJECT_ITEM_H
#define MODEL_OBJECT_ITEM_H

#include "entity/category.h"
#include "entity/component.h"
#include "entity/section.h"
#include "entity/tag.h"

#include <QStandardItem>

Q_DECLARE_METATYPE(alt::Object);
Q_DECLARE_METATYPE(alt::Component);
Q_DECLARE_METATYPE(alt::Category);
Q_DECLARE_METATYPE(alt::Section);
Q_DECLARE_METATYPE(alt::Tag);

namespace alt
{

enum CustomRoles
{
    ObjectRole = Qt::UserRole + 1,
    TypeRole = Qt::UserRole + 2,
    CacheRole = Qt::UserRole + 3,
};

class ModelItem : public QStandardItem
{
public:
    struct Cache;
    enum class Type
    {
        Section,
        Category,
        Component,
        Tag
    };

public:
    explicit ModelItem(Component *component);
    explicit ModelItem(Category *category);
    explicit ModelItem(Section *section);
    explicit ModelItem(Tag *tag);
    ~ModelItem() override = default;

    static QString typeToString(Type type);

public:
    ModelItem(ModelItem &) = delete;
    ModelItem(ModelItem &&) = delete;
    ModelItem &operator=(const ModelItem &) = delete;
    ModelItem &operator=(ModelItem &&) = delete;
};

struct ModelItem::Cache
{
    bool hasRelationToEdition = false;
};

} // namespace alt

Q_DECLARE_METATYPE(alt::ModelItem::Cache);

#endif // MODEL_OBJECT_ITEM_H
