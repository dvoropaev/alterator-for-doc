#include "logger.h"

namespace ab
{
namespace logger
{
void Logger::setLogLevel(QtMsgType level)
{
    this->minLogLevel = level;
}

bool Logger::isLogLevel(QtMsgType level)
{
    return level >= this->minLogLevel;
}

void Logger::logMessage(const LoggerMessage &message)
{
    if (isLogLevel(message.msgType))
    {
        log(message);
    }
}
} // namespace logger
} // namespace ab
