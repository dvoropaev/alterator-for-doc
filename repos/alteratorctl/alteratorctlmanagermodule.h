#ifndef ALTERATORCTL_MANAGERMODULE_H
#define ALTERATORCTL_MANAGERMODULE_H

#include <gio/gio.h>
#include <glib-object.h>

#include "alteratorctlapp.h"
#include "alteratorctlcommon.h"
#include "alteratorctlgdbussource.h"

#define TYPE_ALTERATOR_CTL_MANAGER_MODULE (alterator_ctl_manager_module_get_type())
#define ALTERATOR_CTL_MANAGER_MODULE(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_ALTERATOR_CTL_MANAGER_MODULE, AlteratorCtlManagerModule))
#define IS_ALTERATOR_CTL_MANAGER_MODULE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_ALTERATOR_CTL_MANAGER_MODULE))
#define ALTERATOR_CTL_MANAGER_MODULE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_ALTERATOR_CTL_MANAGER_MODULE, AlteratorCtlManagerModuleClass))
#define IS_ALTERATOR_CTL_MANAGER_MODULE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_ALTERATOR_CTL_MANAGER_MODULE))
#define ALTERATOR_CTL_MANAGER_MODULE_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_ALTERATOR_CTL_MANAGER_MODULE, AlteratorCtlManagerModuleClass))
enum manager_sub_commands
{
    MANAGER_GETOBJECTS,
    MANAGER_GETIFACES,
    MANAGER_GETSIGNALS
};

typedef struct
{
    GObjectClass parent_class;

} AlteratorCtlManagerModuleClass;

typedef struct
{
    GObject parnet_instance;

    GHashTable *commands;

    AlteratorGDBusSource *gdbus_system_bus_source;

    AlteratorGDBusSource *gdbus_session_bus_source;

    AlteratorCtlApp *alterator_ctl_app;

} AlteratorCtlManagerModule;

alterator_ctl_module_t *get_manager_module();

int manager_module_run_with_args(gpointer self, int argc, char **argv);

int manager_module_run(gpointer self, gpointer data);

int manager_module_print_help(gpointer self);

#endif //ALTERATORCTL_MANAGERMODULE_H
