#ifndef I_LOGGER_H
#define I_LOGGER_H

#include <chrono>
#include <string>

namespace alt
{
class ILogger
{
public:
    enum class Level : char
    {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };
    struct Entry;

public:
    virtual ~ILogger() = default;

public:
    virtual void write(const Entry &entry) = 0;
    virtual void write(Level level,
                       const std::string &message,
                       std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now())
        = 0;
};

struct ILogger::Entry
{
    Level level;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
};

} // namespace alt
#endif // I_LOGGER_H
