/**/

#ifndef _ALTERATOR_MANAGER_BACKEND_MONITOR_H
#define _ALTERATOR_MANAGER_BACKEND_MONITOR_H

#include "alterator_manager_interface.h"

G_BEGIN_DECLS

#define BACKENDS_DIR_ETC "/etc/alterator/backends/"
#define BACKENDS_DIR_ETC_SYSTEM "/etc/alterator/backends/system/"
#define BACKENDS_DIR_ETC_USER "/etc/alterator/backends/user/"
#define BACKENDS_DIR_SHARE "/usr/share/alterator/backends/"
#define BACKENDS_DIR_SHARE_SYSTEM "/usr/share/alterator/backends/system/"
#define BACKENDS_DIR_SHARE_USER "/usr/share/alterator/backends/user/"

void alterator_manager_backend_monitor_init(void);
gboolean alterator_manager_backend_monitor_backends_changed(gpointer);

G_END_DECLS

#endif
