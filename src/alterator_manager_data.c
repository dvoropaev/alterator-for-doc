/**/

#include "alterator_manager_data.h"


static ManagerData manager_data = {NULL, NULL, NULL, FALSE};

ManagerData* alterator_manager_data_get_data(void) {
    return &manager_data;
}

void alterator_manager_data_init(gboolean user_mode) {
    manager_data.backends_data = alterator_manager_backends_get_data();
    manager_data.sender_environment_data = alterator_manager_sender_environment_get_data();
    manager_data.authority = alterator_manager_polkit_get_authority();
    manager_data.user_mode = user_mode;
}
