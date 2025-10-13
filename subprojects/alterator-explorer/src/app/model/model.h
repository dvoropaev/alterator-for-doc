#ifndef AB_MODEL_MODEL_H
#define AB_MODEL_MODEL_H

#include <memory>
#include <set>

#include "../../aobuilder/objects/category.h"
#include "../../aobuilder/objects/localappobject.h"

namespace ab::model
{
class Model
{
public:
    Model();
    ~Model() = default;

    using CategoryPtr = std::unique_ptr<ao_builder::Category>;
    using AppPtr = std::unique_ptr<ao_builder::LocalAppObject>;
    using ObjectPtr = std::unique_ptr<ao_builder::Object>;

    void translateModel(QString locale);
    void clear();

    void build(std::vector<CategoryPtr> categories, std::vector<AppPtr> apps, std::vector<ObjectPtr> objects);

    std::vector<CategoryPtr::pointer> getCategories();
    std::vector<ObjectPtr::pointer> getObjects(ao_builder::Category *c);
    std::vector<AppPtr::pointer> getApps(const QString &interface);

public:
    Model(const Model &) = delete;
    Model(Model &&) = delete;
    Model &operator=(const Model &) = delete;
    Model &operator=(Model &&) = delete;

private:
    std::vector<AppPtr> m_app_storage{};
    std::vector<CategoryPtr> m_category_storage{};

    struct CategoryCompare
    {
        bool operator()(const CategoryPtr::pointer &a, const CategoryPtr::pointer &b) const
        {
            return a->m_weight == b->m_weight ? a->m_displayName < b->m_displayName : a->m_weight > b->m_weight;
        }
    };

    struct ObjectCompare
    {
        bool operator()(const ObjectPtr &a, const ObjectPtr &b) const { return a->m_displayName < b->m_displayName; }
    };

    using ObjectSet = std::set<ObjectPtr, ObjectCompare>;
    using ObjectIndex = std::map<CategoryPtr::pointer, ObjectSet, CategoryCompare>;

    ObjectIndex m_object_index{};
    std::multimap<QString, AppPtr::pointer> m_app_index{};
};
} // namespace ab::model

#endif // AB_MODEL_MODEL_H
