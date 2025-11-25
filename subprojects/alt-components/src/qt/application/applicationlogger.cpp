#include "applicationlogger.h"

#include <QDateTime>
#include <QDebug>
#include <qlogging.h>

namespace alt
{
ApplicationLogger::ApplicationLogger()
    : warningsModel(std::make_unique<QStandardItemModel>(0, 2))
{}

ApplicationLogger::~ApplicationLogger() = default;

void ApplicationLogger::write(const Entry &entry)
{
    write(entry.level, entry.message, entry.timestamp);
}

void ApplicationLogger::write(Level level, const std::string &message, std::chrono::system_clock::time_point timestamp)
{
    std::ignore = timestamp;
    switch (level)
    {
    case Level::Error:
    case Level::Warning:
        qWarning().noquote() << message;
        break;
    case Level::Trace:
    case Level::Debug:
        qDebug().noquote() << message;
        break;
    case Level::Info:
        qInfo().noquote() << message;
        break;
    case Level::Critical:
    default:
        qCritical().noquote() << message;
    }

    auto type = toQtMsgType(level);
    auto *typeItem = new QStandardItem(toLocalizedString(type));
    typeItem->setData(type, Qt::UserRole);
    auto *item = new QStandardItem(QString::fromStdString(message));
    item->setData(type, Qt::UserRole);
    warningsModel->appendRow(QList<QStandardItem *>()
                             << new QStandardItem(QDateTime::currentDateTime().toString("hh:mm:ss")) << typeItem
                             << item);
}

QAbstractItemModel *ApplicationLogger::model()
{
    return warningsModel.get();
}

QtMsgType ApplicationLogger::toQtMsgType(Level level)
{
    switch (level)
    {
    case Level::Error:
    case Level::Warning:
        return QtMsgType::QtWarningMsg;
    case Level::Trace:
    case Level::Debug:
        return QtMsgType::QtDebugMsg;
    case Level::Info:
        return QtMsgType::QtInfoMsg;
    case Level::Critical:
    default:
        return QtMsgType::QtCriticalMsg;
    }
}

QString ApplicationLogger::toLocalizedString(const QtMsgType &level)
{
    switch (level)
    {
    case QtMsgType::QtCriticalMsg:
        return QObject::tr("Error");
    case QtMsgType::QtWarningMsg:
        return QObject::tr("Warning");
    default:
        return QObject::tr("Info");
    }
}

void ApplicationLogger::retranslate()
{
    if (!warningsModel)
    {
        return;
    }

    for (int row = 0; row < warningsModel->rowCount(); ++row)
    {
        if (auto *typeItem = warningsModel->item(row, 1))
        {
            QVariant levelData = typeItem->data(Qt::UserRole);
            if (levelData.isValid())
            {
                QtMsgType level = static_cast<QtMsgType>(levelData.toInt());
                typeItem->setText(toLocalizedString(level));
            }
        }
    }
}

} // namespace alt
