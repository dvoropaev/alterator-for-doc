#include "baseobjectbuilder.h"
#include "objects/object.h"
#include "parsers/objectparserinterface.h"

#include <qdebug.h>

namespace ao_builder
{
bool BaseObjectBuilder::buildFieldWithLocale(ObjectParserInterface *parser,
                                             QString entryName,
                                             QString &field,
                                             QMap<QString, QString> &localeStorage,
                                             QString sectionName)
{
    auto sections = parser->getSections();

    auto section = sectionName.isEmpty() ? sections : sections.value(sectionName).toMap();
    if (section.isEmpty())
    {
        return false;
    }

    auto data = section.value(entryName);
    if (data.canConvert<QVariantMap>())
    {
        auto locales = data.toMap();
        auto defaultName = locales.value("en");
        if (!defaultName.isValid())
            return false;

        for (auto it = locales.cbegin(); it != locales.cend(); it++)
            localeStorage.insert(it.key(), it.value().toString());

        field = defaultName.toString();
    }
    else
    {
        field = data.toString();
        localeStorage.insert("en", field);
    }

    return true;
}

// Build common fields
bool BaseObjectBuilder::buildBase(ObjectParserInterface *parser,
                                  Object *object,
                                  const KeyProvider &provider,
                                  const QString &sectionName)
{
    object->m_type = type();

    QString name = parser->getValue(provider.key<NAME>(), sectionName);
    if (name.isEmpty())
    {
        return false;
    }
    object->m_name = name;

    auto weight = parser->getValue(provider.key<WEIGHT>(), sectionName);
    if (!weight.isEmpty())
    {
        bool ok = true;
        int weight_ = weight.toInt(&ok);
        if (ok)
        {
            object->m_weight = weight_;
        }
        else
        {
            qWarning() << "Cannot parse" << provider.key<WEIGHT>() << "of object" << object->m_name << "using default";
            object->m_weight = DEFAULT_WEIGHT;
        }
    }
    else
    {
        qInfo() << provider.key<WEIGHT>() << "of object" << object->m_name << "is not specified, using default";
        object->m_weight = DEFAULT_WEIGHT;
    }

    return true;
}

std::vector<QString> BaseObjectBuilder::parseValuesFromKey(ObjectParserInterface *parser,
                                                           QString key,
                                                           QString delimiter,
                                                           QString section)
{
    QString values = parser->getValue(key, section);
    if (values.isEmpty())
    {
        return {};
    }

    if (values.back() == delimiter)
    {
        values.truncate(values.size() - 1);
    }

    QStringList valuesList = values.split(delimiter);

    std::vector<QString> result;
    for (QString &currentValue : valuesList)
    {
        if (!currentValue.isEmpty())
        {
            result.push_back(currentValue);
        }
    }

    return result;
}

} // namespace ao_builder
