/**/

#ifndef _ALTERATOR_MANAGER_BACKENDS_H
#define _ALTERATOR_MANAGER_BACKENDS_H

#include "alterator_manager_module_info.h"
#include <toml.h>

G_BEGIN_DECLS

#define TOP_SECTION "Alterator Entry"
#define TYPE_NAME "type"
#define TYPE_BACKEND "Backend"
#define MODULE_NAME "module"
#define NODE_NAME "name"
#define THREAD_LIMIT_NAME "thread_limit"
#define THREAD_LIMIT 10
#define INTERFACE_NAME "interface"
#define ACTION_ID "action_id"
#define TABLE_METHODS "methods"
#define TABLE_ENVIRONMENT "environment"
#define ENVR_DEFAULT_VALUE "default"
#define ENVR_REQUIRED "required"
#define KEY_FILES_PATH_SYS "/usr/share/alterator/backends/"
#define KEY_FILES_PATH_SYS_NEW "/usr/share/alterator/backends/system/"
#define KEY_FILES_PATH_USER "/usr/share/alterator/backends/user/"
#define KEY_FILES_PATH_ETC_SYS "/etc/alterator/backends/"
#define KEY_FILES_PATH_ETC_SYS_NEW "/etc/alterator/backends/system/"
#define KEY_FILES_PATH_ETC_USER "/etc/alterator/backends/user/"
#define INTERFACE_PREFIX "org.altlinux.alterator."
#define KEY_FILE_NAME_SUFFIX ".backend"
#define MAX_NAME_LENGTH 100

gboolean alterator_manager_backends_init(gboolean);
GHashTable* alterator_manager_backends_get_data(void);

G_END_DECLS

#endif
