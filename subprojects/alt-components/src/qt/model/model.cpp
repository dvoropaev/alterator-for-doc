#include "model.h"

#include "../application/application.h"
#include "entity/component.h"
#include "entity/constants.h"
#include "entity/section.h"
#include "item.h"
#include "repository/componentrepository.h"

#define TOML_EXCEPTIONS 0
#include <toml++/impl/formatter.inl>

#include <algorithm>
#include <boost/range/algorithm/transform.hpp>
#include <iterator>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <QDebug>
#include <QPainter>
#include <QPalette>

namespace alt
{
Model::TextMode Model::textMode = Model::TextMode::NamesOnly;

Model::Model(const std::shared_ptr<ComponentRepository> &components,
             const std::shared_ptr<EditionRepository> &editions,
             QObject *parent)
    : QStandardItemModel(parent)
    , componentRepository(components)
    , editionRepository(editions)
{}

QVariant Model::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        const auto *object = index.data(CustomRoles::ObjectRole).value<Object *>();
        auto lang = QLocale::languageToCode(Application::getLocale().language()).toStdString();
        if (object == nullptr)
        {
            return {};
        }

        if (Model::textMode == Model::TextMode::NamesAndIDs)
        {
            return QString("%1 (%2)").arg(object->displayName(lang)).arg(object->name);
        }
        else if (Model::textMode == Model::TextMode::NamesOnly)
        {
            return object->displayName(lang).data();
        }
        else if (Model::textMode == Model::TextMode::IDsOnly)
        {
            return QString::fromStdString(object->name);
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

void Model::setTextMode(TextMode mode)
{
    this->textMode = mode;
}

template<typename T, typename... Args>
std::unique_ptr<T> parse(const std::string &info, const Args &...args)
{
    const auto parse_result = toml::parse(info);
    if (parse_result.failed())
    {
        qWarning() << "Failed to parse component:" << parse_result.error().description();
        return nullptr;
    }
    return std::make_unique<T>(parse_result.table(), args...);
}

ModelItem *Model::buildCategory(const std::string &categoryName,
                                std::unordered_map<std::string, std::set<std::string>> &sectionCategories,
                                std::unordered_map<std::string, std::set<std::string>> &sectionComponents,
                                const std::unordered_set<std::string> &editionComponents)
{
    auto category = componentRepository->get<Category>(std::string(categoryName));
    auto *item = new ModelItem(&category.value().get());

    for (const auto &childCategoryName : sectionCategories[categoryName])
    {
        item->appendRow(buildCategory(childCategoryName, sectionCategories, sectionComponents, editionComponents));
    }

    for (const auto &childComponentName : sectionComponents[categoryName])
    {
        auto component = componentRepository->get<Component>(std::string(childComponentName));
        auto *childItem = new ModelItem(&component->get());
        childItem->setData(QVariant::fromValue(ModelItem::Cache{
                               .hasRelationToEdition = editionComponents.contains(component->get().name),
                           }),
                           CustomRoles::CacheRole);
        item->appendRow(childItem);
    }

    return item;
}

void Model::buildItemTree(QStandardItem *insertable,
                          const std::unordered_set<std::string> &components,
                          const std::unordered_set<std::string> &editionComponents)
{
    std::unordered_map<std::string, std::set<std::string>> indexComponents;
    for (const auto &name : components)
    {
        auto optionalComponent = componentRepository->get<Component>(std::string(name));
        if (!optionalComponent.has_value())
        {
            qWarning() << "Could not find component" << name;
            continue;
        }
        indexComponents[optionalComponent->get().category].insert(name);
    }

    auto indexCategories = std::unordered_map<std::string, std::set<std::string>>{};
    for (const auto &[parent, component] : indexComponents)
    {
        for (auto parentIt = parent; !parentIt.empty();)
        {
            auto optionalCategory = componentRepository->get<Category>(std::string(parentIt));
            if (!optionalCategory.has_value())
            {
                qWarning() << "Category" << parentIt << "does not exist";
                break;
            }
            auto nextParent = std::string(optionalCategory->get().category);
            indexCategories[nextParent].insert(parentIt);
            parentIt = nextParent;
        }
    }

    for (const auto &topLevelCategory : indexCategories[""])
    {
        insertable->insertRow(insertable->rowCount(),
                              buildCategory(topLevelCategory, indexCategories, indexComponents, editionComponents));
    }
}

ModelItem *Model::buildSectionItem(Section &section)
{
    auto *sectionItem = new ModelItem(&section);
    buildItemTree(sectionItem, section.components, section.components);
    return sectionItem;
}

ModelItem *Model::buildTagItem(Tag &tag)
{
    auto *tagItem = new ModelItem(&tag);

    auto tagComponents = std::unordered_set<std::string>{};
    for (const auto &[name, component] : componentRepository->getAll<Component>())
    {
        if (component.tags.contains(tag.name))
        {
            tagComponents.insert(name);
        }
    }
    buildItemTree(tagItem, tagComponents, tagComponents);
    return tagItem;
}

ModelItem *Model::buildDefaultSectionItem(const std::vector<Section> &sections)
{
    toml::table defaultSectionData;
    defaultSectionData.insert(entity::COMPONENT_NAME_KEY_NAME, entity::DEFAULT_SECTION_NAME);
    toml::table defaultSectionDisplayNamesData;
    defaultSectionDisplayNamesData.insert("ru", "Другие компоненты");
    defaultSectionDisplayNamesData.insert("en", "Other components");
    defaultSectionData.insert(entity::COMPONENT_DISPLAY_NAME_KEY_NAME, defaultSectionDisplayNamesData);
    auto *defaultSection = new Section(defaultSectionData, entity::DEFAULT_SECTION_WEIGHT);
    auto *sectionItem = new ModelItem(defaultSection);

    std::set<std::string> componentsInSections;
    for (const auto &section : sections)
    {
        componentsInSections.insert(section.components.begin(), section.components.end());
    }

    std::set<std::string> allComponents;
    for (const auto &[name, _] : componentRepository->getAll<Component>())
    {
        allComponents.insert(name);
    }

    std::unordered_set<std::string> defaultComponents;
    std::set_difference(allComponents.begin(),
                        allComponents.end(),
                        componentsInSections.begin(),
                        componentsInSections.end(),
                        std::inserter(defaultComponents, defaultComponents.begin()));

    buildItemTree(sectionItem, defaultComponents, {});

    return sectionItem;
}

std::unordered_set<std::string> Model::getEditionComponents()
{
    const auto currentEdition = editionRepository->current();
    if (!currentEdition)
    {
        return {};
    }
    std::unordered_set<std::string> editionComponents;
    for (const auto &section : currentEdition->get().sections)
    {
        editionComponents.insert(section.components.begin(), section.components.end());
    }

    return editionComponents;
}

void Model::buildBySections(bool hard)
{
    clear();

    if (hard)
    {
        componentRepository->update();
        editionRepository->update();
    }

    // NOTE(chernigin): it is important that we do NOT copy current_edition->sections here, and use reference instead
    auto emptySections = std::vector<Section>{};
    const auto currentEdition = editionRepository->current();
    auto &sections = currentEdition ? currentEdition->get().sections : emptySections;

    for (auto &section : sections)
    {
        appendRow(buildSectionItem(section));
    }
    appendRow(buildDefaultSectionItem(sections));
}

void Model::buildPlain(bool hard)
{
    clear();

    if (hard)
    {
        componentRepository->update();
        editionRepository->update();
    }
    std::unordered_set<std::string> components;
    boost::range::transform(componentRepository->getAll<Component>(),
                            std::inserter(components, components.begin()),
                            [](const auto &p) { return p.first; });

    // NOTE(chernigin): we need to copy map here as it is being consume while building
    buildItemTree(invisibleRootItem(), components, getEditionComponents());
}

void Model::buildByTags(bool hard)
{
    clear();

    if (hard)
    {
        componentRepository->update();
        editionRepository->update();
    }

    for (auto &tag : editionRepository->current()->get().tags)
    {
        appendRow(buildTagItem(tag));
    }
}
} // namespace alt
