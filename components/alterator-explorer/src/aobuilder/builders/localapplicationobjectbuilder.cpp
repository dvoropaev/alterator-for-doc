#include "localapplicationobjectbuilder.h"
#include "constants.h"
#include "objects/localappobject.h"

#include <memory>
#include <qdebug.h>

namespace ao_builder
{
Object::Type LocalApplicationObjectBuilder::type()
{
    return Object::APPLICATION;
}

std::unique_ptr<ao_builder::Object> LocalApplicationObjectBuilder::buildObject(ObjectParserInterface *parser)
{
    auto result = std::make_unique<LocalAppObject>();

    LocalAppObject *localApplication = result.get();

    if (!buildBase(parser, localApplication, KeyProvider::tomlKeys))
    {
        qWarning() << "Error parsing" << localApplication->m_genericName << "Info: cannot build names";
        return {};
    }

    QString type = parser->getValue(KeyProvider::tomlKeys.key<TYPE>());
    if (type.isEmpty() || type != type::APPLICATION)
    {
        qWarning() << "Error parsing" << localApplication << "Info: no Type";
        return {};
    }

    QString name = parser->getValue(KeyProvider::tomlKeys.key<NAME>());
    if (name.isEmpty())
    {
        qWarning() << "Error parsing" << localApplication << "Info: no Name";
        return {};
    }

    if (!buildFieldWithLocale(parser,
                              key::alterator::DISPLAY_NAME,
                              localApplication->m_displayName,
                              localApplication->m_displayNameLocaleStorage))
    {
        return {};
    }

    QString exec = parser->getValue(KeyProvider::tomlKeys.key<EXEC>());
    if (exec.isEmpty())
    {
        qWarning() << "Error parsing" << localApplication << "Info: no Exec";
        return {};
    }

    QString appInterfaces = parser->getValue(key::alterator::INTERFACE);
    if (appInterfaces.isEmpty())
    {
        qWarning() << "Error parsing" << localApplication << "Info: no Interface";
        return {};
    }

    localApplication->m_type = type;
    localApplication->m_name = name;
    localApplication->m_exec = exec;
    localApplication->m_interfaces.push_back(appInterfaces);

    return result;
}
} // namespace ao_builder
