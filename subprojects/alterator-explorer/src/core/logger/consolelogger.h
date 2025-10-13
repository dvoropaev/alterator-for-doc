#ifndef AB_CONSOLE_LOGGER_H
#define AB_CONSOLE_LOGGER_H

#include "../core.h"
#include "logger.h"
#include "loggermessage.h"

namespace ab
{
namespace logger
{
class AB_CORE_EXPORT ConsoleLogger : public Logger
{
public:
    ConsoleLogger();

private:
    void log(const LoggerMessage &message) override;

    bool hasColorSupport = false;
};
} // namespace logger
} // namespace ab

#endif // AB_CONSOLE_LOGGER_H
