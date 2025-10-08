/**/

#include "alterator_manager_polkit.h"

static PolkitAuthority *authority = NULL;

gboolean alterator_manager_polkit_init(void) {
    GError *error = NULL;

    authority = polkit_authority_get_sync (NULL, &error);
    if (authority == NULL) {
        g_warning("alterator_manager_polkit_init(): polkit_authority_get_sync "
                  "returned NULL. Error message: %s.", error->message);
        g_error_free(error);
        return FALSE;
    }

    return TRUE;
}

PolkitAuthority* alterator_manager_polkit_get_authority(void) {
    return authority;
}
