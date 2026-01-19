#ifndef ALTERATORCTL_EDITIONS_H
#define ALTERATORCTL_EDITIONS_H

#include <gio/gio.h>
#include <glib-object.h>

#include "alteratorctlapp.h"
#include "alteratorctlcommon.h"
#include "alteratorctlgdbussource.h"

#define TYPE_ALTERATOR_CTL_EDITIONS_MODULE (alterator_ctl_editions_module_get_type())
#define ALTERATOR_CTL_EDITIONS_MODULE(obj)                                 \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_ALTERATOR_CTL_EDITIONS_MODULE, \
                                AlteratorCtlEditionsModule))
#define IS_ALTERATOR_CTL_EDITIONS_MODULE(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_ALTERATOR_CTL_EDITIONS_MODULE))
#define ALTERATOR_CTL_EDITIONS_MODULE_CLASS(klass)                        \
    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_ALTERATOR_CTL_EDITIONS_MODULE, \
                             AlteratorCtlEditionsModuleClass))
#define IS_ALTERATOR_CTL_EDITIONS_MODULE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_ALTERATOR_CTL_EDITIONS_MODULE))
#define ALTERATOR_CTL_EDITIONS_MODULE_GET_CLASS(obj)                      \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_ALTERATOR_CTL_EDITIONS_MODULE, \
                               AlteratorCtlEditionsModuleClass))

enum editions_sub_commands
{
    EDITIONS_LIST,
    EDITIONS_LIST_PRIV,
    EDITIONS_INFO,
    EDITIONS_SET,
    EDITIONS_GET,
    EDITIONS_DESCRIPTION,
    EDITIONS_LICENSE
};

typedef struct
{
    GObjectClass parent_class;

} AlteratorCtlEditionsModuleClass;

typedef struct
{
    GObject parnet_instance;

    GHashTable* commands;

    AlteratorGDBusSource* gdbus_source;

    AlteratorCtlApp* alterator_ctl_app;

} AlteratorCtlEditionsModule;

alterator_ctl_module_t* get_editions_module();

int editions_module_run_with_args(gpointer self, int argc, char** argv);

int editions_module_run(gpointer self, gpointer data);

int editions_module_print_help(gpointer self);

#endif // ALTERATORCTL_EDITIONS_H
