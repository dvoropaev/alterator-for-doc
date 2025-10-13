#ifndef AB_SYSLOG_LOGGER_H
#define AB_SYSLOG_LOGGER_H

#include "../core.h"
#include "logger.h"
#include "loggermessage.h"

#include <fstream>

namespace ab
{
namespace logger
{
class AB_CORE_EXPORT SyslogLogger : public Logger
{
public:
    SyslogLogger();
    ~SyslogLogger();

private:
    void log(const LoggerMessage &message) override;
};
} // namespace logger
} // namespace ab

#endif // AB_SYSLOG_LOGGER_H
