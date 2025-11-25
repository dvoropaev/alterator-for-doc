#include "item.h"

#include "entity/category.h"
#include "entity/component.h"
#include "entity/section.h"

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

    setData(QVariant::fromValue(component), alt::ObjectRole);
    setData(QVariant::fromValue(Type::Component), alt::TypeRole);

    if (!component->icon.empty() && QIcon::hasThemeIcon(component->icon.data()))
    {
        setIcon(QIcon::fromTheme(component->icon.data()));
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

    setData(QVariant::fromValue(category), alt::ObjectRole);
    setData(QVariant::fromValue(Type::Category), alt::TypeRole);

    if (!category->icon.empty() && QIcon::hasThemeIcon(category->icon.data()))
    {
        setIcon(QIcon::fromTheme(category->icon.data()));
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

    auto font = QFont();
    font.setBold(true);
    setFont(font);

    setData(QVariant::fromValue(tag), alt::ObjectRole);
    setData(QVariant::fromValue(Type::Tag), alt::TypeRole);
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
} // namespace alt
