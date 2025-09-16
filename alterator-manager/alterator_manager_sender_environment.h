/**/

#ifndef _ALTERATOR_MANAGER_SENDER_ENVIRONMENT_H
#define _ALTERATOR_MANAGER_SENDER_ENVIRONMENT_H

#include "alterator_manager_module_info.h"

G_BEGIN_DECLS

#define FREEDESKTOP_DBUS_BUS_NAME       "org.freedesktop.DBus"
#define FREEDESKTOP_DBUS_OBJECT_PATH    "/org/freedesktop/DBus"
#define FREEDESKTOP_DBUS_INTERFACE_NAME "org.freedesktop.DBus"
#define FREEDESKTOP_DBUS_METHOD_NAME    "ListNames"

gboolean alterator_manager_sender_environment_init(void);
void alterator_manager_sender_environment_clean(void);
gboolean alterator_manager_sender_environment_check_senders(gpointer data);
GHashTable* alterator_manager_sender_environment_get_data(void);
GHashTable* alterator_manager_sender_environment_get_table(const gchar* sender);
gboolean is_valid_environment_name(const gchar* name);

G_END_DECLS

#endif
