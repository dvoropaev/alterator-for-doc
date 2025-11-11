#ifndef ALTERATORCTL_SERVICES_H
#define ALTERATORCTL_SERVICES_H

#include <gio/gio.h>
#include <glib-object.h>

#include "alteratorctlapp.h"
#include "alteratorctlcommon.h"
#include "alteratorctlgdbussource.h"
#include "json-glib/json-glib.h"

#define TYPE_ALTERATOR_CTL_SERVICES_MODULE (alterator_ctl_services_module_get_type())
#define ALTERATOR_CTL_SERVICES_MODULE(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_ALTERATOR_CTL_SERVICES_MODULE, AlteratorCtlServicesModule))
#define IS_ALTERATOR_CTL_SERVICES_MODULE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_ALTERATOR_CTL_SERVICES_MODULE))
#define ALTERATOR_CTL_SERVICES_MODULE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_ALTERATOR_CTL_SERVICES_MODULE, AlteratorCtlServicesModuleClass))
#define IS_ALTERATOR_CTL_SERVICES_MODULE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_ALTERATOR_CTL_SERVICES_MODULE))
#define ALTERATOR_CTL_SERVICES_MODULE_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_ALTERATOR_CTL_SERVICES_MODULE, AlteratorCtlServicesModuleClass))

enum services_sub_commands
{
    SERVICES_BACKUP,
    SERVICES_CONFIGURE,
    SERVICES_DEPLOY,
    SERVICES_DIAGNOSE,
    SERVICES_INFO,
    SERVICES_RESOURCES,
    SERVICES_RESTORE,
    SERVICES_STATUS,
    SERVICES_START,
    SERVICES_STOP,
    SERVICES_UNDEPLOY,
    SERVICES_LIST,
    SERVICES_PLAY
};

typedef struct
{
    GObjectClass parent_class;

} AlteratorCtlServicesModuleClass;

typedef struct
{
    GObject parnet_instance;

    GHashTable *commands;

    AlteratorGDBusSource *gdbus_source;

    JsonNode *json;

    GNode *info;

    AlteratorCtlApp *alterator_ctl_app;

} AlteratorCtlServicesModule;

alterator_ctl_module_t *get_services_module();

int services_module_run_with_args(gpointer self, int argc, char **argv);

int services_module_run(gpointer self, gpointer data);

int services_module_print_help(gpointer self);

#endif //ALTERATORCTL_SERVICES_H
