/**/

#ifndef _ALTERATOR_MANAGER_INTERFACE_H
#define _ALTERATOR_MANAGER_INTERFACE_H

#include "alterator_manager_module_info.h"

G_BEGIN_DECLS

#define INTROSPECTION_FILES_PATH "/usr/share/dbus-1/interfaces/"

const AlteratorManagerInterface* alterator_manager_interface(void);
gboolean alterator_manager_interface_modules_are_busy(void);

G_END_DECLS

#endif
