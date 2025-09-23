/**/

#ifndef _ALTERATOR_MANAGER_POLKIT_H
#define _ALTERATOR_MANAGER_POLKIT_H

#include <polkit/polkit.h>

G_BEGIN_DECLS

gboolean alterator_manager_polkit_init(void);
PolkitAuthority* alterator_manager_polkit_get_authority(void);

G_END_DECLS

#endif
