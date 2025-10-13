#ifndef AB_ABSTRACT_LOGGER_H
#define AB_ABSTRACT_LOGGER_H

#include "../core.h"
#include "loggermessage.h"

#include <fstream>
#include <unordered_map>
#include <QtMessageHandler>

#define LOG_LEVEL_DISABLED static_cast<QtMsgType>(-1)

namespace ab
{
namespace logger
{
class AB_CORE_EXPORT Logger
{
public:
    Logger() = default;
    virtual ~Logger() = default;

    void setLogLevel(QtMsgType level);
    bool isLogLevel(QtMsgType level);

    void logMessage(const LoggerMessage &message);

private:
    Logger(const Logger &) = delete;            // copy ctor
    Logger(Logger &&) = delete;                 // move ctor
    Logger &operator=(const Logger &) = delete; // copy assignment
    Logger &operator=(Logger &&) = delete;      // move assignment

private:
    virtual void log(const LoggerMessage &message) = 0;

    QtMsgType minLogLevel = QtDebugMsg;

protected:
    const std::unordered_map<QtMsgType, const char *> logLevelMap = {
        {QtDebugMsg, "DEBUG"},
        {QtInfoMsg, "INFO"},
        {QtWarningMsg, "WARNING"},
        {QtCriticalMsg, "CRITICAL"},
        {QtFatalMsg, "FATAL"},
    };
};
} // namespace logger
} // namespace ab

#endif // AB_ABSTRACT_LOGGER_H
