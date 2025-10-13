/**/

#ifndef _ALTERATOR_MANAGER_DBUS_H
#define _ALTERATOR_MANAGER_DBUS_H

#include <gio/gio.h>

G_BEGIN_DECLS

#define DBUS_OBJECT_NAME "org.altlinux.alterator"
#define DBUS_OBJECT_PATH "/org/altlinux/alterator"

guint alterator_manager_dbus_init(gboolean);

G_END_DECLS

#endif
