#include "model/modelbuilder.h"
#include "application.h"
#include "constants.h"
#include "controller/controller.h"
#include "dbus/dbusproxy.h"
#include "model/item.h"
#include "model/model.h"
#include "model/objects/category.h"
#include "model/objects/component.h"
#include "model/objects/edition.h"
#include "model/objects/section.h"
#include "repository/componentrepository.h"

#include <boost/range/algorithm/transform.hpp>
#include <memory>
#include <set>
#include <unordered_map>
#include <QDBusInterface>
#include <QDBusReply>
#include <QtConcurrent/QtConcurrent>
#include <qassert.h>
#include <qlogging.h>

namespace alt
{
namespace
{
template<typename T, typename... Args>
std::unique_ptr<T> parse(const QString &info, const Args &...args)
{
    const auto parse_result = toml::parse(info.toStdString());
    if (parse_result.failed())
    {
        qWarning() << "Failed to parse component:" << parse_result.error().description();
        return nullptr;
    }
    return std::make_unique<T>(parse_result.table(), args...);
}

ModelItem *buildCategory(const QString &categoryName,
                         std::unordered_map<QString, std::set<QString>> &sectionCategories,
                         std::unordered_map<QString, std::set<QString>> &sectionComponents)
{
    auto category = ComponentRepository::get<Category>(categoryName);
    auto *item = new ModelItem(&category.value().get());
    bool hasCheckableChildren = false;

    for (const auto &childCategoryName : sectionCategories[categoryName])
    {
        auto *childCategoryItem = buildCategory(childCategoryName, sectionCategories, sectionComponents);
        hasCheckableChildren |= childCategoryItem->isCheckable();
        item->appendRow(childCategoryItem);
    }

    for (const auto &childComponentName : sectionComponents[categoryName])
    {
        auto component = ComponentRepository::get<Component>(childComponentName);
        auto *childComponentItem = new ModelItem(&component.value().get());
        hasCheckableChildren |= childComponentItem->isCheckable();
        item->appendRow(childComponentItem);
    }

    if (hasCheckableChildren)
    {
        item->setCheckable(true);
    }

    return item;
}

template<typename ComponentsIterable, typename QtRowInsertable>
void buildItemTree(QtRowInsertable &insertable, const ComponentsIterable &components)
{
    std::unordered_map<QString, std::set<QString>> indexComponents;
    for (const auto &name : components)
    {
        auto optionalComponent = ComponentRepository::get<Component>(name);
        if (!optionalComponent.has_value())
        {
            qWarning() << "Could not find component" << name;
            continue;
        }
        indexComponents[optionalComponent->get().category].insert(name);
    }

    auto indexCategories = std::unordered_map<QString, std::set<QString>>{};
    for (const auto &[parent, component] : indexComponents)
    {
        for (QString parentIt = parent; !parentIt.isEmpty();)
        {
            auto optionalCategory = ComponentRepository::get<Category>(parentIt);
            if (!optionalCategory.has_value())
            {
                qWarning() << "Category" << parentIt << "does not exist";
                break;
            }
            auto nextParent = optionalCategory->get().category;
            indexCategories[nextParent].insert(parentIt);
            parentIt = nextParent;
        }
    }

    for (const auto &topLevelCategory : indexCategories[""])
    {
        insertable->insertRow(insertable->rowCount(), buildCategory(topLevelCategory, indexCategories, indexComponents));
    }
}

ModelItem *buildSectionItem(Section &section)
{
    auto *sectionItem = new ModelItem(&section);
    buildItemTree(sectionItem, section.components);

    int componentCount = sectionItem->countComponentsRecursive();
    sectionItem->setText(QString("%1 (%2)").arg(section.displayName).arg(componentCount));

    return sectionItem;
}

ModelItem *buildTagItem(Tag &tag)
{
    auto *tagItem = new ModelItem(&tag);

    auto tagComponents = std::vector<QString>{};
    for (const auto &[name, component] : ComponentRepository::getAll<Component>())
    {
        if (component.tags.contains(tag.name))
        {
            tagComponents.push_back(name);
        }
    }
    buildItemTree(tagItem, tagComponents);

    int componentCount = tagItem->countComponentsRecursive();
    tagItem->setText(QString("%1 (%2)").arg(tag.displayName).arg(componentCount));
    return tagItem;
}

ModelItem *buildDefaultSectionItem(const QList<Section> &sections)
{
    auto *defaultSection = new Section();
    defaultSection->sort_weight = DEFAULT_SECTION_WEIGHT;
    defaultSection->name = DEFAULT_SECTION_NAME;
    defaultSection->displayNameLocaleStorage = {
        {"ru", "Другие компоненты"},
        {"en", "Other components"},
    };
    const auto lang = alt::Application::getLocale().name().split("_")[0];
    defaultSection->displayName = defaultSection->displayNameLocaleStorage.contains(lang)
                                      ? defaultSection->displayNameLocaleStorage[lang]
                                      : defaultSection->displayNameLocaleStorage[SECTION_DEFAULT_LANUAGE];
    auto *sectionItem = new ModelItem(defaultSection);

    QSet<QString> componentsInSections;
    for (const auto &section : sections)
    {
        componentsInSections.unite(QSet<QString>(section.components.begin(), section.components.end()));
    }

    QSet<QString> allComponents;
    for (const auto &[name, _] : ComponentRepository::getAll<Component>())
    {
        allComponents.insert(name);
    }

    QSet<QString> defaultComponentsSet = allComponents - componentsInSections;
    std::vector<QString> defaultComponents(defaultComponentsSet.begin(), defaultComponentsSet.end());

    buildItemTree(sectionItem, defaultComponents);

    int componentCount = sectionItem->countComponentsRecursive();
    sectionItem->setText(QString("%1 (%2)").arg(defaultSection->displayName).arg(componentCount));

    return sectionItem;
}
} // namespace

ModelBuilder::ModelBuilder(Controller *ctrl)
    : controller(ctrl)
{
    connect(&DBusProxy::get(), &DBusProxy::errorOccured, controller, &Controller::issueMessage);
}

void ModelBuilder::buildBySections(Model *model, bool hard)
{
    emit buildStarted();

    model->clear();

    if (hard)
    {
        ComponentRepository::update();
    }

    // NOTE(chernigin): it is important that we do NOT copy current_edition->sections here, and use reference instead
    auto emptySections = QList<Section>{};
    auto &sections = alt::Model::current_edition ? alt::Model::current_edition->sections : emptySections;

    for (auto &section : sections)
    {
        model->appendRow(buildSectionItem(section));
    }
    model->appendRow(buildDefaultSectionItem(sections));

    model->correctCheckItemStates();

    emit buildDone(alt::Model::current_edition.get(), model->countComponents().total, model->countEditionComponents());
}

void ModelBuilder::buildPlain(Model *model, bool hard)
{
    emit buildStarted();

    model->clear();

    if (hard)
    {
        ComponentRepository::update();
    }
    std::vector<QString> components;
    boost::range::transform(ComponentRepository::getAll<Component>(),
                            std::back_inserter(components),
                            [](const auto &p) { return p.first; });

    // NOTE(chernigin): we need to copy map here as it is being consume while building
    buildItemTree(model, components);

    model->correctCheckItemStates();

    emit buildDone(alt::Model::current_edition.get(), model->countComponents().total, model->countEditionComponents());
}

void ModelBuilder::buildByTags(Model *model, bool hard)
{
    emit buildStarted();

    model->clear();

    if (hard)
    {
        ComponentRepository::update();
    }

    for (auto &tag : alt::Model::current_edition->tags)
    {
        model->appendRow(buildTagItem(tag));
    }

    model->correctCheckItemStates();

    emit buildDone(alt::Model::current_edition.get(), model->countComponents().total, model->countEditionComponents());
}

std::unique_ptr<Edition> ModelBuilder::buildEdition()
{
    const auto current_edition_info = DBusProxy::get().getCurrentEditionInfo();
    if (current_edition_info.isEmpty())
    {
        return nullptr;
    }
    return parse<Edition>(current_edition_info);
}

} // namespace alt
