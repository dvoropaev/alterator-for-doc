#ifndef ADT_MODEL_BUILDER_DBUS_H
#define ADT_MODEL_BUILDER_DBUS_H

#include "adtmodelbuilderinterface.h"
#include "dbusconfig.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QString>

#include <model/adttool.h>

class ADTModelBuilderDbus : public ADTModelBuilderInterface
{
public:
    static const QString LIST_METHOD;
    static const QString INFO_METHOD;

    ADTModelBuilderDbus(DBusConfig iface = {});

public:
    std::shared_ptr<TreeModel> buildModel() override;

private:
    QStringList getObjectsPathByInterface(QDBusInterface *iface, QString interface);

    std::vector<std::unique_ptr<ADTTool>> buildToolsFromBus(QDBusConnection connection,
                                                            QDBusInterface *iface,
                                                            ADTTool::BusType busType);

    std::unique_ptr<ADTTool> buildADTTool(QDBusConnection connection, const QString &path, ADTTool::BusType busType);

    std::vector<std::unique_ptr<ADTTool>> mergeTools(std::vector<std::unique_ptr<ADTTool>> sessionTools,
                                                     std::vector<std::unique_ptr<ADTTool>> systemTools);

    std::shared_ptr<TreeModel> buildTreeModel(std::vector<std::unique_ptr<ADTTool>> tools);

    bool mergeVars(std::unique_ptr<ADTTool> &systemTool, std::unique_ptr<ADTTool> &sessionTool);

private:
    DBusConfig m_dbusInterfaceConfig{};

    QDBusConnection m_systemBus;
    QDBusConnection m_sessionBus;
    std::unique_ptr<QDBusInterface> m_systemBusInterface{};
    std::unique_ptr<QDBusInterface> m_sessionBusInterface{};
};

#endif // ADT_MODEL_BUILDER_DBUS_H
