#ifndef MODEL_OBJECT_ITEM_H
#define MODEL_OBJECT_ITEM_H

#include "model/objects/category.h"
#include "model/objects/component.h"
#include "model/objects/section.h"
#include "model/objects/tag.h"

#include <QStandardItem>

namespace alt
{

enum CustomRoles
{
    ObjectRole = Qt::UserRole + 1,
    TypeRole = Qt::UserRole + 2
};

class ModelItem : public QStandardItem
{
public:
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

    void translateItem(const QString &locale);
    int countComponentsRecursive() const;

    static QString typeToString(Type type);

public:
    ModelItem(ModelItem &) = delete;
    ModelItem(ModelItem &&) = delete;
    ModelItem &operator=(const ModelItem &) = delete;
    ModelItem &operator=(ModelItem &&) = delete;
};
} // namespace alt

#endif // MODEL_OBJECT_ITEM_H
