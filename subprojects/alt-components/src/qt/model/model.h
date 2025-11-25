#ifndef MODEL_MODEL_H
#define MODEL_MODEL_H

#include "item.h"
#include "repository/componentrepository.h"
#include "repository/editionrepository.h"

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
    Model(const std::shared_ptr<ComponentRepository> &components,
          const std::shared_ptr<EditionRepository> &editions,
          QObject *parent = nullptr);

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
    void buildBySections(bool hard = true);
    void buildByTags(bool hard = true);
    void buildPlain(bool hard = true);

private:
    void buildItemTree(QStandardItem *insertable,
                       const std::unordered_set<std::string> &components,
                       const std::unordered_set<std::string> &editionComponents);
    ModelItem *buildDefaultSectionItem(const std::vector<Section> &sections);
    ModelItem *buildTagItem(Tag &tag);
    ModelItem *buildSectionItem(Section &section);
    ModelItem *buildCategory(const std::string &categoryName,
                             std::unordered_map<std::string, std::set<std::string>> &sectionCategories,
                             std::unordered_map<std::string, std::set<std::string>> &sectionComponents,
                             const std::unordered_set<std::string> &editionComponents);
    std::unordered_set<std::string> getEditionComponents();

public:
    static TextMode textMode;

private:
    std::shared_ptr<ComponentRepository> componentRepository;
    std::shared_ptr<EditionRepository> editionRepository;
};
} // namespace alt

#endif // MODEL_MODEL_H
