#include "alteratorctlapp.h"
#include "alteratorctlcomponentsmodule.h"
#include "alteratorctldiagmodule.h"
#include "alteratorctleditionsmodule.h"
#include "alteratorctlmanagermodule.h"
#include "alteratorctlpackagesmodule.h"
#include "alteratorctlsysteminfomodule.h"

#include <glib.h>
#include <locale.h>

int main(int argc, char **argv)
{
    int ret = 0;

    setlocale(LC_ALL, "");
    bindtextdomain("alteratorctl", "/usr/share/alteratorctl/lang"); //*.mo files should be located in
                                                                    // domain_path/<locale>/LC_MESSAGES
    textdomain("alteratorctl");

    AlteratorCtlApp *app = alterator_ctl_app_new();

    alterator_ctl_module_t *manager_module = get_manager_module();

    alterator_ctl_module_t *packages_module = get_packages_module();

    alterator_ctl_module_t *components_module = get_components_module();

    alterator_ctl_module_t *editions_module = get_editions_module();

    alterator_ctl_module_t *diag_module = get_diag_module();

    alterator_ctl_module_t *systeminfo_module = get_systeminfo_module();

    alterator_ctl_register_module(app, manager_module);
    alterator_ctl_register_module(app, packages_module);
    alterator_ctl_register_module(app, components_module);
    alterator_ctl_register_module(app, editions_module);
    alterator_ctl_register_module(app, diag_module);
    alterator_ctl_register_module(app, systeminfo_module);

    ret = alterator_ctl_app_run(app, argc, argv);

    alterator_ctl_app_free(app);

end:
    return ret;
}
