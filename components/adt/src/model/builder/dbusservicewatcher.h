/***********************************************************************************************************************
**
** Copyright (C) 2023 BaseALT Ltd. <org@basealt.ru>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
***********************************************************************************************************************/

#ifndef ADTSERVICECHECKER_H
#define ADTSERVICECHECKER_H

#include <memory>
#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QObject>

class ADTServiceChecker : public QObject
{
    Q_OBJECT
public:
    ADTServiceChecker(QString dbusServiceName, QString dbusPath, QString dbusInterface);
    ~ADTServiceChecker() = default;

    bool checkServiceInterfaceAvailability();

private slots:
    void on_dbusServiceUnregistered();
    void on_dbusServiceRegistered();
    void on_dbusServiceOwnerChanged();

signals:
    void serviceUnregistered();
    void serviceRegistered();
    void serviceOwnerChanged();

private:
    QString m_dbusServiceName{};
    QString m_dbusPath{};
    QString m_dbusIntefaceName{};

    std::unique_ptr<QDBusServiceWatcher> m_watcherForDBusServiceRegistered{};
    std::unique_ptr<QDBusServiceWatcher> m_watcherForDBusServiceUnregistered{};
    std::unique_ptr<QDBusServiceWatcher> m_watcherForDBusServiceOwnerChanged{};

    std::unique_ptr<QDBusConnection> m_dbusConnection{};

private:
    ADTServiceChecker(const ADTServiceChecker &)            = delete;
    ADTServiceChecker(ADTServiceChecker &&)                 = delete;
    ADTServiceChecker &operator=(const ADTServiceChecker &) = delete;
    ADTServiceChecker &operator=(ADTServiceChecker &&)      = delete;
};

#endif // ADTSERVICECHECKER_H
