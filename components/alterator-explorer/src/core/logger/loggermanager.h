#ifndef AB_LOGGER_MANAGER_H
#define AB_LOGGER_MANAGER_H

#include "../core.h"
#include "logger.h"

#include <memory>
#include <mutex>
#include <vector>

#include <QString>

namespace ab
{
namespace logger
{
class LoggerManagerPrivate;

class AB_CORE_EXPORT LoggerManager
{
public:
    static LoggerManager *globalInstance();
    static void destroyInstance();

    LoggerManager();
    ~LoggerManager();

    void addLogger(std::shared_ptr<Logger> logger);
    void removeLogger(std::shared_ptr<Logger> logger);
    void clearLoggers();
    size_t getLoggerCount() const;

    void log(const QtMsgType &msgType,
             const std::string &message,
             const std::string &file,
             const std::string &function,
             const uint32_t line);

    template<typename T>
    void addLogger(const QtMsgType &level)
    {
        auto logger = std::make_shared<T>();
        logger->setLogLevel(level);
        this->addLogger(logger);
    }

private:
    LoggerManager(const LoggerManager &) = delete;            // copy ctor
    LoggerManager(LoggerManager &&) = delete;                 // move ctor
    LoggerManager &operator=(const LoggerManager &) = delete; // copy assignment
    LoggerManager &operator=(LoggerManager &&) = delete;      // move assignment

private:
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    static std::shared_ptr<LoggerManager> instance;
    LoggerManagerPrivate *d;
};
} // namespace logger
} // namespace ab

#endif // AB_LOGGER_MANAGER_H
