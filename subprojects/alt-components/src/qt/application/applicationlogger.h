#ifndef APPLICATION_LOGGER_H
#define APPLICATION_LOGGER_H

#include "interface/ilogger.h"

#include <memory>
#include <QStandardItemModel>

namespace alt
{
class ApplicationLogger : public ILogger, public QObject
{
public:
    ApplicationLogger();
    ~ApplicationLogger() override;

public:
    void write(const Entry &entry) override;
    void write(Level level, const std::string &message, std::chrono::system_clock::time_point timestamp) override;

public:
    void retranslate();
    QAbstractItemModel *model();

private:
    QString toLocalizedString(const QtMsgType &level);
    QtMsgType toQtMsgType(Level level);

private:
    std::unique_ptr<QStandardItemModel> warningsModel;
};
} // namespace alt
#endif // APPLICATION_LOGGER_H
