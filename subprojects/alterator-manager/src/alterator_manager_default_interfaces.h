/**/

#ifndef _ALTERATOR_MANAGER_DEFAULT_INTERFACES_H
#define _ALTERATOR_MANAGER_DEFAULT_INTERFACES_H

#include <gio/gio.h>

G_BEGIN_DECLS

#define INTERFACE_MANAGER "org.altlinux.alterator.manager"
#define METHOD_GET_OBJECTS "GetObjects"
#define METHOD_GET_SIGNALS "GetSignals"
#define METHOD_GET_INTERFACES "GetInterfaces"
#define METHOD_GET_ENVIRONMENT "GetEnvValue"
#define METHOD_SET_ENVIRONMENT "SetEnvValue"
#define METHOD_UNSET_ENVIRONMENT "UnsetEnvValue"
#define METHOD_GET_ALL_OBJECTS "GetAllObjects"
#define METHOD_GET_ALL_INTERFACES "GetAllInterfaces"
#define METHOD_GET_OBJECTS_ACTION_ID "org.altlinux.alterator.manager.get-objects"
#define METHOD_GET_SIGNALS_ACTION_ID "org.altlinux.alterator.manager.get-signals"
#define METHOD_GET_INTERFACES_ACTION_ID "org.altlinux.alterator.manager.get-interfaces"
#define METHOD_GET_ENVIRONMENT_ACTION_ID "org.altlinux.alterator.manager.get-env-value"
#define METHOD_SET_ENVIRONMENT_ACTION_ID "org.altlinux.alterator.manager.set-env-value"
#define METHOD_UNSET_ENVIRONMENT_ACTION_ID "org.altlinux.alterator.manager.unset-env-value"
#define METHOD_GET_ALL_OBJECTS_ACTION_ID "org.altlinux.alterator.manager.get-all-objects"
#define METHOD_GET_ALL_INTERFACES_ACTION_ID "org.altlinux.alterator.manager.get-all-interfaces"
#define SIGNAL_SUFFIX "_signal_name"
#define OBJECT_PATH_PREFIX "/org/altlinux/alterator/"
#define ERROR_NAME "org.altlinux.alterator"

typedef struct {
    const gchar *sender;
    const gchar *method_name;
    GVariant *parameters;
    GDBusMethodInvocation *invocation;
} ManagersMethodsParam;

GList* alterator_manager_default_interfaces_get_intorspections(void);
const GDBusInterfaceVTable* alterator_manager_default_interfaces_get_vtable(void);
void alterator_manager_default_interfaces_init(gboolean);

G_END_DECLS

#endif
