#ifndef MODULEINTERFACE_H
#define MODULEINTERFACE_H

#include <glib-object.h>
#include <glib.h>
#include <glib/gi18n.h>

#define TYPE_ALTERATOR_CTL_MODULE (alterator_ctl_module_get_type())

#define ALTERATOR_CTL_MODULE(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_ALTERATOR_CTL_MODULE, AlteratorCtlModuleInterface))

#define IS_ALTERATOR_CTL_MODULE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_ALTERATOR_CTL_MODULE))

#define ALTERATOR_CTL_MODULE_INTERFACE(klass) \
    ((AlteratorModuleInterface *) g_type_interface_peek((klass), TYPE_ALTERATOR_CTL_MODULE))
#define GET_ALTERATOR_CTL_MODULE_INTERFACE(obj) \
    (G_TYPE_INSTANCE_GET_INTERFACE((obj), TYPE_ALTERATOR_CTL_MODULE, AlteratorCtlModuleInterface))

typedef struct AlteratorCtlModuleInterface
{
    GTypeInterface parent_iface;

    int (*run_with_args)(gpointer module, int argc, char **argv);
    int (*run)(gpointer self, gpointer data);
    int (*print_help)(gpointer self);
    int (*get_command_id)(GHashTable *table, char *subcommand);

} AlteratorCtlModuleInterface;

GType alterator_ctl_module_get_type(void);

#endif // MODULEINTERFACE_H
