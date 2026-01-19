#ifndef ALTERATORCTL_REMOTE_H
#define ALTERATORCTL_REMOTE_H

#include <gio/gio.h>
#include <glib-object.h>

#include "alteratorctlapp.h"
#include "alteratorctlcommon.h"
#include "alteratorctlgdbussource.h"

#define TYPE_ALTERATOR_CTL_REMOTE_MODULE (alterator_ctl_remote_module_get_type())
#define ALTERATOR_CTL_REMOTE_MODULE(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_ALTERATOR_CTL_REMOTE_MODULE, AlteratorCtlRemoteModule))
#define IS_ALTERATOR_CTL_REMOTE_MODULE(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_ALTERATOR_CTL_REMOTE_MODULE))
#define ALTERATOR_CTL_REMOTE_MODULE_CLASS(klass)                        \
    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_ALTERATOR_CTL_REMOTE_MODULE, \
                             AlteratorCtlRemoteModuleClass))
#define IS_ALTERATOR_CTL_REMOTE_MODULE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_ALTERATOR_CTL_REMOTE_MODULE))
#define ALTERATOR_CTL_REMOTE_MODULE_GET_CLASS(obj)                      \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_ALTERATOR_CTL_REMOTE_MODULE, \
                               AlteratorCtlRemoteModuleClass))

enum remote_sub_commands
{
    REMOTE_CONNECT,
    REMOTE_DISCONNECT,
    REMOTE_LIST
};

typedef struct
{
    GObjectClass parent_class;

} AlteratorCtlRemoteModuleClass;

typedef struct
{
    GObject parnet_instance;

    GHashTable* commands;

    AlteratorGDBusSource* gdbus_source;

    AlteratorCtlApp* alterator_ctl_app;

} AlteratorCtlRemoteModule;

alterator_ctl_module_t* get_remote_module();

int remote_module_run_with_args(gpointer self, int argc, char** argv);

int remote_module_run(gpointer self, gpointer data);

int remote_module_print_help(gpointer self);

#endif // ALTERATORCTL_REMOTE_H
