#include "sysloglogger.h"

#include <sstream>
#include <syslog.h>

namespace ab
{
namespace logger
{
SyslogLogger::SyslogLogger()
{
    openlog("alterator-explorer", (LOG_CONS | LOG_PID), LOG_DAEMON);
}

SyslogLogger::~SyslogLogger()
{
    closelog();
}

void SyslogLogger::log(const LoggerMessage &message)
{
    const char *prefix = this->logLevelMap.at(message.msgType);

    int logFlag = LOG_DEBUG;
    switch (message.msgType)
    {
    case QtInfoMsg:
        logFlag = LOG_INFO;
        break;
    case QtWarningMsg:
        logFlag = LOG_WARNING;
        break;
    case QtCriticalMsg:
        logFlag = LOG_ERR;
        break;
    case QtFatalMsg:
        logFlag = LOG_CRIT;
        break;
    default:
        break;
    }

    syslog(logFlag, "%s: %s (%s:%u)", prefix, message.message.c_str(), message.filePath.c_str(), message.line);
}
} // namespace logger
} // namespace ab
