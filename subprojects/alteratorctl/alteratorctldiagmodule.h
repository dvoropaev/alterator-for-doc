#ifndef ALTERATORCTL_DIAGMODULE_H
#define ALTERATORCTL_DIAGMODULE_H

#include <gio/gio.h>
#include <glib-object.h>

#include "alteratorctlapp.h"
#include "alteratorctlcommon.h"
#include "alteratorctlgdbussource.h"

#define TYPE_ALTERATOR_CTL_DIAG_MODULE (alterator_ctl_diag_module_get_type())
#define ALTERATOR_CTL_DIAG_MODULE(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_ALTERATOR_CTL_DIAG_MODULE, AlteratorCtlDiagModule))
#define IS_ALTERATOR_CTL_DIAG_MODULE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_ALTERATOR_CTL_DIAG_MODULE))
#define ALTERATOR_CTL_DIAG_MODULE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_ALTERATOR_CTL_DIAG_MODULE, AlteratorCtlDiagModuleClass))
#define IS_ALTERATOR_CTL_DIAG_MODULE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_ALTERATOR_CTL_DIAG_MODULE))
#define ALTERATOR_CTL_DIAG_MODULE_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_ALTERATOR_CTL_DIAG_MODULE, AlteratorCtlDiagModuleClass))

enum diag_sub_commands
{
    DIAG_INFO,
    DIAG_LIST,
    DIAG_LIST_ALL_PRIV,
    DIAG_LIST_TESTS_PRIV,
    DIAG_REPORT,
    DIAG_RUN,
    DIAG_RUN_TOOL_TEST_PRIV,
    DIAG_RUN_TOOL_PRIV,
    DIAG_HELP
};

typedef struct
{
    GObjectClass parent_class;

} AlteratorCtlDiagModuleClass;

typedef struct
{
    GObject parnet_instance;

    GHashTable *commands;

    AlteratorGDBusSource *gdbus_system_source;

    AlteratorGDBusSource *gdbus_session_source;

    AlteratorCtlApp *alterator_ctl_app;

    int exit_code;

} AlteratorCtlDiagModule;

alterator_ctl_module_t *get_diag_module();

int diag_module_run_with_args(gpointer self, int argc, char **argv);

int diag_module_run(gpointer self, gpointer data);

int diag_module_print_help(gpointer self);

#endif //ALTERATOR_CTL_DIAGMODULE_H
