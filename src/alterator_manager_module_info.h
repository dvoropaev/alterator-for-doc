/**/

#ifndef _ALTERATOR_MANAGER_MODULE_INFO_H
#define _ALTERATOR_MANAGER_MODULE_INFO_H

#include <gio/gio.h>
#include <polkit/polkit.h>

G_BEGIN_DECLS

#define ALTERATOR_MODULE_INTERFACE_VERSION 14

typedef struct {
    gboolean required;
    gchar *default_value;
} EnvironmentObjectInfo;

typedef struct {
    GHashTable *method_data;
    GHashTable *environment;
    GHashTable *method_data_arrays;
} MethodObjectInfo;

typedef struct {
    gchar *module_name;
    gchar *action_id;
    GDBusInterfaceInfo *interface_introspection;
    GHashTable *methods;
    const GDBusInterfaceVTable *interface_vtable;
    gint thread_limit;
} InterfaceObjectInfo;

typedef struct {
    GHashTable *backends_data;
    GHashTable *sender_environment_data;
    PolkitAuthority* authority;
    /* connection of handlers of g_bus_own_name(). Do not free it. */
    GDBusConnection *connection;
    /* Is the service running in system or user mode? If user_mode == FALSE,
    then system. */
    gboolean user_mode;
} ManagerData;

typedef struct {
    gint                       (*interface_version) (void);
    gboolean                   (*init)              (ManagerData *manager_data);
    void                       (*destroy)           (void);
    gboolean                   (*is_module_busy)    (void);
} AlteratorModuleInterface;

typedef struct {
    gboolean (*register_module) (const AlteratorModuleInterface *interface);
    gboolean (*interface_validation) (GDBusInterfaceInfo *interface_info,
                                      gchar *interface_name);
} AlteratorManagerInterface;

inline gint alterator_module_interface_version(void)
{
    return ALTERATOR_MODULE_INTERFACE_VERSION;
}

G_END_DECLS

#endif
