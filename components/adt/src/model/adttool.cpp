#include "adttool.h"
#include "builder/adttoolvarsmodelbuilder.h"

#include <QDBusInterface>
#include <QDBusReply>

ADTTool::ADTTool()
    : m_id()
    , m_bus(BusType::None)
    , m_type()
    , m_category()
    , m_icon()
    , m_displayName()
    , m_comment()
    , m_reportSuffix()
    , m_reportSuffixUser()
    , m_dbusPath()
    , m_dbusIface()
    , m_dbusServiceName()
    , m_dbusRunMethodName()
    , m_dbusReportMethodName()
    , m_dbusInfoMethodName()
    , m_displayNameLocaleStorage{}
    , m_commentLocaleStorage{}
    , m_tests{}
    , m_filter{}
    , m_vars{}
{}

void ADTTool::setLocale(QString &locale)
{
    auto displayNameIt = m_displayNameLocaleStorage.find(locale);
    if (displayNameIt != m_displayNameLocaleStorage.end())
    {
        m_displayName = *displayNameIt;
    }
    else
    {
        m_displayName = m_displayNameLocaleStorage["en"];
    }

    auto commentIt = m_commentLocaleStorage.find(locale);
    if (commentIt != m_commentLocaleStorage.end())
    {
        m_comment = *commentIt;
    }

    std::for_each(m_tests.begin(), m_tests.end(), [&locale](std::unique_ptr<ADTTest> &test) {
        test->setLocale(locale);
    });

    std::for_each(m_vars.begin(), m_vars.end(), [&locale](std::unique_ptr<ADTVarInterface> &var) {
        var->translate(locale);
    });
}

void ADTTool::setFilter(QString filter)
{
    m_filter = filter;
}

QString ADTTool::getFilter()
{
    return m_filter;
}

int ADTTool::getReport(QDBusConnection conn, QByteArray &result)
{
    QDBusInterface iface(m_dbusServiceName, m_dbusPath, m_dbusIface, conn);

    if (!iface.isValid())
    {
        return 1;
    }

    QDBusReply<QByteArray> reply = iface.call(m_dbusReportMethodName);

    if (!reply.isValid())
    {
        return 1;
    }

    result = reply.value();

    return 0;
}

std::unique_ptr<QStandardItemModel> ADTTool::getVarsModel()
{
    ADTToolVarsModelBuilder builder;

    return builder.buildVarsModel(m_vars, this);
}

ADTTool::BusType ADTTool::bus() const
{
    return m_bus;
}

QString ADTTool::id()
{
    return m_id;
}

QString ADTTool::type() const
{
    return m_type;
}

QString ADTTool::category() const
{
    return m_category;
}

QString ADTTool::icon() const
{
    return m_icon;
}

QString ADTTool::displayName() const
{
    return m_displayName;
}

QString ADTTool::comment() const
{
    return m_comment;
}

QString ADTTool::reportSuffix(ADTTool::BusType bus) const
{
    switch (bus) {
        case ADTTool::System:
            return m_reportSuffix;
        case ADTTool::Session:
            return m_reportSuffixUser;
        default:
            qCritical() << "failed to get report suffix: invalid BusType";
            return {};
    }
}

QString ADTTool::dbusPath() const
{
    return m_dbusPath;
}

QString ADTTool::dbusIface() const
{
    return m_dbusIface;
}

QString ADTTool::dbusServiceName() const
{
    return m_dbusServiceName;
}

QString ADTTool::runMethodName() const
{
    return m_dbusRunMethodName;
}

QString ADTTool::reportMethodName() const
{
    return m_dbusReportMethodName;
}

QString ADTTool::infoMethodName() const
{
    return m_dbusInfoMethodName;
}

QStringList ADTTool::getVarNames() const
{
    QStringList toolVars;

    std::for_each(m_vars.cbegin(), m_vars.cend(), [&toolVars](const std::unique_ptr<ADTVarInterface> &var) {
        toolVars.append(var->id());
    });

    return toolVars;
}

ADTVarInterface *ADTTool::getVar(QString &varId)
{
    ADTVarInterface *var = nullptr;

    for (auto &v : m_vars)
    {
        if (varId == v->id())
        {
            var = v.get();
            break;
        }
    }

    return var;
}

QStringList ADTTool::getFilteredTests()
{
    std::vector<ADTTest *> filteredTests{};
    std::for_each(m_tests.begin(), m_tests.end(), [this, &filteredTests](std::unique_ptr<ADTTest> &test) {
        if (test->displayName().contains(m_filter))
        {
            filteredTests.push_back(test.get());
        }
    });

    QStringList result;
    for (auto test : filteredTests)
    {
        result.append(test->id());
    }

    return result;
}

QStringList ADTTool::getTests(BusType bus)
{
    if (bus == BusType::None)
        bus = BusType::All;

    QStringList tests{};

    std::for_each(m_tests.begin(), m_tests.end(), [this, &tests, &bus](std::unique_ptr<ADTTest> &test) {
        if (test->bus() & bus)
            tests.push_back(test->id());
    });

    return tests;
}

ADTTest *ADTTool::getTest(QString id)
{
    ADTTest *test = nullptr;

    std::for_each(m_tests.begin(), m_tests.end(), [this, &id, &test](std::unique_ptr<ADTTest> &testPtr) {
        if (!QString::compare(testPtr->id(), id, Qt::CaseSensitive))
            test = testPtr.get();
    });

    return test;
}

void ADTTool::clearTestsLogs()
{
    std::for_each(m_tests.begin(), m_tests.end(), [](std::unique_ptr<ADTTest> &test) { test->clearLogs(); });
}
