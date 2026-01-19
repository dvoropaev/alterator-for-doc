#include "alteratorctlmoduleinterface.h"

static int alterator_ctl_module_run_with_args(gpointer self, int argc, char** argv)
{
    AlteratorCtlModuleInterface* iface;

    g_return_val_if_fail(IS_ALTERATOR_CTL_MODULE(self), -1);

    iface = GET_ALTERATOR_CTL_MODULE_INTERFACE(self);
    g_return_val_if_fail(iface->run_with_args != NULL, -1);
    iface->run_with_args(self, argc, argv);

    return 0;
}

static int alterator_ctl_module_run(gpointer self, gpointer data)
{
    AlteratorCtlModuleInterface* iface;

    g_return_val_if_fail(IS_ALTERATOR_CTL_MODULE(self), -1);
    g_return_val_if_fail(data != NULL, -1);

    iface = GET_ALTERATOR_CTL_MODULE_INTERFACE(self);
    g_return_val_if_fail(iface->run != NULL, -1);
    iface->run(self, data);

    return 0;
}

static int alterator_ctl_module_print_help(gpointer self)
{
    AlteratorCtlModuleInterface* iface;

    g_return_val_if_fail(IS_ALTERATOR_CTL_MODULE(self), -1);

    iface = GET_ALTERATOR_CTL_MODULE_INTERFACE(self);
    g_return_val_if_fail(iface->print_help != NULL, -1);
    iface->print_help(self);

    return 0;
}

static int get_command_id(GHashTable* table, char* subcommand)
{
    if (!table || !subcommand)
        return -1;

    if (!g_hash_table_contains(table, subcommand))
        return -1;

    return *(int*) g_hash_table_lookup(table, subcommand);
}

static void alterator_module_interface_init(gpointer iface, gpointer data)
{
    AlteratorCtlModuleInterface* interface = (AlteratorCtlModuleInterface*) iface;

    interface->get_command_id = get_command_id;

    interface->run_with_args = alterator_ctl_module_run_with_args;

    interface->print_help = alterator_ctl_module_print_help;

    interface->run = alterator_ctl_module_run;

    /* add properties and signals here, will only be called once */
}

void alterator_module_interface_finalize(gpointer* iface, gpointer data) {}

GType alterator_ctl_module_get_type(void)
{
    static gsize alterator_module_interface = 0;
    if (g_once_init_enter(&alterator_module_interface))
    {
        const GTypeInfo alterator_module_interface_info = {
            sizeof(AlteratorCtlModuleInterface),
            NULL,                                         /* base_init */
            NULL,                                         /* base_finalize */
            alterator_module_interface_init,              /* class_init */
            NULL /*alterator_module_interface_finalize*/, /* class_finalize */
            NULL,                                         /* class_data */
            0,                                            /* instance_size */
            0,                                            /* n_preallocs */
            NULL                                          /* instance_init */
        };
        GType alterator_module_interface_type = g_type_register_static(
            G_TYPE_INTERFACE, "AlteratorModuleInterface", &alterator_module_interface_info, 0);

        g_once_init_leave(&alterator_module_interface, alterator_module_interface_type);
    }

    return alterator_module_interface;
}
