#ifndef AB_FILE_LOGGER_H
#define AB_FILE_LOGGER_H

#include "../core.h"
#include "logger.h"
#include "loggermessage.h"

#include <fstream>

namespace ab
{
namespace logger
{
class AB_CORE_EXPORT FileLogger : public Logger
{
public:
    explicit FileLogger(const char *filename = "alterator-explorer.log");
    ~FileLogger();

private:
    void log(const LoggerMessage &message) override;

    static std::string getHomeDir();
    static bool ensureDir(const char *path);

    std::fstream logFileStream = {};
};
} // namespace logger
} // namespace ab

#endif // AB_FILE_LOGGER_H
