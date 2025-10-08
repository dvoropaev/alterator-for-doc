#include "model.h"

#include <QDebug>
#include <QStandardItemModel>
#include <qstandarditemmodel.h>

#include <algorithm>
#include <memory>
#include <set>
#include <vector>

#include "../../aobuilder/constants.h"
#include "../aobuilder/objects/legacyobject.h"

namespace ab::model
{
Model::Model()
{
    this->clear();
}

void Model::translateModel(QString locale)
{
    ObjectIndex new_index;

    while (!m_object_index.empty())
    {
        auto node = m_object_index.extract(m_object_index.begin());

        auto category = node.key();
        category->setLocale(locale);

        ObjectSet new_set;

        while (!node.mapped().empty())
        {
            auto *obj = node.mapped().extract(node.mapped().begin()).value().release();
            obj->setLocale(locale);
            new_set.insert(std::unique_ptr<ao_builder::Object>{obj});
        }

        new_index[category] = std::move(new_set);
    }
    m_object_index = std::move(new_index);
}

void Model::clear()
{
    m_app_storage.clear();
    m_category_storage.clear();

    m_app_index.clear();
    m_object_index.clear();
}

const QString IGNORE_UI = "html";
const QString IGNORE_CATEGORY = "X-Alterator-Hidden";

void Model::build(std::vector<CategoryPtr> categories, std::vector<AppPtr> apps, std::vector<ObjectPtr> objects)
{
    this->clear();

    m_app_storage = std::move(apps);
    m_category_storage = std::move(categories);

    auto defaultCategory = std::find_if(m_category_storage.cbegin(), m_category_storage.cend(), [](auto &c) {
        return c->m_name == ao_builder::category::NAME;
    });

    for (auto &app : m_app_storage)
        for (auto &interface : app->m_interfaces)
            m_app_index.insert(std::pair{interface, app.get()});

    for (ObjectPtr &object : objects)
    {
        const auto legacyObject = dynamic_cast<ao_builder::LegacyObject *>(object.get());
        if (object->m_isLegacy && legacyObject != nullptr)
        {
            if (legacyObject->m_x_Alterator_UI == IGNORE_UI)
            {
                qInfo() << "Ignoring object with html UI:" << object->m_name;
                continue;
            }

            if (legacyObject->m_categoryId == IGNORE_CATEGORY)
            {
                qInfo() << "Ignoring object with hidden category:" << object->m_name;
                continue;
            }
        }

        auto it = std::find_if(m_category_storage.cbegin(), m_category_storage.cend(), [&object](auto &c) {
            return c->m_name == object->m_categoryId;
        });

        auto &set = m_object_index[it != m_category_storage.cend() ? it->get() : defaultCategory->get()];

        if (object->m_isLegacy)
        {
            // search for overriding object
            auto override = std::find_if(set.begin(), set.end(), [&object](auto &obj) {
                return obj->m_dbus_path == object->m_dbus_path && obj->m_override == object->m_interface;
            });

            if (override != set.end())
            {
                // do not override if app does not exist
                if (getApps((*override)->m_interface).empty())
                    set.erase(override);
                else
                    continue;
            }
        }
        else if (!object->m_override.isEmpty())
        {
            // search for legacy object
            auto old = std::find_if(set.begin(), set.end(), [&object](auto &obj) {
                return obj->m_dbus_path == object->m_dbus_path && obj->m_interface == object->m_override;
            });

            if (old != set.end())
            {
                // do not override if app does not exist
                if (getApps(object->m_interface).empty())
                    continue;
                else
                    set.erase(old);
            }
        }
        else
        {
            // also skip non-overriding objects without app
            if (getApps(object->m_interface).empty())
                continue;
        }

        set.insert(std::move(object));
    }
}

std::vector<Model::AppPtr::pointer> Model::getApps(const QString &interface)
{
    auto its = m_app_index.equal_range(interface);
    if (its.first == m_app_index.end())
        return {};
    std::vector<Model::AppPtr::pointer> result{};
    for (; its.first != its.second; its.first++)
        result.push_back(its.first->second);

    return result;
}

std::vector<ao_builder::Category *> Model::getCategories()
{
    std::vector<ao_builder::Category *> result{};
    for (auto &[cat, objects] : m_object_index)
        if ( !objects.empty() )
         result.push_back(cat);
    return result;
}
std::vector<ao_builder::Object *> Model::getObjects(ao_builder::Category *c)
{
    std::vector<ObjectPtr::pointer> result{};
    auto it = m_object_index.find(c);
    if (it != m_object_index.end())
        for (auto &obj : it->second)
            result.push_back(obj.get());

    return result;
}

} // namespace ab::model
