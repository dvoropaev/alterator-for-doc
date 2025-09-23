#include "model.h"
#include "application.h"
#include "constants.h"
#include "model/item.h"
#include "model/objects/component.h"

#include <functional>
#include <map>
#include <QDebug>
#include <QPainter>
#include <QPalette>

namespace alt
{
std::unique_ptr<Edition> Model::current_edition = nullptr;
Model::TextMode Model::textMode = Model::TextMode::NamesOnly;
QBrush Model::CHECKED_BACKGROUND_BRUSH = QColor(0, 255, 0, 50);
QBrush Model::UNCHECKED_BACKGROUND_BRUSH = QColor(255, 0, 0, 50);
QBrush Model::MIXED_STATE_BACKGROUND_BRUSH{};

Model::Model(QObject *parent)
    : QStandardItemModel(parent)
{
    MIXED_STATE_BACKGROUND_BRUSH = getMixedStateBackgroundBrush();
}

void Model::translate(const QString &locale)
{
    for (int i = 0; i < rowCount(); ++i)
    {
        auto *currentItem = dynamic_cast<ModelItem *>(item(i));
        currentItem->translateItem(locale);
    }
}

void Model::resetCurrentState()
{
    for (int i = 0; i < rowCount(); ++i)
    {
        auto *item = dynamic_cast<ModelItem *>(invisibleRootItem()->child(i));
        auto checkStateOfCategory = resetCurrentStateInner(item);

        if (item->isCheckable())
        {
            item->setCheckState(checkStateOfCategory);
        }
    }
}

Qt::CheckState Model::resetCurrentStateInner(ModelItem *parent)
{
    std::map<Qt::CheckState, uint32_t> hasVariant = {
        {Qt::Unchecked, 0},
        {Qt::PartiallyChecked, 0},
        {Qt::Checked, 0},
    };
    parent->setBackground(Application::palette().color(QPalette::Base));
    const size_t numberOfChildren = parent->rowCount();
    for (uint32_t i = 0; i < numberOfChildren; ++i)
    {
        auto *childItem = dynamic_cast<ModelItem *>(parent->child(i));
        const auto childType = childItem->data(alt::TypeRole).value<ModelItem::Type>();
        if (childType == ModelItem::Type::Category)
        {
            auto checkStateOfCategory = resetCurrentStateInner(childItem);
            if (childItem->isCheckable())
            {
                childItem->setCheckState(checkStateOfCategory);
            }
            ++hasVariant[checkStateOfCategory];
        }
        else
        {
            auto *component = childItem->data(alt::ObjectRole).value<Component *>();
            if (component != nullptr && childItem->isCheckable())
            {
                auto checkStateOfComponent = static_cast<Qt::CheckState>(component->state);
                if (childItem->isCheckable())
                {
                    childItem->setCheckState(checkStateOfComponent);
                }
                ++hasVariant[checkStateOfComponent];
            }
        }
    }

    if (hasVariant[Qt::Unchecked] == numberOfChildren
        || (hasVariant[Qt::Unchecked] == 0 && hasVariant[Qt::Checked] == 0))
    {
        return Qt::Unchecked;
    }
    if (hasVariant[Qt::Checked] == numberOfChildren)
    {
        return Qt::Checked;
    }
    return Qt::PartiallyChecked;
}

QMap<QString, ComponentState> Model::getCurrentState()
{
    QMap<QString, ComponentState> state{};
    for (int i = 0; i < rowCount(); ++i)
    {
        auto *categoryItem = dynamic_cast<ModelItem *>(invisibleRootItem()->child(i));
        getCurrentStateInner(categoryItem, state);
    }

    return state;
}

void Model::getCurrentStateInner(ModelItem *parent, QMap<QString, ComponentState> &state)
{
    const size_t numberOfChildren = parent->rowCount();
    for (uint32_t i = 0; i < numberOfChildren; ++i)
    {
        auto *childItem = dynamic_cast<ModelItem *>(parent->child(i));
        const auto childType = childItem->data(alt::TypeRole).value<ModelItem::Type>();
        switch (childType)
        {
            using Type = ModelItem::Type;
        case Type::Component: {
            auto *component = childItem->data(alt::ObjectRole).value<Component *>();
            if (component != nullptr)
            {
                state.insert(component->name, static_cast<ComponentState>(childItem->checkState()));
            }
            break;
        }
        case Type::Category:
            getCurrentStateInner(childItem, state);
            break;
        default:
            break;
        }
    }
}

std::optional<Qt::CheckState> Model::getItemCheckStateBasedOnChildren(const QStandardItem *parent)
{
    std::map<Qt::CheckState, uint32_t> hasVariant = {
        {Qt::Unchecked, 0},
        {Qt::PartiallyChecked, 0},
        {Qt::Checked, 0},
    };

    size_t numberOfChildren = parent->rowCount();
    for (int row = 0; row < parent->rowCount(); ++row)
    {
        auto *child = dynamic_cast<ModelItem *>(parent->child(row));
        const auto childType = child->data(alt::TypeRole).value<ModelItem::Type>();

        switch (childType)
        {
            using Type = ModelItem::Type;
        case Type::Component: {
            const auto state = child->checkState();
            ++hasVariant[state];
            break;
        }
        case Type::Category: {
            if (const auto state = getItemCheckStateBasedOnChildren(child); state.has_value())
            {
                QStandardItemModel::setData(child->index(), state.value(), Qt::CheckStateRole);
                ++hasVariant[state.value()];
            }
            break;
        }
        case Type::Section:
        case Type::Tag: {
            getItemCheckStateBasedOnChildren(child);
            break;
        }
        default:
            break;
        }
    }

    if (hasVariant[Qt::Unchecked] == numberOfChildren)
    {
        return Qt::Unchecked;
    }
    if (hasVariant[Qt::Checked] == numberOfChildren)
    {
        return Qt::Checked;
    }
    return Qt::PartiallyChecked;
}

QColor Model::getItemColorBasedOnChildren(const QStandardItem *parent)
{
    for (int row = 0; row < parent->rowCount(); ++row)
    {
        auto *child = parent->child(row);
        if (child->background().color() == Application::palette().color(QPalette::Mid))
        {
            return Application::palette().color(QPalette::Mid);
        }
    }

    return Application::palette().color(QPalette::Base);
}

void Model::correctCheckItemStates()
{
    getItemCheckStateBasedOnChildren(invisibleRootItem());
}

Model::ComponentsCount Model::countComponents(QStandardItem *parent)
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
            if ((childItem->checkState() == Qt::Checked && childItem->background() != CHECKED_BACKGROUND_BRUSH)
                || childItem->background() == UNCHECKED_BACKGROUND_BRUSH)
            {
                count.installed++;
            }
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

Model::ComponentsCount Model::countComponents()
{
    return countComponents(invisibleRootItem());
}

int Model::countEditionComponents() const
{
    if (current_edition == nullptr)
    {
        return 0;
    }

    QList<Section> &sections = current_edition->sections;

    int totalSize = std::accumulate(sections.begin(),
                                    sections.end(),
                                    0,
                                    [](int acc, const Section &section){ return acc + section.components.size(); });


    return totalSize;
}

void Model::setCheckableAndForeground(QStandardItem *item, bool enable)
{
    if (!enable)
    {
        item->setCheckable(enable);
        item->setForeground(Qt::gray);
    }
    else
    {
        if (item->data(Qt::ForegroundRole).value<QBrush>().color() == QColor(Qt::gray))
        {
            item->setCheckable(enable);
            item->setForeground(Qt::black);
        }
    }
}

void Model::itemSetEnable(QStandardItem *item, bool enable)
{
    if (!item || item->isCheckable() == enable)
        return;

    setCheckableAndForeground(item, enable);
    updateEnableChildrenAndParents(item, enable);
}

bool Model::getItemEnableStateBasedOnChildren(const QStandardItem *parent)
{
    if (!parent || parent->rowCount() == 0)
        return false;

    bool hasEnabled = false;
    bool hasDisabled = false;

    for (int row = 0; row < parent->rowCount(); ++row)
    {
        auto *child = parent->child(row);
        bool childEnabled = child->isCheckable();

        if (child->rowCount() > 0)
        {
            childEnabled = getItemEnableStateBasedOnChildren(child);
        }

        if (childEnabled)
            hasEnabled = true;
        else
            hasDisabled = true;

        if (hasEnabled && hasDisabled)
        {
            break;
        }
    }

    return hasEnabled;
}

void Model::updateEnableChildrenAndParents(const QStandardItem *item, bool enable)
{
    for (int i = 0; i < item->rowCount(); ++i)
    {
        QStandardItem *child = item->child(i);
        if (child->isCheckable() != enable)
        {
            setCheckableAndForeground(child, enable);
            updateEnableChildrenAndParents(child, enable);
        }
    }

    QStandardItem *parent = item->parent();
    while (parent)
    {
        bool shouldEnable = getItemEnableStateBasedOnChildren(parent);
        if (parent->isCheckable() != shouldEnable)
        {
            setCheckableAndForeground(parent, shouldEnable);
        }
        parent = parent->parent();
    }
}

void Model::updateChildrenAndParents(const QStandardItem *item, const QVariant &value, int role)
{
    // (un)check all children on item (un)check
    for (int i = 0; i < item->rowCount(); ++i)
    {
        QStandardItem *child = item->child(i);
        if (child->isCheckable())
        {
            setData(child->index(), value, role);
        }
    }

    // update all parents state
    for (QStandardItem *parent = item->parent(); parent != nullptr && parent->isCheckable(); parent = parent->parent())
    {
        if (const auto state = getItemCheckStateBasedOnChildren(parent); state.has_value())
        {
            QStandardItemModel::setData(parent->index(), state.value(), role);
            parent->setBackground(getItemColorBasedOnChildren(parent));
        }
    }
}

bool Model::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
    {
        return false;
    }

    const QStandardItem *item = itemFromIndex(index);

    if (!item->isEnabled())
        return false;

    const bool rv = QStandardItemModel::setData(index, value, role);

    if (role == Qt::CheckStateRole)
    {
        // synchronize all other instances of the same component
        if (auto *modelItem = dynamic_cast<const ModelItem *>(item); modelItem != nullptr)
        {
            const auto itemType = modelItem->data(alt::TypeRole).value<ModelItem::Type>();
            if (itemType == ModelItem::Type::Component)
            {
                if (auto *component = modelItem->data(alt::ObjectRole).value<Component *>(); component != nullptr)
                {
                    syncComponentInstances(component, static_cast<Qt::CheckState>(value.toInt()), item);
                }
            }
        }

        updateChildrenAndParents(item, value, role);
    }

    return rv;
}

void Model::syncComponentInstances(Component *component, Qt::CheckState state, const QStandardItem *triggerItem)
{
    std::function<void(QStandardItem *)> syncRecursive = [&](QStandardItem *parent) {
        if (parent == triggerItem)
        {
            return;
        }

        for (int i = 0; i < parent->rowCount(); ++i)
        {
            QStandardItem *child = parent->child(i);
            if (auto *modelItem = dynamic_cast<ModelItem *>(child); modelItem != nullptr)
            {
                const auto itemType = modelItem->data(alt::TypeRole).value<ModelItem::Type>();
                if (itemType == ModelItem::Type::Component)
                {
                    if (auto *childComponent = modelItem->data(alt::ObjectRole).value<Component *>();
                        childComponent == component)
                    {
                        QStandardItemModel::setData(child->index(), static_cast<int>(state), Qt::CheckStateRole);
                        updateChildrenAndParents(child, state, Qt::CheckStateRole);
                    }
                }
            }

            // recursively check children
            if (child->hasChildren())
            {
                syncRecursive(child);
            }
        }
    };

    syncRecursive(invisibleRootItem());
}

void Model::setTextMode(TextMode mode)
{
    this->textMode = mode;
}

// NOTE(sheriffkorov): requires initialized QGuiApplication
QBrush Model::getMixedStateBackgroundBrush() const
{
    QPixmap pixmap(64, 32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    auto pen = painter.pen();
    pen.setColor(UNCHECKED_BACKGROUND_BRUSH.color());
    pen.setWidth(12);
    painter.setPen(pen);
    painter.drawLine(QLine(-32, 32, 0, 0));
    painter.drawLine(QLine(0, 32, 32, 0));
    painter.drawLine(QLine(32, 32, 64, 0));
    painter.drawLine(QLine(64, 32, 96, 0));
    pen.setColor(CHECKED_BACKGROUND_BRUSH.color());
    painter.setPen(pen);
    painter.drawLine(QLine(-16, 32, 16, 0));
    painter.drawLine(QLine(16, 32, 48, 0));
    painter.drawLine(QLine(48, 32, 80, 0));
    painter.end();
    return pixmap;
}
} // namespace alt
