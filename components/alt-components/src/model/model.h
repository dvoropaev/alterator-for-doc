#ifndef MODEL_MODEL_H
#define MODEL_MODEL_H

#include "model/item.h"
#include "model/objects/component.h"
#include "model/objects/edition.h"

#include <QTreeWidget>

namespace alt
{
class Model : public QStandardItemModel
{
public:
    enum class TextMode
    {
        NamesAndIDs,
        NamesOnly,
        IDsOnly,
    };

public:
    explicit Model(QObject *parent = nullptr);

public:
    void translate(const QString &locale);
    void setTextMode(TextMode mode);
    void resetCurrentState();
    QMap<QString, ComponentState> getCurrentState();
    std::optional<Qt::CheckState> getItemCheckStateBasedOnChildren(const QStandardItem *parent);
    bool getItemEnableStateBasedOnChildren(const QStandardItem *parent);
    static QColor getItemColorBasedOnChildren(const QStandardItem *parent);
    void itemSetEnable(QStandardItem *item, bool enable);
    void correctCheckItemStates();
    QMap<QString, Component> getComponentsBySection(const QString &section);

    struct ComponentsCount
    {
        int total = 0;
        int installed = 0;
    };

    ComponentsCount countComponents();
    ComponentsCount countComponents(QStandardItem *parent);
    int countEditionComponents() const;

private:
    Qt::CheckState resetCurrentStateInner(ModelItem *parent);
    void getCurrentStateInner(ModelItem *parent, QMap<QString, ComponentState> &state);
    void getComponentsInner(ModelItem *parent, QMap<QString, Component> &components);
    void syncComponentInstances(Component *component, Qt::CheckState state, const QStandardItem *triggerItem);
    void updateChildrenAndParents(const QStandardItem *item, const QVariant &value, int role);
    void updateEnableChildrenAndParents(const QStandardItem *item, bool enable);
    void setCheckableAndForeground(QStandardItem *item, bool enable);
    QBrush getMixedStateBackgroundBrush() const;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

public:
    static std::unique_ptr<Edition> current_edition;
    static TextMode textMode;

    static QBrush CHECKED_BACKGROUND_BRUSH;
    static QBrush UNCHECKED_BACKGROUND_BRUSH;
    static QBrush MIXED_STATE_BACKGROUND_BRUSH;
};
} // namespace alt

#endif // MODEL_MODEL_H
