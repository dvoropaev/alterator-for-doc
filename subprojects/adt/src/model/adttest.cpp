#include "adttest.h"

ADTTest::ADTTest()
    : m_id()
    , m_testId()
    , m_toolId()
    , m_bus(BusType::None)
    , m_icon()
    , m_displayName()
    , m_comment()
    , m_exitCode(-1)
    , m_stringStdout()
    , m_stringStderr()
    , m_log()
    , m_displayNameLocaleStorage()
    , m_commentLocaleStorage()
{}

void ADTTest::setLocale(QString locale)
{
    auto displayNameIt = m_displayNameLocaleStorage.find(locale);
    if (displayNameIt != m_displayNameLocaleStorage.end())
    {
        m_displayName = *displayNameIt;
    }

    auto commentIt = m_commentLocaleStorage.find(locale);
    if (commentIt != m_commentLocaleStorage.end())
    {
        m_comment = *commentIt;
    }
}

void ADTTest::clearLogs()
{
    m_stringStdout.clear();
    m_stringStderr.clear();
    m_log.clear();
}

int ADTTest::exitCode() const
{
    return m_exitCode;
}

void ADTTest::setExitCode(int newExitCode)
{
    m_exitCode = newExitCode;
}

void ADTTest::appendToStdout(QString text)
{
    m_stringStdout.append(text);
}

QString ADTTest::getStdout()
{
    return m_stringStdout;
}

void ADTTest::appendToStderr(QString text)
{
    m_stringStderr.append(text);
}

QString ADTTest::getStderr()
{
    return m_stringStderr;
}

void ADTTest::getStdout(QString out)
{
    m_log.append(out + "\n");
    m_stringStdout.append(out + "\n");
    emit getStdoutText(m_toolId, m_id, out);
}

void ADTTest::getStderr(QString err)
{
    m_log.append(err + "\n");
    m_stringStderr.append(err + "\n");
    emit getStderrText(m_toolId, m_id, err);
}

QString ADTTest::testId() const
{
    return m_testId;
}

QString ADTTest::id() const
{
    return m_id;
}

QString ADTTest::toolId() const
{
    return m_toolId;
}

QString ADTTest::displayName() const
{
    return m_displayName;
}

QString ADTTest::icon() const
{
    return m_icon;
}

QString ADTTest::comment() const
{
    return m_comment;
}

QString ADTTest::stringStdout() const
{
    return m_stringStdout;
}

QString ADTTest::stringStderr() const
{
    return m_stringStderr;
}

QString ADTTest::log() const
{
    return m_log;
}

ADTTest::BusType ADTTest::bus() const
{
    return m_bus;
}
