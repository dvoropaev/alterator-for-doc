#include "transactionservice.h"
#include "constants.h"
#include "dbus/dbusproxy.h"
#include "service/transaction.h"

#include <boost/range/algorithm/transform.hpp>
#include <QApplication>
#include <QDBusConnection>

namespace alt
{
void TransactionService::SafeMode::set(Target variant)
{
    value = static_cast<Target>(variant | value);
}

void TransactionService::SafeMode::reset(Target variant)
{
    value = static_cast<Target>(~variant & value);
}

bool TransactionService::SafeMode::test(Target variant) const
{
    return value & variant;
}

TransactionService::SafeMode::operator TransactionService::SafeMode::Target() const
{
    return value;
}

Transaction TransactionService::m_current{};
TransactionService::SafeMode TransactionService::m_safeMode = TransactionService::SafeMode{SafeMode::All};

TransactionService::TransactionService()
{
    DBusProxy::get().resetManagerLocale();

    auto signalSuffix = DBusProxy::rawConnection().baseService();
    signalSuffix.replace(':', '_');
    signalSuffix.replace('.', '_');

    const auto aptSignals = {
        APT_INSTALL_STDOUT_SIGNAL_NAME,
        APT_INSTALL_STDERR_SIGNAL_NAME,
        APT_REMOVE_STDOUT_SIGNAL_NAME,
        APT_REMOVE_STDERR_SIGNAL_NAME,
    };

    for (const auto &signal : aptSignals)
    {
        DBusProxy::rawConnection().connect(ALTERATOR_MANAGER_SERVICE_NAME,
                                           APT1_PATH,
                                           APT1_INTERFACE_NAME,
                                           signal + signalSuffix,
                                           this,
                                           SLOT(onAptNewLine(const QString &)));
    }
}

TransactionService::~TransactionService() = default;

TransactionService &TransactionService::instance()
{
    static TransactionService service;
    return service;
}

Transaction &TransactionService::create()
{
    m_current = Transaction();
    return current();
}

Transaction &TransactionService::current()
{
    return m_current;
}

TransactionService::SafeMode TransactionService::safeMode()
{
    return m_safeMode;
}

void TransactionService::setSafeMode(SafeMode mode)
{
    m_safeMode = mode;
}

void TransactionService::onAptNewLine(const QString &line)
{
    emit aptNewLine(line);
}
} // namespace alt
