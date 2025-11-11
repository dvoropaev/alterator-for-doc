#ifndef MODEL_MODEL_H
#define MODEL_MODEL_H

#include "item.h"
#include "model/objects/edition.h"

#include <QStandardItemModel>
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
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void setTextMode(TextMode mode);

    struct ComponentsCount
    {
        int total = 0;
        int installed = 0;
    };

    ComponentsCount countComponents() const;
    ComponentsCount countComponents(QStandardItem *parent) const;
    int countEditionComponents() const;

public:
    static std::unique_ptr<Edition> current_edition;
    static TextMode textMode;
};
} // namespace alt

#endif // MODEL_MODEL_H
