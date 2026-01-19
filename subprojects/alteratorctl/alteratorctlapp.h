#ifndef ALTERATORCTLAPP_H
#define ALTERATORCTLAPP_H

#define POLKIT_AGENT_I_KNOW_API_IS_SUBJECT_TO_CHANGE

#include "alteratorctlcommon.h"
#include "alteratorctlmoduleinterface.h"

#include <gio/gio.h>

#include <glib-object.h>
#include <glib/gi18n.h>

#include <polkit/polkit.h>
#include <polkitagent/polkitagent.h>

#define TYPE_ALTERATOR_CTL_APP (alterator_ctl_app_get_type())
#define ALTERATOR_CTL_APP(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_ALTERATOR_CTL_APP, AlteratorCtlApp))
#define IS_ALTERATOR_CTL_APP(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_ALTERATOR_CTL_APP))
#define ALTERATOR_CTL_APP_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_ALTERATOR_CTL_APP, AlteratorCtlClass))
#define IS_ALTERATOR_CTL_APP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_ALTERATOR_CTL_APP))
#define ALTERATOR_CTL_APP_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_ALTERATOR_CTL_APP, AlteratorCtlClass))

typedef struct
{
    GObjectClass parent_class;
} AlteratorCtlAppClass;

typedef struct
{
    GObject parnet_instance;

    GHashTable* modules;

    alteratorctl_arguments_t* arguments;

    PolkitAgentListener* local_polkit_agent;

    gpointer local_agent_handle;
} AlteratorCtlApp;

typedef struct
{
    void* module_instance;

    AlteratorCtlModuleInterface* module_iface;

    void (*free_module_func)(gpointer);
} AlteratorCtlModule;

AlteratorCtlApp* alterator_ctl_app_new(void);

void alterator_ctl_app_free(AlteratorCtlApp* alterator_ctl);

void alterator_ctl_register_module(AlteratorCtlApp* app, alterator_ctl_module_t* module);

int alterator_ctl_app_run(AlteratorCtlApp* app, int argc, char** argv);

int alterator_ctl_get_registered_module(AlteratorCtlApp* app, gchar* module_name,
                                        AlteratorCtlModule** module);

#endif
