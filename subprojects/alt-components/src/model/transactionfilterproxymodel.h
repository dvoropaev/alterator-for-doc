#ifndef TRANSACTION_FILTER_PROXY_MODEL_H
#define TRANSACTION_FILTER_PROXY_MODEL_H

#include <cstdint>
#include <sys/types.h>
#include <QBrush>
#include <QSortFilterProxyModel>

namespace alt
{
class TransactionFilterProxyModel : public QSortFilterProxyModel
{
public:
    void setSourceModel(QAbstractItemModel *sourceModel) override;
    QVariant data(const QModelIndex &proxyIndex, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &proxyIndex, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    [[nodiscard]] bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    [[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    void reset();
    void setDisabledBaseComponents(bool value);

public:
    explicit TransactionFilterProxyModel(QObject *parent = nullptr);

private:
    struct ModelIndexState
    {
        enum SelectionState : uint8_t
        {
            None,
            OnInstallation,
            OnRemoval,
            Both,
        };
        Qt::CheckState checkState;
        SelectionState selection;
        bool isEnabled = true;
    };

private:
    void buildSelf(const QModelIndex &sourceIndex = {});
    void syncServiceState();
    void depthFirstTraversal(const QModelIndex &parent, std::function<void(const QModelIndex &)> callback);
    void updateState(const QModelIndex &proxyIndex, Qt::CheckState state);
    void updateStateHelper(const QPersistentModelIndex &index, const ModelIndexState &state);
    void updateStatesRecursively(const QModelIndex &proxyIndex, Qt::CheckState state);
    void updateStatesOfParents(const QModelIndex &proxyIndex);
    std::optional<ModelIndexState> getStateBasedOnChildren(const QModelIndex &proxyIndex = {});
    void syncInstances(const QModelIndex &triggerProxyIndex,
                       const ModelIndexState &state,
                       const QModelIndex &parentProxyIndex = {});
    static bool matchType(const QAbstractItemModel *model, const QModelIndex &index);
    static QBrush selectionToBrush(ModelIndexState::SelectionState selection);
    QBrush getMixedStateBackgroundBrush() const;

private:
    QHash<QPersistentModelIndex, ModelIndexState> states;
    bool disableBaseComponents;
    static QBrush INSTALL_BACKGROUND_BRUSH;
    static QBrush REMOVE_BACKGROUND_BRUSH;
    static QBrush MIXED_STATE_BACKGROUND_BRUSH;
};
} // namespace alt

#endif // TRANSACTION_FILTER_PROXY_MODEL_H
