#ifndef ALTERATORCTL_SYSTEM_INFO_H
#define ALTERATORCTL_SYSTEM_INFO_H

#include <gio/gio.h>
#include <glib-object.h>

#include "alteratorctlapp.h"
#include "alteratorctlcommon.h"
#include "alteratorctlgdbussource.h"

#define TYPE_ALTERATOR_CTL_SYSTEMINFO_MODULE (alterator_ctl_systeminfo_module_get_type())
#define ALTERATOR_CTL_SYSTEMINFO_MODULE(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_ALTERATOR_CTL_SYSTEMINFO_MODULE, AlteratorCtlSystemInfoModule))
#define IS_ALTERATOR_CTL_SYSTEMINFO_MODULE(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_ALTERATOR_CTL_SYSTEMINFO_MODULE))
#define ALTERATOR_CTL_SYSTEMINFO_MODULE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_ALTERATOR_CTL_SYSTEMINFO_MODULE, AlteratorCtlSystemInfoModuleClass))
#define IS_ALTERATOR_CTL_SYSTEMINFO_MODULE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_ALTERATOR_CTL_SYSTEMINFO_MODULE))
#define ALTERATOR_CTL_SYSTEMINFO_MODULE_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_ALTERATOR_CTL_SYSTEMINFO_MODULE, AlteratorCtlSystemInfoModuleClass))

enum systeminfo_sub_commands
{
    SYSTEMINFO_GET_DESCRIPTION,
    SYSTEMINFO_GET_HOST_NAME,
    SYSTEMINFO_GET_OPERATION_SYSTEM_NAME,
    SYSTEMINFO_GET_ARCH,
    SYSTEMINFO_GET_BRANCH,
    SYSTEMINFO_GET_KERNEL,
    SYSTEMINFO_GET_CPU,
    SYSTEMINFO_GET_GPU,
    SYSTEMINFO_GET_MEMORY,
    SYSTEMINFO_GET_DRIVE,
    SYSTEMINFO_GET_MONITOR,
    SYSTEMINFO_GET_MOTHERBOARD,
    SYSTEMINFO_GET_LOCALES,
    SYSTEMINFO_GET_DESKTOP_ENVIRONMENT
};

typedef struct
{
    GObjectClass parent_class;

} AlteratorCtlSystemInfoModuleClass;

typedef struct
{
    GObject parnet_instance;

    GHashTable *commands;

    AlteratorGDBusSource *gdbus_source;

    AlteratorCtlApp *alterator_ctl_app;

} AlteratorCtlSystemInfoModule;

alterator_ctl_module_t *get_systeminfo_module();

int systeminfo_module_run_with_args(gpointer self, int argc, char **argv);

int systeminfo_module_run(gpointer self, gpointer data);

int systeminfo_module_print_help(gpointer self);

#endif // ALTERATORCTL_SYSTEM_INFO_H
