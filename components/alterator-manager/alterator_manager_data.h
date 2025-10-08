/**/

#ifndef _ALTERATOR_MANAGER_DATA_H
#define _ALTERATOR_MANAGER_DATA_H

#include "alterator_manager_module_info.h"
#include "alterator_manager_polkit.h"
#include "alterator_manager_backends.h"
#include "alterator_manager_sender_environment.h"

G_BEGIN_DECLS

ManagerData* alterator_manager_data_get_data(void);
void alterator_manager_data_init(gboolean user_mode);

G_END_DECLS

#endif
