#include "item.h"
#include "model/modelbuilder.h"
#include "model/objects/category.h"
#include "model/objects/component.h"
#include "model/objects/section.h"

#include <QDebug>
#include <QFont>
#include <QStandardItemModel>
#include <QVariant>
#include <QtGlobal>

namespace alt
{
ModelItem::ModelItem(Component *component)
{
    setEditable(false);
    setCheckable(false);

    if (component->packages.size() > 0)
    {
        setCheckable(true);
        switch (component->state)
        {
        case ComponentState::installed:
            setCheckState(Qt::Checked);
            break;
        case ComponentState::partially_installed:
            setCheckState(Qt::PartiallyChecked);
            break;
        case ComponentState::not_installed:
            setCheckState(Qt::Unchecked);
            break;
        default:
            qWarning() << "Undefined item state";
            setCheckState(Qt::Unchecked);
        }
    }

    setText(component->displayName);
    setData(QVariant::fromValue(component), alt::ObjectRole);
    setData(QVariant::fromValue(Type::Component), alt::TypeRole);

    if (!component->icon.isEmpty() && QIcon::hasThemeIcon(component->icon))
    {
        setIcon(QIcon::fromTheme(component->icon));
    }
    else
    {
        setIcon(QIcon::fromTheme("application-x-addon"));
    }
}

ModelItem::ModelItem(Category *category)
{
    setEditable(false);
    setCheckable(false);

    setText(category->displayName);
    setData(QVariant::fromValue(category), alt::ObjectRole);
    setData(QVariant::fromValue(Type::Category), alt::TypeRole);

    if (!category->icon.isEmpty() && QIcon::hasThemeIcon(category->icon))
    {
        setIcon(QIcon::fromTheme(category->icon));
    }
    else
    {
        setIcon(QIcon::fromTheme("folder"));
    }
}

ModelItem::ModelItem(Section *section)
{
    setEditable(false);
    setCheckable(false);

    setText(section->displayName);

    auto font = QFont();
    font.setBold(true);
    setFont(font);

    setData(QVariant::fromValue(section), alt::ObjectRole);
    setData(QVariant::fromValue(Type::Section), alt::TypeRole);
}

ModelItem::ModelItem(Tag *tag)
{
    setEditable(false);
    setCheckable(false);

    setText(tag->displayName);

    auto font = QFont();
    font.setBold(true);
    setFont(font);

    setData(QVariant::fromValue(tag), alt::ObjectRole);
    setData(QVariant::fromValue(Type::Tag), alt::TypeRole);
}

void ModelItem::translateItem(const QString &locale)
{
    const auto itemType = data(alt::TypeRole).value<Type>();
    Object *obj = [&]() -> Object * {
        switch (itemType)
        {
        case Type::Component:
            return data(alt::ObjectRole).value<Component *>();
        case Type::Category:
            return data(alt::ObjectRole).value<Category *>();
        case Type::Section:
            return data(alt::ObjectRole).value<Section *>();
        case Type::Tag:
            return data(alt::ObjectRole).value<Tag *>();
        default:
            qWarning() << "Unknown ModelItem type in translateItem";
            return nullptr;
        }
    }();

    if (obj != nullptr)
    {
        obj->setLocale(locale);

        if (itemType == Type::Section || itemType == Type::Tag)
        {
            int componentCount = this->countComponentsRecursive();
            if (Model::textMode == Model::TextMode::NamesAndIDs)
            {
                setText(QString("%1 (%2) (%3)").arg(obj->displayName).arg(obj->name).arg(componentCount));
            }
            else if (Model::textMode == Model::TextMode::NamesOnly)
            {
                setText(QString("%1 (%2)").arg(obj->displayName).arg(componentCount));
            }
            else if (Model::textMode == Model::TextMode::IDsOnly)
            {
                setText(QString("%1 (%2)").arg(obj->name).arg(componentCount));
            }
        }
        else
        {
            if (Model::textMode == Model::TextMode::NamesAndIDs)
            {
                setText(QString("%1 (%2)").arg(obj->displayName).arg(obj->name));
            }
            else if (Model::textMode == Model::TextMode::NamesOnly)
            {
                setText(obj->displayName);
            }
            else if (Model::textMode == Model::TextMode::IDsOnly)
            {
                setText(obj->name);
            }
        }
    }

    for (int i = 0; i < rowCount(); ++i)
    {
        auto *item = dynamic_cast<ModelItem *>(child(i));
        item->translateItem(locale);
    }
}

QString ModelItem::typeToString(Type type)
{
    switch (type)
    {
    case Type::Section:
        return QObject::tr("Section");
    case Type::Category:
        return QObject::tr("Category");
    case Type::Component:
        return QObject::tr("Component");
    case Type::Tag:
        return QObject::tr("Tag");
    default:
        return QObject::tr("Unknowns type");
    }
}

int ModelItem::countComponentsRecursive() const
{
    int count = 0;
    for (int i = 0; i < rowCount(); ++i)
    {
        ModelItem *childItem = static_cast<ModelItem *>(child(i));
        const auto childType = childItem->data(alt::TypeRole).value<Type>();
        if (childType == Type::Component)
        {
            count++;
        }
        else if (childType == Type::Category || childType == Type::Section)
        {
            count += childItem->countComponentsRecursive();
        }
    }
    return count;
}
} // namespace alt
