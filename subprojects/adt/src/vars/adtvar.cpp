#include "adtvar.h"

ADTVar::ADTVar(const QString &id,
               int value,
               const QMap<QString, QString> &displayNames,
               const QMap<QString, QString> &comments)
    : m_type(ADTVarType::INT)
    , m_currentValue(std::make_unique<QVariant>(QVariant::fromValue(value)))
    , m_default(nullptr)
    , m_enumStorage(nullptr)
    , m_displayNames(displayNames)
    , m_comments(comments)
    , m_id(id)
    , m_displayName(m_displayNames[""])
    , m_comment(m_comments[""])
{
    m_displayName = *m_displayNames.find("en");
    m_comment     = *m_comments.find("en");
}

ADTVar::ADTVar(const QString &id,
               const QString &value,
               const QMap<QString, QString> &displayNames,
               const QMap<QString, QString> &comments)
    : m_type(ADTVarType::STRING)
    , m_currentValue(std::make_unique<QVariant>(QVariant::fromValue(value)))
    , m_default(nullptr)
    , m_enumStorage(nullptr)
    , m_displayNames(displayNames)
    , m_comments(comments)
    , m_id(id)
    , m_displayName(m_displayNames[""])
    , m_comment(m_comments[""])
{
    m_displayName = *m_displayNames.find("en");
    m_comment     = *m_comments.find("en");
}

ADTVar::ADTVar(const QString &id,
               const QList<int> &value,
               const int currentValue,
               const QMap<QString, QString> &displayNames,
               const QMap<QString, QString> &comments)
    : m_type(ADTVarType::ENUM_INT)
    , m_currentValue(std::make_unique<QVariant>(QVariant::fromValue(currentValue)))
    , m_default(std::make_unique<QVariant>(QVariant::fromValue(currentValue)))
    , m_enumStorage(std::make_unique<QVariant>(QVariant::fromValue(value)))
    , m_displayNames(displayNames)
    , m_comments(comments)
    , m_id(id)
    , m_displayName(m_displayNames[""])
    , m_comment(m_comments[""])
{
    m_displayName = *m_displayNames.find("en");
    m_comment     = *m_comments.find("en");
}

ADTVar::ADTVar(const QString &id,
               const QList<int> &value,
               const int currentValue,
               const int defaultValue,
               const QMap<QString, QString> &displayNames,
               const QMap<QString, QString> &comments)
    : m_type(ADTVarType::ENUM_INT)
    , m_currentValue(std::make_unique<QVariant>(QVariant::fromValue(currentValue)))
    , m_default(std::make_unique<QVariant>(QVariant::fromValue(defaultValue)))
    , m_enumStorage(std::make_unique<QVariant>(QVariant::fromValue(value)))
    , m_displayNames(displayNames)
    , m_comments(comments)
    , m_id(id)
    , m_displayName(m_displayNames[""])
    , m_comment(m_comments[""])
{
    m_displayName = *m_displayNames.find("en");
    m_comment     = *m_comments.find("en");
}

ADTVar::ADTVar(const QString &id,
               const QList<QString> &values,
               const QString &currentValue,
               const QMap<QString, QString> &displayNames,
               const QMap<QString, QString> &comments)
    : m_type(ADTVarType::ENUM_STRING)
    , m_currentValue(std::make_unique<QVariant>(QVariant::fromValue(currentValue)))
    , m_default(std::make_unique<QVariant>(QVariant::fromValue(currentValue)))
    , m_enumStorage(std::make_unique<QVariant>(QVariant::fromValue(values)))
    , m_displayNames(displayNames)
    , m_comments(comments)
    , m_id(id)
    , m_displayName(m_displayNames[""])
    , m_comment(m_comments[""])
{
    m_displayName = *m_displayNames.find("en");
    m_comment     = *m_comments.find("en");
}

ADTVar::ADTVar(const QString &id,
               const QList<QString> &values,
               const QString &currentValue,
               const QString &defaultValue,
               const QMap<QString, QString> &displayNames,
               const QMap<QString, QString> &comments)
    : m_type(ADTVarType::ENUM_STRING)
    , m_currentValue(std::make_unique<QVariant>(QVariant::fromValue(currentValue)))
    , m_default(std::make_unique<QVariant>(QVariant::fromValue(defaultValue)))
    , m_enumStorage(std::make_unique<QVariant>(QVariant::fromValue(values)))
    , m_displayNames(displayNames)
    , m_comments(comments)
    , m_id(id)
    , m_displayName(m_displayNames[""])
    , m_comment(m_comments[""])
{
    m_displayName = *m_displayNames.find("en");
    m_comment     = *m_comments.find("en");
}

ADTVar::~ADTVar() {}

bool ADTVar::set(const int value)
{
    if (m_type == ADTVarType::INT)
    {
        m_currentValue = std::make_unique<QVariant>(QVariant::fromValue(value));
        return true;
    }
    else if (m_type == ADTVarType::ENUM_INT)
    {
        if (m_enumStorage.get()->toList().contains(QVariant::fromValue(value)))
        {
            m_currentValue = std::make_unique<QVariant>(QVariant::fromValue(value));
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool ADTVar::get(int *value) const
{
    if (m_type == ADTVarType::INT || m_type == ADTVar::ENUM_INT)
    {
        if (m_currentValue.get()->typeId() == QVariant::Int)
        {
            *value = m_currentValue.get()->toInt();
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool ADTVar::set(const QString &string)
{
    if (m_type == ADTVarType::STRING)
    {
        m_currentValue = std::make_unique<QVariant>(QVariant::fromValue(string));
        return true;
    }
    else if (m_type == ADTVarType::ENUM_STRING)
    {
        if (m_enumStorage.get()->toList().contains(QVariant::fromValue(string)))
        {
            m_currentValue = std::make_unique<QVariant>(QVariant::fromValue(string));
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool ADTVar::get(QString *value) const
{
    if (m_type == ADTVarType::STRING || m_type == ADTVar::ENUM_STRING)
    {
        if (m_currentValue.get()->typeId() == QVariant::String)
        {
            *value = m_currentValue.get()->toString();
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool ADTVar::hasDefault() const
{
    return m_default.get();
}

bool ADTVar::getDefault(int *value) const
{
    if (m_type == ADTVarType::ENUM_INT)
    {
        if (m_default.get()->typeId() == QVariant::Int)
        {
            *value = m_default.get()->toInt();
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool ADTVar::getDefault(QString *value) const
{
    if (m_type == ADTVarType::ENUM_STRING)
    {
        if (m_default.get()->typeId() == QVariant::String)
        {
            *value = m_default.get()->toString();
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool ADTVar::getEnumValues(QList<QVariant> *values) const
{
    if (m_type == ADTVarType::ENUM_INT || m_type == ADTVarType::ENUM_STRING)
    {
        *values = m_enumStorage.get()->toList();
        return true;
    }
    return false;
}

QString ADTVar::id() const
{
    return m_id;
}

const QString ADTVar::getDisplayName() const
{
    return m_displayName;
}

const QString ADTVar::getComment() const
{
    return m_comment;
}

void ADTVar::translate(const QString &locale)
{
    auto displayNameIt = m_displayNames.find(locale);
    if (displayNameIt != m_displayNames.end())
    {
        m_displayName = *displayNameIt;
    }

    auto commentIt = m_comments.find(locale);
    if (commentIt != m_comments.end())
    {
        m_comment = *commentIt;
    }
}

ADTVarInterface::ADTVarType ADTVar::getType() const
{
    return m_type;
}
