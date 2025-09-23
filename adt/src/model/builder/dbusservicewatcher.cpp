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

#include "dbusservicewatcher.h"

#include <QDBusInterface>
#include <QDebug>

ADTServiceChecker::ADTServiceChecker(QString dbusServiceName, QString dbusPath, QString dbusInterface)
    : m_dbusServiceName(dbusServiceName)
    , m_dbusPath(dbusPath)
    , m_dbusIntefaceName(dbusInterface)
{
    m_dbusConnection = std::make_unique<QDBusConnection>(QDBusConnection::systemBus());

    m_watcherForDBusServiceOwnerChanged = std::make_unique<QDBusServiceWatcher>(m_dbusServiceName,
                                                                                *m_dbusConnection,
                                                                                QDBusServiceWatcher::WatchForOwnerChange);
    m_watcherForDBusServiceRegistered   = std::make_unique<QDBusServiceWatcher>(m_dbusServiceName,
                                                                              *m_dbusConnection,
                                                                              QDBusServiceWatcher::WatchForRegistration);
    m_watcherForDBusServiceUnregistered
        = std::make_unique<QDBusServiceWatcher>(m_dbusServiceName,
                                                *m_dbusConnection,
                                                QDBusServiceWatcher::WatchForUnregistration);

    connect(m_watcherForDBusServiceOwnerChanged.get(),
            &QDBusServiceWatcher::serviceOwnerChanged,
            this,
            &ADTServiceChecker::on_dbusServiceOwnerChanged);
    connect(m_watcherForDBusServiceRegistered.get(),
            &QDBusServiceWatcher::serviceRegistered,
            this,
            &ADTServiceChecker::on_dbusServiceRegistered);
    connect(m_watcherForDBusServiceUnregistered.get(),
            &QDBusServiceWatcher::serviceUnregistered,
            this,
            &ADTServiceChecker::on_dbusServiceUnregistered);
}

bool ADTServiceChecker::checkServiceInterfaceAvailability()
{
    return m_dbusConnection->isConnected()
           && QDBusInterface{m_dbusServiceName, m_dbusPath, m_dbusIntefaceName, *m_dbusConnection}.isValid();
}

void ADTServiceChecker::on_dbusServiceUnregistered()
{
    emit serviceUnregistered();
}

void ADTServiceChecker::on_dbusServiceRegistered()
{
    emit serviceRegistered();
}

void ADTServiceChecker::on_dbusServiceOwnerChanged()
{
    emit serviceOwnerChanged();
}
