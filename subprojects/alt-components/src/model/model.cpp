#include "model.h"

#include "model/item.h"
#include "model/objects/component.h"

#include <QDebug>
#include <QPainter>
#include <QPalette>

namespace alt
{
std::unique_ptr<Edition> Model::current_edition = nullptr;
Model::TextMode Model::textMode = Model::TextMode::NamesOnly;

Model::Model(QObject *parent)
    : QStandardItemModel(parent)
{}

QVariant Model::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        const auto *object = index.data(CustomRoles::ObjectRole).value<Object *>();
        if (object == nullptr)
        {
            return {};
        }

        if (Model::textMode == Model::TextMode::NamesAndIDs)
        {
            return QString("%1 (%2)").arg(object->displayName()).arg(object->name);
        }
        else if (Model::textMode == Model::TextMode::NamesOnly)
        {
            return object->displayName();
        }
        else if (Model::textMode == Model::TextMode::IDsOnly)
        {
            return object->name;
        }
    }

    return QStandardItemModel::data(index, role);
}

Model::ComponentsCount Model::countComponents(QStandardItem *parent) const
{
    if (parent == nullptr)
    {
        return {0, 0};
    }

    ComponentsCount count = {0, 0};
    for (int i = 0; i < parent->rowCount(); ++i)
    {
        auto *childItem = dynamic_cast<ModelItem *>(parent->child(i));
        const auto childType = childItem->data(alt::TypeRole).value<ModelItem::Type>();
        if (childType == ModelItem::Type::Component)
        {
            count.total++;
            const auto *component = childItem->data(CustomRoles::ObjectRole).value<Component *>();
            count.installed += component != nullptr && component->state == ComponentState::installed;
        }
        else
        {
            const auto childCount = countComponents(childItem);
            count.total += childCount.total;
            count.installed += childCount.installed;
        }
    }
    return count;
};

Model::ComponentsCount Model::countComponents() const
{
    return countComponents(invisibleRootItem());
}

int Model::countEditionComponents() const
{
    return current_edition != nullptr
               ? std::accumulate(current_edition->sections.begin(),
                                 current_edition->sections.end(),
                                 0,
                                 [](int acc, const Section &section) { return acc + section.components.size(); })
               : 0;
}

void Model::setTextMode(TextMode mode)
{
    this->textMode = mode;
}
} // namespace alt
