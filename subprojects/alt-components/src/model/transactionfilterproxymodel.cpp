#include "transactionfilterproxymodel.h"

#include "application.h"
#include "model.h"
#include "model/item.h"
#include "model/objects/component.h"
#include "service/transactionservice.h"

#include <QPainter>

namespace alt
{
QBrush TransactionFilterProxyModel::INSTALL_BACKGROUND_BRUSH = QColor(0, 255, 0, 50);
QBrush TransactionFilterProxyModel::REMOVE_BACKGROUND_BRUSH = QColor(255, 0, 0, 50);
QBrush TransactionFilterProxyModel::MIXED_STATE_BACKGROUND_BRUSH{};

TransactionFilterProxyModel::TransactionFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , disableBaseComponents(false)
{
    MIXED_STATE_BACKGROUND_BRUSH = getMixedStateBackgroundBrush();
}

void TransactionFilterProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (sourceModel == this->sourceModel())
    {
        return;
    }

    auto builder = [this] { buildSelf(); };
    if (this->sourceModel() != nullptr)
    {
        disconnect(this->sourceModel(), nullptr, this, nullptr);
    }

    // NOTE(sheriffkorov): for breaking changes in the model
    // we need to clean up transaction from unexisting items
    auto builderAndTransactor = [this] {
        buildSelf();
        syncServiceState();
    };

    QSortFilterProxyModel::setSourceModel(sourceModel);
    if (sourceModel == nullptr)
    {
        return;
    }

    states.clear();
    connect(sourceModel, &QAbstractItemModel::modelReset, this, builderAndTransactor);
    connect(sourceModel, &QAbstractItemModel::rowsInserted, this, builder);
    connect(sourceModel, &QAbstractItemModel::rowsRemoved, this, builderAndTransactor);
    connect(sourceModel, &QAbstractItemModel::dataChanged, this, builder);
    connect(sourceModel, &QAbstractItemModel::layoutChanged, this, builderAndTransactor);
}

void TransactionFilterProxyModel::buildSelf(const QModelIndex &sourceIndex)
{
    const int rowCount = sourceModel()->rowCount(sourceIndex);
    if (rowCount == 0)
    {
        const auto *component = sourceIndex.data(CustomRoles::ObjectRole).value<Component *>();
        const auto componentState = component != nullptr ? component->state : ComponentState::not_installed;
        const auto persistentIndex = QPersistentModelIndex(sourceIndex);
        ModelIndexState newState{
            .checkState = componentState == ComponentState::installed ? Qt::CheckState::Checked
                                                                      : Qt::CheckState::Unchecked,
            .selection = ModelIndexState::SelectionState::None,
            .isEnabled = true,
        };
        if (!states.contains(persistentIndex))
        {
            updateStateHelper(persistentIndex, newState);
        }
    }
    for (int row = 0; row < rowCount; ++row)
    {
        const auto childIndex = sourceModel()->index(row, 0, sourceIndex);
        buildSelf(childIndex);
    }
    if (!sourceIndex.isValid())
    {
        // NOTE(sheriffkorov): clean invalid indexes
        for (auto it = states.begin(); it != states.end();)
        {
            if (!it.key().isValid())
            {
                it = states.erase(it);
            }
            else
            {
                ++it;
            }
        }
        setDisabledBaseComponents(disableBaseComponents);
    }
}

void TransactionFilterProxyModel::syncServiceState()
{
    const auto transactionComponents = TransactionService::current().components();
    std::unordered_set<QString> requestedComponents;
    // NOTE(sheriffkorov): get requested components from this model
    for (const auto &[index, state] : states.asKeyValueRange())
    {
        if (!index.isValid())
        {
            continue;
        }
        const auto *component = index.data(CustomRoles::ObjectRole).value<Component *>();
        if (component == nullptr || state.selection == ModelIndexState::SelectionState::None)
        {
            continue;
        }
        requestedComponents.insert(component->name);
    }
    // NOTE(sheriffkorov): clean service from invalid components
    for (const auto &[name, component] : transactionComponents)
    {
        if (!requestedComponents.contains(name))
        {
            TransactionService::current().discard(component);
        }
    }
}

QVariant TransactionFilterProxyModel::data(const QModelIndex &proxyIndex, int role) const
{
    if (sourceModel() == nullptr || !proxyIndex.isValid())
    {
        return {};
    }
    const auto sourceIndex = mapToSource(proxyIndex);

    switch (role)
    {
    case Qt::CheckStateRole:
        // NOTE(sheriffkorov): idk why does checking flags hide checkbox of disabled items
        if (!matchType(sourceModel(), sourceIndex))
        {
            return {};
        }
        return states[QPersistentModelIndex(sourceIndex)].checkState;
    case Qt::ForegroundRole: {
        const auto &state = states[QPersistentModelIndex(sourceIndex)];
        if (!state.isEnabled)
        {
            return QColor(Qt::gray);
        }
        else if (state.selection != ModelIndexState::SelectionState::None)
        {
            return QColor(Qt::black);
        }
    }
    break;
    case Qt::BackgroundRole:
        return selectionToBrush(states[QPersistentModelIndex(sourceIndex)].selection);
        break;
    default:
        break;
    }
    return sourceIndex.data(role);
}

bool TransactionFilterProxyModel::setData(const QModelIndex &proxyIndex, const QVariant &value, int role)
{
    if (sourceModel() == nullptr || !proxyIndex.isValid())
    {
        return false;
    }
    const auto sourceIndex = mapToSource(proxyIndex);

    if (role == Qt::CheckStateRole)
    {
        updateStatesRecursively(proxyIndex, value.value<Qt::CheckState>());
        return true;
    }

    return sourceModel()->setData(sourceIndex, value, role);
}

Qt::ItemFlags TransactionFilterProxyModel::flags(const QModelIndex &index) const
{
    if (sourceModel() == nullptr)
    {
        return {};
    }

    auto flags = QSortFilterProxyModel::flags(index);
    if (matchType(sourceModel(), mapToSource(index)))
    {
        flags = flags.setFlag(Qt::ItemFlag::ItemIsUserCheckable, true);
    }
    const auto persistentIndex = QPersistentModelIndex(mapToSource(index));
    if (!states[persistentIndex].isEnabled)
    {
        flags = flags.setFlag(Qt::ItemFlag::ItemIsUserCheckable, false);
    }
    return flags;
}

bool TransactionFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    std::ignore = std::tie(sourceRow, sourceParent);
    return true;
}

bool TransactionFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    std::ignore = std::tie(left, right);
    return false;
}

void TransactionFilterProxyModel::setDisabledBaseComponents(bool value)
{
    disableBaseComponents = value;
    if (sourceModel() == nullptr)
    {
        return;
    }

    if (Model::current_edition == nullptr)
    {
        getStateBasedOnChildren();
        return;
    }

    auto it = std::find_if(Model::current_edition->sections.begin(),
                           Model::current_edition->sections.end(),
                           [](const auto &section) { return section.name == "base"; });
    if (it == Model::current_edition->sections.end())
    {
        return;
    }

    const bool enableBaseComponents = !value;
    const auto &sectionBase = it->components;
    for (const auto &[index, state] : states.asKeyValueRange())
    {
        const auto *component = index.data(CustomRoles::ObjectRole).value<Component *>();
        if (component != nullptr && component->state == ComponentState::installed
            && sectionBase.contains(component->name))
        {
            ModelIndexState newState{
                .checkState = Qt::CheckState::Checked,
                .selection = ModelIndexState::SelectionState::None,
                .isEnabled = enableBaseComponents,
            };
            updateStateHelper(index, newState);
        }
    }
    getStateBasedOnChildren();
}

void TransactionFilterProxyModel::reset()
{
    for (const auto &[index, state] : states.asKeyValueRange())
    {
        const auto *component = index.data(CustomRoles::ObjectRole).value<Component *>();
        if (component != nullptr)
        {
            ModelIndexState newState{
                .checkState = component->state == ComponentState::installed ? Qt::CheckState::Checked
                                                                            : Qt::CheckState::Unchecked,
                .selection = ModelIndexState::SelectionState::None,
                .isEnabled = state.isEnabled,
            };
            updateStateHelper(index, newState);
            updateStatesOfParents(mapFromSource(index));
        }
    }
}

void TransactionFilterProxyModel::updateStatesRecursively(const QModelIndex &proxyIndex, Qt::CheckState state)
{
    const int rowCount = this->rowCount(proxyIndex);
    for (int row = 0; row < rowCount; ++row)
    {
        const auto childIndex = index(row, 0, proxyIndex);
        if (flags(childIndex).testFlag(Qt::ItemIsUserCheckable))
        {
            updateStatesRecursively(childIndex, state);
        }
    }
    if (rowCount == 0)
    {
        updateState(proxyIndex, state);
        updateStatesOfParents(proxyIndex);
    }
}

void TransactionFilterProxyModel::updateStatesOfParents(const QModelIndex &proxyIndex)
{
    for (QModelIndex parentIndex = proxyIndex.parent();
         parentIndex.isValid() && flags(parentIndex).testFlag(Qt::ItemIsUserCheckable);
         parentIndex = parentIndex.parent())
    {
        if (const auto state = getStateBasedOnChildren(parentIndex); state.has_value())
        {
            const auto persistentIndex = QPersistentModelIndex(mapToSource(parentIndex));
            updateStateHelper(persistentIndex, state.value());
        }
    }
}

void TransactionFilterProxyModel::updateState(const QModelIndex &proxyIndex, Qt::CheckState state)
{
    if (!flags(proxyIndex).testFlag(Qt::ItemIsUserCheckable))
    {
        return;
    }

    using SelectionState = ModelIndexState::SelectionState;
    const auto persistentIndex = QPersistentModelIndex(mapToSource(proxyIndex));
    ModelIndexState currentState = states[persistentIndex];
    if (state == currentState.checkState)
    {
        return;
    }
    if (currentState.selection & SelectionState::Both)
    {
        currentState.selection = SelectionState::None;
    }
    else
    {
        currentState.selection = state == Qt::CheckState::Checked ? SelectionState::OnInstallation
                                                                  : SelectionState::OnRemoval;
    }
    currentState.checkState = state;
    updateStateHelper(persistentIndex, currentState);
}

void TransactionFilterProxyModel::updateStateHelper(const QPersistentModelIndex &index, const ModelIndexState &state)
{
    const auto proxyIndex = mapFromSource(index);
    if (!states.contains(index))
    {
        states[index] = state;
        emit dataChanged(proxyIndex, proxyIndex, {Qt::CheckStateRole, Qt::BackgroundRole, Qt::ForegroundRole});
    }

    auto &currentState = states[index];
    QList<int> roles;

    if (state.checkState != currentState.checkState)
    {
        currentState.checkState = state.checkState;
        roles << Qt::CheckStateRole;
    }
    if (state.selection != currentState.selection)
    {
        currentState.selection = state.selection;
        roles << Qt::BackgroundRole;
    }
    if (state.isEnabled != currentState.isEnabled)
    {
        currentState.isEnabled = state.isEnabled;
        roles << Qt::ForegroundRole;
    }
    if (!roles.empty())
    {
        emit dataChanged(proxyIndex, proxyIndex, roles);
    }
}

std::optional<TransactionFilterProxyModel::ModelIndexState> TransactionFilterProxyModel::getStateBasedOnChildren(
    const QModelIndex &proxyIndex)
{
    using SelectionState = ModelIndexState::SelectionState;

    bool hasCheckedChildren = false;
    bool hasUncheckedChildren = false;
    uint8_t selectionStateBasedOnChildren = SelectionState::None;
    bool isEnabledBasedOnChildren = false;

    const auto sourceIndex = mapToSource(proxyIndex);
    const auto persistentIndex = QPersistentModelIndex(sourceIndex);

    const int numberOfChildren = this->rowCount(proxyIndex);
    for (int row = 0; row < numberOfChildren; ++row)
    {
        const auto childIndex = index(row, 0, proxyIndex);
        if (const auto state = getStateBasedOnChildren(childIndex); state.has_value())
        {
            hasCheckedChildren |= state->checkState != Qt::CheckState::Unchecked;
            hasUncheckedChildren |= state->checkState != Qt::CheckState::Checked;
            selectionStateBasedOnChildren |= state->selection;
            isEnabledBasedOnChildren |= state->isEnabled;
        }
    }

    if (!matchType(sourceModel(), sourceIndex))
    {
        return std::nullopt;
    }
    if (numberOfChildren == 0)
    {
        return states[persistentIndex];
    }
    auto generalCheckState = hasCheckedChildren
                                 ? (hasUncheckedChildren ? Qt::CheckState::PartiallyChecked : Qt::CheckState::Checked)
                                 : Qt::CheckState::Unchecked;
    ModelIndexState newState{.checkState = generalCheckState,
                             .selection = static_cast<SelectionState>(selectionStateBasedOnChildren),
                             .isEnabled = isEnabledBasedOnChildren};
    updateStateHelper(persistentIndex, newState);
    return newState;
}

void TransactionFilterProxyModel::syncInstances(const QModelIndex &triggerProxyIndex,
                                                const ModelIndexState &state,
                                                const QModelIndex &parentProxyIndex)
{
    if (triggerProxyIndex == parentProxyIndex)
    {
        return;
    }

    const int rowCount = this->rowCount(parentProxyIndex);
    for (int row = 0; row < rowCount; ++row)
    {
        const auto childIndex = index(row, 0, parentProxyIndex);
        syncInstances(triggerProxyIndex, state, childIndex);
    }

    const auto *triggerObject = triggerProxyIndex.data(CustomRoles::ObjectRole).value<Object *>();
    const auto *currentObject = parentProxyIndex.data(CustomRoles::ObjectRole).value<Object *>();
    if (currentObject != nullptr && currentObject == triggerObject)
    {
        updateState(parentProxyIndex, state.checkState);
        updateStatesOfParents(parentProxyIndex);
    }
}

bool TransactionFilterProxyModel::matchType(const QAbstractItemModel *model, const QModelIndex &index)
{
    using Type = ModelItem::Type;
    const auto type = model->data(index, CustomRoles::TypeRole).value<Type>();
    return type == Type::Category || type == Type::Component;
}

QBrush TransactionFilterProxyModel::selectionToBrush(ModelIndexState::SelectionState selection)
{
    switch (selection)
    {
    case ModelIndexState::SelectionState::None:
        return Application::palette().base();
    case ModelIndexState::SelectionState::OnInstallation:
        return INSTALL_BACKGROUND_BRUSH;
    case ModelIndexState::SelectionState::OnRemoval:
        return REMOVE_BACKGROUND_BRUSH;
    case ModelIndexState::SelectionState::Both:
        return MIXED_STATE_BACKGROUND_BRUSH;
    default:
        return {};
    }
}

// NOTE(sheriffkorov): requires initialized QGuiApplication
QBrush TransactionFilterProxyModel::getMixedStateBackgroundBrush() const
{
    QPixmap pixmap(64, 32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    auto pen = painter.pen();
    pen.setColor(REMOVE_BACKGROUND_BRUSH.color());
    pen.setWidth(12);
    painter.setPen(pen);
    painter.drawLine(QLine(-32, 32, 0, 0));
    painter.drawLine(QLine(0, 32, 32, 0));
    painter.drawLine(QLine(32, 32, 64, 0));
    painter.drawLine(QLine(64, 32, 96, 0));
    pen.setColor(INSTALL_BACKGROUND_BRUSH.color());
    painter.setPen(pen);
    painter.drawLine(QLine(-16, 32, 16, 0));
    painter.drawLine(QLine(16, 32, 48, 0));
    painter.drawLine(QLine(48, 32, 80, 0));
    painter.end();
    return pixmap;
}
} // namespace alt
