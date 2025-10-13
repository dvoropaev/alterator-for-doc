#pragma once

#include <QObject>

class AppSettings : public QObject
{
    Q_OBJECT

#define PROPERTY(type, name) \
    Q_PROPERTY(type name READ name WRITE set_##name NOTIFY name##Changed) \
    type name(); \
    Q_SIGNAL void name##Changed(); \
    Q_SLOT void set_##name(const type & new_##name);

public:
    PROPERTY(bool, editorTableMode)
    PROPERTY(bool, tablesDetailed)
    PROPERTY(bool, lowerUnrequired)
    PROPERTY(QByteArray, mainWindowState)
    PROPERTY(Qt::ToolButtonStyle, toolButtonStyle)

#undef PROPERTY

public:
    explicit AppSettings(QObject *parent = nullptr);
    ~AppSettings();

private:
    class Private;
    Private* d;
};

