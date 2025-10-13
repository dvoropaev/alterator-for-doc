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

#include "adtmodelbuilderdbus.h"
#include "adttoolbuilder.h"
#include "model/adttest.h"
#include "model/treeitem.h"

#include <memory>
#include <QDBusReply>
#include <QDebug>

const QString ADTModelBuilderDbus::LIST_METHOD = "List";
const QString ADTModelBuilderDbus::INFO_METHOD = "Info";

ADTModelBuilderDbus::ADTModelBuilderDbus(DBusConfig dbusInterfaceConfig)
    : m_dbusInterfaceConfig(dbusInterfaceConfig)
    , m_systemBus(QDBusConnection::systemBus())
    , m_sessionBus(QDBusConnection::sessionBus())
    , m_systemBusInterface(std::make_unique<QDBusInterface>(dbusInterfaceConfig.managerService,
                                                            dbusInterfaceConfig.managerPath,
                                                            dbusInterfaceConfig.managerInterface,
                                                            m_systemBus))
    , m_sessionBusInterface(std::make_unique<QDBusInterface>(dbusInterfaceConfig.managerService,
                                                             dbusInterfaceConfig.managerPath,
                                                             dbusInterfaceConfig.managerInterface,
                                                             m_sessionBus))
{}

std::shared_ptr<TreeModel> ADTModelBuilderDbus::buildModel()
{
    //Build ADTTool
    std::vector<std::unique_ptr<ADTTool>> sessionTools = buildToolsFromBus(m_sessionBus,
                                                                           m_sessionBusInterface.get(),
                                                                           ADTTool::BusType::Session);
    if (sessionTools.empty())
    {
        qWarning() << "Warning: can't find tools on session bus.";
    }

    std::vector<std::unique_ptr<ADTTool>> systemtTools = buildToolsFromBus(m_systemBus,
                                                                           m_systemBusInterface.get(),
                                                                           ADTTool::BusType::System);
    if (systemtTools.empty())
    {
        qWarning() << "Warning: can't find tools on system bus.";
    }

    //Merge ADTTool objects
    std::vector<std::unique_ptr<ADTTool>> tools = mergeTools(std::move(sessionTools), std::move(systemtTools));

    return buildTreeModel(std::move(tools));
}

QStringList ADTModelBuilderDbus::getObjectsPathByInterface(QDBusInterface *iface, QString interface)
{
    QDBusReply<QList<QDBusObjectPath>> reply = iface->call(m_dbusInterfaceConfig.managerGetMethod, interface);
    QList<QDBusObjectPath> pathList          = reply.value();

    QStringList paths{};
    for (const auto &path : pathList)
    {
        paths.push_back(path.path());
    }
    return paths;
}

std::vector<std::unique_ptr<ADTTool>> ADTModelBuilderDbus::buildToolsFromBus(QDBusConnection connection,
                                                                             QDBusInterface *iface,
                                                                             ADTTool::BusType busType)
{
    QStringList objectsPaths = getObjectsPathByInterface(iface, m_dbusInterfaceConfig.diagInterface);

    if (objectsPaths.isEmpty())
        return {};

    std::vector<std::unique_ptr<ADTTool>> tools{};
    for (const QString &path : objectsPaths)
    {
        std::unique_ptr<ADTTool> tool = buildADTTool(connection, path, busType);
        if (!tool)
            continue;

        tools.push_back(std::move(tool));
    }

    return tools;
}

std::unique_ptr<ADTTool> ADTModelBuilderDbus::buildADTTool(QDBusConnection connection,
                                                           const QString &path,
                                                           ADTTool::BusType busType)
{
    QDBusInterface iface(m_dbusInterfaceConfig.managerService, path, m_dbusInterfaceConfig.diagInterface, connection);

    if (!iface.isValid())
    {
        qWarning() << "ERROR: object interface " << path << " is not valid. Skipping tool.";
        return nullptr;
    }

    QDBusReply<QStringList> testsReply = iface.call(m_dbusInterfaceConfig.diagListMethod);
    if (!testsReply.isValid())
    {
        qWarning() << "ERROR: Reply of " << path << " is not valid, can't get list of tests. Skipping tool.";
        return nullptr;
    }

    QStringList tests = testsReply.value();
    if (tests.empty())
    {
        qWarning() << "WARNING: No tests in " << path << ". Skipping tool.";
        return nullptr;
    }

    for (QString &testName : tests)
        testName = testName.trimmed();

    QDBusReply<QByteArray> infoReply = iface.call(m_dbusInterfaceConfig.diagInfoMethod);
    if (!infoReply.isValid())
    {
        qWarning() << "ERROR: Reply of " << path << " is not valid, can't get tool info. Skipping tool.";
        return nullptr;
    }

    QString info(infoReply.value());
    if (info.isEmpty())
    {
        qWarning() << "ERROR: The info of " << path << " is empty. Skipping tool.";
        return nullptr;
    }
    ADTToolBuilder builder(info,
                           tests,
                           busType,
                           m_dbusInterfaceConfig.managerService,
                           path,
                           m_dbusInterfaceConfig.diagInterface,
                           m_dbusInterfaceConfig.diagInfoMethod,
                           m_dbusInterfaceConfig.diagRunMethod,
                           m_dbusInterfaceConfig.diagReportMethod);

    return builder.buildTool();
}

std::vector<std::unique_ptr<ADTTool>> ADTModelBuilderDbus::mergeTools(std::vector<std::unique_ptr<ADTTool>> sessionTools,
                                                                      std::vector<std::unique_ptr<ADTTool>> systemTools)
{
    if (sessionTools.empty() && systemTools.empty())
        return {};

    if (systemTools.empty())
        return sessionTools;

    if (sessionTools.empty())
        return systemTools;

    //Move tests from session tool to system tool with same id
    for (auto sysIt = systemTools.begin(); sysIt != systemTools.end(); sysIt++)
    {
        bool stopFlag = false;
        while (!stopFlag)
        {
            for (auto sesIt = sessionTools.begin(); sesIt != sessionTools.end(); sesIt++)
            {
                stopFlag = true;
                if (!QString::compare((*sysIt)->id(), (*sesIt)->id(), Qt::CaseSensitive))
                {
                    (*sysIt)->m_reportSuffixUser = (*sesIt)->m_reportSuffix;

                    //Move vars
                    if (mergeVars(*sysIt, *sesIt))
                    {
                        //If found in session bus, change bus to  both
                        (*sysIt)->m_bus = ADTTool::BusType::All;
                        //Move tests
                        std::move((*sesIt)->m_tests.begin(),
                                  (*sesIt)->m_tests.end(),
                                  std::back_inserter((*sysIt)->m_tests));
                    }

                    //Remove session tool and start search in session tools again
                    stopFlag = true;
                    sessionTools.erase(sesIt);

                    break;
                }
            }

            if (!sessionTools.size())
                stopFlag = true;
        }
    }

    //Move remaining session tools to system tools
    std::move(sessionTools.begin(), sessionTools.end(), std::back_inserter(systemTools));

    return systemTools;
}

std::shared_ptr<TreeModel> ADTModelBuilderDbus::buildTreeModel(std::vector<std::unique_ptr<ADTTool>> tools)
{
    auto model    = std::make_shared<TreeModel>();
    auto rootItem = static_cast<TreeItem *>(model->parent(QModelIndex()).internalPointer());

    for (auto &tool : tools)
    {
        auto toolItem = std::make_unique<TreeItem>(QList<QVariant>{""}, tool.release(), rootItem);
        rootItem->appendChild(toolItem.release());
    }

    return model;
}

bool ADTModelBuilderDbus::mergeVars(std::unique_ptr<ADTTool> &systemTool, std::unique_ptr<ADTTool> &sessionTool)
{
    for (auto sysIt = systemTool->m_vars.begin(); sysIt != systemTool->m_vars.end(); sysIt++)
    {
        bool stopFlag = false;
        while (!stopFlag)
        {
            for (auto sesIt = sessionTool->m_vars.begin(); sesIt != sessionTool->m_vars.end(); sesIt++)
            {
                stopFlag = true;
                if (!QString::compare((*sysIt)->id(), (*sesIt)->id(), Qt::CaseSensitive))
                {
                    stopFlag = false;
                    sessionTool->m_vars.erase(sesIt);
                    break;
                }
            }

            if (!sessionTool->m_vars.size())
                stopFlag = true;
        }
    }

    std::move(sessionTool->m_vars.begin(), sessionTool->m_vars.end(), std::back_inserter(systemTool->m_vars));

    return true;
}
