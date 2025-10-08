#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <toml++/toml.hpp>
#include <qdebug.h>

#include "baseobjectparser.h"

namespace ao_builder
{
const ObjectParserInterface::Nodes &BaseObjectParser::getSections()
{
    return m_nodes;
}

QVariant nodeToVariant(const toml::node &);

inline void processTable(const toml::table &table, BaseObjectParser::Nodes &storage)
{
    for (const auto &[name, node] : table)
        storage[QString::fromStdString(name.data())] = nodeToVariant(node);
}

QVariant nodeToVariant(const toml::node &n)
{
    switch (n.type())
    {
    case toml::node_type::string:
        return QString::fromStdString(n.as_string()->get());

    case toml::node_type::boolean:
        return n.as_boolean()->get();

    case toml::node_type::integer:
        return qlonglong{n.as_integer()->get()};

    case toml::node_type::floating_point:
        return n.as_floating_point()->get();

    case toml::node_type::array: {
        QList<QVariant> list;
        for (const auto &node : *n.as_array())
            list.append(nodeToVariant(node));
        return list;
    }
    case toml::node_type::table: {
        BaseObjectParser::Nodes map;
        processTable(*n.as_table(), map);
        return map;
    }
    default:
        return {};
    }
}

QString getKeyLocale(const QString &keyName)
{
    auto indexOfOpeningBracket = keyName.lastIndexOf("[");
    auto indexOfClosingBracket = keyName.lastIndexOf("]");

    if (indexOfOpeningBracket >= indexOfClosingBracket || indexOfOpeningBracket == -1 || indexOfClosingBracket == -1)
    {
        return {};
    }

    return keyName.mid(indexOfOpeningBracket + 1, indexOfClosingBracket - indexOfOpeningBracket - 1);
}

QString getKeyNameWithoutLocale(const QString &keyName)
{
    auto indexOfOpeningBracket = keyName.lastIndexOf("[");
    auto indexOfClosingBracket = keyName.lastIndexOf("]");

    if (indexOfOpeningBracket >= indexOfClosingBracket || indexOfOpeningBracket == -1 || indexOfClosingBracket == -1)
    {
        return keyName;
    }

    return keyName.mid(0, indexOfOpeningBracket);
}

QVariantMap processIniSection(const boost::property_tree::ptree &data)
{
    BaseObjectParser::Nodes res;
    BaseObjectParser::Nodes localized;

    for (auto &[key, value] : data)
    {
        auto _key = getKeyNameWithoutLocale(QString::fromStdString(key));
        auto _value = QString::fromStdString(value.get_value(""));

        auto locale = getKeyLocale(QString::fromStdString(key));
        if (!locale.isEmpty())
        {
            auto map = localized[_key].toMap();
            map[locale] = _value;
            localized[_key] = map;
        }
        else
        {
            res[_key] = _value;
        }
    }

    for (auto it = localized.begin(); it != localized.end(); it++)
    {
        auto map = it.value().toMap();
        map["en"] = res[it.key()];
        res[it.key()] = map;
    }

    return res;
}

bool BaseObjectParser::parse(const QString &data)
{
    m_nodes.clear();

    try
    {
        try
        {
            std::istringstream iStream(data.toStdString());
            processTable(toml::parse(iStream), m_nodes);
        }
        catch (std::exception &e)
        {
            qWarning() << "failed parse as toml: " << e.what() << " falling back to ini";
            std::istringstream iStream(data.toStdString());
            boost::property_tree::ptree pt;
            boost::property_tree::ini_parser::read_ini(iStream, pt);

            for (auto &section : pt)
                m_nodes[QString::fromStdString(section.first)] = processIniSection(section.second);
        }
    }
    catch (std::exception &e)
    {
        qWarning() << "failed parse as ini: " << e.what();
        return false;
    }

    return true;
}

QString BaseObjectParser::getValue(const QString &key, const QString &section)
{
    auto val = section.isEmpty() ? m_nodes.value(key) : m_nodes.value(section).toMap().value(key);

    return val.canConvert<QVariantMap>() ? val.toMap()["en"].toString() : val.toString();
}
} // namespace ao_builder
