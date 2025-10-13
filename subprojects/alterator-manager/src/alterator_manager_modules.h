/**/

#ifndef _ALTERATOR_MANAGER_MODULES_H
#define _ALTERATOR_MANAGER_MODULES_H

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define MODULES_DIR "/usr/libexec/alterator/"

gboolean alterator_manager_modules_init(void);

G_END_DECLS

#endif
