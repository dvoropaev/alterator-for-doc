#include "alteratorctlapp.h"
#include "alteratorctlcomponentsmodule.h"
#include "alteratorctldiagmodule.h"
#include "alteratorctleditionsmodule.h"
#include "alteratorctlmanagermodule.h"
#include "alteratorctlpackagesmodule.h"
#include "alteratorctlservicesmodule.h"
#include "alteratorctlsysteminfomodule.h"

#include "alteratorctlcommon.h"
#include <glib.h>
#include <locale.h>

int main(int argc, char **argv)
{
    int ret = 0;

    setlocale(LC_ALL, "");
    // If current codeset isn't UTF-8, force English C locale for ASCII-safe output
    if (!alterator_ctl_is_utf8_locale())
    {
        setenv("LC_ALL", "C", 1);
        setenv("LANG", "C", 1);
        setlocale(LC_ALL, "");
    }
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

    alterator_ctl_module_t *services_module = get_services_module();

    alterator_ctl_register_module(app, manager_module);
    alterator_ctl_register_module(app, packages_module);
    alterator_ctl_register_module(app, components_module);
    alterator_ctl_register_module(app, editions_module);
    alterator_ctl_register_module(app, diag_module);
    alterator_ctl_register_module(app, systeminfo_module);
    alterator_ctl_register_module(app, services_module);

    ret = alterator_ctl_app_run(app, argc, argv);

    alterator_ctl_app_free(app);

end:
    return ret;
}
