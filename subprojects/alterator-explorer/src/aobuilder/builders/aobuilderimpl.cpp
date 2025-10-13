#include "../constants.h"

#include "../objects/category.h"
#include "aobuilderimpl.h"
#include "localapplicationobjectbuilder.h"
#include "objectbuilderfactory.h"
#include "objectbulderinterface.h"

#include "../datasource/datasourcedbusimpl.h"

#include "../parsers/baseobjectparser.h"
#include <qdebug.h>

namespace ao_builder
{
class AOBuilderImplPrivate
{
public:
    AOBuilderImplPrivate()
        : m_localAppBuilder(new LocalApplicationObjectBuilder())
        , m_dataSource(new DataSourceDBusImpl(dbus::SERVICE_NAME))
        , m_parser(new BaseObjectParser())
    {}

public:
    std::unique_ptr<ObjectBuilderInterface> m_localAppBuilder;
    std::unique_ptr<DataSourceInterface> m_dataSource;
    std::unique_ptr<ObjectParserInterface> m_parser;
};

AOBuilderImpl::AOBuilderImpl()
    : d(new AOBuilderImplPrivate())
{}

std::unique_ptr<Category> AOBuilderImpl::buildDefaultCategory()
{
    auto result = std::make_unique<Category>();
    result->m_name = category::NAME;

    result->m_displayName = category::DISPLAY_NAME;
    result->m_displayNameLocaleStorage.insert("ru", category::DISPLAY_NAME_RU);
    result->m_displayNameLocaleStorage.insert("en", category::DISPLAY_NAME_EN);

    result->m_comment = category::COMMENT;
    result->m_commentLocaleStorage.insert("ru", category::COMMENT_RU);
    result->m_commentLocaleStorage.insert("en", category::COMMENT_EN);

    result->m_icon = category::ICON;
    result->m_weight = 0;

    return result;
}

AOBuilderImpl::~AOBuilderImpl()
{
    delete d;
}

std::vector<std::unique_ptr<LocalAppObject>> AOBuilderImpl::buildLocalApps()
{
    QStringList appsList = d->m_dataSource->getLocalAppPaths();

    std::vector<std::unique_ptr<LocalAppObject>> result{};

    for (QString currentApp : appsList)
    {
        QString currentAppInfo = d->m_dataSource->getLocalAppInfo(currentApp);
        std::unique_ptr<LocalAppObject> currentObject{
            dynamic_cast<LocalAppObject *>(buildObject(currentAppInfo).release())};

        if (!currentObject)
        {
            qWarning() << "Cannot build app" << currentApp;
            continue;
        }

        qInfo() << "Successfully built app" << currentApp;
        result.push_back(std::move(currentObject));
    }
    return result;
}

std::vector<std::unique_ptr<Category>> AOBuilderImpl::buildCategories()
{
    QStringList catList = d->m_dataSource->getCategoriesList();

    std::vector<std::unique_ptr<Category>> result;

    for (QString currentCategory : catList)
    {
        QString currentCatInfo = d->m_dataSource->getCategoryInfo(currentCategory);
        std::unique_ptr<Category> currentObject{dynamic_cast<Category *>(buildObject(currentCatInfo).release())};

        if (currentObject)
        {
            qInfo() << "Successfully built category" << currentCategory;
            result.push_back(std::move(currentObject));
        }
    }

    result.push_back(buildDefaultCategory());

    return result;
}

std::vector<std::unique_ptr<Object>> AOBuilderImpl::buildObjects(std::set<QString> interfaces)
{
    std::vector<std::unique_ptr<Object>> result;

    for (auto interface : interfaces)
    {
        QStringList objectsList = d->m_dataSource->getObjectsPath(interface);

        for (QString currentObjectPath : objectsList)
        {
            QString currentObjectInfo = d->m_dataSource->getObjectInfo(currentObjectPath, interface);
            std::unique_ptr<Object> currentObject = buildObject(currentObjectInfo);

            if (currentObject)
            {
                qInfo() << "Successfully built object" << currentObjectPath;
                currentObject->m_interface = interface;
                currentObject->m_dbus_path = currentObjectPath;
                result.push_back(std::move(currentObject));
            }
        }
    }

    return result;
}

std::unique_ptr<Object> AOBuilderImpl::buildObject(const QString &info)
{
    if (info.isEmpty())
    {
        qWarning() << "Cannot parse object: Info returned empty string";
        return {};
    }

    if (!d->m_parser->parse(info))
    {
        qWarning() << "Cannot parse object";
        return {};
    }

    ObjectBuilderFactory factory{};
    std::unique_ptr<ObjectBuilderInterface> objectBuilder = factory.getBuilder(d->m_parser.get());

    if (!objectBuilder)
    {
        qWarning() << "Cannot get builder for an object";
        return {};
    }

    return objectBuilder->buildObject(d->m_parser.get());
}
} // namespace ao_builder
