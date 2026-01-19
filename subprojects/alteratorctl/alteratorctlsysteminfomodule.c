#include "alteratorctlsysteminfomodule.h"

#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>

#define SYSTEMINFO_INTERFACE_NAME "org.altlinux.alterator.systeminfo1"
#define SYSTEMINFO_GET_LICENSE_METHOD_NAME "GetLicense"
#define SYSTEMINFO_GET_HOST_NAME_METHOD_NAME "GetHostName"
#define SYSTEMINFO_GET_SYSTEM_NAME_METHOD_NAME "GetOperationSystemName"
#define SYSTEMINFO_GET_SYSTEM_ARCH_METHOD_NAME "GetArch"
#define SYSTEMINFO_GET_BRANCH_METHOD_NAME "GetBranch"
#define SYSTEMINFO_GET_KERNEL_METHOD_NAME "GetKernel"
#define SYSTEMINFO_GET_CPU_METHOD_NAME "GetCPU"
#define SYSTEMINFO_GET_GPU_METHOD_NAME "GetGPU"
#define SYSTEMINFO_GET_MEMORY_METHOD_NAME "GetMemory"
#define SYSTEMINFO_GET_DRIVE_METHOD_NAME "GetDrive"
#define SYSTEMINFO_GET_MONITOR_METHOD_NAME "GetMonitor"
#define SYSTEMINFO_GET_MOTHERBOARD_METHOD_NAME "GetMotherboard"
#define SYSTEMINFO_GET_LOCALES_METHOD_NAME "GetLocale"
#define SYSTEMINFO_GET_DESKTOP_ENVIRONMENTS_METHOD_NAME "ListDesktopEnvironments"

#define SYSTEMINFO_DESCRIPRION_HOST_NAME_KEY "Host:"
#define SYSTEMINFO_DESCRIPRION_OS_NAME_KEY "Name:"
#define SYSTEMINFO_DESCRIPRION_ARCH_KEY "Arch:"
#define SYSTEMINFO_DESCRIPRION_BRANCH_KEY "Branch:"
#define SYSTEMINFO_DESCRIPRION_KERNEL_KEY "Kernel:"
#define SYSTEMINFO_DESCRIPRION_CPU_KEY "CPU:"
#define SYSTEMINFO_DESCRIPRION_GPU_KEY "GPU:"
#define SYSTEMINFO_DESCRIPRION_MEMORY_KEY "Memory:"
#define SYSTEMINFO_DESCRIPRION_DRIVE_KEY "Drive:"
#define SYSTEMINFO_DESCRIPRION_MONITOR_KEY "Monitor:"
#define SYSTEMINFO_DESCRIPRION_MOTHERBOARD_KEY "Motherboard:"
#define SYSTEMINFO_DESCRIPRION_LOCALES_KEY "Locales:"
#define SYSTEMINFO_DESCRIPRION_DESKTOPS_ENVIRONMENTS_KEY "Desktop environments:"

typedef struct systeminfo_module_subcommands_t
{
    char* subcommand;
    enum systeminfo_sub_commands id;
} systeminfo_module_subcommands_t;

static systeminfo_module_subcommands_t systeminfo_module_subcommands_list[] =
    {{"description", SYSTEMINFO_GET_DESCRIPTION},
     {"hostname", SYSTEMINFO_GET_HOST_NAME},
     {"name", SYSTEMINFO_GET_OPERATION_SYSTEM_NAME},
     {"arch", SYSTEMINFO_GET_ARCH},
     {"branch", SYSTEMINFO_GET_BRANCH},
     {"kernel", SYSTEMINFO_GET_KERNEL},
     {"cpu", SYSTEMINFO_GET_CPU},
     {"gpu", SYSTEMINFO_GET_GPU},
     {"memory", SYSTEMINFO_GET_MEMORY},
     {"drive", SYSTEMINFO_GET_DRIVE},
     {"monitor", SYSTEMINFO_GET_MONITOR},
     {"motherboard", SYSTEMINFO_GET_MOTHERBOARD},
     {"locales", SYSTEMINFO_GET_LOCALES},
     {"desktops", SYSTEMINFO_GET_DESKTOP_ENVIRONMENT}};

static GObjectClass* systeminfo_module_parent_class = NULL;
static alterator_ctl_module_t systeminfo_module     = {0};
static gboolean is_dbus_call_error                  = FALSE;

static void systeminfo_module_class_init(AlteratorCtlSystemInfoModuleClass* klass);
static void systeminfo_ctl_class_finalize(GObject* klass);

static void systeminfo_module_alterator_interface_init(gpointer iface, gpointer iface_data);
static void systeminfo_module_alterator_interface_finalize(gpointer iface, gpointer iface_data);

AlteratorCtlSystemInfoModule* systeminfo_module_new(gpointer app);
void systeminfo_module_free(AlteratorCtlSystemInfoModule* module);

static void fill_command_hash_table(GHashTable* command);

static int systeminfo_module_parse_arguments(AlteratorCtlSystemInfoModule* module, int argc,
                                             char** argv, alteratorctl_ctx_t** ctx);
static int systeminfo_module_get_description_subcommand(AlteratorCtlSystemInfoModule* module,
                                                        alteratorctl_ctx_t** ctx);
static int systeminfo_module_get_hostname_subcommand(AlteratorCtlSystemInfoModule* module,
                                                     alteratorctl_ctx_t** ctx);
static int
systeminfo_module_get_operation_system_name_subcommand(AlteratorCtlSystemInfoModule* module,
                                                       alteratorctl_ctx_t** ctx);
static int systeminfo_module_get_arch_subcommand(AlteratorCtlSystemInfoModule* module,
                                                 alteratorctl_ctx_t** ctx);
static int systeminfo_module_get_branch_subcommand(AlteratorCtlSystemInfoModule* module,
                                                   alteratorctl_ctx_t** ctx);
static int systeminfo_module_get_kernel_subcommand(AlteratorCtlSystemInfoModule* module,
                                                   alteratorctl_ctx_t** ctx);
static int systeminfo_module_get_cpu_subcommand(AlteratorCtlSystemInfoModule* module,
                                                alteratorctl_ctx_t** ctx);
static int systeminfo_module_get_gpu_subcommand(AlteratorCtlSystemInfoModule* module,
                                                alteratorctl_ctx_t** ctx);
static int systeminfo_module_get_memory_subcommand(AlteratorCtlSystemInfoModule* module,
                                                   alteratorctl_ctx_t** ctx);
static int systeminfo_module_get_drive_subcommand(AlteratorCtlSystemInfoModule* module,
                                                  alteratorctl_ctx_t** ctx);
static int systeminfo_module_get_monitor_subcommand(AlteratorCtlSystemInfoModule* module,
                                                    alteratorctl_ctx_t** ctx);
static int systeminfo_module_get_motherboard_subcommand(AlteratorCtlSystemInfoModule* module,
                                                        alteratorctl_ctx_t** ctx);
static int systeminfo_module_get_locales_subcommand(AlteratorCtlSystemInfoModule* module,
                                                    alteratorctl_ctx_t** ctx);
static int systeminfo_module_get_desktops_subcommand(AlteratorCtlSystemInfoModule* module,
                                                     alteratorctl_ctx_t** ctx);

static int systeminfo_module_handle_results(AlteratorCtlSystemInfoModule* module,
                                            alteratorctl_ctx_t** ctx);
static int systeminfo_module_get_license_handle_result(AlteratorCtlSystemInfoModule* module,
                                                       alteratorctl_ctx_t** ctx);
static int systeminfo_module_get_description_handle_result(AlteratorCtlSystemInfoModule* module,
                                                           alteratorctl_ctx_t** ctx);
static int systeminfo_module_get_hostname_handle_result(AlteratorCtlSystemInfoModule* module,
                                                        alteratorctl_ctx_t** ctx,
                                                        GPtrArray** result_str);
static int systeminfo_module_get_operation_system_name_handle_result(
    AlteratorCtlSystemInfoModule* module, alteratorctl_ctx_t** ctx, GPtrArray** result_str);
static int systeminfo_module_get_arch_handle_result(AlteratorCtlSystemInfoModule* module,
                                                    alteratorctl_ctx_t** ctx,
                                                    GPtrArray** result_str);
static int systeminfo_module_get_branch_handle_result(AlteratorCtlSystemInfoModule* module,
                                                      alteratorctl_ctx_t** ctx,
                                                      GPtrArray** result_str);
static int systeminfo_module_get_kernel_handle_result(AlteratorCtlSystemInfoModule* module,
                                                      alteratorctl_ctx_t** ctx,
                                                      GPtrArray** result_str);
static int systeminfo_module_get_cpu_handle_result(AlteratorCtlSystemInfoModule* module,
                                                   alteratorctl_ctx_t** ctx,
                                                   GPtrArray** result_str);
static int systeminfo_module_get_gpu_handle_result(AlteratorCtlSystemInfoModule* module,
                                                   alteratorctl_ctx_t** ctx,
                                                   GPtrArray** result_str);
static int systeminfo_module_get_memory_handle_result(AlteratorCtlSystemInfoModule* module,
                                                      alteratorctl_ctx_t** ctx,
                                                      GPtrArray** result_str);
static int systeminfo_module_get_drive_handle_result(AlteratorCtlSystemInfoModule* module,
                                                     alteratorctl_ctx_t** ctx,
                                                     GPtrArray** result_str);
static int systeminfo_module_get_monitor_handle_result(AlteratorCtlSystemInfoModule* module,
                                                       alteratorctl_ctx_t** ctx,
                                                       GPtrArray** result_str);
static int systeminfo_module_get_motherboard_handle_result(AlteratorCtlSystemInfoModule* module,
                                                           alteratorctl_ctx_t** ctx,
                                                           GPtrArray** result_str);
static int systeminfo_module_get_locales_handle_result(AlteratorCtlSystemInfoModule* module,
                                                       alteratorctl_ctx_t** ctx,
                                                       GPtrArray** result_str);
static int systeminfo_module_get_desktops_handle_result(AlteratorCtlSystemInfoModule* module,
                                                        alteratorctl_ctx_t** ctx,
                                                        GPtrArray** result_str);

static gint systeminfo_module_sort_result(gconstpointer a, gconstpointer b);

GType alterator_ctl_systeminfo_module_get_type(void)
{
    static GType systeminfo_module_type = 0;

    if (!systeminfo_module_type)
    {
        static const GTypeInfo systeminfo_module_info =
            {sizeof(AlteratorCtlSystemInfoModuleClass),     /* class structure size */
             NULL,                                          /* base class initializer */
             NULL,                                          /* base class finalizer */
             (GClassInitFunc) systeminfo_module_class_init, /* class initializer */
             NULL,                                          /* class finalizer */
             NULL,                                          /* class data */
             sizeof(AlteratorCtlSystemInfoModule),          /* instance structure size */
             1,                                             /* preallocated instances */
             NULL,                                          /* instance initializers */
             NULL};

        const GInterfaceInfo alterator_module_interface_info = {
            (GInterfaceInitFunc) systeminfo_module_alterator_interface_init, /* interface_init */
            (GInterfaceFinalizeFunc)
                systeminfo_module_alterator_interface_finalize, /* interface_finalize */
            NULL                                                /* interface_data */
        };

        systeminfo_module_type = g_type_register_static(G_TYPE_OBJECT, /* parent class */
                                                        "AlteratorCtlSystemInfoModule",
                                                        &systeminfo_module_info, 0);

        g_type_add_interface_static(systeminfo_module_type, TYPE_ALTERATOR_CTL_MODULE,
                                    &alterator_module_interface_info);
    }

    return systeminfo_module_type;
}

static void systeminfo_module_class_init(AlteratorCtlSystemInfoModuleClass* klass)
{
    GObjectClass* obj_class = G_OBJECT_CLASS(klass);

    obj_class->finalize = systeminfo_ctl_class_finalize;

    systeminfo_module_parent_class = g_type_class_peek_parent(klass);
}

static void systeminfo_ctl_class_finalize(GObject* klass)
{
    AlteratorCtlSystemInfoModuleClass* obj = (AlteratorCtlSystemInfoModuleClass*) klass;

    G_OBJECT_CLASS(systeminfo_module_parent_class)->finalize(klass);
}

static void systeminfo_module_alterator_interface_init(gpointer iface, gpointer iface_data)
{
    AlteratorCtlModuleInterface* interface = iface;

    interface->run_with_args = systeminfo_module_run_with_args;

    interface->run = systeminfo_module_run;

    interface->print_help = systeminfo_module_print_help;
}

static void systeminfo_module_alterator_interface_finalize(gpointer iface, gpointer iface_data) {}

alterator_ctl_module_t* get_systeminfo_module()
{
    int ret                                 = 0;
    static gsize systeminfo_ctl_module_init = 0;
    if (g_once_init_enter(&systeminfo_ctl_module_init))
    {
        gsize module_id_size = g_strlcpy(systeminfo_module.id, ALTERATOR_CTL_SYSTEMINFO_MODULE_NAME,
                                         strlen(ALTERATOR_CTL_SYSTEMINFO_MODULE_NAME) + 1);

        if (module_id_size != strlen(ALTERATOR_CTL_SYSTEMINFO_MODULE_NAME))
        {
            g_printerr(_(
                "Internal error in get_systeminfo_module: unvaliable id of systeminfo module.\n"));
            ERR_EXIT();
        }

        systeminfo_module.new_object_func  = (gpointer) systeminfo_module_new;
        systeminfo_module.free_object_func = (gpointer) systeminfo_module_free;

        gsize tmp = 42;

        g_once_init_leave(&systeminfo_ctl_module_init, tmp);
    }

    return &systeminfo_module;

end:
    return NULL;
}

AlteratorCtlSystemInfoModule* systeminfo_module_new(gpointer app)
{
    AlteratorCtlSystemInfoModule* object = g_object_new(TYPE_ALTERATOR_CTL_SYSTEMINFO_MODULE, NULL);

    object->commands = g_hash_table_new(g_str_hash, g_str_equal);
    fill_command_hash_table(object->commands);

    object->alterator_ctl_app = (AlteratorCtlApp*) app;

    object->gdbus_source = alterator_gdbus_source_new(object->alterator_ctl_app->arguments->verbose,
                                                      G_BUS_TYPE_SYSTEM);

    return object;
}

void systeminfo_module_free(AlteratorCtlSystemInfoModule* module)
{
    g_hash_table_destroy(module->commands);

    if (module->gdbus_source)
    {
        alterator_gdbus_source_free(module->gdbus_source);
    }

    g_object_unref(module);
}

static void fill_command_hash_table(GHashTable* command)
{
    for (int i = 0;
         i < sizeof(systeminfo_module_subcommands_list) / sizeof(systeminfo_module_subcommands_t);
         i++)
        g_hash_table_insert(command, systeminfo_module_subcommands_list[i].subcommand,
                            &systeminfo_module_subcommands_list[i].id);
}

static int systeminfo_module_parse_arguments(AlteratorCtlSystemInfoModule* module, int argc,
                                             char** argv, alteratorctl_ctx_t** ctx)
{
    int ret                            = 0;
    AlteratorCtlModuleInterface* iface = GET_ALTERATOR_CTL_MODULE_INTERFACE((void*) module);

    if (!iface)
    {
        g_printerr(
            _("Internal error in systeminfo module while parsing arguments: *iface is NULL.\n"));
        ERR_EXIT();
    }

    if (module->alterator_ctl_app->arguments->module_help)
    {
        iface->print_help(module);
        goto end;
    }

    if (argc > 3)
    {
        g_printerr(_("Wrong arguments in systeminfo module\n"));
        iface->print_help(module);
        ERR_EXIT();
    }

    // -1 - default command
    int selected_subcommand = iface->get_command_id(module->commands, argv[2]);
    switch (selected_subcommand)
    {
    case SYSTEMINFO_GET_DESCRIPTION:
        (*ctx) = alteratorctl_ctx_init_systeminfo(SYSTEMINFO_GET_DESCRIPTION, NULL, NULL);
        break;

    case SYSTEMINFO_GET_HOST_NAME:
        (*ctx) = alteratorctl_ctx_init_systeminfo(SYSTEMINFO_GET_HOST_NAME, NULL, NULL);
        break;

    case SYSTEMINFO_GET_OPERATION_SYSTEM_NAME:
        (*ctx) = alteratorctl_ctx_init_systeminfo(SYSTEMINFO_GET_OPERATION_SYSTEM_NAME, NULL, NULL);
        break;

    case SYSTEMINFO_GET_ARCH:
        (*ctx) = alteratorctl_ctx_init_systeminfo(SYSTEMINFO_GET_ARCH, NULL, NULL);
        break;

    case SYSTEMINFO_GET_BRANCH:
        (*ctx) = alteratorctl_ctx_init_systeminfo(SYSTEMINFO_GET_BRANCH, NULL, NULL);
        break;

    case SYSTEMINFO_GET_KERNEL:
        (*ctx) = alteratorctl_ctx_init_systeminfo(SYSTEMINFO_GET_KERNEL, NULL, NULL);
        break;

    case SYSTEMINFO_GET_CPU:
        (*ctx) = alteratorctl_ctx_init_systeminfo(SYSTEMINFO_GET_CPU, NULL, NULL);
        break;

    case SYSTEMINFO_GET_GPU:
        (*ctx) = alteratorctl_ctx_init_systeminfo(SYSTEMINFO_GET_GPU, NULL, NULL);
        break;

    case SYSTEMINFO_GET_MEMORY:
        (*ctx) = alteratorctl_ctx_init_systeminfo(SYSTEMINFO_GET_MEMORY, NULL, NULL);
        break;

    case SYSTEMINFO_GET_DRIVE:
        (*ctx) = alteratorctl_ctx_init_systeminfo(SYSTEMINFO_GET_DRIVE, NULL, NULL);
        break;

    case SYSTEMINFO_GET_MONITOR:
        (*ctx) = alteratorctl_ctx_init_systeminfo(SYSTEMINFO_GET_MONITOR, NULL, NULL);
        break;

    case SYSTEMINFO_GET_MOTHERBOARD:
        (*ctx) = alteratorctl_ctx_init_systeminfo(SYSTEMINFO_GET_MOTHERBOARD, NULL, NULL);
        break;

    case SYSTEMINFO_GET_LOCALES:
        (*ctx) = alteratorctl_ctx_init_systeminfo(SYSTEMINFO_GET_LOCALES, NULL, NULL);
        break;

    case SYSTEMINFO_GET_DESKTOP_ENVIRONMENT:
        (*ctx) = alteratorctl_ctx_init_systeminfo(SYSTEMINFO_GET_DESKTOP_ENVIRONMENT, NULL, NULL);
        break;

    default:
        if (argc == 2)
            (*ctx) = alteratorctl_ctx_init_systeminfo(SYSTEMINFO_GET_DESCRIPTION, NULL, NULL);
        else
        {
            g_printerr(_("Unknown module systeminfo command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;
    };

end:
    return ret;
}

int systeminfo_module_run_with_args(gpointer self, int argc, char** argv)
{
    int ret                              = 0;
    alteratorctl_ctx_t* ctx              = NULL;
    AlteratorCtlSystemInfoModule* module = ALTERATOR_CTL_SYSTEMINFO_MODULE(self);

    if (!module)
    {
        g_printerr(_("Internal data error in systeminfo module with args: "
                     "AlteratorCtlSystemInfoModule *module is NULL.\n"));
        ERR_EXIT();
    }

    if (systeminfo_module_parse_arguments(module, argc, argv, &ctx) < 0)
        ERR_EXIT();

    if (systeminfo_module_run(module, ctx) < 0)
        ERR_EXIT();

    if (!is_dbus_call_error && (systeminfo_module_handle_results(module, &ctx) < 0))
        ERR_EXIT();

end:
    if (ctx)
        alteratorctl_ctx_free(ctx);
    return ret;
}

int systeminfo_module_run(gpointer self, gpointer data)
{
    int ret                              = 0;
    AlteratorCtlModuleInterface* iface   = GET_ALTERATOR_CTL_MODULE_INTERFACE(self);
    AlteratorCtlSystemInfoModule* module = ALTERATOR_CTL_SYSTEMINFO_MODULE(self);

    if (module->alterator_ctl_app->arguments->module_help)
        goto end;

    if (!self || !data)
    {
        g_printerr(_("Internal error in systeminfo module run: *module or *context is NULL.\n"));
        ERR_EXIT();
    }

    alteratorctl_ctx_t* ctx = (alteratorctl_ctx_t*) data;

    switch (g_variant_get_int32(ctx->subcommands_ids))
    {
    case SYSTEMINFO_GET_DESCRIPTION:
        if (systeminfo_module_get_description_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_HOST_NAME:
        if (systeminfo_module_get_hostname_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_OPERATION_SYSTEM_NAME:
        if (systeminfo_module_get_operation_system_name_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_ARCH:
        if (systeminfo_module_get_arch_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_BRANCH:
        if (systeminfo_module_get_branch_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_KERNEL:
        if (systeminfo_module_get_kernel_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_CPU:
        if (systeminfo_module_get_cpu_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_GPU:
        if (systeminfo_module_get_gpu_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_MEMORY:
        if (systeminfo_module_get_memory_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_DRIVE:
        if (systeminfo_module_get_drive_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_MONITOR:
        if (systeminfo_module_get_monitor_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_MOTHERBOARD:
        if (systeminfo_module_get_motherboard_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_LOCALES:
        if (systeminfo_module_get_locales_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_DESKTOP_ENVIRONMENT:
        if (systeminfo_module_get_desktops_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    default:
        g_printerr(_("Try to run unknown systeminfo subcommand.\n"));
        ERR_EXIT();
    }

end:
    return ret;
}

static int systeminfo_module_get_license_subcommand(AlteratorCtlSystemInfoModule* module,
                                                    alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    dbus_ctx_t* d_ctx       = NULL;
    GError* dbus_call_error = NULL;
    gchar* locale           = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in systeminfo module - AlteratorCtlSystemInfoModule *module "
                     "is NULL in \"systeminfo "
                     "license\".\n"));
        ERR_EXIT();
    }

    if (!(locale = alterator_ctl_get_effective_locale()))
        ERR_EXIT();

    if (module->gdbus_source->alterator_gdbus_source_set_env_value(module->gdbus_source, "LC_ALL",
                                                                   locale)
        < 0)
        ERR_EXIT();

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, ALTERATOR_SYSTEMINFO_PATH,
                          SYSTEMINFO_INTERFACE_NAME, SYSTEMINFO_GET_LICENSE_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in systeminfo_module_get_license_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(ayi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Object %s and/or interface %s doesn't "
                     "exists.\n"),
                   ALTERATOR_SYSTEMINFO_PATH, SYSTEMINFO_INTERFACE_NAME);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    if (d_ctx->result)
        (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);
    else
    {
        g_printerr(_("D-Bus error in systeminfo module while calling GetLicense(): failed to "
                     "produce a result\n"));
        ERR_EXIT();
    }

end:
    g_clear_error(&dbus_call_error);

    if (d_ctx)
        dbus_ctx_free(d_ctx);

    g_free(locale);

    return ret;
}

static int systeminfo_module_get_description_subcommand(AlteratorCtlSystemInfoModule* module,
                                                        alteratorctl_ctx_t** ctx)
{
    int ret                  = 0;
    GVariant* result         = NULL;
    GVariantBuilder* builder = NULL;

    (*ctx)->results      = (gpointer) g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
                                                            (GDestroyNotify) g_variant_unref);
    (*ctx)->free_results = (gpointer) g_hash_table_unref;

    if (systeminfo_module_get_hostname_subcommand(module, ctx) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_operation_system_name_subcommand(module, ctx) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_arch_subcommand(module, ctx) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_branch_subcommand(module, ctx) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_kernel_subcommand(module, ctx) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_cpu_subcommand(module, ctx) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_gpu_subcommand(module, ctx) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_memory_subcommand(module, ctx) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_drive_subcommand(module, ctx) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_monitor_subcommand(module, ctx) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_motherboard_subcommand(module, ctx) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_locales_subcommand(module, ctx) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_desktops_subcommand(module, ctx) < 0)
        ERR_EXIT();

    builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

    GHashTableIter iter;
    g_hash_table_iter_init(&iter, (GHashTable*) (*ctx)->results);
    gpointer key = NULL, value = NULL;
    while (g_hash_table_iter_next(&iter, &key, &value))
        g_variant_builder_add(builder, "{sv}", key, value);

    result = g_variant_builder_end(builder);
    if (!result)
    {
        g_printerr(
            _("Failed to save the result of describing all the information about the system.\n"));
        ERR_EXIT();
    }

    g_hash_table_unref((GHashTable*) (*ctx)->results);
    (*ctx)->results      = g_variant_ref(result);
    (*ctx)->free_results = (gpointer) g_variant_unref;

end:
    if (result)
        g_variant_unref(result);

    if (builder)
        g_variant_builder_unref(builder);

    return ret;
}

static int systeminfo_module_get_hostname_subcommand(AlteratorCtlSystemInfoModule* module,
                                                     alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    dbus_ctx_t* d_ctx       = NULL;
    GError* dbus_call_error = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in systeminfo module - AlteratorCtlSystemInfoModule *module "
                     "is NULL in \"systeminfo "
                     "device\".\n"));
        ERR_EXIT();
    }

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, ALTERATOR_SYSTEMINFO_PATH,
                          SYSTEMINFO_INTERFACE_NAME, SYSTEMINFO_GET_HOST_NAME_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in systeminfo_module_get_device_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Object %s and/or interface %s doesn't "
                     "exists.\n"),
                   ALTERATOR_SYSTEMINFO_PATH, SYSTEMINFO_INTERFACE_NAME);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    if (d_ctx->result)
        if ((*ctx)->results)
            g_hash_table_insert((GHashTable*) (*ctx)->results,
                                (gpointer) SYSTEMINFO_DESCRIPRION_HOST_NAME_KEY,
                                (gpointer) (g_variant_ref(d_ctx->result)));
        else
            (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);
    else
    {
        g_printerr(_("D-Bus error in systeminfo module while calling GetDeviceName(): failed to "
                     "produce a result.\n"));
        ERR_EXIT();
    }

end:
    g_clear_error(&dbus_call_error);

    if (d_ctx)
        dbus_ctx_free(d_ctx);

    return ret;
}

static int
systeminfo_module_get_operation_system_name_subcommand(AlteratorCtlSystemInfoModule* module,
                                                       alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    dbus_ctx_t* d_ctx       = NULL;
    GError* dbus_call_error = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in systeminfo module - AlteratorCtlSystemInfoModule *module "
                     "is NULL in \"systeminfo "
                     "name\".\n"));
        ERR_EXIT();
    }

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, ALTERATOR_SYSTEMINFO_PATH,
                          SYSTEMINFO_INTERFACE_NAME, SYSTEMINFO_GET_SYSTEM_NAME_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in "
                     "systeminfo_module_get_operation_system_name_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Object %s and/or interface %s doesn't "
                     "exists.\n"),
                   ALTERATOR_SYSTEMINFO_PATH, SYSTEMINFO_INTERFACE_NAME);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    if (d_ctx->result)
        if ((*ctx)->results)
            g_hash_table_insert((GHashTable*) (*ctx)->results,
                                (gpointer) SYSTEMINFO_DESCRIPRION_OS_NAME_KEY,
                                (gpointer) (g_variant_ref(d_ctx->result)));
        else
            (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);
    else
    {
        g_printerr(_("D-Bus error in systeminfo module while calling GetOperationSystemName(): "
                     "failed to produce a "
                     "result.\n"));
        ERR_EXIT();
    }

end:
    g_clear_error(&dbus_call_error);

    if (d_ctx)
        dbus_ctx_free(d_ctx);

    return ret;
}

static int systeminfo_module_get_arch_subcommand(AlteratorCtlSystemInfoModule* module,
                                                 alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    dbus_ctx_t* d_ctx       = NULL;
    GError* dbus_call_error = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in systeminfo module - AlteratorCtlSystemInfoModule *module "
                     "is NULL in \"systeminfo "
                     "arch\".\n"));
        ERR_EXIT();
    }

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, ALTERATOR_SYSTEMINFO_PATH,
                          SYSTEMINFO_INTERFACE_NAME, SYSTEMINFO_GET_SYSTEM_ARCH_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in systeminfo_module_get_arch_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Object %s and/or interface %s doesn't "
                     "exists.\n"),
                   ALTERATOR_SYSTEMINFO_PATH, SYSTEMINFO_INTERFACE_NAME);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    if (d_ctx->result)
        if ((*ctx)->results)
            g_hash_table_insert((GHashTable*) (*ctx)->results,
                                (gpointer) SYSTEMINFO_DESCRIPRION_ARCH_KEY,
                                (gpointer) (g_variant_ref(d_ctx->result)));
        else
            (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);
    else if (!(*ctx)->results) // Hide property if no value is received
    {
        g_printerr(_("D-Bus error in systeminfo module while calling Arch(): failed to produce a "
                     "result.\n"));
        ERR_EXIT();
    }

end:
    g_clear_error(&dbus_call_error);

    if (d_ctx)
        dbus_ctx_free(d_ctx);

    return ret;
}

static int systeminfo_module_get_branch_subcommand(AlteratorCtlSystemInfoModule* module,
                                                   alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    dbus_ctx_t* d_ctx       = NULL;
    GError* dbus_call_error = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in systeminfo module - AlteratorCtlSystemInfoModule *module "
                     "is NULL in \"systeminfo "
                     "branch\".\n"));
        ERR_EXIT();
    }

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, ALTERATOR_SYSTEMINFO_PATH,
                          SYSTEMINFO_INTERFACE_NAME, SYSTEMINFO_GET_BRANCH_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in systeminfo_module_get_branch_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Object %s and/or interface %s doesn't "
                     "exists.\n"),
                   ALTERATOR_SYSTEMINFO_PATH, SYSTEMINFO_INTERFACE_NAME);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    if (d_ctx->result)
        if ((*ctx)->results)
            g_hash_table_insert((GHashTable*) (*ctx)->results,
                                (gpointer) SYSTEMINFO_DESCRIPRION_BRANCH_KEY,
                                (gpointer) (g_variant_ref(d_ctx->result)));
        else
            (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);
    else if (!(*ctx)->results) // Hide property if no value is received
    {
        g_printerr(_("D-Bus error in systeminfo module while calling GetBranch(): failed to "
                     "produce a result.\n"));
        ERR_EXIT();
    }

end:
    g_clear_error(&dbus_call_error);

    if (d_ctx)
        dbus_ctx_free(d_ctx);

    return ret;
}

static int systeminfo_module_get_kernel_subcommand(AlteratorCtlSystemInfoModule* module,
                                                   alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    dbus_ctx_t* d_ctx       = NULL;
    GError* dbus_call_error = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in systeminfo module - AlteratorCtlSystemInfoModule *module "
                     "is NULL in \"systeminfo "
                     "kernel\".\n"));
        ERR_EXIT();
    }

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, ALTERATOR_SYSTEMINFO_PATH,
                          SYSTEMINFO_INTERFACE_NAME, SYSTEMINFO_GET_KERNEL_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in systeminfo_module_get_kernel_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Object %s and/or interface %s doesn't "
                     "exists.\n"),
                   ALTERATOR_SYSTEMINFO_PATH, SYSTEMINFO_INTERFACE_NAME);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    if (d_ctx->result)
        if ((*ctx)->results)
            g_hash_table_insert((GHashTable*) (*ctx)->results,
                                (gpointer) SYSTEMINFO_DESCRIPRION_KERNEL_KEY,
                                (gpointer) (g_variant_ref(d_ctx->result)));
        else
            (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);
    else if (!(*ctx)->results) // Hide property if no value is received
    {
        g_printerr(_("D-Bus error in systeminfo module while calling GetKernel(): failed to "
                     "produce a result.\n"));
        ERR_EXIT();
    }

end:
    g_clear_error(&dbus_call_error);

    if (d_ctx)
        dbus_ctx_free(d_ctx);

    return ret;
}

static int systeminfo_module_get_cpu_subcommand(AlteratorCtlSystemInfoModule* module,
                                                alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    dbus_ctx_t* d_ctx       = NULL;
    GError* dbus_call_error = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in systeminfo module - AlteratorCtlSystemInfoModule *module "
                     "is NULL in \"systeminfo "
                     "cpu\".\n"));
        ERR_EXIT();
    }
    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, ALTERATOR_SYSTEMINFO_PATH,
                          SYSTEMINFO_INTERFACE_NAME, SYSTEMINFO_GET_CPU_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in systeminfo_module_get_cpu_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Object %s and/or interface %s doesn't "
                     "exists.\n"),
                   ALTERATOR_SYSTEMINFO_PATH, SYSTEMINFO_INTERFACE_NAME);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    if (d_ctx->result)
        if ((*ctx)->results)
            g_hash_table_insert((GHashTable*) (*ctx)->results,
                                (gpointer) SYSTEMINFO_DESCRIPRION_CPU_KEY,
                                (gpointer) (g_variant_ref(d_ctx->result)));
        else
            (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);
    else if (!(*ctx)->results) // Hide property if no value is received
    {
        g_printerr(_("D-Bus error in systeminfo module while calling GetCPU(): failed to produce a "
                     "result.\n"));
        ERR_EXIT();
    }

end:
    g_clear_error(&dbus_call_error);

    if (d_ctx)
        dbus_ctx_free(d_ctx);

    return ret;
}

static int systeminfo_module_get_gpu_subcommand(AlteratorCtlSystemInfoModule* module,
                                                alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    dbus_ctx_t* d_ctx       = NULL;
    GError* dbus_call_error = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in systeminfo module - AlteratorCtlSystemInfoModule *module "
                     "is NULL in \"systeminfo "
                     "gpu\".\n"));
        ERR_EXIT();
    }

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, ALTERATOR_SYSTEMINFO_PATH,
                          SYSTEMINFO_INTERFACE_NAME, SYSTEMINFO_GET_GPU_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in systeminfo_module_get_gpu_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Object %s and/or interface %s doesn't "
                     "exists.\n"),
                   ALTERATOR_SYSTEMINFO_PATH, SYSTEMINFO_INTERFACE_NAME);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    if (d_ctx->result)
        if ((*ctx)->results)
            g_hash_table_insert((GHashTable*) (*ctx)->results,
                                (gpointer) SYSTEMINFO_DESCRIPRION_GPU_KEY,
                                (gpointer) (g_variant_ref(d_ctx->result)));
        else
            (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);
    else if (!(*ctx)->results) // Hide property if no value is received
    {
        g_printerr(_("D-Bus error in systeminfo module while calling GetGPU(): failed to produce a "
                     "result.\n"));
        ERR_EXIT();
    }

end:
    g_clear_error(&dbus_call_error);

    if (d_ctx)
        dbus_ctx_free(d_ctx);

    return ret;
}

static int systeminfo_module_get_memory_subcommand(AlteratorCtlSystemInfoModule* module,
                                                   alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    dbus_ctx_t* d_ctx       = NULL;
    GError* dbus_call_error = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in systeminfo module - AlteratorCtlSystemInfoModule *module "
                     "is NULL in \"systeminfo "
                     "memory\".\n"));
        ERR_EXIT();
    }

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, ALTERATOR_SYSTEMINFO_PATH,
                          SYSTEMINFO_INTERFACE_NAME, SYSTEMINFO_GET_MEMORY_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in systeminfo_module_get_memory_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Object %s and/or interface %s doesn't "
                     "exists.\n"),
                   ALTERATOR_SYSTEMINFO_PATH, SYSTEMINFO_INTERFACE_NAME);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    if (d_ctx->result)
        if ((*ctx)->results)
            g_hash_table_insert((GHashTable*) (*ctx)->results,
                                (gpointer) SYSTEMINFO_DESCRIPRION_MEMORY_KEY,
                                (gpointer) (g_variant_ref(d_ctx->result)));
        else
            (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);
    else if (!(*ctx)->results) // Hide property if no value is received
    {
        g_printerr(_("D-Bus error in systeminfo module while calling GetMemory(): failed to "
                     "produce a result.\n"));
        ERR_EXIT();
    }

end:
    g_clear_error(&dbus_call_error);

    if (d_ctx)
        dbus_ctx_free(d_ctx);

    return ret;
}

static int systeminfo_module_get_drive_subcommand(AlteratorCtlSystemInfoModule* module,
                                                  alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    dbus_ctx_t* d_ctx       = NULL;
    GError* dbus_call_error = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in systeminfo module - AlteratorCtlSystemInfoModule *module "
                     "is NULL in \"systeminfo "
                     "drive\".\n"));
        ERR_EXIT();
    }

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, ALTERATOR_SYSTEMINFO_PATH,
                          SYSTEMINFO_INTERFACE_NAME, SYSTEMINFO_GET_DRIVE_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in systeminfo_module_get_drive_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Object %s and/or interface %s doesn't "
                     "exists.\n"),
                   ALTERATOR_SYSTEMINFO_PATH, SYSTEMINFO_INTERFACE_NAME);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    if (d_ctx->result)
        if ((*ctx)->results)
            g_hash_table_insert((GHashTable*) (*ctx)->results,
                                (gpointer) SYSTEMINFO_DESCRIPRION_DRIVE_KEY,
                                (gpointer) (g_variant_ref(d_ctx->result)));
        else
            (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);
    else if (!(*ctx)->results) // Hide property if no value is received
    {
        g_printerr(_("D-Bus error in systeminfo module while calling GetDrive(): failed to produce "
                     "a result.\n"));
        ERR_EXIT();
    }

end:
    g_clear_error(&dbus_call_error);

    if (d_ctx)
        dbus_ctx_free(d_ctx);

    return ret;
}

static int systeminfo_module_get_monitor_subcommand(AlteratorCtlSystemInfoModule* module,
                                                    alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    dbus_ctx_t* d_ctx       = NULL;
    GError* dbus_call_error = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in systeminfo module - AlteratorCtlSystemInfoModule *module "
                     "is NULL in \"systeminfo "
                     "monitor\".\n"));
        ERR_EXIT();
    }

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, ALTERATOR_SYSTEMINFO_PATH,
                          SYSTEMINFO_INTERFACE_NAME, SYSTEMINFO_GET_MONITOR_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in systeminfo_module_get_monitor_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Object %s and/or interface %s doesn't "
                     "exists.\n"),
                   ALTERATOR_SYSTEMINFO_PATH, SYSTEMINFO_INTERFACE_NAME);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    if (d_ctx->result)
        if ((*ctx)->results)
            g_hash_table_insert((GHashTable*) (*ctx)->results,
                                (gpointer) SYSTEMINFO_DESCRIPRION_MONITOR_KEY,
                                (gpointer) (g_variant_ref(d_ctx->result)));
        else
            (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);
    else if (!(*ctx)->results) // Hide property if no value is received
    {
        g_printerr(_("D-Bus error in systeminfo module while calling GetMonitor(): failed to "
                     "produce a result.\n"));
        ERR_EXIT();
    }

end:
    g_clear_error(&dbus_call_error);

    if (d_ctx)
        dbus_ctx_free(d_ctx);

    return ret;
}

static int systeminfo_module_get_motherboard_subcommand(AlteratorCtlSystemInfoModule* module,
                                                        alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    dbus_ctx_t* d_ctx       = NULL;
    GError* dbus_call_error = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in systeminfo module - AlteratorCtlSystemInfoModule *module "
                     "is NULL in \"systeminfo "
                     "motherboard\".\n"));
        ERR_EXIT();
    }
    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, ALTERATOR_SYSTEMINFO_PATH,
                          SYSTEMINFO_INTERFACE_NAME, SYSTEMINFO_GET_MOTHERBOARD_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(
            _("Can't allocate dbus_ctx_t in systeminfo_module_get_motherboard_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Object %s and/or interface %s doesn't "
                     "exists.\n"),
                   ALTERATOR_SYSTEMINFO_PATH, SYSTEMINFO_INTERFACE_NAME);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    if (d_ctx->result)
        if ((*ctx)->results)
            g_hash_table_insert((GHashTable*) (*ctx)->results,
                                (gpointer) SYSTEMINFO_DESCRIPRION_MOTHERBOARD_KEY,
                                (gpointer) (g_variant_ref(d_ctx->result)));
        else
            (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);
    else if (!(*ctx)->results) // Hide property if no value is received
    {
        g_printerr(_("D-Bus error in systeminfo module while calling GetMotherboard(): failed to "
                     "produce a result.\n"));
        ERR_EXIT();
    }

end:
    g_clear_error(&dbus_call_error);

    if (d_ctx)
        dbus_ctx_free(d_ctx);

    return ret;
}

static int systeminfo_module_get_locales_subcommand(AlteratorCtlSystemInfoModule* module,
                                                    alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    dbus_ctx_t* d_ctx       = NULL;
    GError* dbus_call_error = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in systeminfo module - AlteratorCtlSystemInfoModule *module "
                     "is NULL in \"systeminfo "
                     "locales\".\n"));
        ERR_EXIT();
    }

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, ALTERATOR_SYSTEMINFO_PATH,
                          SYSTEMINFO_INTERFACE_NAME, SYSTEMINFO_GET_LOCALES_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in systeminfo_module_get_locales_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Object %s and/or interface %s doesn't "
                     "exists.\n"),
                   ALTERATOR_SYSTEMINFO_PATH, SYSTEMINFO_INTERFACE_NAME);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    if (d_ctx->result)
        if ((*ctx)->results)
            g_hash_table_insert((GHashTable*) (*ctx)->results,
                                (gpointer) SYSTEMINFO_DESCRIPRION_LOCALES_KEY,
                                (gpointer) (g_variant_ref(d_ctx->result)));
        else
            (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);
    else if (!(*ctx)->results) // Hide property if no value is received
    {
        g_printerr(_("D-Bus error in systeminfo module while calling GetLocales(): failed to "
                     "produce a result.\n"));
        ERR_EXIT();
    }

end:
    g_clear_error(&dbus_call_error);

    if (d_ctx)
        dbus_ctx_free(d_ctx);

    return ret;
}

static int systeminfo_module_get_desktops_subcommand(AlteratorCtlSystemInfoModule* module,
                                                     alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    dbus_ctx_t* d_ctx       = NULL;
    GError* dbus_call_error = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in systeminfo module - AlteratorCtlSystemInfoModule *module "
                     "is NULL in \"systeminfo "
                     "desktops\".\n"));
        ERR_EXIT();
    }

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, ALTERATOR_SYSTEMINFO_PATH,
                          SYSTEMINFO_INTERFACE_NAME,
                          SYSTEMINFO_GET_DESKTOP_ENVIRONMENTS_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in systeminfo_module_get_desktops_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Object %s and/or interface %s doesn't "
                     "exists.\n"),
                   ALTERATOR_SYSTEMINFO_PATH, SYSTEMINFO_INTERFACE_NAME);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    if (d_ctx->result)
        if ((*ctx)->results)
            g_hash_table_insert((GHashTable*) (*ctx)->results,
                                (gpointer) SYSTEMINFO_DESCRIPRION_DESKTOPS_ENVIRONMENTS_KEY,
                                (gpointer) (g_variant_ref(d_ctx->result)));
        else
            (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);
    else if (!(*ctx)->results) // Hide property if no value is received
    {
        g_printerr(_("D-Bus error in systeminfo module while calling ListDesktopEnvironments(): "
                     "failed to produce a "
                     "result.\n"));
        ERR_EXIT();
    }

end:
    g_clear_error(&dbus_call_error);

    if (d_ctx)
        dbus_ctx_free(d_ctx);

    return ret;
}

static int systeminfo_module_handle_results(AlteratorCtlSystemInfoModule* module,
                                            alteratorctl_ctx_t** ctx)
{
    int ret = 0;
    if (!module)
    {
        g_printerr(_("Internal error in systeminfo module when handle result: module is NULL.\n"));
        ERR_EXIT();
    }

    if (module->alterator_ctl_app->arguments->module_help)
        goto end;

    if (!(*ctx))
    {
        g_printerr(_("Internal error in systeminfo module when handle results: *ctx is NULL.\n"));
        ERR_EXIT();
    }

    switch (g_variant_get_int32((*ctx)->subcommands_ids))
    {
    case SYSTEMINFO_GET_DESCRIPTION:
        if (systeminfo_module_get_description_handle_result(module, ctx) < 0)
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_HOST_NAME:
        if (systeminfo_module_get_hostname_handle_result(module, ctx, NULL) < 0)
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_OPERATION_SYSTEM_NAME:
        if (systeminfo_module_get_operation_system_name_handle_result(module, ctx, NULL) < 0)
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_ARCH:
        if (systeminfo_module_get_arch_handle_result(module, ctx, NULL) < 0)
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_BRANCH:
        if (systeminfo_module_get_branch_handle_result(module, ctx, NULL) < 0)
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_KERNEL:
        if (systeminfo_module_get_kernel_handle_result(module, ctx, NULL) < 0)
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_CPU:
        if (systeminfo_module_get_cpu_handle_result(module, ctx, NULL) < 0)
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_GPU:
        if (systeminfo_module_get_gpu_handle_result(module, ctx, NULL) < 0)
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_MEMORY:
        if (systeminfo_module_get_memory_handle_result(module, ctx, NULL) < 0)
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_DRIVE:
        if (systeminfo_module_get_drive_handle_result(module, ctx, NULL) < 0)
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_MONITOR:
        if (systeminfo_module_get_monitor_handle_result(module, ctx, NULL) < 0)
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_MOTHERBOARD:
        if (systeminfo_module_get_motherboard_handle_result(module, ctx, NULL) < 0)
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_LOCALES:
        if (systeminfo_module_get_locales_handle_result(module, ctx, NULL) < 0)
            ERR_EXIT();
        break;

    case SYSTEMINFO_GET_DESKTOP_ENVIRONMENT:
        if (systeminfo_module_get_desktops_handle_result(module, ctx, NULL) < 0)
            ERR_EXIT();
        break;

    default:
        g_printerr(_("Can't handle result of unknown systeminfo subcommand.\n"));
        ERR_EXIT();
    }

end:
    return ret;
}

static int systeminfo_module_get_license_handle_result(AlteratorCtlSystemInfoModule* module,
                                                       alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    GVariant* answer_array  = NULL;
    gchar* dbus_result_text = NULL;
    GVariant* exit_code     = NULL;

    if (!(*ctx)->results)
    {
        g_printerr(
            _("D-Bus error in systeminfo license handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(ayi)")))
    {
        g_printerr(_("Wrong answer type in systeminfo license.\n"));
        ERR_EXIT();
    }

    answer_array = g_variant_get_child_value((*ctx)->results, 0);
    exit_code    = g_variant_get_child_value((*ctx)->results, 1);

    ret = g_variant_get_int32(exit_code);

    if (ret)
    {
        g_printerr(_("Error while handling systeminfo license result. Exit code: %i.\n"), ret);
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);

    gsize array_size;
    gconstpointer gvar_info = g_variant_get_fixed_array(answer_array, &array_size, sizeof(guint8));

    dbus_result_text = g_malloc0(array_size + 1);
    if (!dbus_result_text)
        ERR_EXIT();

    memcpy(dbus_result_text, gvar_info, array_size);

    if (alterator_ctl_print_html(dbus_result_text) < 0)
        ERR_EXIT();

end:
    if (exit_code)
        g_variant_unref(exit_code);

    if (answer_array)
        g_variant_unref(answer_array);

    g_free(dbus_result_text);

    return ret;
}

static int systeminfo_module_get_description_handle_result(AlteratorCtlSystemInfoModule* module,
                                                           alteratorctl_ctx_t** ctx)
{
    int ret           = 0;
    GPtrArray* result = g_ptr_array_new_full(0, (GDestroyNotify) g_free);

    if (systeminfo_module_get_hostname_handle_result(module, ctx, &result) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_operation_system_name_handle_result(module, ctx, &result) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_arch_handle_result(module, ctx, &result) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_branch_handle_result(module, ctx, &result) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_kernel_handle_result(module, ctx, &result) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_cpu_handle_result(module, ctx, &result) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_gpu_handle_result(module, ctx, &result) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_memory_handle_result(module, ctx, &result) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_drive_handle_result(module, ctx, &result) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_monitor_handle_result(module, ctx, &result) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_motherboard_handle_result(module, ctx, &result) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_locales_handle_result(module, ctx, &result) < 0)
        ERR_EXIT();

    if (systeminfo_module_get_desktops_handle_result(module, ctx, &result) < 0)
        ERR_EXIT();

    g_ptr_array_sort(result, systeminfo_module_sort_result);
    for (gsize i = 0; i < result->len; i++)
        g_print("%s\n", (gchar*) result->pdata[i]);

end:
    if (result)
        g_ptr_array_unref(result);

    return ret;
}

static int systeminfo_module_get_hostname_handle_result(AlteratorCtlSystemInfoModule* module,
                                                        alteratorctl_ctx_t** ctx,
                                                        GPtrArray** result_str)
{
    int ret                         = 0;
    GVariant* answer                = NULL;
    GVariantDict* answer_dictionary = NULL;
    GVariant* dict_value            = NULL;
    GVariant* exit_code             = NULL;
    gchar* textual_result           = NULL;

    if (!(*ctx)->results)
    {
        if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
            return ret;
        g_printerr(
            _("D-Bus error in systeminfo hostname handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asi)"))
        && !g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        g_printerr(_("Wrong answer type in systeminfo hostname.\n"));
        ERR_EXIT();
    }

    if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        if (!result_str)
        {
            g_printerr(_("Can't save hostname to description\n"));
            ERR_EXIT();
        }

        answer_dictionary = g_variant_dict_new((GVariant*) (*ctx)->results);
        if (!(dict_value = g_variant_dict_lookup_value(answer_dictionary,
                                                       SYSTEMINFO_DESCRIPRION_HOST_NAME_KEY, NULL)))
        {
            g_printerr(_("Failed to get host name.\n"));
            ERR_EXIT();
        }
    }

    answer = dict_value ? g_variant_get_child_value(dict_value, 0)
                        : g_variant_get_child_value((*ctx)->results, 0);

    gchar** str_result_part_arr = (gchar**) g_variant_dup_strv(answer, NULL);
    gchar* str_result_part = str_result_part_arr ? g_strjoinv(NULL, str_result_part_arr) : NULL;
    if (str_result_part && strlen(str_result_part))
    {
        if (answer_dictionary)
            textual_result = g_strconcat(SYSTEMINFO_DESCRIPRION_HOST_NAME_KEY, " ", str_result_part,
                                         NULL);
        else
            textual_result = g_strdup(str_result_part);
    }
    else
    {
        if (!answer_dictionary)
        {
            g_printerr(_("Empty systeminfo hostname result.\n"));
            ERR_EXIT();
        }

        goto end;
    }
    g_free(str_result_part);
    g_strfreev(str_result_part_arr);

    exit_code = dict_value ? g_variant_get_child_value(dict_value, 1)
                           : g_variant_get_child_value((*ctx)->results, 1);
    ret       = g_variant_get_int32(exit_code);

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")) && ret)
    {
        g_printerr(_("Error while handling systeminfo hostname result. Exit code: %i.\n"), ret);
        ERR_EXIT();
    }
    else if (ret)
        goto end;

    if (textual_result && result_str && *result_str)
        g_ptr_array_add(*result_str, g_strdup(textual_result));
    else if (!textual_result && !answer_dictionary)
    {
        g_printerr(_("Empty systeminfo hostname result.\n"));
        ERR_EXIT();
    }

    else if (!answer_dictionary)
        g_print("%s\n", textual_result);

end:
    if (exit_code)
        g_variant_unref(exit_code);

    if (answer)
        g_variant_unref(answer);

    if (answer_dictionary)
        g_variant_dict_unref(answer_dictionary);

    if (dict_value)
        g_variant_unref(dict_value);

    g_free(textual_result);

    return ret;
}

static int systeminfo_module_get_operation_system_name_handle_result(
    AlteratorCtlSystemInfoModule* module, alteratorctl_ctx_t** ctx, GPtrArray** result_str)
{
    int ret                         = 0;
    GVariant* answer                = NULL;
    GVariantDict* answer_dictionary = NULL;
    GVariant* dict_value            = NULL;
    GVariant* exit_code             = NULL;
    gchar* textual_result           = NULL;

    if (!(*ctx)->results)
    {
        if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
            return ret;
        g_printerr(
            _("D-Bus error in systeminfo name handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asi)"))
        && !g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        g_printerr(_("Wrong answer type in systeminfo name.\n"));
        ERR_EXIT();
    }

    if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        if (!result_str)
        {
            g_printerr(_("Can't save OS name to description\n"));
            ERR_EXIT();
        }

        answer_dictionary = g_variant_dict_new((GVariant*) (*ctx)->results);
        if (!(dict_value = g_variant_dict_lookup_value(answer_dictionary,
                                                       SYSTEMINFO_DESCRIPRION_OS_NAME_KEY, NULL)))
        {
            g_printerr(_("Failed to get OS name.\n"));
            ERR_EXIT();
        }
    }

    answer = dict_value ? g_variant_get_child_value(dict_value, 0)
                        : g_variant_get_child_value((*ctx)->results, 0);

    gchar** str_result_part_arr = (gchar**) g_variant_dup_strv(answer, NULL);
    gchar* str_result_part = str_result_part_arr ? g_strjoinv(NULL, str_result_part_arr) : NULL;
    if (str_result_part && strlen(str_result_part))
    {
        if (answer_dictionary)
            textual_result = g_strconcat(SYSTEMINFO_DESCRIPRION_OS_NAME_KEY, " ", str_result_part,
                                         NULL);
        else
            textual_result = g_strdup(str_result_part);
    }
    else
    {
        if (!answer_dictionary)
        {
            g_printerr(_("Empty systeminfo name result.\n"));
            ERR_EXIT();
        }

        goto end;
    }
    g_free(str_result_part);
    g_strfreev(str_result_part_arr);

    exit_code = dict_value ? g_variant_get_child_value(dict_value, 1)
                           : g_variant_get_child_value((*ctx)->results, 1);
    ret       = g_variant_get_int32(exit_code);

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")) && ret)
    {
        g_printerr(_("Error while handling systeminfo name result. Exit code: %i.\n"), ret);
        ERR_EXIT();
    }
    else if (ret)
        goto end;

    if (textual_result && result_str && *result_str)
        g_ptr_array_add(*result_str, g_strdup(textual_result));
    else if (!textual_result && !answer_dictionary)
    {
        g_printerr(_("Empty systeminfo name result.\n"));
        ERR_EXIT();
    }

    else if (!answer_dictionary)
        g_print("%s\n", textual_result);

end:
    if (exit_code)
        g_variant_unref(exit_code);

    if (answer)
        g_variant_unref(answer);

    if (answer_dictionary)
        g_variant_dict_unref(answer_dictionary);

    if (dict_value)
        g_variant_unref(dict_value);

    g_free(textual_result);

    return ret;
}

static int systeminfo_module_get_arch_handle_result(AlteratorCtlSystemInfoModule* module,
                                                    alteratorctl_ctx_t** ctx,
                                                    GPtrArray** result_str)
{
    int ret                         = 0;
    GVariant* answer                = NULL;
    GVariantDict* answer_dictionary = NULL;
    GVariant* dict_value            = NULL;
    GVariant* exit_code             = NULL;
    gchar* textual_result           = NULL;

    if (!(*ctx)->results)
    {
        if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
            return ret;
        g_printerr(
            _("D-Bus error in systeminfo arch handle result: failed to produce a result.\n"));
        ERR_EXIT();
        return ret;
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asi)"))
        && !g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        g_printerr(_("Wrong answer type in systeminfo arch.\n"));
        ERR_EXIT();
    }

    if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        if (!result_str)
        {
            g_printerr(_("Can't save arch to description\n"));
            ERR_EXIT();
        }

        answer_dictionary = g_variant_dict_new((GVariant*) (*ctx)->results);
        if (!(dict_value = g_variant_dict_lookup_value(answer_dictionary,
                                                       SYSTEMINFO_DESCRIPRION_ARCH_KEY, NULL)))
        {
            g_printerr(_("Failed to get arch.\n"));
            ERR_EXIT();
        }
    }

    answer = dict_value ? g_variant_get_child_value(dict_value, 0)
                        : g_variant_get_child_value((*ctx)->results, 0);

    gchar** str_result_part_arr = (gchar**) g_variant_dup_strv(answer, NULL);
    gchar* str_result_part = str_result_part_arr ? g_strjoinv(NULL, str_result_part_arr) : NULL;
    if (str_result_part && strlen(str_result_part))
    {
        if (answer_dictionary)
            textual_result = g_strconcat(SYSTEMINFO_DESCRIPRION_ARCH_KEY, " ", str_result_part,
                                         NULL);
        else
            textual_result = g_strdup(str_result_part);
    }
    else
    {
        if (!answer_dictionary)
        {
            g_printerr(_("Empty systeminfo arch result.\n"));
            ERR_EXIT();
        }

        goto end;
    }
    g_free(str_result_part);
    g_strfreev(str_result_part_arr);

    exit_code = dict_value ? g_variant_get_child_value(dict_value, 1)
                           : g_variant_get_child_value((*ctx)->results, 1);
    ret       = g_variant_get_int32(exit_code);

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")) && ret)
    {
        g_printerr(_("Error while handling systeminfo arch result. Exit code: %i.\n"), ret);
        ERR_EXIT();
    }
    else if (ret)
        goto end;

    if (textual_result && result_str && *result_str)
        g_ptr_array_add(*result_str, g_strdup(textual_result));
    else if (!textual_result && !answer_dictionary)
    {
        g_printerr(_("Empty systeminfo arch result.\n"));
        ERR_EXIT();
    }

    else if (!answer_dictionary)
        g_print("%s\n", textual_result);

end:
    if (exit_code)
        g_variant_unref(exit_code);

    if (answer)
        g_variant_unref(answer);

    if (answer_dictionary)
        g_variant_dict_unref(answer_dictionary);

    if (dict_value)
        g_variant_unref(dict_value);

    g_free(textual_result);

    return ret;
}

static int systeminfo_module_get_branch_handle_result(AlteratorCtlSystemInfoModule* module,
                                                      alteratorctl_ctx_t** ctx,
                                                      GPtrArray** result_str)
{
    int ret                         = 0;
    GVariant* answer                = NULL;
    GVariantDict* answer_dictionary = NULL;
    GVariant* dict_value            = NULL;
    GVariant* exit_code             = NULL;
    gchar* textual_result           = NULL;

    if (!(*ctx)->results)
    {
        if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
            return ret;
        g_printerr(
            _("D-Bus error in systeminfo branch handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asi)"))
        && !g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        g_printerr(_("Wrong answer type in systeminfo branch.\n"));
        ERR_EXIT();
    }

    if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        if (!result_str)
        {
            g_printerr(_("Can't save branch to description\n"));
            ERR_EXIT();
        }

        answer_dictionary = g_variant_dict_new((GVariant*) (*ctx)->results);
        if (!(dict_value = g_variant_dict_lookup_value(answer_dictionary,
                                                       SYSTEMINFO_DESCRIPRION_BRANCH_KEY, NULL)))
        {
            g_printerr(_("Failed to get branch name.\n"));
            ERR_EXIT();
        }
    }

    answer = dict_value ? g_variant_get_child_value(dict_value, 0)
                        : g_variant_get_child_value((*ctx)->results, 0);

    gchar** str_result_part_arr = (gchar**) g_variant_dup_strv(answer, NULL);
    gchar* str_result_part = str_result_part_arr ? g_strjoinv(NULL, str_result_part_arr) : NULL;
    if (str_result_part && strlen(str_result_part))
    {
        if (answer_dictionary)
            textual_result = g_strconcat(SYSTEMINFO_DESCRIPRION_BRANCH_KEY, " ", str_result_part,
                                         NULL);
        else
            textual_result = g_strdup(str_result_part);
    }
    else
    {
        if (!answer_dictionary)
        {
            g_printerr(_("Empty systeminfo branch result.\n"));
            ERR_EXIT();
        }

        goto end;
    }
    g_free(str_result_part);
    g_strfreev(str_result_part_arr);

    exit_code = dict_value ? g_variant_get_child_value(dict_value, 1)
                           : g_variant_get_child_value((*ctx)->results, 1);
    ret       = g_variant_get_int32(exit_code);

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")) && ret)
    {
        g_printerr(_("Error while handling systeminfo branch result. Exit code: %i.\n"), ret);
        ERR_EXIT();
    }
    else if (ret)
        goto end;

    if (textual_result && result_str && *result_str)
        g_ptr_array_add(*result_str, g_strdup(textual_result));
    else if (!textual_result && !answer_dictionary)
    {
        g_printerr(_("Empty systeminfo branch result.\n"));
        ERR_EXIT();
    }

    else if (!answer_dictionary)
        g_print("%s\n", textual_result);

end:
    if (exit_code)
        g_variant_unref(exit_code);

    if (answer)
        g_variant_unref(answer);

    if (answer_dictionary)
        g_variant_dict_unref(answer_dictionary);

    if (dict_value)
        g_variant_unref(dict_value);

    g_free(textual_result);

    return ret;
}

static int systeminfo_module_get_kernel_handle_result(AlteratorCtlSystemInfoModule* module,
                                                      alteratorctl_ctx_t** ctx,
                                                      GPtrArray** result_str)
{
    int ret                         = 0;
    GVariant* answer                = NULL;
    GVariantDict* answer_dictionary = NULL;
    GVariant* dict_value            = NULL;
    GVariant* exit_code             = NULL;
    gchar* textual_result           = NULL;

    if (!(*ctx)->results)
    {
        if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
            return ret;
        g_printerr(
            _("D-Bus error in systeminfo kernel handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asi)"))
        && !g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        g_printerr(_("Wrong answer type in systeminfo kernel.\n"));
        ERR_EXIT();
    }

    if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        if (!result_str)
        {
            g_printerr(_("Can't save kernel name to description\n"));
            ERR_EXIT();
        }

        answer_dictionary = g_variant_dict_new((GVariant*) (*ctx)->results);
        if (!(dict_value = g_variant_dict_lookup_value(answer_dictionary,
                                                       SYSTEMINFO_DESCRIPRION_KERNEL_KEY, NULL)))
        {
            g_printerr(_("Failed to get kernel.\n"));
            ERR_EXIT();
        }
    }

    answer = dict_value ? g_variant_get_child_value(dict_value, 0)
                        : g_variant_get_child_value((*ctx)->results, 0);

    gchar** str_result_part_arr = (gchar**) g_variant_dup_strv(answer, NULL);
    gchar* str_result_part = str_result_part_arr ? g_strjoinv(NULL, str_result_part_arr) : NULL;
    if (str_result_part && strlen(str_result_part))
    {
        if (answer_dictionary)
            textual_result = g_strconcat(SYSTEMINFO_DESCRIPRION_KERNEL_KEY, " ", str_result_part,
                                         NULL);
        else
            textual_result = g_strdup(str_result_part);
    }
    else
    {
        if (!answer_dictionary)
        {
            g_printerr(_("Empty systeminfo kernel result.\n"));
            ERR_EXIT();
        }

        goto end;
    }
    g_free(str_result_part);
    g_strfreev(str_result_part_arr);

    exit_code = dict_value ? g_variant_get_child_value(dict_value, 1)
                           : g_variant_get_child_value((*ctx)->results, 1);
    ret       = g_variant_get_int32(exit_code);

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")) && ret)
    {
        g_printerr(_("Error while handling systeminfo kernel result. Exit code: %i.\n"), ret);
        ERR_EXIT();
    }
    else if (ret)
        goto end;

    if (textual_result && result_str && *result_str)
        g_ptr_array_add(*result_str, g_strdup(textual_result));
    else if (!textual_result && !answer_dictionary)
    {
        g_printerr(_("Empty systeminfo kernel result.\n"));
        ERR_EXIT();
    }

    else if (!answer_dictionary)
        g_print("%s\n", textual_result);

end:
    if (exit_code)
        g_variant_unref(exit_code);

    if (answer)
        g_variant_unref(answer);

    if (answer_dictionary)
        g_variant_dict_unref(answer_dictionary);

    if (dict_value)
        g_variant_unref(dict_value);

    g_free(textual_result);

    return ret;
}

static int systeminfo_module_get_cpu_handle_result(AlteratorCtlSystemInfoModule* module,
                                                   alteratorctl_ctx_t** ctx, GPtrArray** result_str)
{
    int ret                         = 0;
    GVariant* answer                = NULL;
    GVariantDict* answer_dictionary = NULL;
    GVariant* dict_value            = NULL;
    GVariant* exit_code             = NULL;
    GString* result                 = g_string_new(NULL);
    gchar** str_result_part_arr     = NULL;

    if (!(*ctx)->results)
    {
        if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
            return ret;
        g_printerr(_("D-Bus error in systeminfo cpu handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asi)"))
        && !g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        g_printerr(_("Wrong answer type in systeminfo cpu.\n"));
        ERR_EXIT();
    }

    if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        if (!result_str)
        {
            g_printerr(_("Can't save CPU to description\n"));
            ERR_EXIT();
        }

        answer_dictionary = g_variant_dict_new((GVariant*) (*ctx)->results);
        if (!(dict_value = g_variant_dict_lookup_value(answer_dictionary,
                                                       SYSTEMINFO_DESCRIPRION_CPU_KEY, NULL)))
        {
            g_printerr(_("Failed to get cpu.\n"));
            ERR_EXIT();
        }
    }

    answer              = dict_value ? g_variant_get_child_value(dict_value, 0)
                                     : g_variant_get_child_value((*ctx)->results, 0);
    str_result_part_arr = (gchar**) g_variant_dup_strv(answer, NULL);
    if (!str_result_part_arr || (str_result_part_arr && !g_strv_length(str_result_part_arr)))
    {
        if (!answer_dictionary)
        {
            g_printerr(_("Empty systeminfo cpu result.\n"));
            ERR_EXIT();
        }

        goto end;
    }

    exit_code = dict_value ? g_variant_get_child_value(dict_value, 1)
                           : g_variant_get_child_value((*ctx)->results, 1);
    ret       = g_variant_get_int32(exit_code);

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")) && ret)
    {
        g_printerr(_("Error while handling systeminfo cpu result. Exit code: %i.\n"), ret);
        ERR_EXIT();
    }
    else if (ret)
        goto end;

    if (!str_result_part_arr)
    {
        g_printerr(_("Empty systeminfo cpu result.\n"));
        ERR_EXIT();
    }

    if (g_strv_length(str_result_part_arr) != 3)
    {
        if (answer_dictionary)
            goto end;

        g_printerr(_("Invalid amount of CPU data.\n"));
        ERR_EXIT();
    }

    for (gsize i = 0; i < sizeof(str_result_part_arr); i++)
        if (i == 0)
            g_string_append(result, str_result_part_arr[i]);
        else if (i == 1)
        {
            g_string_append(result, " (");
            g_string_append(result, str_result_part_arr[i]);
            g_string_append(result, ") ");
        }
        else if (i == 2)
        {
            g_string_append(result, str_result_part_arr[i]);
            g_string_append(result, "Hz");
        }

    if (answer_dictionary)
    {
        g_string_prepend(result, SYSTEMINFO_DESCRIPRION_CPU_KEY);
        g_string_insert_c(result, sizeof(SYSTEMINFO_DESCRIPRION_CPU_KEY) - 1, ' ');
    }

    if (result && result_str && *result_str)
        g_ptr_array_add(*result_str, g_strdup(result->str));
    else if (!result && !answer_dictionary)
    {
        g_printerr(_("Empty systeminfo cpu result.\n"));
        ERR_EXIT();
    }
    else if (!answer_dictionary)
        g_print("%s\n", result->str);

end:
    if (result)
        g_string_free(result, TRUE);

    if (exit_code)
        g_variant_unref(exit_code);

    if (answer)
        g_variant_unref(answer);

    if (answer_dictionary)
        g_variant_dict_unref(answer_dictionary);

    if (dict_value)
        g_variant_unref(dict_value);

    g_strfreev(str_result_part_arr);

    return ret;
}

static int systeminfo_module_get_gpu_handle_result(AlteratorCtlSystemInfoModule* module,
                                                   alteratorctl_ctx_t** ctx, GPtrArray** result_str)
{
    int ret                         = 0;
    GVariant* answer                = NULL;
    GVariantDict* answer_dictionary = NULL;
    GVariant* dict_value            = NULL;
    GVariant* exit_code             = NULL;
    gchar* textual_result           = NULL;

    if (!(*ctx)->results)
    {
        if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
            return ret;
        g_printerr(_("D-Bus error in systeminfo gpu handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asi)"))
        && !g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        g_printerr(_("Wrong answer type in systeminfo gpu.\n"));
        ERR_EXIT();
    }

    if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        if (!result_str)
        {
            g_printerr(_("Can't save GPU to description\n"));
            ERR_EXIT();
        }

        answer_dictionary = g_variant_dict_new((GVariant*) (*ctx)->results);
        if (!(dict_value = g_variant_dict_lookup_value(answer_dictionary,
                                                       SYSTEMINFO_DESCRIPRION_GPU_KEY, NULL)))
        {
            g_printerr(_("Failed to get gpu.\n"));
            ERR_EXIT();
        }
    }

    answer = dict_value ? g_variant_get_child_value(dict_value, 0)
                        : g_variant_get_child_value((*ctx)->results, 0);

    gchar** str_result_part_arr = (gchar**) g_variant_dup_strv(answer, NULL);
    gchar* str_result_part = str_result_part_arr ? g_strjoinv(NULL, str_result_part_arr) : NULL;
    if (str_result_part && strlen(str_result_part))
    {
        if (answer_dictionary)
            textual_result = g_strconcat(SYSTEMINFO_DESCRIPRION_GPU_KEY, " ", str_result_part,
                                         NULL);
        else
            textual_result = g_strdup(str_result_part);
    }
    else
    {
        if (!answer_dictionary)
        {
            g_printerr(_("Empty systeminfo gpu result.\n"));
            ERR_EXIT();
        }

        goto end;
    }
    g_free(str_result_part);
    g_strfreev(str_result_part_arr);

    exit_code = dict_value ? g_variant_get_child_value(dict_value, 1)
                           : g_variant_get_child_value((*ctx)->results, 1);
    ret       = g_variant_get_int32(exit_code);

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")) && ret)
    {
        g_printerr(_("Error while handling systeminfo gpu result. Exit code: %i.\n"), ret);
        ERR_EXIT();
    }
    else if (ret)
        goto end;

    if (textual_result && result_str && *result_str)
        g_ptr_array_add(*result_str, g_strdup(textual_result));
    else if (!textual_result && !answer_dictionary)
    {
        g_printerr(_("Empty systeminfo gpu result.\n"));
        ERR_EXIT();
    }

    else if (!answer_dictionary)
        g_print("%s\n", textual_result);

end:
    if (exit_code)
        g_variant_unref(exit_code);

    if (answer)
        g_variant_unref(answer);

    if (answer_dictionary)
        g_variant_dict_unref(answer_dictionary);

    if (dict_value)
        g_variant_unref(dict_value);

    g_free(textual_result);

    return ret;
}

static int systeminfo_module_get_memory_handle_result(AlteratorCtlSystemInfoModule* module,
                                                      alteratorctl_ctx_t** ctx,
                                                      GPtrArray** result_str)
{
    int ret                         = 0;
    GVariant* answer                = NULL;
    GVariantDict* answer_dictionary = NULL;
    GVariant* dict_value            = NULL;
    GVariant* exit_code             = NULL;
    gchar* textual_result           = NULL;

    if (!(*ctx)->results)
    {
        if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
            return ret;
        g_printerr(
            _("D-Bus error in systeminfo memory handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asi)"))
        && !g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        g_printerr(_("Wrong answer type in systeminfo memory.\n"));
        ERR_EXIT();
    }

    if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        if (!result_str)
        {
            g_printerr(_("Can't save RAM volume to description\n"));
            ERR_EXIT();
        }

        answer_dictionary = g_variant_dict_new((GVariant*) (*ctx)->results);
        if (!(dict_value = g_variant_dict_lookup_value(answer_dictionary,
                                                       SYSTEMINFO_DESCRIPRION_MEMORY_KEY, NULL)))
        {
            g_printerr(_("Failed to get memory.\n"));
            ERR_EXIT();
        }
    }

    answer = dict_value ? g_variant_get_child_value(dict_value, 0)
                        : g_variant_get_child_value((*ctx)->results, 0);

    gchar** str_result_part_arr = (gchar**) g_variant_dup_strv(answer, NULL);
    gchar* str_result_part = str_result_part_arr ? g_strjoinv(NULL, str_result_part_arr) : NULL;
    if (str_result_part && strlen(str_result_part))
    {
        if (answer_dictionary)
            textual_result = g_strconcat(SYSTEMINFO_DESCRIPRION_MEMORY_KEY, " ", str_result_part,
                                         NULL);
        else
            textual_result = g_strdup(str_result_part);
    }
    else
    {
        if (!answer_dictionary)
        {
            g_printerr(_("Empty systeminfo memory result.\n"));
            ERR_EXIT();
        }

        goto end;
    }
    g_free(str_result_part);
    g_strfreev(str_result_part_arr);

    exit_code = dict_value ? g_variant_get_child_value(dict_value, 1)
                           : g_variant_get_child_value((*ctx)->results, 1);
    ret       = g_variant_get_int32(exit_code);

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")) && ret)
    {
        g_printerr(_("Error while handling systeminfo memory result. Exit code: %i.\n"), ret);
        ERR_EXIT();
    }
    else if (ret)
        goto end;

    if (textual_result && result_str && *result_str)
        g_ptr_array_add(*result_str, g_strdup(textual_result));
    else if (!textual_result && !answer_dictionary)
    {
        g_printerr(_("Empty systeminfo memory result.\n"));
        ERR_EXIT();
    }

    else if (!answer_dictionary)
        g_print("%s\n", textual_result);

end:
    if (exit_code)
        g_variant_unref(exit_code);

    if (answer)
        g_variant_unref(answer);

    if (answer_dictionary)
        g_variant_dict_unref(answer_dictionary);

    if (dict_value)
        g_variant_unref(dict_value);

    g_free(textual_result);

    return ret;
}

static int systeminfo_module_get_drive_handle_result(AlteratorCtlSystemInfoModule* module,
                                                     alteratorctl_ctx_t** ctx,
                                                     GPtrArray** result_str)
{
    int ret                         = 0;
    GVariant* answer                = NULL;
    GVariantDict* answer_dictionary = NULL;
    GVariant* dict_value            = NULL;
    GVariant* exit_code             = NULL;
    gchar* textual_result           = NULL;

    if (!(*ctx)->results)
    {
        if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
            return ret;
        g_printerr(
            _("D-Bus error in systeminfo drive handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asi)"))
        && !g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        g_printerr(_("Wrong answer type in systeminfo drive.\n"));
        ERR_EXIT();
    }

    if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        if (!result_str)
        {
            g_printerr(_("Can't save total memory volume to description\n"));
            ERR_EXIT();
        }

        answer_dictionary = g_variant_dict_new((GVariant*) (*ctx)->results);
        dict_value        = g_variant_dict_lookup_value(answer_dictionary,
                                                        SYSTEMINFO_DESCRIPRION_DRIVE_KEY, NULL);
        if (!dict_value)
        {
            g_printerr(_("Failed to get drive volume.\n"));
            ERR_EXIT();
        }
    }

    answer = dict_value ? g_variant_get_child_value(dict_value, 0)
                        : g_variant_get_child_value((*ctx)->results, 0);

    gchar** str_result_part_arr = (gchar**) g_variant_dup_strv(answer, NULL);
    gchar* str_result_part = str_result_part_arr ? g_strjoinv(NULL, str_result_part_arr) : NULL;
    if (str_result_part && strlen(str_result_part))
    {
        if (answer_dictionary)
            textual_result = g_strconcat(SYSTEMINFO_DESCRIPRION_DRIVE_KEY, " ", str_result_part,
                                         NULL);
        else
            textual_result = g_strdup(str_result_part);
    }
    else
    {
        if (!answer_dictionary)
        {
            g_printerr(_("Empty systeminfo drive result.\n"));
            ERR_EXIT();
        }

        goto end;
    }
    g_free(str_result_part);
    g_strfreev(str_result_part_arr);

    exit_code = dict_value ? g_variant_get_child_value(dict_value, 1)
                           : g_variant_get_child_value((*ctx)->results, 1);
    ret       = g_variant_get_int32(exit_code);

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")) && ret)
    {
        g_printerr(_("Error while handling systeminfo drive result. Exit code: %i.\n"), ret);
        ERR_EXIT();
    }
    else if (ret)
        goto end;

    if (textual_result && result_str && *result_str)
        g_ptr_array_add(*result_str, g_strdup(textual_result));
    else if (!textual_result && !answer_dictionary)
    {
        g_printerr(_("Empty systeminfo drive result.\n"));
        ERR_EXIT();
    }

    else if (!answer_dictionary)
        g_print("%s\n", textual_result);

end:
    if (exit_code)
        g_variant_unref(exit_code);

    if (answer)
        g_variant_unref(answer);

    if (answer_dictionary)
        g_variant_dict_unref(answer_dictionary);

    if (dict_value)
        g_variant_unref(dict_value);

    g_free(textual_result);

    return ret;
}

static int systeminfo_module_get_monitor_handle_result(AlteratorCtlSystemInfoModule* module,
                                                       alteratorctl_ctx_t** ctx,
                                                       GPtrArray** result_str)
{
    int ret                         = 0;
    GVariant* answer                = NULL;
    GVariantDict* answer_dictionary = NULL;
    GVariant* dict_value            = NULL;
    GVariant* exit_code             = NULL;
    gchar* textual_result           = NULL;

    if (!(*ctx)->results)
    {
        if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
            return ret;
        g_printerr(
            _("D-Bus error in systeminfo monitor handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asi)"))
        && !g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        g_printerr(_("Wrong answer type in systeminfo monitor.\n"));
        ERR_EXIT();
    }

    if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        if (!result_str)
        {
            g_printerr(_("Can't save monitor to description\n"));
            ERR_EXIT();
        }

        answer_dictionary = g_variant_dict_new((GVariant*) (*ctx)->results);
        dict_value        = g_variant_dict_lookup_value(answer_dictionary,
                                                        SYSTEMINFO_DESCRIPRION_MONITOR_KEY, NULL);
        if (!dict_value)
        {
            g_printerr(_("Failed to get monitor.\n"));
            ERR_EXIT();
        }
    }

    answer                      = dict_value ? g_variant_get_child_value(dict_value, 0)
                                             : g_variant_get_child_value((*ctx)->results, 0);
    gchar** str_result_part_arr = (gchar**) g_variant_dup_strv(answer, NULL);
    gchar* str_result_part = str_result_part_arr ? g_strjoinv(", ", str_result_part_arr) : NULL;
    if (str_result_part && strlen(str_result_part))
    {
        if (answer_dictionary)
            textual_result = g_strconcat(SYSTEMINFO_DESCRIPRION_MONITOR_KEY, " ", str_result_part,
                                         NULL);
        else
            textual_result = g_strdup(str_result_part);
    }
    else
    {
        if (!answer_dictionary)
        {
            g_printerr(_("Empty systeminfo monitor result.\n"));
            ERR_EXIT();
        }

        goto end;
    }
    g_free(str_result_part);
    g_strfreev(str_result_part_arr);

    exit_code = dict_value ? g_variant_get_child_value(dict_value, 1)
                           : g_variant_get_child_value((*ctx)->results, 1);
    ret       = g_variant_get_int32(exit_code);

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")) && ret)
    {
        g_printerr(_("Error while handling systeminfo monitor result. Exit code: %i.\n"), ret);
        ERR_EXIT();
    }
    else if (ret)
        goto end;

    if (textual_result && result_str && *result_str)
        g_ptr_array_add(*result_str, g_strdup(textual_result));
    else if (!textual_result && !answer_dictionary)
    {
        g_printerr(_("Empty systeminfo monitor result.\n"));
        ERR_EXIT();
    }

    else if (!answer_dictionary)
        g_print("%s\n", textual_result);

end:

    if (exit_code)
        g_variant_unref(exit_code);

    if (answer)
        g_variant_unref(answer);

    if (answer_dictionary)
        g_variant_dict_unref(answer_dictionary);

    if (dict_value)
        g_variant_unref(dict_value);

    g_free(textual_result);

    return ret;
}

static int systeminfo_module_get_motherboard_handle_result(AlteratorCtlSystemInfoModule* module,
                                                           alteratorctl_ctx_t** ctx,
                                                           GPtrArray** result_str)
{
    int ret                         = 0;
    GVariant* answer                = NULL;
    GVariantDict* answer_dictionary = NULL;
    GVariant* dict_value            = NULL;
    GVariant* exit_code             = NULL;
    gchar* textual_result           = NULL;

    if (!(*ctx)->results)
    {
        if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
            return ret;
        g_printerr(_(
            "D-Bus error in systeminfo motherboard handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asi)"))
        && !g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        g_printerr(_("Wrong answer type in systeminfo motherboard.\n"));
        ERR_EXIT();
    }

    if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        if (!result_str)
        {
            g_printerr(_("Can't save motherboard to description\n"));
            ERR_EXIT();
        }

        answer_dictionary = g_variant_dict_new((GVariant*) (*ctx)->results);
        dict_value        = g_variant_dict_lookup_value(answer_dictionary,
                                                        SYSTEMINFO_DESCRIPRION_MOTHERBOARD_KEY, NULL);
        if (!dict_value)
        {
            g_printerr(_("Failed to get motherboard.\n"));
            ERR_EXIT();
        }
    }

    answer                      = dict_value ? g_variant_get_child_value(dict_value, 0)
                                             : g_variant_get_child_value((*ctx)->results, 0);
    gchar** str_result_part_arr = (gchar**) g_variant_dup_strv(answer, NULL);
    gchar* str_result_part = str_result_part_arr ? g_strjoinv(NULL, str_result_part_arr) : NULL;
    if (str_result_part && strlen(str_result_part))
    {
        if (answer_dictionary)
            textual_result = g_strconcat(SYSTEMINFO_DESCRIPRION_MOTHERBOARD_KEY, " ",
                                         str_result_part, NULL);
        else
            textual_result = g_strdup(str_result_part);
    }
    else
    {
        if (!answer_dictionary)
        {
            g_printerr(_("Empty systeminfo motherboard result.\n"));
            ERR_EXIT();
        }

        goto end;
    }
    g_free(str_result_part);
    g_strfreev(str_result_part_arr);

    exit_code = dict_value ? g_variant_get_child_value(dict_value, 1)
                           : g_variant_get_child_value((*ctx)->results, 1);
    ret       = g_variant_get_int32(exit_code);

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")) && ret)
    {
        g_printerr(_("Error while handling systeminfo motherboard result. Exit code: %i.\n"), ret);
        ERR_EXIT();
    }
    else if (ret)
        goto end;

    if (textual_result && result_str && *result_str)
        g_ptr_array_add(*result_str, g_strdup(textual_result));
    else if (!textual_result && !answer_dictionary)
    {
        g_printerr(_("Empty systeminfo motherboard result.\n"));
        ERR_EXIT();
    }

    else if (!answer_dictionary)
        g_print("%s\n", textual_result);

end:
    if (exit_code)
        g_variant_unref(exit_code);

    if (answer)
        g_variant_unref(answer);

    if (answer_dictionary)
        g_variant_dict_unref(answer_dictionary);

    if (dict_value)
        g_variant_unref(dict_value);

    g_free(textual_result);

    return ret;
}

static int systeminfo_module_get_locales_handle_result(AlteratorCtlSystemInfoModule* module,
                                                       alteratorctl_ctx_t** ctx,
                                                       GPtrArray** result_str)
{
    int ret                         = 0;
    GVariant* answer                = NULL;
    GVariantDict* answer_dictionary = NULL;
    GVariant* dict_value            = NULL;
    GVariant* exit_code             = NULL;
    gchar* textual_result           = NULL;

    if (!(*ctx)->results)
    {
        if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
            return ret;
        g_printerr(
            _("D-Bus error in systeminfo locales handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asi)"))
        && !g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        g_printerr(_("Wrong answer type in systeminfo locales.\n"));
        ERR_EXIT();
    }

    if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        if (!result_str)
        {
            g_printerr(_("Can't save locales to description\n"));
            ERR_EXIT();
        }

        answer_dictionary = g_variant_dict_new((GVariant*) (*ctx)->results);
        dict_value        = g_variant_dict_lookup_value(answer_dictionary,
                                                        SYSTEMINFO_DESCRIPRION_LOCALES_KEY, NULL);
        if (!dict_value)
        {
            g_printerr(_("Failed to get locales.\n"));
            ERR_EXIT();
        }
    }

    answer = dict_value ? g_variant_get_child_value(dict_value, 0)
                        : g_variant_get_child_value((*ctx)->results, 0);

    gchar** str_result_part_arr = (gchar**) g_variant_dup_strv(answer, NULL);
    gchar* str_result_part = str_result_part_arr ? g_strjoinv(NULL, str_result_part_arr) : NULL;
    if (str_result_part && strlen(str_result_part))
    {
        if (answer_dictionary)
            textual_result = g_strconcat(SYSTEMINFO_DESCRIPRION_LOCALES_KEY, " ", str_result_part,
                                         NULL);
        else
            textual_result = g_strdup(str_result_part);
    }
    else
    {
        if (!answer_dictionary)
        {
            g_printerr(_("Empty systeminfo locales result.\n"));
            ERR_EXIT();
        }

        goto end;
    }
    g_free(str_result_part);
    g_strfreev(str_result_part_arr);

    exit_code = dict_value ? g_variant_get_child_value(dict_value, 1)
                           : g_variant_get_child_value((*ctx)->results, 1);
    ret       = g_variant_get_int32(exit_code);

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")) && ret)
    {
        g_printerr(_("Error while handling systeminfo locales result. Exit code: %i.\n"), ret);
        ERR_EXIT();
    }
    else if (ret)
        goto end;

    if (textual_result && result_str && *result_str)
        g_ptr_array_add(*result_str, g_strdup(textual_result));
    else if (!textual_result && !answer_dictionary)
    {
        g_printerr(_("Empty systeminfo locales result.\n"));
        ERR_EXIT();
    }

    else if (!answer_dictionary)
        g_print("%s\n", textual_result);

end:
    if (exit_code)
        g_variant_unref(exit_code);

    if (answer)
        g_variant_unref(answer);

    if (answer_dictionary)
        g_variant_dict_unref(answer_dictionary);

    if (dict_value)
        g_variant_unref(dict_value);

    g_free(textual_result);

    return ret;
}

static int systeminfo_module_get_desktops_handle_result(AlteratorCtlSystemInfoModule* module,
                                                        alteratorctl_ctx_t** ctx,
                                                        GPtrArray** result_str)
{
    int ret                         = 0;
    GVariant* answer                = NULL;
    GVariantDict* answer_dictionary = NULL;
    GVariant* dict_value            = NULL;
    GVariant* exit_code             = NULL;
    gchar* textual_result           = NULL;

    if (!(*ctx)->results)
    {
        if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
            return ret;
        g_printerr(
            _("D-Bus error in systeminfo desktops handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asi)"))
        && !g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        g_printerr(_("Wrong answer type in systeminfo desktops.\n"));
        ERR_EXIT();
    }

    if (g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")))
    {
        if (!result_str)
        {
            g_printerr(_("Can't save desktop environments to description\n"));
            ERR_EXIT();
        }

        answer_dictionary = g_variant_dict_new((GVariant*) (*ctx)->results);
        dict_value        = g_variant_dict_lookup_value(answer_dictionary,
                                                        SYSTEMINFO_DESCRIPRION_DESKTOPS_ENVIRONMENTS_KEY,
                                                        NULL);
        if (!dict_value)
        {
            g_printerr(_("Failed to get desktops.\n"));
            ERR_EXIT();
        }
    }

    answer = dict_value ? g_variant_get_child_value(dict_value, 0)
                        : g_variant_get_child_value((*ctx)->results, 0);

    gchar** str_result_part_arr = (gchar**) g_variant_dup_strv(answer, NULL);
    gchar* str_result_part = str_result_part_arr ? g_strjoinv(NULL, str_result_part_arr) : NULL;
    if (str_result_part && strlen(str_result_part))
    {
        if (answer_dictionary)
            textual_result = g_strconcat(SYSTEMINFO_DESCRIPRION_DESKTOPS_ENVIRONMENTS_KEY, " ",
                                         str_result_part, NULL);
        else
            textual_result = g_strdup(str_result_part);
    }
    else
    {
        if (!answer_dictionary)
        {
            g_printerr(_("Empty systeminfo desktops result.\n"));
            ERR_EXIT();
        }

        goto end;
    }
    g_free(str_result_part);
    g_strfreev(str_result_part_arr);

    exit_code = dict_value ? g_variant_get_child_value(dict_value, 1)
                           : g_variant_get_child_value((*ctx)->results, 1);
    ret       = g_variant_get_int32(exit_code);

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("a{sv}")) && ret)
    {
        g_printerr(_("Error while handling systeminfo desktops result. Exit code: %i.\n"), ret);
        ERR_EXIT();
    }
    else if (ret)
        goto end;

    if (textual_result && result_str && *result_str)
        g_ptr_array_add(*result_str, g_strdup(textual_result));
    else if (!textual_result && !answer_dictionary)
    {
        g_printerr(_("Empty systeminfo desktop environments result.\n"));
        ERR_EXIT();
    }

    else if (!answer_dictionary)
        g_print("%s\n", textual_result);

end:
    if (exit_code)
        g_variant_unref(exit_code);

    if (answer)
        g_variant_unref(answer);

    if (answer_dictionary)
        g_variant_dict_unref(answer_dictionary);

    if (dict_value)
        g_variant_unref(dict_value);

    g_free(textual_result);

    return ret;
}

int systeminfo_module_print_help(gpointer self)
{
    int ret = 0;

    g_print(_("Usage:\n"));
    g_print(_("  alteratorctl systeminfo [COMMAND] [OPTIONS]\n"));
    g_print(_("  alteratorctl systeminfo     Get information about the system (exclude os\n"
              "                              license agreement)\n\n"));
    g_print(_("Commands:\n"));
    g_print(_("  arch                        Get operation system arch\n"));
    g_print(_("  branch                      Get operation system branch\n"));
    g_print(_("  cpu                         Get information about CPU\n"));
    g_print(_("  description                 Get information about the system (exclude os\n"
              "                              license agreement) (default command)\n"));
    g_print(_("  desktops                    Get desktop environments\n"));
    g_print(_("  drive                       Get information about drive volume\n"));
    g_print(_("  gpu                         Get information about GPU\n"));
    g_print(_("  hostname                    Get host name\n"));
    g_print(_("  kernel                      Get operation system kernel\n"));
    g_print(_("  locales                     Get list of user(s) locales\n"));
    g_print(_("  memory                      Get information about RAM memory volume\n"));
    g_print(_("  monitor                     Get information about monitor\n"));
    g_print(_("  motherboard                 Get information about motherboard\n"));
    g_print(_("  name                        Get operation system name\n\n"));
    g_print(_("Options:\n"));
    g_print(_("  -h, --help                  Show module systeminfo usage\n\n"));

end:
    return ret;
}

static gint systeminfo_module_sort_result(gconstpointer a, gconstpointer b)
{
    return g_utf8_collate((const gchar*) ((GPtrArray*) a)->pdata,
                          (const gchar*) ((GPtrArray*) b)->pdata);
}
