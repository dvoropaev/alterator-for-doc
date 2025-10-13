#include "adttoolvarsmodelrowbuilders.h"

void ADTToolVarModelRowBuilderInt::build(int row,
                                         std::unique_ptr<ADTVarInterface> &var,
                                         const QString &toolId,
                                         QStandardItemModel *model)
{
    int val = 0;

    if (var->getType() != ADTVarInterface::ADTVarType::INT)
        return;

    QStandardItem *name    = new QStandardItem(var->getDisplayName());
    QStandardItem *comment = new QStandardItem(var->getComment());
    QStandardItem *tool    = new QStandardItem(toolId);
    QStandardItem *varId   = new QStandardItem(var->id());
    QStandardItem *value   = new QStandardItem(val);
    QStandardItem *type    = new QStandardItem("int");
    type->setData(static_cast<unsigned int>(var->getType()), Qt::UserRole + 1);

    value->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
    type->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
    name->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

    model->setItem(row, 0, name);
    model->setItem(row, 1, type);
    model->setItem(row, 2, value);
    model->setItem(row, 3, comment);
    model->setItem(row, 4, tool);
    model->setItem(row, 5, varId);

    name->setFlags(name->flags() & ~Qt::ItemIsEditable);
    comment->setFlags(comment->flags() & ~Qt::ItemIsEditable);
    value->setFlags(value->flags() & ~Qt::ItemIsEditable);
}

void ADTToolVarModelRowBuilderString::build(int row,
                                            std::unique_ptr<ADTVarInterface> &var,
                                            const QString &toolId,
                                            QStandardItemModel *model)
{
    QStandardItem *name    = new QStandardItem(var->getDisplayName());
    QStandardItem *value   = new QStandardItem("value");
    QStandardItem *type    = new QStandardItem("string");
    QStandardItem *comment = new QStandardItem(var->getComment());
    QStandardItem *tool    = new QStandardItem(toolId);
    QStandardItem *varId   = new QStandardItem(var->id());
    type->setData(static_cast<unsigned int>(var->getType()), Qt::UserRole + 1);

    value->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
    type->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
    name->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

    model->setItem(row, 0, name);
    model->setItem(row, 1, type);
    model->setItem(row, 2, value);
    model->setItem(row, 3, comment);
    model->setItem(row, 4, tool);
    model->setItem(row, 5, varId);

    name->setFlags(name->flags() & ~Qt::ItemIsEditable);
    comment->setFlags(comment->flags() & ~Qt::ItemIsEditable);
    value->setFlags(value->flags() & ~Qt::ItemIsEditable);
}

void ADTToolVarModelRowBuilderEnumInt::build(int row,
                                             std::unique_ptr<ADTVarInterface> &var,
                                             const QString &toolId,
                                             QStandardItemModel *model)
{
    QStandardItem *name    = new QStandardItem(var->getDisplayName());
    QStandardItem *comment = new QStandardItem(var->getComment());
    QStandardItem *tool    = new QStandardItem(toolId);
    QStandardItem *varId   = new QStandardItem(var->id());
    QStandardItem *value   = new QStandardItem("value");
    QStandardItem *type    = new QStandardItem("enum_string");
    type->setData(static_cast<unsigned int>(var->getType()), Qt::UserRole + 1);

    value->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
    type->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
    name->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

    model->setItem(row, 0, name);
    model->setItem(row, 1, type);
    model->setItem(row, 2, value);
    model->setItem(row, 3, comment);
    model->setItem(row, 4, tool);
    model->setItem(row, 5, varId);

    name->setFlags(name->flags() & ~Qt::ItemIsEditable);
    comment->setFlags(comment->flags() & ~Qt::ItemIsEditable);
    value->setFlags(value->flags() & ~Qt::ItemIsEditable);
}

void ADTToolVarModelRowBuilderEnumString::build(int row,
                                                std::unique_ptr<ADTVarInterface> &var,
                                                const QString &toolId,
                                                QStandardItemModel *model)
{
    int val = 0;

    QStandardItem *name    = new QStandardItem(var->getDisplayName());
    QStandardItem *comment = new QStandardItem(var->getComment());
    QStandardItem *tool    = new QStandardItem(toolId);
    QStandardItem *varId   = new QStandardItem(var->id());
    QStandardItem *value   = new QStandardItem(val);
    QStandardItem *type    = new QStandardItem("enum_string");
    type->setData(static_cast<unsigned int>(var->getType()), Qt::UserRole + 1);

    value->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
    type->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
    name->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

    model->setItem(row, 0, name);
    model->setItem(row, 1, type);
    model->setItem(row, 2, value);
    model->setItem(row, 3, comment);
    model->setItem(row, 4, tool);
    model->setItem(row, 5, varId);

    name->setFlags(name->flags() & ~Qt::ItemIsEditable);
    comment->setFlags(comment->flags() & ~Qt::ItemIsEditable);
    value->setFlags(value->flags() & ~Qt::ItemIsEditable);
}
