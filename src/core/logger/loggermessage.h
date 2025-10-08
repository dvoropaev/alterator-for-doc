#ifndef AB_LOGGER_MESSAGE_H
#define AB_LOGGER_MESSAGE_H

#include <ctime>
#include <string>
#include <thread>

#include <QtMessageHandler>

namespace ab
{
namespace logger
{
class LoggerMessage
{
public:
    LoggerMessage(const QtMsgType &msgType_,
                  const std::string &message_,
                  const std::string &filePath_,
                  const std::string &functionName_,
                  const uint32_t line_,
                  const std::tm &time_,
                  const std::thread::id &threadId_)
        : msgType(msgType_)
        , message(message_)
        , filePath(filePath_)
        , functionName(functionName_)
        , line(line_)
        , time(time_)
        , threadId(threadId_)
    {}

    std::string getTimeFormatted(const char *format) const
    {
        char timeString[50];
        // NOTE(mchernigin): do all compilers actually support this?
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
        strftime(timeString, 50, format, &time);
#pragma GCC diagnostic pop
        return std::string(timeString);
    }

    const QtMsgType &msgType;
    const std::string message;
    const std::string filePath;
    const std::string functionName;
    const uint32_t line;
    const std::tm time;
    const std::thread::id threadId;
};
} // namespace logger
} // namespace ab

#endif // AB_LOGGER_MESSAGE_H
