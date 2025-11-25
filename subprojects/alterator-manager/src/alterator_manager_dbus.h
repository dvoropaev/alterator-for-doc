/**/

#ifndef _ALTERATOR_MANAGER_DBUS_H
#define _ALTERATOR_MANAGER_DBUS_H

#include <gio/gio.h>

G_BEGIN_DECLS

#define DBUS_OBJECT_NAME "org.altlinux.alterator"
#define DBUS_OBJECT_PATH "/org/altlinux/alterator"

guint alterator_manager_dbus_init(gboolean);

#ifdef ENABLE_TEST_API
/* Test-oriented helper: register the DBus subtree on an existing connection.
 * This allows tests to bind the service to a GTestDBus connection without
 * relying on g_bus_own_name main loop machinery. */
gboolean alterator_manager_dbus_register_on_connection(
                                                   GDBusConnection *connection);
#endif

G_END_DECLS

#endif
