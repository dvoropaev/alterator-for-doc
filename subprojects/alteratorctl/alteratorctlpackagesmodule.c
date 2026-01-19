#include "alteratorctlpackagesmodule.h"
#include "alteratorctlapp.h"

#include <gio/gioenums.h>
#include <json-glib/json-glib.h>
#include <toml.h>

#define PACKAGES_RPM_INTERFACE_NAME "org.altlinux.alterator.rpm1"
#define PACKAGES_RPM_OBJECT_PATH "/org/altlinux/alterator/rpm"
#define PACKAGES_RPM_FILES_METHOD_NAME "Files"
#define PACKAGES_RPM_INFO_METHOD_NAME "Info"
#define PACKAGES_RPM_INSTALL_METHOD_NAME "Install"
#define PACKAGES_RPM_LIST_METHOD_NAME "List"
#define PACKAGES_RPM_PACKAGE_INFO_METHOD_NAME "PackageInfo"
#define PACKAGES_RPM_REMOVE_METHOD_NAME "Remove"

#define PACKAGES_APT_INTERFACE_NAME "org.altlinux.alterator.apt1"
#define PACKAGES_APT_OBJECT_PATH "/org/altlinux/alterator/apt"
#define PACKAGES_APT_APPLY_ASYNC_METHOD_NAME "ApplyAsync"
#define PACKAGES_APT_CHECK_APPLY_METHOD_NAME "CheckApply"
#define PACKAGES_APT_CHECK_REINSTALL_METHOD_NAME "CheckReinstall"
#define PACKAGES_APT_INFO_METHOD_NAME "Info"
#define PACKAGES_APT_INSTALL_METHOD_NAME "Install"
#define PACKAGES_APT_INSTALL_ASYNC_METHOD_NAME "InstallAsync"
#define PACKAGES_APT_LIST_ALL_PACKAGES_METHOD_NAME "ListAllPackages"
#define PACKAGES_APT_REINSTALL_METHOD_NAME "Reinstall"
#define PACKAGES_APT_REINSTALL_ASYNC_METHOD_NAME "ReinstallAsync"
#define PACKAGES_APT_REMOVE_METHOD_NAME "Remove"
#define PACKAGES_APT_REMOVE_ASYNC_METHOD_NAME "RemoveAsync"
#define PACKAGES_APT_SEARCH_METHOD_NAME "Search"
#define PACKAGES_APT_UPDATE_METHOD_NAME "Update"
#define PACKAGES_APT_UPDATE_ASYNC_METHOD_NAME "UpdateAsync"
#define PACKAGES_APT_LAST_UPDATE_METHOD_NAME "LastUpdate"

#define PACKAGES_APT_REMOVE_ESSENTIAL_APPLY_PHRASE "Yes, do as I say!"

#define PACKAGES_APT_INSTALL_STDOUT_SIGNAL_NAME "apt1_install_stdout_signal"
#define PACKAGES_APT_INSTALL_STDERR_SIGNAL_NAME "apt1_install_stderr_signal"
#define PACKAGES_APT_REMOVE_STDOUT_SIGNAL_NAME "apt1_remove_stdout_signal"
#define PACKAGES_APT_REMOVE_STDERR_SIGNAL_NAME "apt1_remove_stderr_signal"
#define PACKAGES_APT_REINSTALL_STDOUT_SIGNAL_NAME "apt1_reinstall_stdout_signal"
#define PACKAGES_APT_REINSTALL_STDERR_SIGNAL_NAME "apt1_reinstall_stderr_signal"

#define PACKAGES_APT_UPDATE_ASYNC_STDOUT_SIGNAL_NAME "apt1_update_stdout_signal"
#define PACKAGES_APT_UPDATE_ASYNC_STDERR_SIGNAL_NAME "apt1_update_stderr_signal"

#define PACKAGES_REPO_INTERFACE_NAME "org.altlinux.alterator.repo1"
#define PACKAGES_REPO_OBJECT_PATH "/org/altlinux/alterator/repo"
#define PACKAGES_REPO_ADD_METHOD_NAME "Add"
#define PACKAGES_REPO_INFO_METHOD_NAME "Info"
#define PACKAGES_REPO_LIST_METHOD_NAME "List"
#define PACKAGES_REPO_REMOVE_METHOD_NAME "Remove"

#define PACKAGES_SUBMODULES_AMOUNTH 4
#define PACKAGES_ONE_DAY_TIMEOUT 86400000

#define PACKAGES_SUBMODULES_ALTERATOR_ENTRY_TYPE_KEY_NAME "type"
#define PACKAGES_SUBMODULES_ALTERATOR_ENTRY_NAME_KEY_NAME "name"

typedef struct packages_module_subcommands_t
{
    char* subcommand;
    enum packages_sub_commands id;
} packages_module_subcommands_t;

enum packages_hash_table_indices_t
{
    PACKAGES_HASH_TABLE,
    APT_HASH_TABLE,
    RPM_HASH_TABLE,
    REPO_HASH_TABLE
};

gboolean apt_allow_remove_manually    = FALSE;
gboolean apt_force_transaction_commit = FALSE;

static packages_module_subcommands_t packages_module_subcommands_list[] = {{"apt", PACKAGES_APT},
                                                                           {"rpm", PACKAGES_RPM},
                                                                           {"repo", PACKAGES_REPO}};

typedef struct packages_module_apt_subcommands_t
{
    char* subcommand;
    enum packages_apt_subcommand id;
} packages_module_apt_subcommands_t;

typedef struct check_apply_result
{
    GPtrArray* to_install;
    GPtrArray* to_remove;
    GPtrArray* extra_remove;
    gint exit_code;
} check_apply_result;

check_apply_result* check_apply_result_init(GPtrArray* to_install, GPtrArray* to_remove,
                                            GPtrArray* extra_remove, gint exit_code);
void check_apply_result_free(check_apply_result* packages_to_apply);

static packages_module_apt_subcommands_t packages_module_apt_subcommands_list[] =
    {{"info", APT_INFO},           {"install", APT_INSTALL},        {"list", APT_LIST_ALL_PACKAGES},
     {"reinstall", APT_REINSTALL}, {"remove", APT_REMOVE},          {"search", APT_SEARCH},
     {"update", APT_UPDATE},       {"last-update", APT_LAST_UPDATE}};

typedef struct packages_module_rpm_subcommands_t
{
    char* subcommand;
    enum packages_rpm_subcommand id;
} packages_module_rpm_subcommands_t;

static packages_module_rpm_subcommands_t packages_module_rpm_subcommands_list[] =
    {{"files", RPM_FILES},
     {"info", RPM_INFO},
     {"install", RPM_INSTALL},
     {"list", RPM_LIST},
     {"packageinfo", RPM_PACKAGE_INFO},
     {"remove", RPM_REMOVE}};

typedef struct packages_module_repo_subcommands_t
{
    char* subcommand;
    enum packages_repo_subcommand id;
} packages_module_repo_subcommands_t;
static packages_module_repo_subcommands_t packages_module_repo_subcommands_list[] =
    {{"add", REPO_ADD}, {"info", REPO_INFO}, {"list", REPO_LIST}, {"remove", REPO_REMOVE}};

static GObjectClass* packages_module_parent_class = NULL;
static alterator_ctl_module_t packages_module     = {0};
static gboolean is_dbus_call_error                = FALSE;

static void packages_module_class_init(AlteratorCtlPackagesModuleClass* klass);
static void packages_ctl_class_finalize(GObject* klass);

static void packages_module_alterator_interface_init(gpointer iface, gpointer iface_data);
static void packages_module_alterator_interface_finalize(gpointer iface, gpointer iface_data);

static AlteratorCtlPackagesModule* packages_module_new(gpointer app);
static void packages_module_free(AlteratorCtlPackagesModule* module);

static void commands_hash_tables_new(AlteratorCtlPackagesModule* module);
static void commands_hash_tables_finalize(AlteratorCtlPackagesModule* module);

static void fill_command_hash_tables(GHashTable** commands);

static int packages_module_rpm_subcommand(AlteratorCtlPackagesModule* module,
                                          alteratorctl_ctx_t** ctx);
static int packages_module_apt_subcommand(AlteratorCtlPackagesModule* module,
                                          alteratorctl_ctx_t** ctx);
static int packages_module_repo_subcommand(AlteratorCtlPackagesModule* module,
                                           alteratorctl_ctx_t** ctx);
static int packages_module_help_subcommand(AlteratorCtlPackagesModule* module,
                                           alteratorctl_ctx_t** ctx);

void packages_module_apt_sigint_handler(int signal, siginfo_t* info, void* context);

static void packages_module_rpm_print_help(void);
static void packages_module_apt_print_help(void);
static void packages_module_repo_print_help(void);

static int packages_module_parse_options(AlteratorCtlPackagesModule* module, int* argc,
                                         char** argv);
static int packages_module_parse_arguments(AlteratorCtlPackagesModule* module, int argc,
                                           char** argv, alteratorctl_ctx_t** ctx);
static int packages_apt_module_parse_options(AlteratorCtlPackagesModule* module, int* argc,
                                             char** argv);
static int packages_module_parse_apt_arguments(AlteratorCtlPackagesModule* module, int argc,
                                               char** argv, alteratorctl_ctx_t** ctx);
static int packages_module_parse_rpm_arguments(AlteratorCtlPackagesModule* module, int argc,
                                               char** argv, alteratorctl_ctx_t** ctx);
static int packages_module_parse_repo_arguments(AlteratorCtlPackagesModule* module, int argc,
                                                char** argv, alteratorctl_ctx_t** ctx);

static gint packages_module_sort_result(gconstpointer a, gconstpointer b);

static int packages_module_handle_results(AlteratorCtlPackagesModule* module,
                                          alteratorctl_ctx_t** ctx);
static int packages_module_handle_rpm_results(AlteratorCtlPackagesModule* module,
                                              alteratorctl_ctx_t** ctx);
static int packages_module_handle_apt_results(AlteratorCtlPackagesModule* module,
                                              alteratorctl_ctx_t** ctx);
static int packages_module_handle_repo_results(AlteratorCtlPackagesModule* module,
                                               alteratorctl_ctx_t** ctx);

static int packages_module_validate_alterator_entry(AlteratorGDBusSource* source,
                                                    GNode* alterator_entry_data,
                                                    const gchar* submodule);

static void packages_apt_run_stdout_simple_stream_handler(GDBusConnection* connection,
                                                          const gchar* sender_name,
                                                          const gchar* object_path,
                                                          const gchar* interface_name,
                                                          const gchar* signal_name,
                                                          GVariant* parameters, gpointer user_data);

static void packages_apt_run_stderr_simple_stream_handler(GDBusConnection* connection,
                                                          const gchar* sender_name,
                                                          const gchar* object_path,
                                                          const gchar* interface_name,
                                                          const gchar* signal_name,
                                                          GVariant* parameters, gpointer user_data);

static int packages_module_apt_check_reinstall_package(AlteratorCtlPackagesModule* module,
                                                       const gchar* package, gboolean* accepted);

static GHashTable* json_array_to_hash_table(JsonArray* array);

static GPtrArray* copy_hash_table_str_keys_to_ptr_array(GHashTable* table);

static int get_check_apply_result(AlteratorCtlPackagesModule* module, const gchar* packages,
                                  check_apply_result** result);

static int packages_module_apt_check_apply_package_info(
    AlteratorCtlPackagesModule* module, const gchar* packages,
    const gchar** optional_calculated_packages_to_install,
    const gchar** optional_calculated_packages_to_remove,
    const gchar** optional_calculated_packages_extra_remove, gboolean* accepted);

// Handlers for rpm resuls
static int packages_module_handle_rpm_files_results(AlteratorCtlPackagesModule* module,
                                                    alteratorctl_ctx_t** ctx);
static int packages_module_handle_rpm_info_results(AlteratorCtlPackagesModule* module,
                                                   alteratorctl_ctx_t** ctx);
static int packages_module_handle_rpm_install_results(AlteratorCtlPackagesModule* module,
                                                      alteratorctl_ctx_t** ctx);
static int packages_module_handle_rpm_list_results(AlteratorCtlPackagesModule* module,
                                                   alteratorctl_ctx_t** ctx);
static int packages_module_handle_rpm_package_info_results(AlteratorCtlPackagesModule* module,
                                                           alteratorctl_ctx_t** ctx);
static int packages_module_handle_rpm_remove_results(AlteratorCtlPackagesModule* module,
                                                     alteratorctl_ctx_t** ctx);

// Handlers for apt results
static int packages_module_handle_apt_info_results(AlteratorCtlPackagesModule* module,
                                                   alteratorctl_ctx_t** ctx);
static int packages_module_handle_apt_list_all_packages_results(AlteratorCtlPackagesModule* module,
                                                                alteratorctl_ctx_t** ctx);
static int packages_module_handle_apt_reinstall_results(AlteratorCtlPackagesModule* module,
                                                        alteratorctl_ctx_t** ctx);
static int packages_module_handle_apt_search_results(AlteratorCtlPackagesModule* module,
                                                     alteratorctl_ctx_t** ctx);
static int packages_module_handle_apt_last_update_results(AlteratorCtlPackagesModule* module,
                                                          alteratorctl_ctx_t** ctx);

// Handlers for repo results
static int packages_module_handle_repo_add_results(AlteratorCtlPackagesModule* module,
                                                   alteratorctl_ctx_t** ctx);
static int packages_module_handle_repo_info_results(AlteratorCtlPackagesModule* module,
                                                    alteratorctl_ctx_t** ctx);
static int packages_module_handle_repo_list_results(AlteratorCtlPackagesModule* module,
                                                    alteratorctl_ctx_t** ctx);
static int packages_module_handle_repo_remove_results(AlteratorCtlPackagesModule* module,
                                                      alteratorctl_ctx_t** ctx);

static int packages_module_validate_object_and_iface(AlteratorCtlPackagesModule* module,
                                                     const gchar* object, const gchar* iface);

GType alterator_ctl_packages_module_get_type(void)
{
    static GType packages_module_type = 0;

    if (!packages_module_type)
    {
        static const GTypeInfo packages_module_info =
            {sizeof(AlteratorCtlPackagesModuleClass),     /* class structure size */
             NULL,                                        /* base class initializer */
             NULL,                                        /* base class finalizer */
             (GClassInitFunc) packages_module_class_init, /* class initializer */
             NULL,                                        /* class finalizer */
             NULL,                                        /* class data */
             sizeof(AlteratorCtlPackagesModule),          /* instance structure size */
             1,                                           /* preallocated instances */
             NULL,                                        /* instance initializers */
             NULL};

        const GInterfaceInfo alterator_module_interface_info = {
            (GInterfaceInitFunc) packages_module_alterator_interface_init, /* interface_init */
            (GInterfaceFinalizeFunc)
                packages_module_alterator_interface_finalize, /* interface_finalize */
            NULL                                              /* interface_data */
        };

        packages_module_type = g_type_register_static(G_TYPE_OBJECT, /* parent class */
                                                      "AlteratorCtlPackagesModule",
                                                      &packages_module_info, 0);

        g_type_add_interface_static(packages_module_type, TYPE_ALTERATOR_CTL_MODULE,
                                    &alterator_module_interface_info);
    }

    return packages_module_type;
}

static void packages_module_class_init(AlteratorCtlPackagesModuleClass* klass)
{
    GObjectClass* obj_class = G_OBJECT_CLASS(klass);

    obj_class->finalize = packages_ctl_class_finalize;

    packages_module_parent_class = g_type_class_peek_parent(klass);
}
static void packages_ctl_class_finalize(GObject* klass)
{
    AlteratorCtlPackagesModuleClass* obj = (AlteratorCtlPackagesModuleClass*) klass;

    G_OBJECT_CLASS(packages_module_parent_class)->finalize(klass);
}

static void packages_module_alterator_interface_init(gpointer iface, gpointer iface_data)
{
    AlteratorCtlModuleInterface* interface = iface;

    interface->run_with_args = packages_module_run_with_args;

    interface->run = packages_module_run;

    interface->print_help = packages_module_print_help;
}
static void packages_module_alterator_interface_finalize(gpointer iface, gpointer iface_data) {}

alterator_ctl_module_t* get_packages_module()
{
    int ret                               = 0;
    static gsize packages_ctl_module_init = 0;
    if (g_once_init_enter(&packages_ctl_module_init))
    {
        gsize module_id_size = g_strlcpy(packages_module.id, ALTERATOR_CTL_PACKAGES_MODULE_NAME,
                                         strlen(ALTERATOR_CTL_PACKAGES_MODULE_NAME) + 1);

        if (module_id_size != strlen(ALTERATOR_CTL_PACKAGES_MODULE_NAME))
        {
            g_printerr(
                _("Internal error in get_packages_module: unvaliable id of packages module.\n"));
            ERR_EXIT();
        }

        packages_module.new_object_func  = (gpointer) packages_module_new;
        packages_module.free_object_func = (gpointer) packages_module_free;

        gsize tmp = 42;

        g_once_init_leave(&packages_ctl_module_init, tmp);
    }

    return &packages_module;

end:
    return NULL;
}

static AlteratorCtlPackagesModule* packages_module_new(gpointer app)
{
    AlteratorCtlPackagesModule* object = g_object_new(TYPE_ALTERATOR_CTL_PACKAGES_MODULE, NULL);

    commands_hash_tables_new(object);

    object->alterator_ctl_app = (AlteratorCtlApp*) app;

    object->gdbus_source = alterator_gdbus_source_new(object->alterator_ctl_app->arguments->verbose,
                                                      G_BUS_TYPE_SYSTEM);

    return object;
}

static void packages_module_free(AlteratorCtlPackagesModule* module)
{
    commands_hash_tables_finalize(module);

    if (module->gdbus_source)
    {
        alterator_gdbus_source_free(module->gdbus_source);
    }

    g_object_unref(module);
}

static void commands_hash_tables_new(AlteratorCtlPackagesModule* module)
{
    module->commands = (GHashTable**) g_malloc0(PACKAGES_SUBMODULES_AMOUNTH * sizeof(GHashTable*));
    if (!module->commands)
    {
        g_printerr(_("Error of creating commands HashTables in packages module\n"));
        return;
    }

    for (int i = 0; i < PACKAGES_SUBMODULES_AMOUNTH; i++)
        module->commands[i] = g_hash_table_new(g_str_hash, g_str_equal);

    fill_command_hash_tables(module->commands);
}

static void commands_hash_tables_finalize(AlteratorCtlPackagesModule* module)
{
    for (int i = 0; i < PACKAGES_SUBMODULES_AMOUNTH; i++)
        g_hash_table_destroy(module->commands[i]);

    g_free(module->commands);
}

static void fill_command_hash_tables(GHashTable** command)
{
    for (int i = 0;
         i < sizeof(packages_module_subcommands_list) / sizeof(packages_module_subcommands_t); i++)
        g_hash_table_insert(command[PACKAGES_HASH_TABLE],
                            packages_module_subcommands_list[i].subcommand,
                            &packages_module_subcommands_list[i].id);

    for (int i = 0; i < sizeof(packages_module_apt_subcommands_list)
                            / sizeof(packages_module_apt_subcommands_t);
         i++)
        g_hash_table_insert(command[APT_HASH_TABLE],
                            packages_module_apt_subcommands_list[i].subcommand,
                            &packages_module_apt_subcommands_list[i].id);

    for (int i = 0; i < sizeof(packages_module_rpm_subcommands_list)
                            / sizeof(packages_module_rpm_subcommands_t);
         i++)
        g_hash_table_insert(command[RPM_HASH_TABLE],
                            packages_module_rpm_subcommands_list[i].subcommand,
                            &packages_module_rpm_subcommands_list[i].id);

    for (int i = 0; i < sizeof(packages_module_repo_subcommands_list)
                            / sizeof(packages_module_repo_subcommands_t);
         i++)
        g_hash_table_insert(command[REPO_HASH_TABLE],
                            packages_module_repo_subcommands_list[i].subcommand,
                            &packages_module_repo_subcommands_list[i].id);
}

int packages_module_run(gpointer self, gpointer data)
{
    int ret = 0;

    AlteratorCtlPackagesModule* module = ALTERATOR_CTL_PACKAGES_MODULE(self);

    if (!self)
    {
        g_printerr(_("Internal error in packages module run: *module is NULL.\n"));
        ERR_EXIT();
    }

    enum alteratorctl_help_type help_type = module->alterator_ctl_app->arguments->help_type;
    if (help_type == ALTERATORCTL_MODULE_HELP || help_type == ALTERATORCTL_SUBMODULE_HELP)
        goto end;

    if (!data)
    {
        g_printerr(_("Internal error in packages module run: *context is NULL.\n"));
        ERR_EXIT();
    }

    alteratorctl_ctx_t* ctx = (alteratorctl_ctx_t*) data;
    GVariant* submodule     = g_variant_get_child_value(ctx->subcommands_ids, 0);
    int submodule_id        = g_variant_get_int32(submodule);

    switch (submodule_id)
    {
    case PACKAGES_RPM:
        if (packages_module_rpm_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;
    case PACKAGES_APT:
        if (packages_module_apt_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;
    case PACKAGES_REPO:
        if (packages_module_repo_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;

    default:
        g_printerr(_("Try to run unknown packages subcommand.\n"));
        ERR_EXIT();
    }

end:
    return ret;
}

static int packages_module_rpm_subcommand(AlteratorCtlPackagesModule* module,
                                          alteratorctl_ctx_t** ctx)
{
    int ret                      = 0;
    const gchar* parameter1      = NULL;
    GVariant* subcommand_variant = NULL;
    GError* dbus_call_error      = NULL;
    gchar* info_result           = NULL;

    if (!module)
    {
        g_printerr(_("No packages module.\n"));
        ERR_EXIT();
    }

    if (module->alterator_ctl_app->arguments->help_type == ALTERATORCTL_SUBMODULE_HELP)
        goto end;

    if (!(*ctx))
    {
        g_printerr(_("NULL context in rpm module.\n"));
        ERR_EXIT();
    }

    dbus_ctx_t* dbus_ctx = NULL;

    if (packages_module_validate_object_and_iface(module, PACKAGES_RPM_OBJECT_PATH,
                                                  PACKAGES_RPM_INTERFACE_NAME)
        < 0)
        ERR_EXIT();

    subcommand_variant = g_variant_get_child_value((*ctx)->subcommands_ids, 1);
    int subcommand_id  = g_variant_get_int32(subcommand_variant);

    g_variant_get((*ctx)->parameters, "(ms)", &parameter1);

    switch (subcommand_id)
    {
    case RPM_FILES:
        if (!parameter1)
        {
            g_printerr(_("Not enough parameters for packages rpm files subcommand.\n"));
            packages_module_rpm_print_help();
            ERR_EXIT();
        }
        dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, PACKAGES_RPM_OBJECT_PATH,
                                 PACKAGES_RPM_INTERFACE_NAME, PACKAGES_RPM_FILES_METHOD_NAME,
                                 module->alterator_ctl_app->arguments->verbose);
        if (!dbus_ctx)
        {
            g_printerr(_("Can't allocate dbus_ctx_t in packages_module_rpm_subcommand\n"));
            ERR_EXIT();
        }

        dbus_ctx->parameters = g_variant_new("(s)", parameter1);
        dbus_ctx->reply_type = G_VARIANT_TYPE("(asasi)");

        (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

        break;

    case RPM_INFO:
    {
        module->gdbus_source->alterator_gdbus_source_get_text_of_alterator_entry_by_path(
            module->gdbus_source, PACKAGES_RPM_OBJECT_PATH, PACKAGES_RPM_INTERFACE_NAME,
            &info_result);

        if (!info_result)
        {
            g_printerr(_("Error while getting info of packages rpm submodule on alterator entry "
                         "format: result is NULL.\n"));

            ERR_EXIT();
        }

        (*ctx)->free_results = g_free;
        (*ctx)->results      = info_result;

        return ret;
    }
    case RPM_INSTALL:
        if (!parameter1)
        {
            g_printerr(_("Not enough parameters for packages rpm install subcommand.\n"));
            packages_module_rpm_print_help();
            ERR_EXIT();
        }

        dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, PACKAGES_RPM_OBJECT_PATH,
                                 PACKAGES_RPM_INTERFACE_NAME, PACKAGES_RPM_INSTALL_METHOD_NAME,
                                 module->alterator_ctl_app->arguments->verbose);

        if (!dbus_ctx)
        {
            g_printerr(_("Can't allocate dbus_ctx_t in packages_module_rpm_subcommand\n"));
            ERR_EXIT();
        }

        dbus_ctx->parameters = g_variant_new("(s)", parameter1);
        dbus_ctx->reply_type = G_VARIANT_TYPE("(asi)");

        (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;
        break;

    case RPM_LIST:
        dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, PACKAGES_RPM_OBJECT_PATH,
                                 PACKAGES_RPM_INTERFACE_NAME, PACKAGES_RPM_LIST_METHOD_NAME,
                                 module->alterator_ctl_app->arguments->verbose);

        if (!dbus_ctx)
        {
            g_printerr(_("Can't allocate dbus_ctx_t in packages_module_rpm_subcommand\n"));
            ERR_EXIT();
        }

        dbus_ctx->reply_type = G_VARIANT_TYPE("(asasi)");

        (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

        break;

    case RPM_PACKAGE_INFO:
        if (!parameter1)
        {
            g_printerr(_("Not enough parameters for packages rpm packageinfo subcommand.\n"));
            packages_module_rpm_print_help();
            ERR_EXIT();
        }

        dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, PACKAGES_RPM_OBJECT_PATH,
                                 PACKAGES_RPM_INTERFACE_NAME, PACKAGES_RPM_PACKAGE_INFO_METHOD_NAME,
                                 module->alterator_ctl_app->arguments->verbose);

        if (!dbus_ctx)
        {
            g_printerr(_("Can't allocate dbus_ctx_t in packages_module_rpm_subcommand\n"));
            ERR_EXIT();
        }

        dbus_ctx->parameters = g_variant_new("(s)", parameter1);
        dbus_ctx->reply_type = G_VARIANT_TYPE("(asasi)");

        (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

        break;

    case RPM_REMOVE:
        if (!parameter1)
        {
            g_printerr(_("Not enough parameters for packages rpm remove subcommand.\n"));
            packages_module_rpm_print_help();
            ERR_EXIT();
        }

        dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, PACKAGES_RPM_OBJECT_PATH,
                                 PACKAGES_RPM_INTERFACE_NAME, PACKAGES_RPM_REMOVE_METHOD_NAME,
                                 module->alterator_ctl_app->arguments->verbose);

        if (!dbus_ctx)
        {
            g_printerr(_("Can't allocate dbus_ctx_t in packages_module_rpm_subcommand\n"));
            ERR_EXIT();
        }

        dbus_ctx->parameters = g_variant_new("(s)", parameter1);
        dbus_ctx->reply_type = G_VARIANT_TYPE("(asi)");

        (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;
        break;

    default:
        ERR_EXIT();
        break;
    }

    module->gdbus_source->call(module->gdbus_source, dbus_ctx, &dbus_call_error);
    if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    if (dbus_ctx->result)
        (*ctx)->results = (gpointer) g_variant_ref(dbus_ctx->result);
    else
        g_printerr("%s\n", dbus_call_error->message);

    dbus_ctx_free(dbus_ctx);

end:
    if (subcommand_variant)
        g_variant_unref(subcommand_variant);

    if (parameter1)
        g_free((gpointer) parameter1);

    g_clear_error(&dbus_call_error);

    return ret;
}
static int packages_module_apt_subcommand(AlteratorCtlPackagesModule* module,
                                          alteratorctl_ctx_t** ctx)
{
    int ret                            = 0;
    gchar* parameter1                  = NULL;
    GVariant* subcommand_variant       = NULL;
    gchar* info_result                 = NULL;
    dbus_ctx_t* dbus_ctx               = NULL;
    GError* dbus_call_error            = NULL;
    GPtrArray* signals                 = NULL;
    subscribe_signals_t* stdout_signal = NULL;
    subscribe_signals_t* stderr_signal = NULL;
    gboolean is_accepted               = FALSE;

    if (!module)
    {
        g_printerr(_("No packages module.\n"));
        ERR_EXIT();
    }

    if (module->alterator_ctl_app->arguments->help_type == ALTERATORCTL_SUBMODULE_HELP)
        goto end;

    if (!(*ctx))
    {
        g_printerr(_("NULL context in apt module.\n"));
        ERR_EXIT();
    }

    if (packages_module_validate_object_and_iface(module, PACKAGES_APT_OBJECT_PATH,
                                                  PACKAGES_APT_INTERFACE_NAME)
        < 0)
        ERR_EXIT();

    // for some subcommands needed a parameter
    subcommand_variant = g_variant_get_child_value((*ctx)->subcommands_ids, 1);
    int subcommand_id  = g_variant_get_int32(subcommand_variant);

    // Create dbus context and run AlteratorCtlGDBusSource with it.
    // Copy result from context to packages context
    g_variant_get((*ctx)->parameters, "(ms)", &parameter1);
    switch ((subcommand_id & APT_APPLY_ASYNC) == APT_APPLY_ASYNC ? APT_APPLY_ASYNC : subcommand_id)
    {
    case APT_INFO:
        module->gdbus_source->alterator_gdbus_source_get_text_of_alterator_entry_by_path(
            module->gdbus_source, PACKAGES_APT_OBJECT_PATH, PACKAGES_APT_INTERFACE_NAME,
            &info_result);

        if (!info_result)
        {
            g_printerr(_("Error while getting info of packages apt submodule on alterator entry "
                         "format: result is NULL.\n"));
            ERR_EXIT();
        }

        (*ctx)->free_results = g_free;
        (*ctx)->results      = info_result;

        break;

    case APT_LIST_ALL_PACKAGES:
        dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, PACKAGES_APT_OBJECT_PATH,
                                 PACKAGES_APT_INTERFACE_NAME,
                                 PACKAGES_APT_LIST_ALL_PACKAGES_METHOD_NAME,
                                 module->alterator_ctl_app->arguments->verbose);

        if (!dbus_ctx)
        {
            g_printerr(_("Can't allocate dbus_ctx_t in packages_module_apt_subcommand\n"));
            ERR_EXIT();
        }

        dbus_ctx->reply_type = G_VARIANT_TYPE("(asasi)");

        (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

        module->gdbus_source->call(module->gdbus_source, dbus_ctx, &dbus_call_error);

        if (dbus_ctx->result)
            (*ctx)->results = (gpointer) g_variant_ref(dbus_ctx->result);
        else
            g_printerr("%s\n", dbus_call_error->message);

        break;

    case APT_CHECK_APPLY_PRIV:
        if (!parameter1)
        {
            g_printerr(_("Not enough parameters for packages apt install subcommand.\n"));
            packages_module_apt_print_help();
            ERR_EXIT();
        }

        check_apply_result* result = NULL;
        if (get_check_apply_result(module, parameter1, &result) != 0)
            ERR_EXIT();

        GVariant* pack_result[] =
            {g_variant_new_maybe(G_VARIANT_TYPE("as"),
                                 result->to_install
                                     ? g_variant_new_strv((const gchar* const*)
                                                              result->to_install->pdata,
                                                          result->to_install->len)
                                     : NULL),
             g_variant_new_maybe(G_VARIANT_TYPE("as"),
                                 result->to_remove ? g_variant_new_strv((const gchar* const*) result
                                                                            ->to_remove->pdata,
                                                                        result->to_remove->len)
                                                   : NULL),
             g_variant_new_maybe(G_VARIANT_TYPE("as"),
                                 result->extra_remove
                                     ? g_variant_new_strv((const gchar* const*)
                                                              result->extra_remove->pdata,
                                                          result->extra_remove->len)
                                     : NULL),
             g_variant_new_int32(result->exit_code)};

        (*ctx)->results = g_variant_new_tuple(pack_result, 4);

        break;

    case APT_APPLY_ASYNC:
    {
        GVariant* exit_code       = NULL;
        gchar* packages_list      = NULL;
        GVariant* additional_data = (GVariant*) (*ctx)->additional_data;

        if (!parameter1)
        {
            g_printerr(_("Not enough parameters for packages apt install subcommand.\n"));
            packages_module_apt_print_help();
            if (additional_data)
                g_variant_unref(additional_data);
            ERR_EXIT();
        }

        if (additional_data
            && !g_variant_is_of_type(additional_data, G_VARIANT_TYPE("(masmasmasbb)")))
        {
            if (subcommand_id == (APT_APPLY_ASYNC | APT_REMOVE))
                g_printerr(
                    _("Invalid type of additional data for packages apt remove subcommand.\n"));
            else if (subcommand_id == (APT_APPLY_ASYNC | APT_REMOVE))
                g_printerr(
                    _("Invalid type of additional data for packages apt install subcommand.\n"));
            if (additional_data)
                g_variant_unref(additional_data);
            ERR_EXIT();
        }

        gchar** additional_data_to_install_pckgs   = NULL;
        gchar** additional_data_to_remove_pckgs    = NULL;
        gchar** additional_data_extra_remove_pckgs = NULL;
        if (additional_data)
            g_variant_get(additional_data, "(m^asm^asm^asbb)", &additional_data_to_install_pckgs,
                          &additional_data_to_remove_pckgs, &additional_data_extra_remove_pckgs,
                          &apt_allow_remove_manually, &apt_force_transaction_commit);

        if (additional_data)
            g_variant_unref(additional_data);

        if (subcommand_id == (APT_APPLY_ASYNC | APT_REMOVE))
        {
            if (additional_data_to_remove_pckgs && g_strv_length(additional_data_to_remove_pckgs))
            {
                gchar* additional_data_to_remove_pckgs_str =
                    g_strjoinv("- ", additional_data_to_remove_pckgs);
                gchar* tmp                          = additional_data_to_remove_pckgs_str;
                additional_data_to_remove_pckgs_str = g_strconcat(tmp, "-", NULL);
                g_free(tmp);
                packages_list = g_strdup(additional_data_to_remove_pckgs_str);
                g_free(additional_data_to_remove_pckgs_str);
            }

            if (additional_data_extra_remove_pckgs
                && g_strv_length(additional_data_extra_remove_pckgs))
            {
                gchar* additional_data_extra_remove_pckgs_str =
                    g_strjoinv("- ", additional_data_extra_remove_pckgs);

                gchar* tmp = additional_data_extra_remove_pckgs_str;
                additional_data_extra_remove_pckgs_str =
                    g_strconcat(additional_data_to_remove_pckgs
                                        && g_strv_length(additional_data_to_remove_pckgs)
                                    ? " "
                                    : "",
                                tmp, "-", NULL);

                g_free(tmp);
                tmp           = packages_list;
                packages_list = g_strconcat(tmp ? tmp : "", additional_data_extra_remove_pckgs_str,
                                            NULL);
                g_free(tmp);
                g_free(additional_data_extra_remove_pckgs_str);
            }

            // Remove specified package if we have explicit packages apt install/remove call (not
            // from another module)
            if (!packages_list || (packages_list && !strlen(packages_list)))
            {
                gchar** tmp_strv = g_regex_split_simple("\\s+", g_strstrip(parameter1), 0, 0);
                packages_list    = g_strjoinv("- ", tmp_strv);
                gchar* tmp       = packages_list;
                packages_list    = g_strconcat(tmp, "-", NULL);
                g_free(tmp);
                g_strfreev(tmp_strv);
            }
        }
        else
            packages_list = g_strdup(parameter1);

        if (!additional_data_to_install_pckgs && !additional_data_to_remove_pckgs
            && !additional_data_extra_remove_pckgs)
        {
            check_apply_result* check_apply_info;
            if (get_check_apply_result(module, packages_list, &check_apply_info) != 0)
                ERR_EXIT();

            if (!check_apply_info)
            {
                g_printerr(_("Can't check apply packages. Empty result.\n"));
                ERR_EXIT();
            }

            if (check_apply_info->to_install)
            {
                GStrvBuilder* builder = g_strv_builder_new();
                for (gsize i = 0; i < check_apply_info->to_install->len; i++)
                    g_strv_builder_add(builder, (gchar*) check_apply_info->to_install->pdata[i]);
                additional_data_to_install_pckgs = g_strv_builder_unref_to_strv(builder);
            }

            if (check_apply_info->to_remove)
            {
                GStrvBuilder* builder = g_strv_builder_new();
                for (gsize i = 0; i < check_apply_info->to_remove->len; i++)
                    g_strv_builder_add(builder, (gchar*) check_apply_info->to_remove->pdata[i]);
                additional_data_to_remove_pckgs = g_strv_builder_unref_to_strv(builder);
            }

            if (check_apply_info->extra_remove)
            {
                GStrvBuilder* builder = g_strv_builder_new();
                for (gsize i = 0; i < check_apply_info->extra_remove->len; i++)
                    g_strv_builder_add(builder, (gchar*) check_apply_info->extra_remove->pdata[i]);
                additional_data_extra_remove_pckgs = g_strv_builder_unref_to_strv(builder);
            }

            check_apply_result_free(check_apply_info);
        }

        if (packages_module_apt_check_apply_package_info(module, packages_list,
                                                         (const gchar**)
                                                             additional_data_to_install_pckgs,
                                                         (const gchar**)
                                                             additional_data_to_remove_pckgs,
                                                         (const gchar**)
                                                             additional_data_extra_remove_pckgs,
                                                         &is_accepted)
            < 0)
        {
            g_free(packages_list);
            ERR_EXIT();
        }

        if (!is_accepted)
        {
            if (subcommand_id == (APT_APPLY_ASYNC | APT_REMOVE))
                g_free(packages_list);
            return ret;
        }

        signals = g_ptr_array_new();

        stdout_signal = g_malloc0(sizeof(subscribe_signals_t));
        if (!stdout_signal)
        {
            g_printerr(_("Calling of apt %s failed. Can't allocate memory for stdout signal.\n"),
                       subcommand_id == (APT_APPLY_ASYNC | APT_INSTALL)
                           ? "install"
                           : (subcommand_id == (APT_APPLY_ASYNC | APT_REMOVE) ? "remove"
                                                                              : "apply async"));
            if (subcommand_id == (APT_APPLY_ASYNC | APT_REMOVE))
                g_free(packages_list);
            ERR_EXIT();
        }

        stderr_signal = g_malloc0(sizeof(subscribe_signals_t));
        if (!stderr_signal)
        {
            g_printerr(_("Calling of apt %s failed. Can't allocate memory for stderr signal.\n"),
                       subcommand_id == (APT_APPLY_ASYNC | APT_INSTALL)
                           ? "install"
                           : ((subcommand_id == (APT_APPLY_ASYNC | APT_REMOVE) ? "remove"
                                                                               : "apply async")));
            if (subcommand_id == (APT_APPLY_ASYNC | APT_REMOVE))
                g_free(packages_list);
            ERR_EXIT();
        }

        dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, PACKAGES_APT_OBJECT_PATH,
                                 PACKAGES_APT_INTERFACE_NAME, PACKAGES_APT_APPLY_ASYNC_METHOD_NAME,
                                 module->alterator_ctl_app->arguments->verbose);

        if (!dbus_ctx)
        {
            g_printerr(_("Can't allcate dbus_ctx_t in packages_module_apt_subcommand\n"));
            if (subcommand_id == (APT_APPLY_ASYNC | APT_REMOVE))
                g_free(packages_list);
            ERR_EXIT();
        }

        dbus_ctx_set_timeout(dbus_ctx, PACKAGES_ONE_DAY_TIMEOUT);

        stdout_signal->signal_name = PACKAGES_APT_INSTALL_STDOUT_SIGNAL_NAME;
        stdout_signal->callback    = &packages_apt_run_stdout_simple_stream_handler;
        stdout_signal->user_data   = dbus_ctx;

        stderr_signal->signal_name = PACKAGES_APT_INSTALL_STDERR_SIGNAL_NAME;
        stderr_signal->callback    = &packages_apt_run_stderr_simple_stream_handler;
        stderr_signal->user_data   = dbus_ctx;

        g_ptr_array_add(signals, stdout_signal);
        g_ptr_array_add(signals, stderr_signal);

        if (apt_allow_remove_manually && additional_data_extra_remove_pckgs)
        {
            gchar* exclude_packages        = g_strjoinv(" ", additional_data_extra_remove_pckgs);
            gchar* quoted_exclude_packages = g_strdup_printf("\"%s\"", exclude_packages);
            dbus_ctx->parameters = g_variant_new("(ss)", quoted_exclude_packages, packages_list);
            g_free(exclude_packages);
            g_free(quoted_exclude_packages);
        }
        else
            dbus_ctx->parameters = g_variant_new("(ss)", "\"''\"", packages_list);

        g_free(packages_list);

        dbus_ctx->reply_type = G_VARIANT_TYPE("(i)");

        module->gdbus_source->call_with_signals(module->gdbus_source, dbus_ctx, signals,
                                                &dbus_call_error);

        if (dbus_call_error)
        {
            g_printerr("%s\n", dbus_call_error->message);
            is_dbus_call_error = TRUE;
            ERR_EXIT();
        }

        exit_code = g_variant_get_child_value(dbus_ctx->result, 0);
        ret       = g_variant_get_int32(exit_code);
        if (dbus_ctx->result)
            (*ctx)->results = (gpointer) g_variant_ref(dbus_ctx->result);

        g_variant_unref(exit_code);

        break;
    }

    case APT_REINSTALL:
    {
        GVariant* exit_code = NULL;

        if (!parameter1)
        {
            g_printerr(_("Not enough parameters for packages apt reinstall subcommand.\n"));
            packages_module_apt_print_help();
            ERR_EXIT();
        }

        if (packages_module_apt_check_reinstall_package(module, parameter1, &is_accepted) != 0)
            ERR_EXIT();

        if (!is_accepted)
            return ret;

        signals = g_ptr_array_new();

        stdout_signal = g_malloc0(sizeof(subscribe_signals_t));
        if (!stdout_signal)
        {
            g_printerr(_("Calling of apt reinstall async failed. Can't allocate memory for stdout "
                         "signal.\n"));
            ERR_EXIT();
        }

        stderr_signal = g_malloc0(sizeof(subscribe_signals_t));
        if (!stderr_signal)
        {
            g_printerr(_("Calling of apt reinstall async failed. Can't allocate memory for stderr "
                         "signal.\n"));
            ERR_EXIT();
        }

        dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, PACKAGES_APT_OBJECT_PATH,
                                 PACKAGES_APT_INTERFACE_NAME,
                                 PACKAGES_APT_REINSTALL_ASYNC_METHOD_NAME,
                                 module->alterator_ctl_app->arguments->verbose);

        if (!dbus_ctx)
        {
            g_printerr(_("Can't allocate dbus_ctx_t in packages_module_apt_subcommand\n"));
            ERR_EXIT();
        }

        stdout_signal->signal_name = PACKAGES_APT_REINSTALL_STDOUT_SIGNAL_NAME;
        stdout_signal->callback    = &packages_apt_run_stdout_simple_stream_handler;
        stdout_signal->user_data   = dbus_ctx;

        stderr_signal->signal_name = PACKAGES_APT_REINSTALL_STDERR_SIGNAL_NAME;
        stderr_signal->callback    = &packages_apt_run_stderr_simple_stream_handler;
        stderr_signal->user_data   = dbus_ctx;

        g_ptr_array_add(signals, stdout_signal);
        g_ptr_array_add(signals, stderr_signal);

        dbus_ctx_set_timeout(dbus_ctx, PACKAGES_ONE_DAY_TIMEOUT);

        if (!parameter1)
        {
            g_printerr(_("Not enough parameters for packages apt reinstall subcommand.\n"));
            packages_module_apt_print_help();
            ERR_EXIT();
        }

        dbus_ctx->parameters = g_variant_new("(s)", parameter1);
        dbus_ctx->reply_type = G_VARIANT_TYPE("(asi)");

        (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

        module->gdbus_source->call_with_signals(module->gdbus_source, dbus_ctx, signals,
                                                &dbus_call_error);
        if (dbus_call_error)
        {
            g_printerr("%s\n", dbus_call_error->message);
            is_dbus_call_error = TRUE;
            ERR_EXIT();
        }

        exit_code = g_variant_get_child_value(dbus_ctx->result, 0);
        ret       = g_variant_get_int32(exit_code);
        if (dbus_ctx->result)
            (*ctx)->results = (gpointer) g_variant_ref(dbus_ctx->result);

        g_variant_unref(exit_code);

        break;
    }

    case APT_SEARCH:
        if (!parameter1)
        {
            g_printerr(_("Not enough parameters for packages apt search subcommand.\n"));
            packages_module_apt_print_help();
            ERR_EXIT();
        }

        dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, PACKAGES_APT_OBJECT_PATH,
                                 PACKAGES_APT_INTERFACE_NAME, PACKAGES_APT_SEARCH_METHOD_NAME,
                                 module->alterator_ctl_app->arguments->verbose);

        if (!dbus_ctx)
        {
            g_printerr(_("Can't allocate dbus_ctx_t in packages_module_apt_subcommand\n"));
            ERR_EXIT();
        }

        if (!parameter1)
        {
            g_printerr(_("Not enough parameters for packages apt search subcommand.\n"));
            packages_module_apt_print_help();
            ERR_EXIT();
        }

        dbus_ctx->parameters = g_variant_new("(s)", parameter1);
        dbus_ctx->reply_type = G_VARIANT_TYPE("(asasi)");

        (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

        module->gdbus_source->call(module->gdbus_source, dbus_ctx, &dbus_call_error);

        if (dbus_ctx->result)
            (*ctx)->results = (gpointer) g_variant_ref(dbus_ctx->result);
        else
            g_printerr("%s\n", dbus_call_error->message);

        break;

    case APT_UPDATE:
    {
        GVariant* exit_code = NULL;

        signals = g_ptr_array_new();

        stdout_signal = g_malloc0(sizeof(subscribe_signals_t));
        if (!stdout_signal)
        {
            g_printerr(_(
                "Calling of apt update async failed. Can't allocate memory for stdout signal.\n"));
            ERR_EXIT();
        }

        stderr_signal = g_malloc0(sizeof(subscribe_signals_t));
        if (!stderr_signal)
        {
            g_printerr(_(
                "Calling of apt update async failed. Can't allocate memory for stderr signal.\n"));
            ERR_EXIT();
        }

        dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, PACKAGES_APT_OBJECT_PATH,
                                 PACKAGES_APT_INTERFACE_NAME, PACKAGES_APT_UPDATE_ASYNC_METHOD_NAME,
                                 module->alterator_ctl_app->arguments->verbose);

        if (!dbus_ctx)
        {
            g_printerr(_("Can't allocate dbus_ctx_t in packages_module_apt_subcommand\n"));
            ERR_EXIT();
        }

        stdout_signal->signal_name = PACKAGES_APT_UPDATE_ASYNC_STDOUT_SIGNAL_NAME;
        stdout_signal->callback    = &packages_apt_run_stdout_simple_stream_handler;
        stdout_signal->user_data   = dbus_ctx;

        stderr_signal->signal_name = PACKAGES_APT_UPDATE_ASYNC_STDERR_SIGNAL_NAME;
        stderr_signal->callback    = &packages_apt_run_stderr_simple_stream_handler;
        stderr_signal->user_data   = dbus_ctx;

        g_ptr_array_add(signals, stdout_signal);
        g_ptr_array_add(signals, stderr_signal);

        dbus_ctx_set_timeout(dbus_ctx, PACKAGES_ONE_DAY_TIMEOUT);

        dbus_ctx->reply_type = G_VARIANT_TYPE("(i)");

        module->gdbus_source->call_with_signals(module->gdbus_source, dbus_ctx, signals,
                                                &dbus_call_error);

        if (dbus_call_error)
        {
            g_printerr("%s\n", dbus_call_error->message);
            is_dbus_call_error = TRUE;
            ERR_EXIT();
        }

        exit_code = g_variant_get_child_value(dbus_ctx->result, 0);
        ret       = g_variant_get_int32(exit_code);
        if (dbus_ctx->result)
            (*ctx)->results = (gpointer) g_variant_ref(dbus_ctx->result);
        else
            g_printerr("%s\n", dbus_call_error->message);

        g_variant_unref(exit_code);

        break;
    }

    case APT_LAST_UPDATE:
        dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, PACKAGES_APT_OBJECT_PATH,
                                 PACKAGES_APT_INTERFACE_NAME, PACKAGES_APT_LAST_UPDATE_METHOD_NAME,
                                 module->alterator_ctl_app->arguments->verbose);

        if (!dbus_ctx)
        {
            g_printerr(_("Can't allocate dbus_ctx_t in packages_module_apt_subcommand\n"));
            ERR_EXIT();
        }

        dbus_ctx->reply_type = G_VARIANT_TYPE("(asasi)");

        (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

        module->gdbus_source->call(module->gdbus_source, dbus_ctx, &dbus_call_error);

        if (dbus_ctx->result)
            (*ctx)->results = (gpointer) g_variant_ref(dbus_ctx->result);
        else
            g_printerr("%s\n", dbus_call_error->message);

        break;

    default:
        ERR_EXIT();
        break;
    }

end:
    if (dbus_ctx)
        dbus_ctx_free(dbus_ctx);

    if (subcommand_variant)
        g_variant_unref(subcommand_variant);

    g_free(parameter1);

    if (stdout_signal)
        g_free(stdout_signal);

    if (stderr_signal)
        g_free(stderr_signal);

    if (signals)
        g_ptr_array_unref(signals);

    g_clear_error(&dbus_call_error);

    return ret;
}

static int packages_module_repo_subcommand(AlteratorCtlPackagesModule* module,
                                           alteratorctl_ctx_t** ctx)
{
    int ret                      = 0;
    const gchar* parameter1      = NULL;
    GError* dbus_call_error      = NULL;
    GVariant* subcommand_variant = NULL;
    gchar* info_result           = NULL;

    if (!module)
    {
        g_printerr(_("No packages module.\n"));
        ERR_EXIT();
    }

    if (module->alterator_ctl_app->arguments->help_type == ALTERATORCTL_SUBMODULE_HELP)
        goto end;

    if (!(*ctx))
    {
        g_printerr(_("NULL context in repo module.\n"));
        ERR_EXIT();
    }

    // AlteratorCtlModuleInterface *iface = GET_ALTERATOR_CTL_MODULE_INTERFACE(module);

    // Create dbus context and run AlteratorGDBusSource with it. Copy result from context to
    // packages context
    dbus_ctx_t* dbus_ctx = NULL;

    subcommand_variant = g_variant_get_child_value((*ctx)->subcommands_ids, 1);

    g_variant_get((*ctx)->parameters, "(ms)", &parameter1);

    switch (g_variant_get_int32(subcommand_variant))
    {
    case REPO_ADD:
        if (!parameter1)
        {
            g_printerr(_("Not enough parameters for packages repo add subcommand.\n"));
            packages_module_repo_print_help();
            ERR_EXIT();
        }
        dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, PACKAGES_REPO_OBJECT_PATH,
                                 PACKAGES_REPO_INTERFACE_NAME, PACKAGES_REPO_ADD_METHOD_NAME,
                                 module->alterator_ctl_app->arguments->verbose);

        if (!dbus_ctx)
        {
            g_printerr(_("Can't allocate dbus_ctx_t in packages_module_repo_subcommand\n"));
            ERR_EXIT();
        }

        if (!parameter1)
        {
            g_printerr(_("Not enough parameters for packages apt search subcommand.\n"));
            packages_module_apt_print_help();
            ERR_EXIT();
        }

        dbus_ctx->parameters = g_variant_new("(s)", parameter1);
        dbus_ctx->reply_type = G_VARIANT_TYPE("(asi)");

        (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

        break;

    case REPO_INFO:
    {
        module->gdbus_source->alterator_gdbus_source_get_text_of_alterator_entry_by_path(
            module->gdbus_source, PACKAGES_REPO_OBJECT_PATH, PACKAGES_REPO_INTERFACE_NAME,
            &info_result);

        if (!info_result)
        {
            g_printerr(_("Error while getting info of packages repo submodule on alterator entry "
                         "format: result is NULL.\n"));
            ERR_EXIT();
        }

        (*ctx)->free_results = g_free;
        (*ctx)->results      = info_result;

        return ret;
    }
    case REPO_LIST:
        dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, PACKAGES_REPO_OBJECT_PATH,
                                 PACKAGES_REPO_INTERFACE_NAME, PACKAGES_REPO_LIST_METHOD_NAME,
                                 module->alterator_ctl_app->arguments->verbose);

        if (!dbus_ctx)
        {
            g_printerr(_("Can't allocate dbus_ctx_t in packages_module_repo_subcommand\n"));
            ERR_EXIT();
        }

        dbus_ctx->reply_type = G_VARIANT_TYPE("(asasi)");

        (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

        break;

    case REPO_REMOVE:
        if (!parameter1)
        {
            g_printerr(_("Not enough parameters for packages repo remove subcommand.\n"));
            packages_module_repo_print_help();
            ERR_EXIT();
        }

        dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, PACKAGES_REPO_OBJECT_PATH,
                                 PACKAGES_REPO_INTERFACE_NAME, PACKAGES_REPO_REMOVE_METHOD_NAME,
                                 module->alterator_ctl_app->arguments->verbose);

        if (!dbus_ctx)
        {
            g_printerr(_("Can't allocate dbus_ctx_t in packages_module_repo_subcommand\n"));
            ERR_EXIT();
        }

        if (!parameter1)
        {
            g_printerr(_("Not enough parameters for packages apt search subcommand.\n"));
            packages_module_apt_print_help();
            ERR_EXIT();
        }

        dbus_ctx->parameters = g_variant_new("(s)", parameter1);
        dbus_ctx->reply_type = G_VARIANT_TYPE("(asi)");

        (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;
        break;

    default:
        ERR_EXIT();
        break;
    }

    module->gdbus_source->call(module->gdbus_source, dbus_ctx, &dbus_call_error);
    if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    if (dbus_ctx->result)
        (*ctx)->results = (gpointer) g_variant_ref(dbus_ctx->result);
    else
        g_printerr("%s\n", dbus_call_error->message);

    dbus_ctx_free(dbus_ctx);

end:
    if (subcommand_variant)
        g_variant_unref(subcommand_variant);

    if (parameter1)
        g_free((gpointer) parameter1);

    g_clear_error(&dbus_call_error);

    return ret;
}
static int packages_module_help_subcommand(AlteratorCtlPackagesModule* module,
                                           alteratorctl_ctx_t** ctx)
{
    int ret = 0;

    AlteratorCtlModuleInterface* iface = GET_ALTERATOR_CTL_MODULE_INTERFACE(module);

    iface->print_help(module);

end:
    return ret;
}

void packages_module_apt_sigint_handler(int signal, siginfo_t* info, void* context)
{
    g_printerr(_("\nRegardless of the forced termination of alteratorctl, apt-get called via D-Bus "
                 "will continue to run.\n"));

    // Kill current process with SIGINT and without handler after error message
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    if (sigaction(SIGINT, &sa, NULL) == -1)
        g_printerr(_("Error of register SIGINT signal handler in packages apt submodule.\n"));

    raise(SIGINT);
}

int packages_module_run_with_args(gpointer self, int argc, char** argv)
{
    int ret = 0;

    AlteratorCtlPackagesModule* module = ALTERATOR_CTL_PACKAGES_MODULE(self);

    alteratorctl_ctx_t* ctx = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in manager module run: *module is NULL.\n"));
        ERR_EXIT();
    }

    if (packages_module_parse_arguments(module, argc, argv, &ctx) < 0)
        ERR_EXIT();

    if (packages_module_run(module, ctx) < 0)
        ERR_EXIT();

    if (!is_dbus_call_error && packages_module_handle_results(module, &ctx) < 0)
        ERR_EXIT();

end:
    if (ctx)
        alteratorctl_ctx_free(ctx);
    return ret;
}

static int packages_module_handle_results(AlteratorCtlPackagesModule* module,
                                          alteratorctl_ctx_t** ctx)
{
    int ret = 0;

    if (!module)
    {
        g_printerr(_("Internal error in packages module when handle result: module is NULL.\n"));
        ERR_EXIT();
    }

    enum alteratorctl_help_type help_type = module->alterator_ctl_app->arguments->help_type;
    if (help_type == ALTERATORCTL_MODULE_HELP || help_type == ALTERATORCTL_SUBMODULE_HELP)
        goto end;

    if (!(*ctx))
    {
        g_printerr(_("Internal error in packages module when handle results: *ctx is NULL.\n"));
        ERR_EXIT();
    }

    GVariant* submodule_variant = g_variant_get_child_value((*ctx)->subcommands_ids, 0);
    switch (g_variant_get_int32(submodule_variant))
    {
    case PACKAGES_RPM:
        if (packages_module_handle_rpm_results(module, ctx) < 0)
            ERR_EXIT();

        break;

    case PACKAGES_APT:
        if (packages_module_handle_apt_results(module, ctx) < 0)
            ERR_EXIT();

        break;

    case PACKAGES_REPO:
        if (packages_module_handle_repo_results(module, ctx) < 0)
            ERR_EXIT();

        break;

    default:
        break;
    }

end:
    return ret;
}

static int packages_module_handle_rpm_results(AlteratorCtlPackagesModule* module,
                                              alteratorctl_ctx_t** ctx)
{
    int ret                      = 0;
    GVariant* subcommand_variant = g_variant_get_child_value((*ctx)->subcommands_ids, 1);
    switch (g_variant_get_int32(subcommand_variant))
    {
    case RPM_FILES:
        if (packages_module_handle_rpm_files_results(module, ctx) < 0)
            ERR_EXIT();
        break;

    case RPM_INFO:
        if (packages_module_handle_rpm_info_results(module, ctx) < 0)
            ERR_EXIT();
        break;

    case RPM_INSTALL:
        if (packages_module_handle_rpm_install_results(module, ctx) < 0)
            ERR_EXIT();
        break;

    case RPM_LIST:
        if (packages_module_handle_rpm_list_results(module, ctx) < 0)
            ERR_EXIT();
        break;

    case RPM_PACKAGE_INFO:
        if (packages_module_handle_rpm_package_info_results(module, ctx) < 0)
            ERR_EXIT();
        break;

    case RPM_REMOVE:
        if (packages_module_handle_rpm_remove_results(module, ctx) < 0)
            ERR_EXIT();
        break;
    }

end:
    return ret;
}

static int packages_module_validate_alterator_entry(AlteratorGDBusSource* source,
                                                    GNode* alterator_entry_data,
                                                    const gchar* submodule)
{
    int ret = 0;
    if (!submodule)
    {
        g_printerr(
            _("Can't validate packages submodule alterator entry. Submodule name is empty.\n"));
        ERR_EXIT();
    }

    if (!source->info_parser->alterator_ctl_module_info_parser_find_value(
            source->info_parser, alterator_entry_data, NULL,
            PACKAGES_SUBMODULES_ALTERATOR_ENTRY_TYPE_KEY_NAME, NULL))
    {
        g_printerr(_("Can't get alterator entry data for validation in submodule packages %s: "
                     "field \"%s\" is missing.\n"),
                   submodule, PACKAGES_SUBMODULES_ALTERATOR_ENTRY_TYPE_KEY_NAME);
        ERR_EXIT();
    }

    if (!source->info_parser->alterator_ctl_module_info_parser_find_value(
            source->info_parser, alterator_entry_data, NULL,
            PACKAGES_SUBMODULES_ALTERATOR_ENTRY_NAME_KEY_NAME, NULL))
    {
        g_printerr(_("Can't get alterator entry data for validation in submodule packages %s: "
                     "field \"%s\" is missing.\n"),
                   submodule, PACKAGES_SUBMODULES_ALTERATOR_ENTRY_NAME_KEY_NAME);
        ERR_EXIT();
    }

end:
    return ret;
}

static void packages_apt_run_stdout_simple_stream_handler(
    GDBusConnection* connection, const gchar* sender_name, const gchar* object_path,
    const gchar* interface_name, const gchar* signal_name, GVariant* parameters, gpointer user_data)
{
    for (int i = 0; i < g_variant_n_children(parameters); i++)
    {
        GVariant* tmp = g_variant_get_child_value(parameters, i);

        const gchar* str;

        str = g_variant_get_string(tmp, NULL);

        g_print("%s\n", str);

        g_variant_unref(tmp);
    }

    return;
}

static void packages_apt_run_stderr_simple_stream_handler(
    GDBusConnection* connection, const gchar* sender_name, const gchar* object_path,
    const gchar* interface_name, const gchar* signal_name, GVariant* parameters, gpointer user_data)
{
    for (int i = 0; i < g_variant_n_children(parameters); i++)
    {
        GVariant* tmp = g_variant_get_child_value(parameters, i);

        const gchar* str;

        str = g_variant_get_string(tmp, NULL);

        g_printerr("%s\n", str);

        g_variant_unref(tmp);
    }

    return;
}

check_apply_result* check_apply_result_init(GPtrArray* to_install, GPtrArray* to_remove,
                                            GPtrArray* extra_remove, gint exit_code)
{
    check_apply_result* result = NULL;

    result               = g_malloc0(sizeof(check_apply_result));
    result->to_install   = to_install ? g_ptr_array_ref(to_install) : NULL;
    result->to_remove    = to_remove ? g_ptr_array_ref(to_remove) : NULL;
    result->extra_remove = extra_remove ? g_ptr_array_ref(extra_remove) : NULL;
    result->exit_code    = exit_code;

    return result;
}

void check_apply_result_free(check_apply_result* packages_to_apply)
{
    if (!packages_to_apply)
        return;

    if (packages_to_apply->to_install)
        g_ptr_array_unref(packages_to_apply->to_install);

    if (packages_to_apply->to_remove)
        g_ptr_array_unref(packages_to_apply->to_remove);

    if (packages_to_apply->extra_remove)
        g_ptr_array_unref(packages_to_apply->extra_remove);

    g_free(packages_to_apply);
}

static GHashTable* json_array_to_hash_table(JsonArray* array)
{
    GHashTable* result = NULL;
    if (!array)
    {
        g_printerr(_("Can't convert empty JSON array to hash table.\n"));
        return NULL;
    }

    result = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);
    for (gsize i = 0; i < json_array_get_length(array); i++)
        g_hash_table_add(result, g_strdup(json_array_get_string_element(array, i)));

    return result;
}

static GPtrArray* copy_hash_table_str_keys_to_ptr_array(GHashTable* table)
{
    if (!table)
    {
        g_printerr(_("Can't copy keys of empty hash table to array.\n"));
        return NULL;
    }

    GPtrArray* result = g_ptr_array_new_with_free_func(g_free);
    GHashTableIter iter;
    g_hash_table_iter_init(&iter, table);
    gpointer key = NULL;
    while (g_hash_table_iter_next(&iter, &key, NULL))
        g_ptr_array_add(result, g_strdup((gchar*) key));

    return result;
}

static int get_check_apply_result(AlteratorCtlPackagesModule* module, const gchar* packages,
                                  check_apply_result** result)
{
    int ret                 = 0;
    dbus_ctx_t* dbus_ctx    = NULL;
    GError* dbus_call_error = NULL;
    GVariant* answer_array  = NULL;
    GVariant* errors_array  = NULL;
    GVariant* exit_code     = NULL;
    if (!module)
    {
        g_printerr(_("Can't run check install in not existed alterator module.\n"));
        ERR_EXIT();
    }

    if (!packages || !strlen(packages))
    {
        g_printerr(_(
            "It is not possible to check if an unspecified packages can be installed or removed in "
            "packages apt.\n"));
        ERR_EXIT();
    }

    if (!result)
    {
        g_printerr(_("Unable to check if packages can be installed or removed. There is nowhere to "
                     "save the result.\n"));
        ERR_EXIT();
    }

    dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, PACKAGES_APT_OBJECT_PATH,
                             PACKAGES_APT_INTERFACE_NAME, PACKAGES_APT_CHECK_APPLY_METHOD_NAME,
                             module->alterator_ctl_app->arguments->verbose);

    if (!dbus_ctx)
    {
        g_printerr(_("Can't allcate dbus_ctx_t in get_check_apply_result.\n"));
        ERR_EXIT();
    }

    dbus_ctx->parameters = g_variant_new("(s)", packages);
    dbus_ctx->reply_type = G_VARIANT_TYPE("(asasi)");

    dbus_ctx_set_timeout(dbus_ctx, PACKAGES_ONE_DAY_TIMEOUT);

    module->gdbus_source->call(module->gdbus_source, dbus_ctx, &dbus_call_error);

    if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    if (!g_variant_is_of_type(dbus_ctx->result, G_VARIANT_TYPE("(asasi)")))
    {
        g_printerr(_("Wrong type of CheckApply result.\n"));
        ERR_EXIT();
    }

    answer_array = g_variant_get_child_value(dbus_ctx->result, 0);
    errors_array = g_variant_get_child_value(dbus_ctx->result, 1);
    exit_code    = g_variant_get_child_value(dbus_ctx->result, 2);

    if (!answer_array || !errors_array || !exit_code)
    {
        g_printerr(_("CheckApply error: empty result.\n"));
        ERR_EXIT();
    }

    if ((ret = g_variant_get_int32(exit_code)))
    {
        gchar** stderr_answer = g_variant_dup_strv(errors_array, NULL);
        for (gsize i = 0; i < g_strv_length(stderr_answer); i++)
            g_printerr("%s\n", stderr_answer[i]);
        g_strfreev(stderr_answer);
        goto end;
    }

    gchar** stdout_answer_strv = g_variant_dup_strv(answer_array, NULL);
    gchar* stdout_json_answer  = g_strjoinv(",", stdout_answer_strv);
    g_strfreev(stdout_answer_strv);

    GError* regex_error                   = NULL;
    GRegex* invalid_packages_string_regex = g_regex_new("(\"\\(.*?\\)\",)", 0, 0, &regex_error);
    gchar* stdout_result_tmp              = stdout_json_answer;
    stdout_json_answer = g_regex_replace(invalid_packages_string_regex, stdout_result_tmp, -1, 0,
                                         "", 0, &regex_error);
    if (regex_error)
    {
        g_printerr("%s\n", regex_error->message);
        g_regex_unref(invalid_packages_string_regex);
        g_free(stdout_json_answer);
        g_free(stdout_result_tmp);
        ERR_EXIT();
    }
    g_free(stdout_result_tmp);
    g_regex_unref(invalid_packages_string_regex);

    JsonParser* parser = json_parser_new();
    if (!json_parser_load_from_data(parser, stdout_json_answer, -1, NULL))
    {
        g_printerr(_("Failed to parse JSON result of CheckApply.\n"));
        g_object_unref(parser);
        g_free(stdout_json_answer);
        ERR_EXIT();
    }
    JsonNode* json_reply_root             = json_parser_get_root(parser);
    JsonObject* json_reply_object         = json_node_get_object(json_reply_root);
    GHashTable* packages_to_install_table = NULL;
    if (!(packages_to_install_table = json_array_to_hash_table(
              json_object_get_array_member(json_reply_object, "install_packages"))))
    {
        g_printerr("Failed to parse of CheckApply packages to install list.\n");
        g_object_unref(parser);
        g_free(stdout_json_answer);
        ERR_EXIT();
    }
    GPtrArray* to_install = copy_hash_table_str_keys_to_ptr_array(packages_to_install_table);
    g_ptr_array_sort(to_install, packages_module_sort_result);
    g_hash_table_destroy(packages_to_install_table);

    GHashTable* extra_remove_packages_table = NULL;
    if (!(extra_remove_packages_table = json_array_to_hash_table(
              json_object_get_array_member(json_reply_object, "extra_remove_packages"))))
    {
        g_printerr("Failed to parse of CheckApply extra remove packages list.\n");
        g_object_unref(parser);
        g_free(stdout_json_answer);
        ERR_EXIT();
    }
    GPtrArray* to_extra_remove = copy_hash_table_str_keys_to_ptr_array(extra_remove_packages_table);
    g_ptr_array_sort(to_extra_remove, packages_module_sort_result);
    g_hash_table_destroy(extra_remove_packages_table);

    GHashTable* packages_to_remove_table = NULL;
    if (!(packages_to_remove_table = json_array_to_hash_table(
              json_object_get_array_member(json_reply_object, "remove_packages"))))
    {
        g_printerr("Failed to parse of CheckApply packages to remove list.\n");
        g_object_unref(parser);
        g_free(stdout_json_answer);
        ERR_EXIT();
    }
    for (gsize i = 0; i < to_extra_remove->len; i++)
    {
        if (g_hash_table_contains(packages_to_remove_table, (gchar*) to_extra_remove->pdata[i]))
            g_hash_table_remove(packages_to_remove_table, (gchar*) to_extra_remove->pdata[i]);
    }
    GPtrArray* to_remove = copy_hash_table_str_keys_to_ptr_array(packages_to_remove_table);
    g_ptr_array_sort(to_remove, packages_module_sort_result);
    g_hash_table_destroy(packages_to_remove_table);

    g_object_unref(parser);
    g_free(stdout_json_answer);

    *result = check_apply_result_init(to_install, to_remove, to_extra_remove, ret);
    g_ptr_array_unref(to_install);
    g_ptr_array_unref(to_remove);
    g_ptr_array_unref(to_extra_remove);

end:
    if (dbus_ctx)
        dbus_ctx_free(dbus_ctx);

    g_clear_error(&dbus_call_error);

    if (answer_array)
        g_variant_unref(answer_array);

    if (errors_array)
        g_variant_unref(errors_array);

    if (exit_code)
        g_variant_unref(exit_code);

    return ret;
}

static int packages_module_apt_check_apply_package_info(
    AlteratorCtlPackagesModule* module, const gchar* packages,
    const gchar** optional_calculated_packages_to_install,
    const gchar** optional_calculated_packages_to_remove,
    const gchar** optional_calculated_packages_extra_remove, gboolean* accepted)
{
    int ret                    = 0;
    gchar* install_result      = NULL;
    gchar* remove_result       = NULL;
    gchar* extra_remove_result = NULL;
    gchar is_accept_symbol     = 'n';

    gchar remove_essential_phrase[128];
    memset(remove_essential_phrase, 0, 128);

    if (!module)
    {
        g_printerr(_("Can't run check install in not existed alterator module.\n"));
        ERR_EXIT();
    }

    if (!packages || !strlen(packages))
    {
        g_printerr(_(
            "It is not possible to check if an unspecified packages can be installed or removed in "
            "packages apt.\n"));
        ERR_EXIT();
    }

    if (!accepted)
    {
        g_printerr(_("Unable to get approval to review application packages %s for using in "
                     "packages apt. Accepted flag "
                     "is null.\n"),
                   packages);
        ERR_EXIT();
    }

    if (optional_calculated_packages_to_install
        && g_strv_length((gchar**) optional_calculated_packages_to_install)
        && !(install_result = columnize_text((gchar**) optional_calculated_packages_to_install)))
        ERR_EXIT();

    if (optional_calculated_packages_to_remove
        && g_strv_length((gchar**) optional_calculated_packages_to_remove)
        && !(remove_result = columnize_text((gchar**) optional_calculated_packages_to_remove)))
        ERR_EXIT();

    if (optional_calculated_packages_extra_remove
        && g_strv_length((gchar**) optional_calculated_packages_extra_remove)
        && !(extra_remove_result = columnize_text(
                 (gchar**) optional_calculated_packages_extra_remove)))
        ERR_EXIT();

    if (install_result && strlen(install_result))
    {
        g_print(_("Packages to install:\n"));
        g_print("%s\n", install_result);

        if (remove_result && strlen(remove_result))
            g_print("\n");
    }

    if (remove_result && strlen(remove_result))
    {
        g_print(_("Packages to remove:\n"));
        g_print("%s\n", remove_result);
    }

    if (extra_remove_result && extra_remove_result)
    {
        if (!apt_allow_remove_manually)
            g_printerr(_("The ability to remove manually installed packages is disabled because it "
                         "may accidentally remove "
                         "system-critical packages:\n"));
        else
            g_print(_("Manually installed packages to remove:\n"));

        g_print("%s\n", extra_remove_result);

        if (!apt_allow_remove_manually)
            ERR_EXIT();
    }

    if ((install_result && strlen(install_result) || remove_result && strlen(remove_result)
         || extra_remove_result && strlen(extra_remove_result))
        && ret == 0)
    {
        if (apt_force_transaction_commit)
        {
            *accepted = TRUE;
            goto end;
        }

        if (extra_remove_result && strlen(extra_remove_result))
        {
            g_print(_("WARNING: This action may remove packages that are important for the system "
                      "to function.\n"
                      "You must clearly understand the possible consequences!\n\n"));
            g_print(_("To continue, enter the phrase \"Yes, do as I say!\".\n"));
            g_print(" ?] ");
            scanf("%[^\n]", remove_essential_phrase);
        }
        else
        {
            g_print(_("Do you want to continue? [y/N] "));
            scanf("%c", &is_accept_symbol);
        }
        g_print("\n");

        if (is_accept_symbol == 'Y' || is_accept_symbol == 'y'
            || (apt_allow_remove_manually
                && g_strcmp0(remove_essential_phrase, PACKAGES_APT_REMOVE_ESSENTIAL_APPLY_PHRASE)
                       == 0))
        {
            (*accepted) = TRUE;
            goto end;
        }
        else
        {
            g_print(_("Aborted.\n"));
            (*accepted) = FALSE;
            goto end;
        }
    }
    else
    {
        gchar** packages_arr             = g_strsplit(packages, " ", -1);
        GPtrArray* already_installed     = g_ptr_array_new();
        GPtrArray* already_non_installed = g_ptr_array_new_with_free_func(g_free);
        for (gsize i = 0; i < g_strv_length(packages_arr); i++)
        {
            gchar* package = packages_arr[i];
            if (package[strlen(package) - 1] == '-')
                g_ptr_array_add(already_non_installed,
                                g_utf8_substring(package, 0, strlen(package) - 1));
            else
                g_ptr_array_add(already_installed, package);
        }

        if (already_installed->len)
        {
            g_ptr_array_sort(already_installed, packages_module_sort_result);
            gchar* installed_packages = g_strjoinv(" ", (gchar**) already_installed->pdata);
            g_print(_("These packages are already installed:\n  %s\n\n"), installed_packages);
            g_free(installed_packages);
        }

        if (already_non_installed->len)
        {
            g_ptr_array_sort(already_non_installed, packages_module_sort_result);
            gchar* non_installed_packages = g_strjoinv(" ", (gchar**) already_non_installed->pdata);
            g_print(_("These packages aren't installed and cannot be removed:\n  %s\n\n"),
                    non_installed_packages);
            g_free(non_installed_packages);
        }

        if (already_non_installed)
            g_ptr_array_sort(already_non_installed, packages_module_sort_result);

        if (already_installed)
            g_ptr_array_unref(already_installed);
        if (already_non_installed)
            g_ptr_array_unref(already_non_installed);
        g_strfreev(packages_arr);

        g_print(_("Aborted.\n"));
        (*accepted) = FALSE;
    }

end:

    g_free(install_result);

    g_free(remove_result);

    g_free(extra_remove_result);

    return ret;
}

static int packages_module_apt_check_reinstall_package(AlteratorCtlPackagesModule* module,
                                                       const gchar* package, gboolean* accepted)
{
    int ret                      = 0;
    dbus_ctx_t* dbus_ctx         = NULL;
    GVariant* stdout_result      = NULL;
    GVariant* stderr_result      = NULL;
    GVariant* exit_code          = NULL;
    gchar** packages_to_install  = NULL;
    gchar** packages_to_remove   = NULL;
    GPtrArray* to_install_result = NULL;
    GPtrArray* to_remove_result  = NULL;
    gchar is_accept_symbol;

    if (!module)
    {
        g_printerr(_("Can't run check reinstall package in not existed alterator module.\n"));
        ERR_EXIT();
    }

    if (!package || !strlen(package))
    {
        g_printerr(
            _("It is not possible to check if an unspecified package can be reinstalled.\n"));
        ERR_EXIT();
    }

    if (!accepted)
        g_printerr(_("Can't get approving for reinstall package %s. Accepted flag is null.\n"),
                   package);

    dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, PACKAGES_APT_OBJECT_PATH,
                             PACKAGES_APT_INTERFACE_NAME, PACKAGES_APT_CHECK_REINSTALL_METHOD_NAME,
                             module->alterator_ctl_app->arguments->verbose);

    if (!dbus_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in packages_module_apt_check_reinstall_package\n"));
        ERR_EXIT();
    }

    dbus_ctx->parameters = g_variant_new("(s)", package);
    dbus_ctx->reply_type = G_VARIANT_TYPE("(asasi)");

    dbus_ctx_set_timeout(dbus_ctx, PACKAGES_ONE_DAY_TIMEOUT);

    module->gdbus_source->call(module->gdbus_source, dbus_ctx, NULL);

    if (!dbus_ctx->result)
    {
        g_printerr(_("It is impossible to get the list of packages to reinstall when reinstalling "
                     "package %s.\n"),
                   package);
        ERR_EXIT();
    }

    if (!g_variant_is_of_type(dbus_ctx->result, G_VARIANT_TYPE("(asasi)")))
    {
        g_printerr(_("Wrong type of apt CheckReinstall result.\n"));
        ERR_EXIT();
    }

    stdout_result       = g_variant_get_child_value(dbus_ctx->result, 0);
    packages_to_install = (gchar**) g_variant_get_strv(stdout_result, NULL);
    to_install_result = g_ptr_array_new_take_null_terminated((gpointer*) packages_to_install, NULL);
    g_ptr_array_sort(to_install_result, packages_module_sort_result);

    stderr_result      = g_variant_get_child_value(dbus_ctx->result, 1);
    packages_to_remove = (gchar**) g_variant_get_strv(stderr_result, NULL);
    to_remove_result   = g_ptr_array_new_take_null_terminated((gpointer*) packages_to_remove, NULL);
    g_ptr_array_sort(to_remove_result, packages_module_sort_result);

    exit_code = g_variant_get_child_value(dbus_ctx->result, 2);

    ret = g_variant_get_int32(exit_code);
    if (ret == 0)
    {
        for (gsize i = 0; i < to_install_result->len; i++)
        {
            if (i == 0)
                g_print(_("New packages to install:\n"));
            g_print("%s\n", (gchar*) to_install_result->pdata[i]);
        }

        if (!to_install_result->len && to_remove_result->len)
            g_print("\n");
    }

    // Error message or packages list to remove
    for (gsize i = 0; i < to_remove_result->len; i++)
    {
        if (i == 0 && ret == 0)
            g_print(_("Packages to remove:\n"));
        g_print("%s\n", (gchar*) to_remove_result->pdata[i]);
    }

    if ((to_install_result->len || to_remove_result->len) && ret == 0)
    {
        g_print(_("Do you want to continue? [y/N] "));
        scanf("%c", &is_accept_symbol);
        g_print("\n");

        if (is_accept_symbol == 'Y' || is_accept_symbol == 'y')
        {
            (*accepted) = TRUE;
            goto end;
        }
        else
        {
            (*accepted) = FALSE;
            g_print(_("Aborted.\n"));
            goto end;
        }
    }
    else
        (*accepted) = TRUE;

end:
    if (dbus_ctx)
        dbus_ctx_free(dbus_ctx);

    if (stdout_result)
        g_variant_unref(stdout_result);

    if (stderr_result)
        g_variant_unref(stderr_result);

    if (exit_code)
        g_variant_unref(exit_code);

    if (to_install_result)
        g_ptr_array_unref(to_install_result);

    if (to_remove_result)
        g_ptr_array_unref(to_remove_result);

    return ret;
}

static int packages_module_handle_rpm_files_results(AlteratorCtlPackagesModule* module,
                                                    alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    const gchar* parameter1 = NULL;
    GVariant* answer_array  = NULL;
    GVariant* errors_array  = NULL;
    GVariant* exit_code     = NULL;

    if (!(*ctx)->results)
    {
        g_printerr(
            _("D-Bus error in packages rpm files handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asasi)")))
    {
        g_printerr(_("Wrong answer type in rpm files.\n"));
        ERR_EXIT();
    }

    answer_array = g_variant_get_child_value((*ctx)->results, 0);
    errors_array = g_variant_get_child_value((*ctx)->results, 1);
    exit_code    = g_variant_get_child_value((*ctx)->results, 2);

    if ((*ctx)->parameters)
        g_variant_get((*ctx)->parameters, "(ms)", &parameter1);
    else
    {
        g_printerr(_("Error while running rpm Files() method: parameter is NULL.\n"));
        ERR_EXIT();
    }

    if (!exit_code)
    {
        g_printerr(_("Error while running rpm Files() method in %s: exit_code is NULL.\n"),
                   parameter1);
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);
    if (ret)
    {
        GVariantIter* iter = NULL;
        gchar* str         = NULL;

        g_variant_get(errors_array, "as", &iter);
        while (g_variant_iter_loop(iter, "s", &str))
        {
            g_print("%s\n", str);
        }

        g_variant_iter_free(iter);
    }

    GVariantIter* iter = NULL;
    gchar* str         = NULL;
    GString* buffer    = g_string_new(NULL);

    g_variant_get(answer_array, "as", &iter);
    while (g_variant_iter_loop(iter, "s", &str))
        g_string_append_printf(buffer, "%s\n", str);

    if (buffer->len > 0)
        print_with_pager(buffer->str);
    else
        g_print("\n");

    g_string_free(buffer, TRUE);
    g_variant_iter_free(iter);

end:
    if (parameter1)
        g_free((gpointer) parameter1);

    if (errors_array)
        g_variant_unref(errors_array);

    if (exit_code)
        g_variant_unref(exit_code);

    if (answer_array)
        g_variant_unref(answer_array);

    return ret;
}

static int packages_module_handle_rpm_info_results(AlteratorCtlPackagesModule* module,
                                                   alteratorctl_ctx_t** ctx)
{
    int ret                           = 0;
    GNode* parsed_rpm_alterator_entry = NULL;
    AlteratorGDBusSource* source      = (AlteratorGDBusSource*) module->gdbus_source;
    if (!(*ctx)->results)
    {
        g_printerr(
            _("D-Bus error in packages rpm info handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (source->info_parser->alterator_ctl_module_info_parser_get_specified_object_data(
            source->info_parser, source, PACKAGES_RPM_OBJECT_PATH, PACKAGES_RPM_INTERFACE_NAME,
            &parsed_rpm_alterator_entry)
        < 0)
    {
        g_printerr(_("Error in packages rpm submodule: can't get alterator entry data of %s.\n"),
                   PACKAGES_RPM_OBJECT_PATH);
        ERR_EXIT();
    }

    if (packages_module_validate_alterator_entry(source, parsed_rpm_alterator_entry, "rpm") < 0)
        ERR_EXIT();

    print_with_pager((gchar*) (*ctx)->results);

end:
    return ret;
}

static int packages_module_handle_rpm_install_results(AlteratorCtlPackagesModule* module,
                                                      alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    const gchar* parameter1 = NULL;
    GVariant* errors_array  = NULL;
    GVariant* exit_code     = NULL;
    if (!(*ctx)->results)
    {
        g_printerr(
            _("D-Bus error in packages rpm install handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asi)")))
    {
        g_printerr(_("Wrong answer type in rpm install.\n"));
        ERR_EXIT();
    }

    errors_array = g_variant_get_child_value((*ctx)->results, 0);
    exit_code    = g_variant_get_child_value((*ctx)->results, 1);

    if ((*ctx)->parameters)
        g_variant_get((*ctx)->parameters, "(ms)", &parameter1);
    else
    {
        g_printerr(_("Error while running rpm Install() method: parameter is NULL.\n"));
        ERR_EXIT();
    }

    if (!exit_code)
    {
        g_printerr(_("Error while running rpm Install() method in %s: exit_code is NULL.\n"),
                   parameter1);
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);
    if (ret)
    {
        GVariantIter* iter = NULL;
        gchar* str         = NULL;

        g_variant_get(errors_array, "as", &iter);
        while (g_variant_iter_loop(iter, "s", &str))
        {
            g_print("%s\n", str);
        }

        g_variant_iter_free(iter);
    }

end:
    if (errors_array)
        g_variant_unref(errors_array);

    if (exit_code)
        g_variant_unref(exit_code);

    if (parameter1)
        g_free((gpointer) parameter1);

    return ret;
}
static int packages_module_handle_rpm_list_results(AlteratorCtlPackagesModule* module,
                                                   alteratorctl_ctx_t** ctx)
{
    int ret                = 0;
    GPtrArray* result      = NULL;
    GVariant* array        = NULL;
    GVariant* errors_array = NULL;
    GVariant* exit_code    = NULL;

    if (!(*ctx)->results)
    {
        g_printerr(
            _("D-Bus error in packages rpm list handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asasi)")))
    {
        g_printerr(_("Wrong answer type in rpm list.\n"));
        ERR_EXIT();
    }

    array        = g_variant_get_child_value((*ctx)->results, 0);
    errors_array = g_variant_get_child_value((*ctx)->results, 1);
    exit_code    = g_variant_get_child_value((*ctx)->results, 2);

    if (!exit_code)
    {
        g_printerr(_("Error while running rpm List() method: exit_code is NULL.\n"));
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);
    if (ret)
    {
        GVariantIter* iter = NULL;
        gchar* str         = NULL;

        g_variant_get(errors_array, "as", &iter);
        while (g_variant_iter_loop(iter, "s", &str))
        {
            g_print("%s\n", str);
        }

        g_variant_iter_free(iter);
    }

    result = g_ptr_array_new_full(g_variant_n_children(array), (GDestroyNotify) g_free);

    GVariantIter* iter = NULL;
    gchar* str         = NULL;
    GString* buffer    = g_string_new(NULL);

    g_variant_get(array, "as", &iter);
    while (g_variant_iter_loop(iter, "s", &str))
        g_ptr_array_add(result, g_strdup(str));

    g_ptr_array_sort(result, packages_module_sort_result);

    for (guint i = 0; i < result->len; i++)
        g_string_append(buffer, (gchar*) g_ptr_array_index(result, i)),
            g_string_append_c(buffer, '\n');

    if (buffer->len > 0)
        print_with_pager(buffer->str);
    else
        g_print("\n");

    g_string_free(buffer, TRUE);
    g_variant_iter_free(iter);

end:
    if (result)
        g_ptr_array_unref(result);

    if (exit_code)
        g_variant_unref(exit_code);

    if (array)
        g_variant_unref(array);

    if (errors_array)
        g_variant_unref(errors_array);

    return ret;
}
static int packages_module_handle_rpm_package_info_results(AlteratorCtlPackagesModule* module,
                                                           alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    const gchar* parameter1 = NULL;
    GVariant* array         = NULL;
    GVariant* errors_array  = NULL;
    GVariant* exit_code     = NULL;

    if (!(*ctx)->results)
    {
        g_printerr(_("D-Bus error in packages rpm packageinfo handle result: failed to produce a "
                     "result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asasi)")))
    {
        g_printerr(_("Wrong answer type in rpm packageinfo.\n"));
        ERR_EXIT();
    }

    array        = g_variant_get_child_value((*ctx)->results, 0);
    errors_array = g_variant_get_child_value((*ctx)->results, 1);
    exit_code    = g_variant_get_child_value((*ctx)->results, 2);

    if ((*ctx)->parameters)
        g_variant_get((*ctx)->parameters, "(ms)", &parameter1);
    else
    {
        g_printerr(_("Error while running rpm PackageInfo() method: parameter is NULL.\n"));
        ERR_EXIT();
    }

    if (!exit_code)
    {
        g_printerr(_("Error while running rpm PackageInfo() method in %s: exit_code is NULL.\n"),
                   parameter1);
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);
    if (ret)
    {
        GVariantIter* iter = NULL;
        gchar* str         = NULL;

        g_variant_get(errors_array, "as", &iter);
        while (g_variant_iter_loop(iter, "s", &str))
        {
            g_print("%s\n", str);
        }

        g_variant_iter_free(iter);
    }

    GVariantIter* iter = NULL;
    gchar* str         = NULL;

    g_variant_get(array, "as", &iter);
    while (g_variant_iter_loop(iter, "s", &str))
    {
        g_print("%s\n", str);
    }

    g_variant_iter_free(iter);

end:
    if (parameter1)
        g_free((gpointer) parameter1);

    if (exit_code)
        g_variant_unref(exit_code);

    if (array)
        g_variant_unref(array);

    if (errors_array)
        g_variant_unref(errors_array);

    return ret;
}
static int packages_module_handle_rpm_remove_results(AlteratorCtlPackagesModule* module,
                                                     alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    const gchar* parameter1 = NULL;
    GVariant* errors_array  = NULL;
    GVariant* exit_code     = NULL;
    if (!(*ctx)->results)
    {
        g_printerr(
            _("D-Bus error in packages rpm remove handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asi)")))
    {
        g_printerr(_("Wrong answer type in rpm remove.\n"));
        ERR_EXIT();
    }

    errors_array = g_variant_get_child_value((*ctx)->results, 0);
    exit_code    = g_variant_get_child_value((*ctx)->results, 1);

    if ((*ctx)->parameters)
        g_variant_get((*ctx)->parameters, "(ms)", &parameter1);
    else
    {
        g_printerr(_("Error while running rpm Remove() method: parameter is NULL.\n"));
        ERR_EXIT();
    }

    if (!exit_code)
    {
        g_printerr(_("Error while running rpm Remove() method in %s: exit_code is NULL.\n"),
                   parameter1);
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);
    if (ret)
    {
        GVariantIter* iter = NULL;
        gchar* str         = NULL;

        g_variant_get(errors_array, "as", &iter);
        while (g_variant_iter_loop(iter, "s", &str))
        {
            g_print("%s\n", str);
        }

        g_variant_iter_free(iter);
    }

end:
    if (parameter1)
        g_free((gpointer) parameter1);

    if (exit_code)
        g_variant_unref(exit_code);

    if (errors_array)
        g_variant_unref(errors_array);

    return ret;
}

static int packages_module_handle_apt_results(AlteratorCtlPackagesModule* module,
                                              alteratorctl_ctx_t** ctx)
{
    int ret                      = 0;
    GVariant* subcommand_variant = g_variant_get_child_value((*ctx)->subcommands_ids, 1);

    switch (g_variant_get_int32(subcommand_variant))
    {
    case APT_INFO:
        if (packages_module_handle_apt_info_results(module, ctx) < 0)
            ERR_EXIT();
        break;

    case APT_LIST_ALL_PACKAGES:
        if (packages_module_handle_apt_list_all_packages_results(module, ctx) < 0)
            ERR_EXIT();
        break;

    case APT_SEARCH:
        if (packages_module_handle_apt_search_results(module, ctx) < 0)
            ERR_EXIT();
        break;

    case APT_LAST_UPDATE:
        if (packages_module_handle_apt_last_update_results(module, ctx) < 0)
            ERR_EXIT();
        break;
    }

end:
    return ret;
}

static int packages_module_handle_apt_info_results(AlteratorCtlPackagesModule* module,
                                                   alteratorctl_ctx_t** ctx)
{
    int ret                           = 0;
    GNode* parsed_apt_alterator_entry = NULL;
    AlteratorGDBusSource* source      = (AlteratorGDBusSource*) module->gdbus_source;
    if (!(*ctx)->results)
    {
        g_printerr(
            _("D-Bus error in packages apt info handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (source->info_parser->alterator_ctl_module_info_parser_get_specified_object_data(
            source->info_parser, source, PACKAGES_APT_OBJECT_PATH, PACKAGES_APT_INTERFACE_NAME,
            &parsed_apt_alterator_entry)
        < 0)
    {
        g_printerr(_("Error in packages apt submodule: can't get alterator entry data of %s.\n"),
                   PACKAGES_APT_OBJECT_PATH);
        ERR_EXIT();
    }

    if (packages_module_validate_alterator_entry(source, parsed_apt_alterator_entry, "apt") < 0)
        ERR_EXIT();

    print_with_pager((gchar*) (*ctx)->results);

end:

    return ret;
}

static int packages_module_handle_apt_list_all_packages_results(AlteratorCtlPackagesModule* module,
                                                                alteratorctl_ctx_t** ctx)
{
    int ret                = 0;
    GPtrArray* result      = NULL;
    GVariant* array        = NULL;
    GVariant* errors_array = NULL;
    GVariant* exit_code    = NULL;

    if (!(*ctx)->results)
    {
        g_printerr(
            _("D-Bus error in packages apt listall handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asasi)")))
    {
        g_printerr(_("Wrong answer type in apt list all packages.\n"));
        ERR_EXIT();
    }

    array        = g_variant_get_child_value((*ctx)->results, 0);
    errors_array = g_variant_get_child_value((*ctx)->results, 1);
    exit_code    = g_variant_get_child_value((*ctx)->results, 2);

    if (!exit_code)
    {
        g_printerr(_("Error while running apt ListAllPackages() method: exit_code is NULL.\n"));
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);
    if (ret)
    {
        GVariantIter* iter = NULL;
        gchar* str         = NULL;

        g_variant_get(errors_array, "as", &iter);
        while (g_variant_iter_loop(iter, "s", &str))
            g_print("%s\n", str);

        g_variant_iter_free(iter);
    }

    result = g_ptr_array_new_full(g_variant_n_children(array), (GDestroyNotify) g_free);

    GVariantIter* iter = NULL;
    gchar* str         = NULL;
    GString* buffer    = g_string_new(NULL);

    g_variant_get(array, "as", &iter);
    while (g_variant_iter_loop(iter, "s", &str))
        g_ptr_array_add(result, g_strdup(str));

    g_ptr_array_sort(result, packages_module_sort_result);

    for (guint i = 0; i < result->len; i++)
    {
        g_string_append(buffer, (gchar*) g_ptr_array_index(result, i));
        g_string_append_c(buffer, '\n');
    }
    if (buffer->len > 0)
        print_with_pager(buffer->str);
    else
        g_print("\n");

    g_string_free(buffer, TRUE);
    g_variant_iter_free(iter);

end:
    if (result)
        g_ptr_array_unref(result);

    if (exit_code)
        g_variant_unref(exit_code);

    if (array)
        g_variant_unref(array);

    if (errors_array)
        g_variant_unref(errors_array);

    return ret;
}

static int packages_module_handle_apt_reinstall_results(AlteratorCtlPackagesModule* module,
                                                        alteratorctl_ctx_t** ctx)
{
    int ret = 0;

end:
    return ret;
}

static int packages_module_handle_apt_search_results(AlteratorCtlPackagesModule* module,
                                                     alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    const gchar* parameter1 = NULL;
    GPtrArray* result       = NULL;
    GVariant* array         = NULL;
    GVariant* errors_array  = NULL;
    GVariant* exit_code     = NULL;

    if (!(*ctx)->results)
    {
        g_printerr(
            _("D-Bus error in packages apt search handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asasi)")))
    {
        g_printerr(_("Wrong answer type in apt list all search.\n"));
        ERR_EXIT();
    }

    array        = g_variant_get_child_value((*ctx)->results, 0);
    errors_array = g_variant_get_child_value((*ctx)->results, 1);
    exit_code    = g_variant_get_child_value((*ctx)->results, 2);

    if ((*ctx)->parameters)
        g_variant_get((*ctx)->parameters, "(ms)", &parameter1);
    else
    {
        g_printerr(_("Error while running rpm Search() method: parameter is NULL.\n"));
        ERR_EXIT();
    }

    if (!exit_code)
    {
        g_printerr(_("Error while running apt Search() method in %s: exit_code is NULL.\n"),
                   parameter1);
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);
    if (ret)
    {
        GVariantIter* iter = NULL;
        gchar* str         = NULL;

        g_variant_get(errors_array, "as", &iter);
        while (g_variant_iter_loop(iter, "s", &str))
            g_print("%s\n", str);

        g_variant_iter_free(iter);
    }

    result = g_ptr_array_new_full(g_variant_n_children(array), (GDestroyNotify) g_free);

    GVariantIter* iter = NULL;
    gchar* str         = NULL;
    GString* buffer    = g_string_new(NULL);

    g_variant_get(array, "as", &iter);
    while (g_variant_iter_loop(iter, "s", &str))
        g_ptr_array_add(result, g_strdup(str));

    g_ptr_array_sort(result, packages_module_sort_result);

    for (guint i = 0; i < result->len; i++)
    {
        g_string_append(buffer, (gchar*) g_ptr_array_index(result, i));
        g_string_append_c(buffer, '\n');
    }

    if (buffer->len > 0)
        print_with_pager(buffer->str);
    else
        g_print("\n");

    g_string_free(buffer, TRUE);
    g_variant_iter_free(iter);

end:
    if (result)
        g_ptr_array_unref(result);

    if (parameter1)
        g_free((gpointer) parameter1);

    if (exit_code)
        g_variant_unref(exit_code);

    if (array)
        g_variant_unref(array);

    if (errors_array)
        g_variant_unref(errors_array);

    return ret;
}

static int packages_module_handle_apt_last_update_results(AlteratorCtlPackagesModule* module,
                                                          alteratorctl_ctx_t** ctx)
{
    int ret             = 0;
    GVariant* array     = NULL;
    GVariant* exit_code = NULL;
    if (!(*ctx)->results)
    {
        g_printerr(_("D-Bus error in packages apt last-update handle result: failed to produce a "
                     "result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asasi)")))
    {
        g_printerr(_("Wrong answer type in apt last-update.\n"));
        ERR_EXIT();
    }

    array     = g_variant_get_child_value((*ctx)->results, 0);
    exit_code = g_variant_get_child_value((*ctx)->results, 2);

    if (!exit_code)
    {
        g_printerr(_("Error while running apt LastUpdate() method: exit_code is NULL.\n"));
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);

    GVariantIter* iter = NULL;
    gchar* str         = NULL;

    g_variant_get(array, "as", &iter);
    while (g_variant_iter_loop(iter, "s", &str))
    {
        g_print("%s\n", str);
    }

    g_variant_iter_free(iter);

end:
    if (exit_code)
        g_variant_unref(exit_code);

    if (array)
        g_variant_unref(array);

    return ret;
}

static int packages_module_handle_repo_results(AlteratorCtlPackagesModule* module,
                                               alteratorctl_ctx_t** ctx)
{
    int ret                      = 0;
    GVariant* subcommand_variant = g_variant_get_child_value((*ctx)->subcommands_ids, 1);
    switch (g_variant_get_int32(subcommand_variant))
    {
    case REPO_ADD:
        if (packages_module_handle_repo_add_results(module, ctx) < 0)
            ERR_EXIT();
        break;

    case REPO_INFO:
        if (packages_module_handle_repo_info_results(module, ctx) < 0)
            ERR_EXIT();
        break;

    case REPO_LIST:
        if (packages_module_handle_repo_list_results(module, ctx) < 0)
            ERR_EXIT();
        break;

    case REPO_REMOVE:
        if (packages_module_handle_repo_remove_results(module, ctx) < 0)
            ERR_EXIT();
        break;
    }

end:
    return ret;
}

static int packages_module_handle_repo_add_results(AlteratorCtlPackagesModule* module,
                                                   alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    const gchar* parameter1 = NULL;
    GVariant* errors_array  = NULL;
    GVariant* exit_code     = NULL;
    if (!(*ctx)->results)
    {
        g_printerr(
            _("D-Bus error in packages repo add handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asi)")))
    {
        g_printerr(_("Wrong answer type in repo add.\n"));
        ERR_EXIT();
    }

    errors_array = g_variant_get_child_value((*ctx)->results, 0);
    exit_code    = g_variant_get_child_value((*ctx)->results, 1);

    if ((*ctx)->parameters)
        g_variant_get((*ctx)->parameters, "(ms)", &parameter1);
    else
    {
        g_printerr(_("Error while running rpm Add() method: parameter is NULL.\n"));
        ERR_EXIT();
    }

    if (!exit_code)
    {
        g_printerr(_("Error while running repo Add() method in %s: exit_code is NULL.\n"),
                   parameter1);
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);
    if (ret)
    {
        GVariantIter* iter = NULL;
        gchar* str         = NULL;

        g_variant_get(errors_array, "as", &iter);
        while (g_variant_iter_loop(iter, "s", &str))
        {
            g_print("%s\n", str);
        }

        g_variant_iter_free(iter);
    }

end:
    if (parameter1)
        g_free((gpointer) parameter1);

    if (exit_code)
        g_variant_unref(exit_code);

    if (errors_array)
        g_variant_unref(errors_array);

    return ret;
}
static int packages_module_handle_repo_info_results(AlteratorCtlPackagesModule* module,
                                                    alteratorctl_ctx_t** ctx)
{
    int ret                            = 0;
    GNode* parsed_repo_alterator_entry = NULL;
    AlteratorGDBusSource* source       = (AlteratorGDBusSource*) module->gdbus_source;
    if (!(*ctx)->results)
    {
        g_printerr(
            _("D-Bus error in packages repo info handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (source->info_parser->alterator_ctl_module_info_parser_get_specified_object_data(
            source->info_parser, source, PACKAGES_REPO_OBJECT_PATH, PACKAGES_REPO_INTERFACE_NAME,
            &parsed_repo_alterator_entry)
        < 0)
    {
        g_printerr(_("Error in packages repo submodule: can't get alterator entry data of %s.\n"),
                   PACKAGES_REPO_OBJECT_PATH);
        ERR_EXIT();
    }

    if (packages_module_validate_alterator_entry(source, parsed_repo_alterator_entry, "repo") < 0)
        ERR_EXIT();

    print_with_pager((gchar*) (*ctx)->results);

end:
    return ret;
}
static int packages_module_handle_repo_list_results(AlteratorCtlPackagesModule* module,
                                                    alteratorctl_ctx_t** ctx)
{
    int ret                = 0;
    GPtrArray* result      = NULL;
    GVariant* array        = NULL;
    GVariant* errors_array = NULL;
    GVariant* exit_code    = NULL;

    if (!(*ctx)->results)
    {
        g_printerr(
            _("D-Bus error in packages repo list handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asasi)")))
    {
        g_printerr(_("Wrong answer type in repo list.\n"));
        ERR_EXIT();
    }

    array        = g_variant_get_child_value((*ctx)->results, 0);
    errors_array = g_variant_get_child_value((*ctx)->results, 1);
    exit_code    = g_variant_get_child_value((*ctx)->results, 2);

    if (!exit_code)
    {
        g_printerr(_("Error while running repo List() method: exit_code is NULL.\n"));
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);
    if (ret)
    {
        GVariantIter* iter = NULL;
        gchar* str         = NULL;

        g_variant_get(errors_array, "as", &iter);
        while (g_variant_iter_loop(iter, "s", &str))
        {
            g_print("%s\n", str);
        }

        g_variant_iter_free(iter);
    }

    result = g_ptr_array_new_full(g_variant_n_children(array), (GDestroyNotify) g_free);

    GVariantIter* iter = NULL;
    gchar* str         = NULL;

    g_variant_get(array, "as", &iter);
    while (g_variant_iter_loop(iter, "s", &str))
        g_ptr_array_add(result, g_strdup(str));

    g_ptr_array_sort(result, packages_module_sort_result);

    for (guint i = 0; i < result->len; i++)
        g_print("%s\n", (gchar*) g_ptr_array_index(result, i));

    g_variant_iter_free(iter);

end:
    if (result)
        g_ptr_array_unref(result);

    if (exit_code)
        g_variant_unref(exit_code);

    if (array)
        g_variant_unref(array);

    if (errors_array)
        g_variant_unref(errors_array);

    return ret;
}
static int packages_module_handle_repo_remove_results(AlteratorCtlPackagesModule* module,
                                                      alteratorctl_ctx_t** ctx)
{
    int ret                 = 0;
    const gchar* parameter1 = NULL;
    GVariant* errors_array  = NULL;
    GVariant* exit_code     = NULL;
    if (!(*ctx)->results)
    {
        g_printerr(
            _("D-Bus error in packages repo remove handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asi)")))
    {
        g_printerr(_("Wrong answer type in repo remove.\n"));
        ERR_EXIT();
    }

    errors_array = g_variant_get_child_value((*ctx)->results, 0);
    exit_code    = g_variant_get_child_value((*ctx)->results, 1);

    if ((*ctx)->parameters)
        g_variant_get((*ctx)->parameters, "(ms)", &parameter1);
    else
    {
        g_printerr(_("Error while running rpm Remove() method: parameter is NULL.\n"));
        ERR_EXIT();
    }

    if (!exit_code)
    {
        g_printerr(_("Error while running repo Remove() method in %s: exit_code is NULL.\n"),
                   parameter1);
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);
    if (ret)
    {
        GVariantIter* iter = NULL;
        gchar* str         = NULL;

        g_variant_get(errors_array, "as", &iter);
        while (g_variant_iter_loop(iter, "s", &str))
        {
            g_print("%s\n", str);
        }

        g_variant_iter_free(iter);
    }

end:
    if (parameter1)
        g_free((gpointer) parameter1);

    if (exit_code)
        g_variant_unref(exit_code);

    if (errors_array)
        g_variant_unref(errors_array);

    return ret;
}

int packages_module_print_help(gpointer self)
{
    int ret = 0;
    g_print(_("Usage:\n"));
    g_print(
        _("  alteratorctl packages [OPTIONS] [<SUBMODULE> [COMMAND] [arguments] [OPTIONS]]\n\n"));
    g_print(_("Submodules:\n"));
    g_print(_("  apt                         Runs apt submodule command\n"));
    g_print(_("  repo                        Runs repo submodule command\n"));
    g_print(_("  rpm                         Runs rpm submodule command\n\n"));
    g_print(_("Options:\n"));
    g_print(_("  -h, --help                  Show module packages usage help\n\n"));
    g_print(_("For more information on a specific submodule and its commands, use:\n"
              "  alteratorctl packages <SUBMODULE> (-h | --help)\n\n"));

end:
    return ret;
}

static void packages_module_rpm_print_help(void)
{
    g_print(_("Usage:\n"));
    g_print(_("  alteratorctl packages rpm <COMMAND> [argument] [OPTIONS]\n\n"));
    g_print(_("Commands:\n"));
    g_print(_("  files <name>                Prints files in specified package\n"));
    g_print(_("  info                        Prints submodule info in alterator entry format\n"));
    g_print(_("  install <path>              Install package from rpm file\n"));
    g_print(_("  list                        Prints all installed packages\n"));
    g_print(_("  packageinfo <name>          Prints info for specified package\n"));
    g_print(_("  remove <name>               Remove specified package\n\n"));
    g_print(_("Options:\n"));
    g_print(_("  -h, --help                  Show rpm submodule usage\n\n"));
}

static void packages_module_apt_print_help(void)
{
    g_print(_("Usage:\n"));
    g_print(_("  alteratorctl packages apt <COMMAND> [argument] [OPTIONS]\n\n"));
    g_print(_("Commands:\n"));
    g_print(_("  info                        Prints submodule info in alterator entry format\n"));
    g_print(_("  install <name>              Install specified package\n"));
    g_print(_("  list                        Prints all available packages\n"));
    g_print(_("  last-update                 Loads packages info last update time\n"));
    g_print(_("  reinstall <name>            Reinstalls specified package\n"));
    g_print(_("  remove <name>               Remove specified package\n"));
    g_print(_("  search <name>               Prints info for specified package\n"));
    g_print(_("  update                      Update packages info\n\n"));
    g_print(_("Options:\n"));
    g_print(_("  --allow-remove-manually     Allow removal of manually installed packages.\n"
              "                              Enabling this option may lead to the removal\n"
              "                              of packages required for proper system operation\n"));
    g_print(_("  --force-yes                 Disable confirmation before removing and\n"
              "                              installing a package. Caution, this action may\n"
              "                              result in immediate removal of system-critical\n"
              "                              packages. Use this option only if you fully\n"
              "                              understand the consequences!\n"));
    g_print(_("  -h, --help                  Show apt submodule usage\n\n"));
}

static void packages_module_repo_print_help(void)
{
    g_print(_("Usage:\n"));
    g_print(_("  alteratorctl packages repo <COMMAND> [argument] [OPTIONS]\n\n"));
    g_print(_("Commands:\n"));
    g_print(_("  add <name>                  Adds repo to sources list\n"));
    g_print(_("  info                        Prints submodule info in alterator entry format\n"));
    g_print(_("  list                        Prints repos from source list\n"));
    g_print(_("  remove <name>               Removes repo from sources list\n\n"));
    g_print(_("Options:\n"));
    g_print(_("  -h, --help                  Show repo submodule usage\n\n"));
}

static int packages_module_parse_options(AlteratorCtlPackagesModule* module, int* argc, char** argv)
{
    int ret = 0;
    if (packages_apt_module_parse_options(module, argc, argv) < 0)
        ERR_EXIT();

end:
    return ret;
}

static int packages_module_parse_arguments(AlteratorCtlPackagesModule* module, int argc,
                                           char** argv, alteratorctl_ctx_t** ctx)
{
    int ret = 0;

    AlteratorCtlModuleInterface* iface = GET_ALTERATOR_CTL_MODULE_INTERFACE((void*) module);
    if (!iface)
    {
        g_printerr("%s", _("Internal error in packages module while parsing arguments: *iface is "
                           "NULL.\n"));
        ERR_EXIT();
    }

    if (module->alterator_ctl_app->arguments->help_type == ALTERATORCTL_MODULE_HELP)
    {
        packages_module_help_subcommand(module, NULL);
        goto end;
    }

    if (packages_module_parse_options(module, &argc, argv) < 0)
        ERR_EXIT();

    if ((argc < 3) || (argc > 5))
    {
        g_printerr(_("Wrong arguments in packages module.\n"));
        iface->print_help(module);
        ERR_EXIT();
    }

    if (!module->commands)
    {
        g_printerr(_("Internal error in packages module while parsing arguments: module->commands "
                     "is NULL.\n"));
        ERR_EXIT();
    }

    int selected_subcommand = iface->get_command_id(module->commands[PACKAGES_HASH_TABLE], argv[2]);
    switch (selected_subcommand)
    {
    case PACKAGES_RPM:
        if (packages_module_parse_rpm_arguments(module, argc, argv, ctx) < 0)
            ERR_EXIT();
        break;

    case PACKAGES_APT:
        if (packages_module_parse_apt_arguments(module, argc, argv, ctx) < 0)
            ERR_EXIT();
        break;

    case PACKAGES_REPO:
        if (packages_module_parse_repo_arguments(module, argc, argv, ctx) < 0)
            ERR_EXIT();
        break;

    default:
        g_printerr(_("Unknown packages submodule.\n"));
        iface->print_help(module);
        ERR_EXIT();
        break;
    };

end:
    return ret;
}

static int packages_apt_module_parse_options(AlteratorCtlPackagesModule* module, int* argc,
                                             char** argv)
{
    int ret                        = 0;
    GOptionContext* option_context = NULL;

    // clang-format off
    static GOptionEntry packages_apt_submodule_options[]
        = {{"allow-remove-manually", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &apt_allow_remove_manually,
                                                    "Enable remove manually installed packages",
                                                    "ENABLE_REMOVE_MANUALLY"},
           {"force-yes", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &apt_force_transaction_commit,
                                                    "Enable remove manually installed packages",
                                                    "FORCE_YES"},
           {NULL}};
    // clang-format on

    GError* error  = NULL;
    option_context = g_option_context_new("Packages apt submodule options");
    g_option_context_add_main_entries(option_context, packages_apt_submodule_options, NULL);
    if (!g_option_context_parse(option_context, argc, &argv, &error))
    {
        g_printerr(_("Packages apt submodule options parsing failed: %s\n"), error->message);
        g_error_free(error);
        ERR_EXIT();
    }

end:
    if (option_context)
        g_option_context_free(option_context);

    return ret;
}

static int packages_module_parse_apt_arguments(AlteratorCtlPackagesModule* module, int argc,
                                               char** argv, alteratorctl_ctx_t** ctx)
{
    int ret                            = 0;
    gchar* locale                      = NULL;
    AlteratorCtlModuleInterface* iface = GET_ALTERATOR_CTL_MODULE_INTERFACE((void*) module);

    if (module->alterator_ctl_app->arguments->help_type == ALTERATORCTL_SUBMODULE_HELP)
    {
        packages_module_apt_print_help();
        goto end;
    }

    if (argc < 4)
    {
        g_printerr(_("Wrong apt arguments.\n"));
        packages_module_apt_print_help();
        ERR_EXIT();
    }

    int selected_subcommand = iface->get_command_id(module->commands[APT_HASH_TABLE], argv[3]);
    switch (selected_subcommand)
    {
    case APT_INSTALL:
    case APT_REMOVE:
    case APT_REINSTALL:
    case APT_UPDATE:
        if (!(locale = alterator_ctl_get_effective_locale()))
            ERR_EXIT();

        if (module->gdbus_source->alterator_gdbus_source_set_env_value(module->gdbus_source,
                                                                       "LC_ALL", locale)
            < 0)
            ERR_EXIT();
    default:
        break;
    }

    switch (selected_subcommand)
    {
    case APT_INFO:
        (*ctx) = alteratorctl_ctx_init_packages(PACKAGES_APT, APT_INFO, NULL, NULL, NULL);
        break;

    case APT_INSTALL:
        if (argc == 5)
            (*ctx) = alteratorctl_ctx_init_packages(PACKAGES_APT, APT_APPLY_ASYNC | APT_INSTALL,
                                                    argv[4], NULL, NULL);
        else
        {
            g_printerr(_("Wrong packages apt arguments.\n"));
            packages_module_apt_print_help();
            ERR_EXIT();
        }
        break;

    case APT_LIST_ALL_PACKAGES:
        (*ctx) = alteratorctl_ctx_init_packages(PACKAGES_APT, APT_LIST_ALL_PACKAGES, NULL, NULL,
                                                NULL);
        break;

    case APT_REINSTALL:
        if (argc == 5)
            (*ctx) = alteratorctl_ctx_init_packages(PACKAGES_APT, APT_REINSTALL, argv[4], NULL,
                                                    NULL);
        else
        {
            g_printerr(_("Wrong packages apt arguments.\n"));
            packages_module_apt_print_help();
            ERR_EXIT();
        }
        break;

    case APT_REMOVE:
        if (argc == 5)
            (*ctx) = alteratorctl_ctx_init_packages(PACKAGES_APT, APT_APPLY_ASYNC | APT_REMOVE,
                                                    argv[4], NULL, NULL);
        else
        {
            g_printerr(_("Wrong packages apt arguments.\n"));
            packages_module_apt_print_help();
            ERR_EXIT();
        }
        break;

    case APT_SEARCH:
        if (argc == 5)
            (*ctx) = alteratorctl_ctx_init_packages(PACKAGES_APT, APT_SEARCH, argv[4], NULL, NULL);
        else
        {
            g_printerr(_("Wrong packages apt arguments.\n"));
            packages_module_apt_print_help();
            ERR_EXIT();
        }
        break;

    case APT_UPDATE:
        (*ctx) = alteratorctl_ctx_init_packages(PACKAGES_APT, APT_UPDATE, NULL, NULL, NULL);
        break;

    case APT_LAST_UPDATE:
        (*ctx) = alteratorctl_ctx_init_packages(PACKAGES_APT, APT_LAST_UPDATE, NULL, NULL, NULL);
        break;

    default:
        g_printerr(_("Unknown packages apt module command.\n"));
        packages_module_apt_print_help();
        ERR_EXIT();
    }

    struct sigaction sa;
    sa.sa_flags     = SA_SIGINFO;
    sa.sa_sigaction = packages_module_apt_sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        g_printerr(_("Error of register SIGINT signal handler in packages apt submodule.\n"));
        ERR_EXIT();
    }

end:
    g_free(locale);

    return ret;
}

static int packages_module_parse_rpm_arguments(AlteratorCtlPackagesModule* module, int argc,
                                               char** argv, alteratorctl_ctx_t** ctx)
{
    int ret                            = 0;
    AlteratorCtlModuleInterface* iface = GET_ALTERATOR_CTL_MODULE_INTERFACE((void*) module);

    if (module->alterator_ctl_app->arguments->help_type == ALTERATORCTL_SUBMODULE_HELP)
    {
        packages_module_rpm_print_help();
        goto end;
    }

    if (argc < 4)
    {
        g_printerr(_("Wrong rpm arguments.\n"));
        packages_module_rpm_print_help();
        ERR_EXIT();
    }

    int selected_subcommand = iface->get_command_id(module->commands[RPM_HASH_TABLE], argv[3]);
    switch (selected_subcommand)
    {
    case RPM_FILES:
        if (argc == 5)
            (*ctx) = alteratorctl_ctx_init_packages(PACKAGES_RPM, RPM_FILES, argv[4], NULL, NULL);
        else
        {
            g_printerr(_("Wrong packages rpm files arguments.\n"));
            packages_module_rpm_print_help();
            ERR_EXIT();
        }
        break;

    case RPM_INFO:
        (*ctx) = alteratorctl_ctx_init_packages(PACKAGES_RPM, RPM_INFO, NULL, NULL, NULL);
        break;

    case RPM_INSTALL:
        if (argc == 5)
            (*ctx) = alteratorctl_ctx_init_packages(PACKAGES_RPM, RPM_INSTALL, argv[4], NULL, NULL);
        else
        {
            g_printerr(_("Wrong packages rpm info arguments.\n"));
            packages_module_rpm_print_help();
            ERR_EXIT();
        }
        break;

    case RPM_LIST:
        (*ctx) = alteratorctl_ctx_init_packages(PACKAGES_RPM, RPM_LIST, NULL, NULL, NULL);
        break;

    case RPM_PACKAGE_INFO:
        if (argc == 5)
            (*ctx) = alteratorctl_ctx_init_packages(PACKAGES_RPM, RPM_PACKAGE_INFO, argv[4], NULL,
                                                    NULL);
        else
        {
            g_printerr(_("Wrong rpm package_info arguments.\n"));
            packages_module_rpm_print_help();
            ERR_EXIT();
        }
        break;

    case RPM_REMOVE:
        if (argc == 5)
            (*ctx) = alteratorctl_ctx_init_packages(PACKAGES_RPM, RPM_REMOVE, argv[4], NULL, NULL);
        else
        {
            g_printerr(_("Wrong rpm remove arguments.\n"));
            packages_module_rpm_print_help();
            ERR_EXIT();
        }
        break;

    default:
        g_printerr(_("Unknown rpm module command.\n"));
        packages_module_rpm_print_help();
        ERR_EXIT();
    }

end:
    return ret;
}

static int packages_module_parse_repo_arguments(AlteratorCtlPackagesModule* module, int argc,
                                                char** argv, alteratorctl_ctx_t** ctx)
{
    int ret                            = 0;
    AlteratorCtlModuleInterface* iface = GET_ALTERATOR_CTL_MODULE_INTERFACE((void*) module);

    if (module->alterator_ctl_app->arguments->help_type == ALTERATORCTL_SUBMODULE_HELP)
    {
        packages_module_repo_print_help();
        goto end;
    }

    if (argc < 4)
    {
        g_printerr(_("Wrong repo arguments.\n"));
        packages_module_repo_print_help();
        ERR_EXIT();
    }

    int selected_subcommand = iface->get_command_id(module->commands[REPO_HASH_TABLE], argv[3]);
    switch (selected_subcommand)
    {
    case REPO_ADD:
        if (argc == 5)
            (*ctx) = alteratorctl_ctx_init_packages(PACKAGES_REPO, REPO_ADD, argv[4], NULL, NULL);
        else
        {
            g_printerr(_("Wrong repo add files arguments.\n"));
            packages_module_repo_print_help();
            ERR_EXIT();
        }
        break;

    case REPO_INFO:
        (*ctx) = alteratorctl_ctx_init_packages(PACKAGES_REPO, REPO_INFO, NULL, NULL, NULL);
        break;

    case REPO_LIST:
        (*ctx) = alteratorctl_ctx_init_packages(PACKAGES_REPO, REPO_LIST, NULL, NULL, NULL);
        break;

    case REPO_REMOVE:
        if (argc == 5)
            (*ctx) = alteratorctl_ctx_init_packages(PACKAGES_REPO, REPO_REMOVE, argv[4], NULL,
                                                    NULL);
        else
        {
            g_printerr(_("Wrong repo remove info arguments.\n"));
            packages_module_repo_print_help();
            ERR_EXIT();
        }
        break;

    default:
        g_printerr(_("Unknown repo module command.\n"));
        packages_module_repo_print_help();
        ERR_EXIT();
    }

end:
    return ret;
}

static gint packages_module_sort_result(gconstpointer a, gconstpointer b)
{
    return g_utf8_collate((gchar*) ((GPtrArray*) a)->pdata, (gchar*) ((GPtrArray*) b)->pdata);
}

static int packages_module_validate_object_and_iface(AlteratorCtlPackagesModule* module,
                                                     const gchar* object, const gchar* iface)
{
    int ret          = 0;
    int object_exist = 0;
    int iface_exists = 0;

    // Check object
    if (module->gdbus_source->alterator_gdbus_source_check_object_by_path(module->gdbus_source,
                                                                          object, &object_exist)
        < 0)
    {
        g_printerr(_("The object %s doesn't exist.\n"), object);
        ERR_EXIT();
    }

    if (object_exist == 0)
    {
        g_printerr(_("The object %s doesn't exist.\n"), object);
        ERR_EXIT();
    }

    // check interface of the object
    if (module->gdbus_source->alterator_gdbus_source_check_object_by_iface(module->gdbus_source,
                                                                           object, iface,
                                                                           &iface_exists)
        < 0)
    {
        g_printerr(_("Error when checking if an object: %s has an interface %s.\n"), object, iface);
        ERR_EXIT();
    }

    if (iface_exists == 0)
    {
        g_printerr(_("Object %s has no interface %s.\n"), object, iface);
        ERR_EXIT();
    }

end:
    return ret;
}
