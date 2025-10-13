#ifndef ALTERATORCTL_COMPONENTS_H
#define ALTERATORCTL_COMPONENTS_H

#include <gio/gio.h>
#include <glib-object.h>

#include "alteratorctlapp.h"
#include "alteratorctlcommon.h"
#include "alteratorctlgdbussource.h"

#define TYPE_ALTERATOR_CTL_COMPONENTS_MODULE (alterator_ctl_components_module_get_type())
#define ALTERATOR_CTL_COMPONENTS_MODULE(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_ALTERATOR_CTL_COMPONENTS_MODULE, AlteratorCtlComponentsModule))
#define IS_ALTERATOR_CTL_COMPONENTS_MODULE(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_ALTERATOR_CTL_COMPONENTS_MODULE))
#define ALTERATOR_CTL_COMPONENTS_MODULE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_ALTERATOR_CTL_COMPONENTS_MODULE, AlteratorCtlComponentsModuleClass))
#define IS_ALTERATOR_CTL_COMPONENTS_MODULE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_ALTERATOR_CTL_COMPONENTS_MODULE))
#define ALTERATOR_CTL_COMPONENTS_MODULE_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_ALTERATOR_CTL_COMPONENTS_MODULE, AlteratorCtlComponentsModuleClass))

enum components_sub_commands
{
    COMPONENTS_LIST,
    COMPONENTS_INFO,
    COMPONENTS_INSTALL,
    COMPONENTS_REMOVE,
    COMPONENTS_DESCRIPTION,
    COMPONENTS_SEARCH,
    COMPONENTS_STATUS
};

typedef struct
{
    GObjectClass parent_class;

} AlteratorCtlComponentsModuleClass;

typedef struct
{
    GObject parnet_instance;

    GHashTable *commands;

    AlteratorGDBusSource *gdbus_source;

    AlteratorCtlApp *alterator_ctl_app;

} AlteratorCtlComponentsModule;

alterator_ctl_module_t *get_components_module(void);

int components_module_run_with_args(gpointer self, int argc, char **argv);

int components_module_run(gpointer self, gpointer data);

int components_module_print_help(gpointer self);

#endif //ALTERATORCTL_COMPONENTS_H
