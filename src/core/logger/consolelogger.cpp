#include "consolelogger.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <unistd.h>

static bool checkColorSupport(int fd)
{
    // TODO(mchernigin): use `tput colors`, and use method bellow only if `tput` returns !0

    bool is_tty = isatty(fd);
    const char *TERM = std::getenv("TERM");

    return is_tty && TERM != NULL && strcmp(TERM, "dumb") != 0;
}

static std::string colorize(const std::string &text, const char *params)
{
    return std::string("\033[") + params + "m" + text + "\033[0m";
}

namespace ab
{
namespace logger
{
ConsoleLogger::ConsoleLogger()
{
    this->hasColorSupport = checkColorSupport(STDERR_FILENO);
}

void ConsoleLogger::log(const LoggerMessage &message)
{
    std::string prefix = this->logLevelMap.at(message.msgType);

    if (this->hasColorSupport)
    {
        switch (message.msgType)
        {
        case QtDebugMsg:
            prefix = this->hasColorSupport ? colorize(prefix, "1;96") : prefix;
            break;
        case QtInfoMsg:
            prefix = this->hasColorSupport ? colorize(prefix, "1;34") : prefix;
            break;
        case QtWarningMsg:
            prefix = this->hasColorSupport ? colorize(prefix, "1;33") : prefix;
            break;
        case QtCriticalMsg:
            prefix = this->hasColorSupport ? colorize(prefix, "1;31") : prefix;
            break;
        case QtFatalMsg:
            prefix = this->hasColorSupport ? colorize(prefix, "1;91") : prefix;
            break;
        }
    }

    std::clog << message.getTimeFormatted("%H:%M:%S") << " | " << prefix << ": " << message.message << " ("
              << message.filePath << ":" << message.line << ")" << std::endl;
}
} // namespace logger
} // namespace ab
