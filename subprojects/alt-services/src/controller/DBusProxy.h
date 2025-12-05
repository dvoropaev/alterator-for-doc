#pragma once

#include <QObject>
#include <QStringList>

#include "Controller.h"

class DBusProxy : public QObject
{
    Q_OBJECT
public:
    DBusProxy();
    ~DBusProxy();

    QStringList getServicePaths();
    QString getServiceInfo(const QString& path);
    QString getDiagInfo(const QString& path, bool session = false);
    bool getDiagToolTests(const QString& path, const QString& service, const QString& mode, bool required, QStringList& data, bool session);

    int     status(const QString& path, QByteArray& result);
    bool    deploy(const QString& path, const QString& data);
    bool  undeploy(const QString& path, const QString& data);
    bool configure(const QString& path, const QString& data);
    bool     start(const QString& path);
    bool      stop(const QString& path);
    bool    backup(const QString& path, const QString& data);
    bool   restore(const QString& path, const QString& data);

    void setEnv(const QString& key, const QString& value);
    void clearEnv();

    Controller::Result runDiag(const QString& path, const QString& test, bool session = false);

signals:
    void stdout(QString);
    void stderr(QString);

private:
    class ScopedSignalConnection;
    using ScopedSignalPtr = std::unique_ptr<ScopedSignalConnection>;
    using ScopedSignalPair = std::pair<ScopedSignalPtr, ScopedSignalPtr>;
    ScopedSignalPair makeServiceSignals(const QString& path);
    ScopedSignalPair makeDiagSignals(const QString& path, bool session);

    class Private;
    Private* d;
};

