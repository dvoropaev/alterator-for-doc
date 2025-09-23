#include "alteratorctldiagmodule.h"
#include "alteratorctlapp.h"

#include <gio/gioenums.h>
#include <glib/gprintf.h>
#include <sys/ioctl.h>
#include <toml.h>

#define DIAG_INTERFACE_NAME "org.altlinux.alterator.diag1"
#define DIAG_STDOUT_SIGNAL_NAME "diag1_stdout_signal"
#define DIAG_STDERR_SIGNAL_NAME "diag1_stderr_signal"

#define DIAG_LIST_METHOD_NAME "List"
#define DIAG_INFO_METHOD_NAME "Info"
#define DIAG_REPORT_METHOD_NAME "Report"
#define DIAG_RUN_METHOD_NAME "Run"

#define DIAG_ALTERATOR_ENTRY_TOOLNAME_KEY_NAME "name"
#define DIAG_ALTERATOR_ENTRY_REPORT_KEY_NAME "report_suffix"
#define DIAG_ALTERATOR_ENTRY_DISPLAY_NAME_TABLE_NAME "display_name"

#define DIAG_ALTERATOR_ENTRY_TESTS_TABLE_NAME "tests"

#define DIAG_LIST_TEST_SYSTEM_RESULT_KEY_NAME "system_bus"
#define DIAG_LIST_TEST_SESSION_RESULT_KEY_NAME "session_bus"

#define DIAG_LIST_TOOLS_SYSTEM_RESULT_KEY_NAME "system_bus"
#define DIAG_LIST_TOOLS_SESSION_RESULT_KEY_NAME "session_bus"

typedef struct
{
    char *subcommand;
    enum diag_sub_commands ids;
} diag_module_subcommands_t;

typedef enum
{
    TOOL,
    TEST
} diag_module_elem_type;

typedef enum
{
    PASS,
    FAIL,
    WARN
} test_result;

typedef struct diag_data_t
{
    diag_module_elem_type type;
    gchar *name;
    gchar *display_name;
    gchar *path;
} diag_data_t;

static diag_module_subcommands_t diag_module_subcommands_list[] = {{"info", DIAG_INFO},
                                                                   {"list", DIAG_LIST},
                                                                   {"report", DIAG_REPORT},
                                                                   {"run", DIAG_RUN},
                                                                   {"help", DIAG_HELP}};

static GObjectClass *diag_module_parent_class = NULL;
static alterator_ctl_module_t diag_module     = {0};
static gboolean name_only                     = FALSE;
static gboolean no_name                       = FALSE;
static gboolean path_only                     = FALSE;
static gboolean display_name_only             = FALSE;
static gboolean no_display_name               = FALSE;
static gboolean system_bus                    = FALSE;
static gboolean session_bus                   = FALSE;
static gboolean both_buses                    = FALSE;
static gboolean is_dbus_call_error            = FALSE;

static void diag_module_class_init(AlteratorCtlDiagModuleClass *klass);
static void diag_ctl_class_finalize(GObject *klass);

static void diag_module_alterator_interface_init(gpointer iface, gpointer iface_data);
static void diag_module_alterator_interface_finalize(gpointer iface, gpointer iface_data);

AlteratorCtlDiagModule *diag_module_new(gpointer app);
void diag_module_free(AlteratorCtlDiagModule *module);

static void fill_command_hash_table(GHashTable *commands);

static int diag_module_parse_options(AlteratorCtlDiagModule *module,
                                     int *argc,
                                     char **argv,
                                     enum diag_sub_commands command);
static int diag_module_parse_arguments(AlteratorCtlDiagModule *module, int argc, char **argv, alteratorctl_ctx_t **ctx);

static int diag_module_get_display_name(AlteratorGDBusSource *gdbus_source,
                                        const gchar *diag_str_id,
                                        GNode *data,
                                        gchar **result);
static int diag_module_get_test_display_name(AlteratorGDBusSource *gdbus_source,
                                             const gchar *tool_path,
                                             const gchar *test_name,
                                             gchar **result);
static diag_data_t *diag_module_data_new(AlteratorGDBusSource *gdbus_source,
                                         const gchar *diag_str_id,
                                         GNode *diag_node,
                                         diag_module_elem_type type);
static int diag_module_data_free(diag_data_t *data);
static gint diag_module_sort_result(gconstpointer a, gconstpointer b, gpointer user_data);

static int diag_module_info_subcommand(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx);
static int diag_module_list_tests_subcommand(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx);
static int diag_module_list_all_subcommand(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx);
static int diag_module_report_subcommand(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx);
static int diag_module_run_tool_test_subcommand(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx, int *result);
static int diag_module_run_tool_subcommand(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx, int *result);

static int diag_module_handle_results(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx);
static int diag_module_handle_info_results(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx);
static int diag_module_handle_list_tests_results(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx);
static int diag_module_handle_list_all_results(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx);
static int diag_module_handle_report_results(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx);
static int diag_module_handle_tool_test_results(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx);
static int diag_module_handle_tool_results(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx);

static int diag_module_print_list_with_filters(AlteratorCtlDiagModule *module, diag_data_t *diag_data);

static int diag_module_validate_alterator_entry(AlteratorGDBusSource *source, GNode *alterator_entry_data);

static int diag_module_validate_object_and_iface(AlteratorGDBusSource *source, const gchar *object, const gchar *iface);

static int diag_module_get_list_of_tests(AlteratorCtlDiagModule *module,
                                         AlteratorGDBusSource *source,
                                         const gchar *path,
                                         GHashTable **result);

static int diag_module_get_and_run_test(AlteratorCtlDiagModule *module,
                                        AlteratorGDBusSource *source,
                                        const gchar *diag_tool_str_id,
                                        const gchar *test,
                                        int *result);

static int diag_module_run_test(
    AlteratorCtlDiagModule *module, AlteratorGDBusSource *source, const gchar *path, const gchar *test, int *result);

static int diag_module_get_file_suffix(AlteratorGDBusSource *source, const gchar *path, gchar **file_suffix);

static int diag_module_get_report(AlteratorGDBusSource *source, const gchar *path, GVariant **result);

static void diag_module_run_stdout_signal_handler(GDBusConnection *connection,
                                                  const gchar *sender_name,
                                                  const gchar *object_path,
                                                  const gchar *interface_name,
                                                  const gchar *signal_name,
                                                  GVariant *parameters,
                                                  gpointer user_data);

static void diag_module_run_stderr_signal_handler(GDBusConnection *connection,
                                                  const gchar *sender_name,
                                                  const gchar *object_path,
                                                  const gchar *interface_name,
                                                  const gchar *signal_name,
                                                  GVariant *parameters,
                                                  gpointer user_data);

GType alterator_ctl_diag_module_get_type(void)
{
    static GType diag_module_type = 0;

    if (!diag_module_type)
    {
        static const GTypeInfo diag_module_info = {sizeof(AlteratorCtlDiagModuleClass),     /* class structure size */
                                                   NULL,                                    /* base class initializer */
                                                   NULL,                                    /* base class finalizer */
                                                   (GClassInitFunc) diag_module_class_init, /* class initializer */
                                                   NULL,                                    /* class finalizer */
                                                   NULL,                                    /* class data */
                                                   sizeof(AlteratorCtlDiagModule), /* instance structure size */
                                                   1,                              /* preallocated instances */
                                                   NULL,                           /* instance initializers */
                                                   NULL};

        const GInterfaceInfo alterator_module_interface_info = {
            (GInterfaceInitFunc) diag_module_alterator_interface_init,         /* interface_init */
            (GInterfaceFinalizeFunc) diag_module_alterator_interface_finalize, /* interface_finalize */
            NULL                                                               /* interface_data */
        };

        diag_module_type = g_type_register_static(G_TYPE_OBJECT, /* parent class */
                                                  "AlteratorCtlDiagModule",
                                                  &diag_module_info,
                                                  0);

        g_type_add_interface_static(diag_module_type, TYPE_ALTERATOR_CTL_MODULE, &alterator_module_interface_info);
    }

    return diag_module_type;
}

static void diag_module_class_init(AlteratorCtlDiagModuleClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->finalize = diag_ctl_class_finalize;

    diag_module_parent_class = g_type_class_peek_parent(klass);
}

static void diag_ctl_class_finalize(GObject *klass)
{
    AlteratorCtlDiagModuleClass *obj = (AlteratorCtlDiagModuleClass *) klass;

    G_OBJECT_CLASS(diag_module_parent_class)->finalize(klass);
}

static void diag_module_alterator_interface_init(gpointer iface, gpointer iface_data)
{
    AlteratorCtlModuleInterface *interface = iface;

    interface->run_with_args = diag_module_run_with_args;

    interface->run = diag_module_run;

    interface->print_help = diag_module_print_help;
}

static void diag_module_alterator_interface_finalize(gpointer iface, gpointer iface_data) {}

alterator_ctl_module_t *get_diag_module()
{
    int ret                           = 0;
    static gsize diag_ctl_module_init = 0;
    if (g_once_init_enter(&diag_ctl_module_init))
    {
        gsize module_id_size = g_strlcpy(diag_module.id,
                                         ALTERATOR_CTL_DIAG_MODULE_NAME,
                                         strlen(ALTERATOR_CTL_DIAG_MODULE_NAME) + 1);

        if (module_id_size != strlen(ALTERATOR_CTL_DIAG_MODULE_NAME))
        {
            g_printerr(_("Internal error in get_diag_module: unvaliable id of diag module.\n"));
            ERR_EXIT();
        }

        diag_module.new_object_func  = (gpointer) diag_module_new;
        diag_module.free_object_func = (gpointer) diag_module_free;

        gsize tmp = 42;

        g_once_init_leave(&diag_ctl_module_init, tmp);
    }

    return &diag_module;

end:
    return NULL;
}

AlteratorCtlDiagModule *diag_module_new(gpointer app)
{
    AlteratorCtlDiagModule *object = g_object_new(TYPE_ALTERATOR_CTL_DIAG_MODULE, NULL);

    object->commands = g_hash_table_new(g_str_hash, g_str_equal);
    fill_command_hash_table(object->commands);

    object->alterator_ctl_app = (AlteratorCtlApp *) app;

    object->gdbus_system_source = alterator_gdbus_source_new(object->alterator_ctl_app->arguments->verbose,
                                                             G_BUS_TYPE_SYSTEM);

    if (!alterator_ctl_is_root())
        object->gdbus_session_source = alterator_gdbus_source_new(object->alterator_ctl_app->arguments->verbose,
                                                                  G_BUS_TYPE_SESSION);

    object->exit_code = 0;

    return object;
}

void diag_module_free(AlteratorCtlDiagModule *module)
{
    g_hash_table_destroy(module->commands);

    if (module->gdbus_system_source)
    {
        alterator_gdbus_source_free(module->gdbus_system_source);
    }

    if (module->gdbus_session_source)
    {
        alterator_gdbus_source_free(module->gdbus_session_source);
    }

    g_object_unref(module);
}

int diag_module_run_with_args(gpointer self, int argc, char **argv)
{
    int ret                        = 0;
    alteratorctl_ctx_t *ctx        = NULL;
    AlteratorCtlDiagModule *module = ALTERATOR_CTL_DIAG_MODULE(self);

    if (!module)
    {
        g_printerr(_("The launch of the diag module has failed. Module doesn't exist.\n"));
        ERR_EXIT();
    }

    if (diag_module_parse_arguments(module, argc, argv, &ctx) < 0)
        ERR_EXIT();

    if (diag_module_run(module, ctx) < 0)
        ERR_EXIT();

    if (!is_dbus_call_error && diag_module_handle_results(module, &ctx) < 0)
        ERR_EXIT();

    ret = module->exit_code;

end:
    if (ctx)
        alteratorctl_ctx_free(ctx);

    return ret;
}

static void fill_command_hash_table(GHashTable *commands)
{
    for (int i = 0; i < sizeof(diag_module_subcommands_list) / sizeof(diag_module_subcommands_t); i++)
        g_hash_table_insert(commands, diag_module_subcommands_list[i].subcommand, &diag_module_subcommands_list[i].ids);
}

static int diag_module_get_display_name(AlteratorGDBusSource *gdbus_source,
                                        const gchar *diag_str_id,
                                        GNode *data,
                                        gchar **result)
{
    int ret                                   = 0;
    gchar *locale                             = NULL;
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) gdbus_source->info_parser;

    if (!(locale = alterator_ctl_get_language()))
        ERR_EXIT();

    GHashTable *display_name = NULL;
    if (!info_parser->alterator_ctl_module_info_parser_find_table(info_parser,
                                                                  data,
                                                                  &display_name,
                                                                  DIAG_ALTERATOR_ENTRY_DISPLAY_NAME_TABLE_NAME,
                                                                  NULL))
    {
        g_printerr(_("Can't get display name of diag tool or test %s. Display name data by key %s is empty.\n"),
                   diag_str_id,
                   DIAG_ALTERATOR_ENTRY_DISPLAY_NAME_TABLE_NAME);
        ERR_EXIT();
    }

    toml_value *display_name_locale_value = g_hash_table_lookup(display_name, locale);
    if (!display_name_locale_value)
        display_name_locale_value = g_hash_table_lookup(display_name, LOCALE_FALLBACK);

    (*result) = g_strdup(display_name_locale_value->str_value);

end:
    g_free(locale);

    return ret;
}

static int diag_module_get_test_display_name(AlteratorGDBusSource *gdbus_source,
                                             const gchar *tool_path,
                                             const gchar *test_name,
                                             gchar **result)
{
    int ret                                   = 0;
    GNode *parsed_tool_alterator_entry        = NULL;
    GNode *tests_table                        = NULL;
    GNode *test_node                          = NULL;
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) gdbus_source->info_parser;

    if (!gdbus_source || !tool_path || !test_name || !result)
        ERR_EXIT();

    // Get the tool's alterator entry data
    if (info_parser->alterator_ctl_module_info_parser_get_specified_object_data(info_parser,
                                                                                gdbus_source,
                                                                                tool_path,
                                                                                DIAG_INTERFACE_NAME,
                                                                                &parsed_tool_alterator_entry)
        < 0)
    {
        g_printerr(_("Can't get alterator entry data for tool %s.\n"), tool_path);
        ERR_EXIT();
    }

    // Find the tests table
    if (!(tests_table = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
              info_parser, parsed_tool_alterator_entry, DIAG_ALTERATOR_ENTRY_TESTS_TABLE_NAME)))
    {
        g_printerr(_("Can't find tests table for tool %s.\n"), tool_path);
        ERR_EXIT();
    }

    // Find the specific test node
    if (!(test_node = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser,
                                                                                     tests_table,
                                                                                     test_name)))
    {
        g_printerr(_("Can't find test %s in tool %s.\n"), test_name, tool_path);
        ERR_EXIT();
    }

    // Get the display name for this test
    if (diag_module_get_display_name(gdbus_source, test_name, test_node, result) < 0)
    {
        // If we can't get display name, fall back to test name
        *result = g_strdup(test_name);
    }

end:
    if (parsed_tool_alterator_entry)
        info_parser->alterator_ctl_module_info_parser_result_tree_free(info_parser, parsed_tool_alterator_entry);

    return ret;
}

static diag_data_t *diag_module_data_new(AlteratorGDBusSource *gdbus_source,
                                         const gchar *diag_str_id,
                                         GNode *diag_node,
                                         diag_module_elem_type type)
{
    diag_data_t *result = g_malloc0(sizeof(diag_data_t));
    result->type        = type;
    result->name        = g_strdup(diag_str_id);
    diag_module_get_display_name(gdbus_source, diag_str_id, diag_node, &result->display_name);
    result->path = g_strdup(
        gdbus_source->alterator_gdbus_source_get_path_by_name(gdbus_source, diag_str_id, DIAG_INTERFACE_NAME));

    return result;
}

static int diag_module_data_free(diag_data_t *data)
{
    int ret = 0;
    if (!data)
        return ret;
    g_free(data->display_name);
    g_free(data->name);
    g_free(data->path);
    g_free(data);

end:
    return ret;
}

static gint diag_module_sort_result(gconstpointer a, gconstpointer b, gpointer user_data)
{
    diag_data_t *diag_data_first  = (diag_data_t *) ((GPtrArray *) a)->pdata;
    diag_data_t *diag_data_second = (diag_data_t *) ((GPtrArray *) b)->pdata;

    const gchar *first_comparable_data  = NULL;
    const gchar *second_comparable_data = NULL;
    gboolean is_verbose                 = *((gboolean *) user_data);

    if ((display_name_only || no_name) && !no_display_name && !path_only && !name_only)
    {
        first_comparable_data  = g_strdup((const gchar *) diag_data_first->display_name);
        second_comparable_data = g_strdup((const gchar *) diag_data_second->display_name);
    }
    else if ((no_display_name || name_only) && !path_only && !display_name_only)
    {
        first_comparable_data  = g_strdup((const gchar *) diag_data_first->name);
        second_comparable_data = g_strdup((const gchar *) diag_data_second->name);
    }
    else if (diag_data_first->type == TOOL && diag_data_second->type == TOOL
             && (no_display_name || (is_verbose && no_name) || path_only) && !name_only && !display_name_only)
    {
        first_comparable_data  = g_strdup(diag_data_first->path);
        second_comparable_data = g_strdup(diag_data_second->path);
    }
    else
    {
        // Default
        first_comparable_data  = diag_data_first->display_name;
        second_comparable_data = diag_data_second->display_name;
    }

    return g_utf8_collate(first_comparable_data, second_comparable_data);
}

static int diag_module_parse_arguments(AlteratorCtlDiagModule *module, int argc, char **argv, alteratorctl_ctx_t **ctx)
{
    int ret = 0;

    AlteratorCtlModuleInterface *iface = GET_ALTERATOR_CTL_MODULE_INTERFACE((void *) module);

    if (!iface)
    {
        g_printerr(_("Parsing of diag module arguments failed. Module interface isn't initialized.\n"));
        ERR_EXIT();
    }

    if (module->alterator_ctl_app->arguments->module_help)
    {
        iface->print_help(module);
        goto end;
    }

    // -1 - default command
    int selected_subcommand = iface->get_command_id(module->commands, argv[2]);
    if (diag_module_parse_options(module, &argc, argv, selected_subcommand) < 0)
        ERR_EXIT();

    if ((argc < 2) || (argc > 5))
    {
        g_printerr(_("Wrong arguments in diag module.\n"));
        iface->print_help(module);
        ERR_EXIT();
    }

    switch (selected_subcommand)
    {
    case DIAG_INFO:
        if (argc == 4)
            (*ctx) = alteratorctl_ctx_init_diag(selected_subcommand, argv[3], NULL, NULL, NULL, NULL);
        else
        {
            g_printerr(_("Wrong arguments to diag info command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    case DIAG_LIST:
        if (argc == 3)
            (*ctx) = alteratorctl_ctx_init_diag(DIAG_LIST_ALL_PRIV, NULL, NULL, NULL, NULL, NULL);
        else if (argc == 4)
            (*ctx) = alteratorctl_ctx_init_diag(DIAG_LIST_TESTS_PRIV, argv[3], NULL, NULL, NULL, NULL);
        else
        {
            g_printerr(_("Wrong arguments to diag list command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    case DIAG_REPORT:
        if (argc == 5)
            (*ctx) = alteratorctl_ctx_init_diag(selected_subcommand, argv[3], argv[4], NULL, NULL, NULL);
        else
        {
            g_printerr(_("Wrong arguments to diag report command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    case DIAG_RUN:
        if (argc == 4)
            (*ctx) = alteratorctl_ctx_init_diag(DIAG_RUN_TOOL_PRIV, argv[3], NULL, NULL, NULL, NULL);
        else if (argc == 5)
            (*ctx) = alteratorctl_ctx_init_diag(DIAG_RUN_TOOL_TEST_PRIV, argv[3], argv[4], NULL, NULL, NULL);
        else
        {
            g_printerr(_("Wrong arguments to diag run command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    default:
        if (argc == 2)
            (*ctx) = alteratorctl_ctx_init_diag(DIAG_LIST_ALL_PRIV, NULL, NULL, NULL, NULL, NULL);
        else if (argc == 3)
        {
            (*ctx) = alteratorctl_ctx_init_diag(DIAG_LIST_TESTS_PRIV, argv[2], NULL, NULL, NULL, NULL);
        }
        else
        {
            g_printerr(_("Wrong diag module command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
    }

end:
    return ret;
}

int diag_module_run(gpointer self, gpointer data)
{
    int ret                            = 0;
    AlteratorCtlModuleInterface *iface = GET_ALTERATOR_CTL_MODULE_INTERFACE(self);
    AlteratorCtlDiagModule *module     = ALTERATOR_CTL_DIAG_MODULE(self);

    if (!self)
    {
        g_printerr(_("The launch of the diag module has failed. Module doesn't exist.\n"));
        ERR_EXIT();
    }

    if (module->alterator_ctl_app->arguments->module_help)
        goto end;

    if (!data)
    {
        g_printerr(_("The launch of the diag module has failed. Module doesn't exist.\n"));
        ERR_EXIT();
    }

    alteratorctl_ctx_t *ctx = (alteratorctl_ctx_t *) data;

    int subcommand_id = g_variant_get_int32(ctx->subcommands_ids);

    switch (subcommand_id)
    {
    case DIAG_INFO:
        if (diag_module_info_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;
    case DIAG_LIST_TESTS_PRIV:
        if (diag_module_list_tests_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;
    case DIAG_LIST_ALL_PRIV:
        if (diag_module_list_all_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;
    case DIAG_REPORT:
        if (diag_module_report_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;
    case DIAG_RUN_TOOL_TEST_PRIV:
        if (diag_module_run_tool_test_subcommand(module, &ctx, &module->exit_code) < 0)
            ERR_EXIT();
        break;
    case DIAG_RUN_TOOL_PRIV:
        if (diag_module_run_tool_subcommand(module, &ctx, &module->exit_code) < 0)
            ERR_EXIT();
        break;
    case DIAG_HELP:
        iface->print_help(module);
        break;
    }

    ret = module->exit_code;

end:
    return ret;
}

int diag_module_print_help(gpointer self)
{
    int ret = 0;
    g_print(_("Usage:\n"));
    g_print(_("  alteratorctl diag [COMMAND [args...]] [OPTIONS]\n"));
    g_print(_("  alteratorctl diag [OPTIONS] List all diagnostic tools\n"));
    g_print(_("  alteratorctl diag <path | name> [OPTIONS]\n"
              "                              List all test from specified diagnostic tool\n\n"));
    g_print(_("Commands:\n"));
    g_print(_("  info <path | name>          Print tool info\n"));
    g_print(_("  list [path | name]          List all diagnostic tools or tool tests (default\n"
              "                              command)\n"));
    g_print(_("  report <path | name> <file> Save report to the file with suffix\n"));
    g_print(_("  run <path | name> [test]    Run all tests or specified test\n\n"));
    g_print(_("Options:\n"));
    g_print(_("  -a, --all                   Choise system and session buses\n"));
    g_print(_("  -v, --verbose               Add diagnostic tools paths to diag list output\n"
              "                              or for enable diag run verbose output\n"));
    g_print(_("  -d, --display-name-only     Show only diag tools and tests display names\n"));
    g_print(_("  -D, --no-display-name       Show diag tools and tests without display names\n"));
    g_print(_("  -p, --path-only             Show only diag tools and tests paths on D-Bus\n"));
    g_print(_("  -n, --name-only             Show only diag tools and tests names\n"));
    g_print(_("  -N, --no-name               Hide diag tools and tests names\n"));
    g_print(_("  --session                   Choise session bus\n"));
    g_print(_("  --system                    Choise system bus\n"));
    g_print(_("  -h, --help                  Usage help\n\n"));
end:
    return ret;
}

static int diag_module_info_subcommand(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                              = 0;
    gchar *diag_tool_str_id              = NULL;
    gchar *param2                        = NULL;
    gchar *param3                        = NULL;
    gchar *info_from_system_bus          = NULL;
    gchar *info_from_session_bus         = NULL;
    gboolean object_exist_on_system_bus  = FALSE;
    gboolean object_exist_on_session_bus = FALSE;
    gchar *system_bus_prefix             = _("<<< Info from system bus:\n");
    gchar *session_bus_prefix            = _("<<< Info from session bus:\n");
    gchar *common_info                   = NULL;
    gboolean is_root                     = alterator_ctl_is_root();

    if (!module)
    {
        g_printerr(_("The call to the diag info method failed. The diag module doesn't exist.\n"));
        ERR_EXIT();
    }

    if (!ctx || !(*ctx))
    {
        g_printerr(_("The call to the diag info method failed. Module data is empty.\n"));
        ERR_EXIT();
    }

    g_variant_get((*ctx)->parameters, "(msmsms)", &diag_tool_str_id, param2, param3);
    if (!diag_tool_str_id)
    {
        g_printerr(_("The call to the diag info method failed. Tool name or path to it's object on D-Bus is empty.\n"));
        ERR_EXIT();
    }

    //Object and interface validation
    if (diag_module_validate_object_and_iface(module->gdbus_system_source, diag_tool_str_id, DIAG_INTERFACE_NAME) > 0)
        object_exist_on_system_bus = TRUE;

    if (!is_root
        && diag_module_validate_object_and_iface(module->gdbus_session_source, diag_tool_str_id, DIAG_INTERFACE_NAME)
               > 0)
        object_exist_on_session_bus = TRUE;

    if (!object_exist_on_system_bus || !object_exist_on_session_bus)
    {
        if (is_root && !object_exist_on_system_bus)
        {
            g_printerr(_("Can't find object %s on system bus or it has invalid interface.\n"), diag_tool_str_id);
            ERR_EXIT();
        }

        if (!is_root && !object_exist_on_session_bus && !object_exist_on_system_bus)
        {
            g_printerr(_("Can't find object %s on system or session bus or it has invalid interface.\n"),
                       diag_tool_str_id);
            ERR_EXIT();
        }
    }

    //Get info from system bus
    if (object_exist_on_system_bus && (system_bus || both_buses))
    {
        if (module->gdbus_system_source->alterator_gdbus_source_get_text_of_alterator_entry_by_path(
                module->gdbus_system_source, diag_tool_str_id, DIAG_INTERFACE_NAME, &info_from_system_bus)
            < 0)
            ERR_EXIT();
    }

    //Get info from session bus
    if (!is_root && object_exist_on_session_bus && (session_bus || both_buses))
    {
        if (module->gdbus_session_source->alterator_gdbus_source_get_text_of_alterator_entry_by_path(
                module->gdbus_session_source, diag_tool_str_id, DIAG_INTERFACE_NAME, &info_from_session_bus)
            < 0)
            ERR_EXIT();
    }

    if (info_from_system_bus || info_from_session_bus)
    {
        if (info_from_system_bus)
        {
            if (!is_root && info_from_session_bus)
                common_info = g_strconcat(system_bus_prefix, info_from_system_bus, "\n", NULL);
            else
                common_info = g_strconcat(system_bus_prefix, info_from_system_bus, NULL);
        }

        if (!is_root && info_from_session_bus)
        {
            gchar *temp = common_info;
            if (common_info)
                common_info = g_strconcat(common_info, session_bus_prefix, info_from_session_bus, NULL);
            else
                common_info = g_strconcat(session_bus_prefix, info_from_session_bus, NULL);
            g_free(temp);
        }

        (*ctx)->results      = (gpointer) (common_info);
        (*ctx)->free_results = g_free;
    }
    else
    {
        g_printerr(_("Can't get info of tool %s. The tool doesn't exist.\n"), diag_tool_str_id);
        ERR_EXIT();
    }

end:
    g_free(diag_tool_str_id);

    g_free(param2);

    g_free(param3);

    g_free(info_from_system_bus);

    g_free(info_from_session_bus);

    return ret;
}

static int diag_module_list_tests_subcommand(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                              = 0;
    gchar *diag_tool_str_id              = NULL;
    gchar *param2                        = NULL;
    gchar *param3                        = NULL;
    GHashTable *tests_from_system_bus    = NULL;
    GHashTable *tests_from_session_bus   = NULL;
    GHashTable *result_table             = NULL;
    gboolean object_exist_on_system_bus  = FALSE;
    gboolean object_exist_on_session_bus = FALSE;
    gboolean is_root                     = alterator_ctl_is_root();

    if (!module || !ctx || !(*ctx))
    {
        g_printerr(_("The call to the diag list method failed. The module doesn't exist.\n"));
        ERR_EXIT();
    }

    g_variant_get((*ctx)->parameters, "(msmsms)", &diag_tool_str_id, &param2, &param3);
    if (!diag_tool_str_id)
    {
        g_printerr(_("The call to the diag list method failed. Tool name or path to it's object on D-Bus is empty.\n"));
        ERR_EXIT();
    }

    //Object and interface validation
    if (diag_module_validate_object_and_iface(module->gdbus_system_source, diag_tool_str_id, DIAG_INTERFACE_NAME) > 0)
        object_exist_on_system_bus = TRUE;

    if (!is_root
        && diag_module_validate_object_and_iface(module->gdbus_session_source, diag_tool_str_id, DIAG_INTERFACE_NAME)
               > 0)
        object_exist_on_session_bus = TRUE;

    if (!object_exist_on_system_bus || !object_exist_on_session_bus)
    {
        if (is_root && !object_exist_on_system_bus)
        {
            g_printerr(_("Can't find object %s on system bus or it has invalid interface.\n"), diag_tool_str_id);
            ERR_EXIT();
        }

        if (!is_root && !object_exist_on_session_bus && !object_exist_on_system_bus)
        {
            g_printerr(_("Can't find object %s on system or session bus or it has invalid interface.\n"),
                       diag_tool_str_id);
            ERR_EXIT();
        }
    }

    if (object_exist_on_system_bus && (system_bus || both_buses))
    {
        if (diag_module_get_list_of_tests(module, module->gdbus_system_source, diag_tool_str_id, &tests_from_system_bus)
            < 0)
        {
            g_printerr(
                _("The call to the diag list method failed. Can't get list of tools from object %s on system bus.\n"),
                diag_tool_str_id);
            ERR_EXIT();
        }
    }

    if (!is_root && object_exist_on_session_bus && (session_bus || both_buses))
    {
        if (diag_module_get_list_of_tests(module, module->gdbus_session_source, diag_tool_str_id, &tests_from_session_bus)
            < 0)
        {
            g_printerr(
                _("The call to the diag list method failed. Can't get list of tools from object %s on session bus.\n"),
                diag_tool_str_id);
            ERR_EXIT();
        }
    }

    result_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_hash_table_unref);

    if (tests_from_system_bus)
        g_hash_table_insert(result_table,
                            strdup(DIAG_LIST_TEST_SYSTEM_RESULT_KEY_NAME),
                            g_hash_table_ref(tests_from_system_bus));

    if (tests_from_session_bus)
        g_hash_table_insert(result_table,
                            strdup(DIAG_LIST_TEST_SESSION_RESULT_KEY_NAME),
                            g_hash_table_ref(tests_from_session_bus));

    (*ctx)->results = (gpointer) g_hash_table_ref(result_table);

    (*ctx)->free_results = (void (*)(gpointer)) g_hash_table_unref;

end:

    if (tests_from_system_bus)
        g_hash_table_unref(tests_from_system_bus);

    if (tests_from_session_bus)
        g_hash_table_unref(tests_from_session_bus);

    if (result_table)
        g_hash_table_unref(result_table);

    g_free(diag_tool_str_id);

    g_free(param2);

    g_free(param3);

    return ret;
}

static int diag_module_list_all_subcommand(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx)
{
    int ret            = 0;
    GHashTable *result = NULL;
    gboolean is_root   = alterator_ctl_is_root();

    AlteratorCtlModuleInfoParser *system_info_parser  = module->gdbus_system_source->info_parser;
    AlteratorCtlModuleInfoParser *session_info_parser = !is_root ? module->gdbus_session_source->info_parser : NULL;

    if (!module)
    {
        g_printerr(_("The call to the diag list method failed. The module doesn't exist.\n"));
        ERR_EXIT();
    }

    if (!ctx || !(*ctx))
    {
        g_printerr(_("The call to the diag list method failed. The context doesn't exist.\n"));
        ERR_EXIT();
    }

    result = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_hash_table_unref);

    if ((system_bus || both_buses)
        && system_info_parser
                   ->alterator_ctl_module_info_parser_create_names_by_dbus_paths_table(system_info_parser,
                                                                                       module->gdbus_system_source,
                                                                                       DIAG_INTERFACE_NAME)
               < 0)
        ERR_EXIT();

    if (!is_root && (session_bus || both_buses)
        && session_info_parser
                   ->alterator_ctl_module_info_parser_create_names_by_dbus_paths_table(session_info_parser,
                                                                                       module->gdbus_session_source,
                                                                                       DIAG_INTERFACE_NAME)
               < 0)
        ERR_EXIT();

    if (system_info_parser->names_by_dbus_paths && (system_bus || both_buses))
        g_hash_table_insert(result,
                            strdup(DIAG_LIST_TOOLS_SYSTEM_RESULT_KEY_NAME),
                            g_hash_table_ref(system_info_parser->names_by_dbus_paths));

    if (!is_root && session_info_parser->names_by_dbus_paths && (session_bus || both_buses))
        g_hash_table_insert(result,
                            strdup(DIAG_LIST_TOOLS_SESSION_RESULT_KEY_NAME),
                            g_hash_table_ref(session_info_parser->names_by_dbus_paths));

    (*ctx)->results      = g_hash_table_ref(result);
    (*ctx)->free_results = (void(*)) g_hash_table_unref;

end:

    if (result)
        g_hash_table_unref(result);

    return ret;
}

static int diag_module_report_subcommand(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                              = 0;
    gchar *diag_tool_str_id              = NULL;
    gchar *param2                        = NULL;
    gchar *param3                        = NULL;
    gboolean object_exist_on_system_bus  = FALSE;
    gboolean object_exist_on_session_bus = FALSE;
    GVariant *system_report              = NULL;
    GVariant *session_report             = NULL;
    GPtrArray *result_array              = NULL;
    gboolean is_root                     = alterator_ctl_is_root();

    if (!module)
    {
        g_printerr(_("The call to the diag report method failed. The module doesn't exist.\n"));
        ERR_EXIT();
    }

    if (!ctx || !(*ctx))
    {
        g_printerr(_("The call to the diag report method failed. Module data is empty.\n"));
        ERR_EXIT();
    }

    g_variant_get((*ctx)->parameters, "(msmsms)", &diag_tool_str_id, &param2, &param3);
    if (!diag_tool_str_id)
    {
        g_printerr(
            _("The call to the diag report method failed. Tool name or path to it's object on D-Bus is empty.\n"));
        ERR_EXIT();
    }

    if (diag_module_validate_object_and_iface(module->gdbus_system_source, diag_tool_str_id, DIAG_INTERFACE_NAME) > 0)
        object_exist_on_system_bus = TRUE;

    if (!is_root
        && diag_module_validate_object_and_iface(module->gdbus_session_source, diag_tool_str_id, DIAG_INTERFACE_NAME)
               > 0)
        object_exist_on_session_bus = TRUE;

    if (!object_exist_on_system_bus || !object_exist_on_session_bus)
    {
        if (is_root && !object_exist_on_system_bus)
        {
            g_printerr(_("Can't find object %s on system bus or it has invalid interface.\n"), diag_tool_str_id);
            ERR_EXIT();
        }

        if (!is_root && !object_exist_on_session_bus && !object_exist_on_system_bus)
        {
            g_printerr(_("Can't find object %s on system or session bus or it has invalid interface.\n"),
                       diag_tool_str_id);
            ERR_EXIT();
        }
    }

    //Get reports
    if (object_exist_on_system_bus && (system_bus || both_buses))
    {
        if (diag_module_get_report(module->gdbus_system_source, diag_tool_str_id, &system_report) < 0)
            ERR_EXIT();
    }

    if (!is_root && object_exist_on_session_bus && (session_bus || both_buses))
    {
        if (diag_module_get_report(module->gdbus_session_source, diag_tool_str_id, &session_report) < 0)
            ERR_EXIT();
    }

    result_array = g_ptr_array_new();

    if (system_report)
        g_ptr_array_add(result_array, (gpointer) g_variant_ref(system_report));

    if (session_report)
    {
        g_ptr_array_add(result_array, (gpointer) g_variant_ref(session_report));
    }
    else if (is_root && !system_report)
    {
        g_printerr(_("Can't create session bus report of %s tool in root mode.\n"), diag_tool_str_id);
        ERR_EXIT();
    }

    (*ctx)->results = (gpointer) g_ptr_array_ref(result_array);

    (*ctx)->free_results = (void (*)(gpointer)) g_ptr_array_unref;

end:
    if (result_array)
        g_ptr_array_unref(result_array);

    if (session_report)
        g_variant_unref(session_report);

    if (system_report)
        g_variant_unref(system_report);

    g_free(diag_tool_str_id);

    g_free(param3);

    g_free(param2);

    return ret;
}

static int diag_module_run_tool_test_subcommand(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx, int *result)
{
    int ret                              = 0;
    gchar *diag_tool_str_id              = NULL;
    gchar *test                          = NULL;
    gchar *system_bus_prefix             = _("<<< Tests on the system bus\n");
    gchar *session_bus_prefix            = _("<<< Tests on the session bus\n");
    gchar *current_bus_prefix            = NULL;
    gboolean object_exist_on_system_bus  = FALSE;
    gboolean object_exist_on_session_bus = FALSE;
    AlteratorGDBusSource *source         = NULL;
    gboolean is_root                     = alterator_ctl_is_root();

    if (!module)
    {
        g_printerr(_("The call to the diag run method failed. The module doesn't exist.\n"));
        ERR_EXIT();
    }

    if (!ctx || !(*ctx))
    {
        g_printerr(_("The call to the diag run method failed. Module data is empty.\n"));
        ERR_EXIT();
    }

    g_variant_get((*ctx)->parameters, "(msmsms)", &diag_tool_str_id, &test, NULL);
    if (!diag_tool_str_id)
    {
        g_printerr(_("The call to the diag run method failed. Tool name or path to it's object on D-Bus is empty.\n"));
        ERR_EXIT();
    }

    if (!test)
    {
        g_printerr(_("The call to the diag run method failed. Test name is empty.\n"));
        ERR_EXIT();
    }

    if (both_buses)
    {
        if (diag_module_validate_object_and_iface(module->gdbus_system_source, diag_tool_str_id, DIAG_INTERFACE_NAME)
            > 0)
            object_exist_on_system_bus = TRUE;
        if (!is_root
            && diag_module_validate_object_and_iface(module->gdbus_session_source, diag_tool_str_id, DIAG_INTERFACE_NAME)
                   > 0)
            object_exist_on_session_bus = TRUE;

        if (!object_exist_on_system_bus || !object_exist_on_session_bus)
        {
            if (is_root && !object_exist_on_system_bus)
            {
                g_printerr(_("Can't find object %s on system bus or it has invalid interface.\n"), diag_tool_str_id);
                ERR_EXIT();
            }

            if (!is_root && !object_exist_on_system_bus && !object_exist_on_session_bus)
            {
                g_printerr(_("Can't find object %s on system or session bus or it has invalid interface.\n"),
                           diag_tool_str_id);
                ERR_EXIT();
            }
        }

        if (object_exist_on_system_bus)
        {
            g_print("%s", system_bus_prefix);
            if (diag_module_get_and_run_test(module, module->gdbus_system_source, diag_tool_str_id, test, result) < 0)
                ERR_EXIT();
        }

        if (!is_root && object_exist_on_session_bus)
        {
            if (object_exist_on_system_bus)
                g_print("\n");

            g_print("%s", session_bus_prefix);
            if (diag_module_get_and_run_test(module, module->gdbus_session_source, diag_tool_str_id, test, result) < 0)
                ERR_EXIT();
        }

        goto end;
    }
    else if (system_bus)
    {
        source             = module->gdbus_system_source;
        current_bus_prefix = system_bus_prefix;
    }
    else if (!is_root)
    {
        source             = module->gdbus_session_source;
        current_bus_prefix = session_bus_prefix;
    }

    if (diag_module_validate_object_and_iface(source, diag_tool_str_id, DIAG_INTERFACE_NAME) <= 0)
    {
        if (!object_exist_on_system_bus)
            g_printerr(_("Object %s isn't exists on system bus or has invalid interface.\n"), diag_tool_str_id);
        else if (!is_root && !object_exist_on_session_bus)
            g_printerr(_("Object %s isn't exists on session bus or has invalid interface.\n"), diag_tool_str_id);
        else
            g_printerr(_("Object %s isn't exists or has invalid interface.\n"), diag_tool_str_id);

        ERR_EXIT();
    }

    g_print("%s\n", current_bus_prefix);
    if (diag_module_get_and_run_test(module, source, diag_tool_str_id, test, result) < 0)
        ERR_EXIT();

end:
    g_free(diag_tool_str_id);

    g_free(test);

    return ret;
}

static int diag_module_run_tool_subcommand(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx, int *result)
{
    int ret                              = 0;
    gchar *diag_tool_str_id              = NULL;
    gchar *system_bus_prefix             = _("<<< Tests on the system bus\n");
    gchar *session_bus_prefix            = _("<<< Tests on the session bus\n");
    gchar *current_bus_prefix            = NULL;
    gboolean object_exist_on_system_bus  = FALSE;
    gboolean object_exist_on_session_bus = FALSE;
    AlteratorGDBusSource *source         = NULL;
    gboolean is_root                     = alterator_ctl_is_root();

    if (!module)
    {
        g_printerr(_("The call to the diag run method failed. The module doesn't exist.\n"));
        ERR_EXIT();
    }

    if (!ctx || !(*ctx))
    {
        g_printerr(_("The call to the diag run method failed. Module data is empty.\n"));
        ERR_EXIT();
    }

    g_variant_get((*ctx)->parameters, "(msmsms)", &diag_tool_str_id, NULL, NULL);
    if (!diag_tool_str_id)
    {
        g_printerr(_("The call to the diag run method failed. Tool name or path to it's object on D-Bus is empty.\n"));
        ERR_EXIT();
    }

    if (both_buses)
    {
        if (diag_module_validate_object_and_iface(module->gdbus_system_source, diag_tool_str_id, DIAG_INTERFACE_NAME)
            > 0)
            object_exist_on_system_bus = TRUE;
        if (!is_root
            && diag_module_validate_object_and_iface(module->gdbus_session_source, diag_tool_str_id, DIAG_INTERFACE_NAME)
                   > 0)
            object_exist_on_session_bus = TRUE;

        if (!object_exist_on_system_bus || !object_exist_on_session_bus)
        {
            if (is_root && !object_exist_on_system_bus)
            {
                g_printerr(_("Can't find object %s on system bus or it has invalid interface.\n"), diag_tool_str_id);
                ERR_EXIT();
            }

            if (!is_root && !object_exist_on_session_bus && !object_exist_on_system_bus)
            {
                g_printerr(_("Can't find object %s on system or session bus or it has invalid interface.\n"),
                           diag_tool_str_id);
                ERR_EXIT();
            }
        }

        if (is_root && !object_exist_on_system_bus && object_exist_on_session_bus)
        {
            g_printerr(_("Can't run tests of %s tool in session bus in root mode.\n"), diag_tool_str_id);
            ERR_EXIT();
        }

        if (object_exist_on_system_bus)
        {
            g_print("%s", system_bus_prefix);
            if (diag_module_get_and_run_test(module, module->gdbus_system_source, diag_tool_str_id, NULL, result) < 0)
                ERR_EXIT();
        }

        if (!is_root && object_exist_on_session_bus)
        {
            if (!module->alterator_ctl_app->arguments->verbose && object_exist_on_system_bus)
                g_print("\n");

            g_print("%s", session_bus_prefix);
            if (diag_module_get_and_run_test(module, module->gdbus_session_source, diag_tool_str_id, NULL, result) < 0)
                ERR_EXIT();
        }

        goto end;
    }
    else if (system_bus)
    {
        source             = module->gdbus_system_source;
        current_bus_prefix = system_bus_prefix;
    }
    else if (!is_root)
    {
        source             = module->gdbus_session_source;
        current_bus_prefix = session_bus_prefix;
    }

    if (diag_module_validate_object_and_iface(source, diag_tool_str_id, DIAG_INTERFACE_NAME) <= 0)
    {
        g_printerr(_("Object %s isn't exists or has invalid interface.\n"), diag_tool_str_id);
        ERR_EXIT();
    }

    g_print("%s\n", current_bus_prefix);
    if (diag_module_get_and_run_test(module, source, diag_tool_str_id, NULL, result) < 0)
        ERR_EXIT();

end:
    g_free(diag_tool_str_id);

    return ret;
}

static int diag_module_handle_results(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx)
{
    int ret = 0;

    if (!module)
    {
        g_printerr(_("Can't process the result in diag module. The diag module doesn't exist.\n"));
        ERR_EXIT();
    }

    if (!ctx)
    {
        g_printerr(_("Can't process the result in diag module. Module data is empty.\n"));
        ERR_EXIT();
    }

    if (module->alterator_ctl_app->arguments->module_help)
        goto end;

    switch (g_variant_get_int32((*ctx)->subcommands_ids))
    {
    case DIAG_INFO:
        if (diag_module_handle_info_results(module, ctx) < 0)
            ERR_EXIT();
        break;

    case DIAG_LIST_ALL_PRIV:
        if (diag_module_handle_list_all_results(module, ctx) < 0)
            ERR_EXIT();
        break;

    case DIAG_LIST_TESTS_PRIV:
        if (diag_module_handle_list_tests_results(module, ctx) < 0)
            ERR_EXIT();
        break;

    case DIAG_RUN_TOOL_PRIV:
        if (diag_module_handle_tool_results(module, ctx) < 0)
            ERR_EXIT();
        break;

    case DIAG_RUN_TOOL_TEST_PRIV:
        if (diag_module_handle_tool_test_results(module, ctx) < 0)
            ERR_EXIT();
        break;

    case DIAG_REPORT:
        if (diag_module_handle_report_results(module, ctx) < 0)
            ERR_EXIT();
        break;

    case DIAG_HELP:
        //module->diag_module_print_help(module);
        break;

    default:
        g_printerr(_("Unknown subcommand to handle results in diag module.\n"));
        //module->diag_module_print_help(module);
        ERR_EXIT();
        break;
    }

end:
    return ret;
}

static int diag_module_handle_info_results(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx)
{
    int ret       = 0;
    gchar *path   = NULL;
    gchar *param2 = NULL;
    gchar *param3 = NULL;

    if (!module)
    {
        g_printerr(_("Can't process the result of diag info method. The diag module doesn't exist.\n"));
        ERR_EXIT();
    }

    if (!ctx || !(*ctx))
    {
        g_printerr(_("Can't process the result of diag info method. Module data is empty.\n"));
        ERR_EXIT();
    }

    g_variant_get((*ctx)->parameters, "(msmsms)", &path, &param2, &param3);
    if (!path)
    {
        g_printerr(_("The call to the diag info method failed. Tool path on D-Bus is empty.\n"));
        ERR_EXIT();
    }

    if (!(*ctx)->results)
    {
        g_printerr(_("Empty result of the diag info method\n"));
        ERR_EXIT();
    }

    const gchar *output = (const gchar *) (*ctx)->results;
    if (output)
        print_with_pager(output);
    else
        g_print("\n");

end:
    g_free(param3);
    g_free(param2);
    g_free(path);

    return ret;
}

static int diag_module_handle_list_tests_results(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                                          = 0;
    gchar *diag_tool_str_id                          = NULL;
    gchar *diag_tool_path                            = NULL;
    gboolean exist_in_system_bus                     = FALSE;
    gboolean exist_in_session_bus                    = FALSE;
    GHashTable *system_bus_result_table              = NULL;
    GHashTable *session_bus_result_table             = NULL;
    GPtrArray *system_bus_result                     = NULL;
    GPtrArray *session_bus_result                    = NULL;
    GNode *system_diag_tool_object                   = NULL;
    GNode *session_diag_tool_object                  = NULL;
    gboolean is_root                                 = alterator_ctl_is_root();
    AlteratorCtlModuleInfoParser *system_info_parser = (AlteratorCtlModuleInfoParser *)
                                                           module->gdbus_system_source->info_parser;

    AlteratorCtlModuleInfoParser *session_info_parser = !is_root ? (AlteratorCtlModuleInfoParser *)
                                                                       module->gdbus_session_source->info_parser
                                                                 : NULL;

    g_variant_get((*ctx)->parameters, "(msmsms)", &diag_tool_str_id, NULL, NULL);
    if (!diag_tool_str_id)
    {
        g_printerr(_("Can't get tests list of empty diagnostic tool\n"));
        ERR_EXIT();
    }

    diag_tool_path = diag_tool_str_id[0] != '/'
                         ? g_strdup((gchar *) module->gdbus_system_source
                                        ->alterator_gdbus_source_get_path_by_name(module->gdbus_system_source,
                                                                                  diag_tool_str_id,
                                                                                  DIAG_INTERFACE_NAME))
                         : g_strdup(diag_tool_str_id);

    if (!diag_tool_path && !is_root)
        diag_tool_path = g_strdup(
            (gchar *) module->gdbus_session_source->alterator_gdbus_source_get_path_by_name(module->gdbus_session_source,
                                                                                            diag_tool_str_id,
                                                                                            DIAG_INTERFACE_NAME));

    if (!diag_tool_path)
    {
        g_print(_("Diag tool %s doesn't exist in session bus.\n"), diag_tool_str_id);
        ERR_EXIT();
    }

    if (!module)
    {
        g_printerr(_("Can't process the result of diag list method. The diag module doesn't exist.\n"));
        ERR_EXIT();
    }

    if (diag_module_validate_object_and_iface(module->gdbus_system_source, diag_tool_path, DIAG_INTERFACE_NAME) > 0)
    {
        exist_in_system_bus = TRUE;

        if ((system_bus || both_buses)
            && system_info_parser->alterator_ctl_module_info_parser_get_specified_object_data(system_info_parser,
                                                                                              module->gdbus_system_source,
                                                                                              diag_tool_path,
                                                                                              DIAG_INTERFACE_NAME,
                                                                                              &system_diag_tool_object)
                   < 0)
        {
            ERR_EXIT();
        }
    }

    if (!is_root
        && diag_module_validate_object_and_iface(module->gdbus_session_source, diag_tool_path, DIAG_INTERFACE_NAME) > 0)
    {
        exist_in_session_bus = TRUE;
        if ((session_bus || both_buses)
            && session_info_parser
                       ->alterator_ctl_module_info_parser_get_specified_object_data(session_info_parser,
                                                                                    module->gdbus_session_source,
                                                                                    diag_tool_path,
                                                                                    DIAG_INTERFACE_NAME,
                                                                                    &session_diag_tool_object)
                   < 0)

            ERR_EXIT();
    }

    if (!ctx || !(*ctx))
    {
        g_printerr(_("Can't process the result of diag list method. Module data is empty.\n"));
        ERR_EXIT();
    }

    if (!(*ctx)->results)
    {
        g_printerr(_("Empty result of the diag list method.\n"));
        ERR_EXIT();
    }

    GHashTable *result_table = (*ctx)->results;

    if (g_hash_table_contains(result_table, DIAG_LIST_TEST_SYSTEM_RESULT_KEY_NAME))
        system_bus_result_table = g_hash_table_lookup(result_table, DIAG_LIST_TEST_SYSTEM_RESULT_KEY_NAME);

    if (g_hash_table_contains(result_table, DIAG_LIST_TEST_SESSION_RESULT_KEY_NAME))
        session_bus_result_table = g_hash_table_lookup(result_table, DIAG_LIST_TEST_SESSION_RESULT_KEY_NAME);

    if (exist_in_system_bus && system_bus_result_table && (system_bus || both_buses))
    {
        system_bus_result = g_ptr_array_new_full(g_hash_table_size(system_bus_result_table),
                                                 (GDestroyNotify) diag_module_data_free);

        GNode *tests = NULL;

        if (!(tests = system_info_parser->alterator_ctl_module_info_parser_get_node_by_name(
                  system_info_parser, system_diag_tool_object, DIAG_ALTERATOR_ENTRY_TESTS_TABLE_NAME)))
        {
            g_printerr(_("Empty tests of diagnostic tool %s in system bus\n"), diag_tool_str_id);
            ERR_EXIT();
        }

        g_print(_("<<< Tests on system bus:\n"));

        for (GNode *test = tests->children; test != NULL; test = test->next)
        {
            alterator_entry_node *diag_data = (alterator_entry_node *) test->data;
            diag_data_t *data = diag_module_data_new(module->gdbus_system_source, diag_data->node_name, test, TEST);
            g_ptr_array_add(system_bus_result, data);
        }

        g_ptr_array_sort_with_data(system_bus_result,
                                   diag_module_sort_result,
                                   (gpointer) &module->alterator_ctl_app->arguments->verbose);

        for (guint i = 0; i < system_bus_result->len; i++)
            diag_module_print_list_with_filters(module, (diag_data_t *) g_ptr_array_index(system_bus_result, i));
    }

    if (exist_in_session_bus && session_bus_result_table && (session_bus || both_buses))
    {
        session_bus_result = g_ptr_array_new_full(g_hash_table_size(session_bus_result_table),
                                                  (GDestroyNotify) diag_module_data_free);

        GNode *tests = NULL;

        if (!(tests = session_info_parser->alterator_ctl_module_info_parser_get_node_by_name(
                  session_info_parser, session_diag_tool_object, DIAG_ALTERATOR_ENTRY_TESTS_TABLE_NAME)))
        {
            g_printerr(_("Empty tests of diagnostic tool %s in session bus\n"), diag_tool_str_id);
            ERR_EXIT();
        }

        g_print(_("<<< Tests on session bus:\n"));

        for (GNode *test = tests->children; test != NULL; test = test->next)
        {
            alterator_entry_node *diag_data = (alterator_entry_node *) test->data;
            diag_data_t *data = diag_module_data_new(module->gdbus_session_source, diag_data->node_name, test, TEST);
            g_ptr_array_add(session_bus_result, data);
        }

        g_ptr_array_sort_with_data(session_bus_result,
                                   diag_module_sort_result,
                                   (gpointer) &module->alterator_ctl_app->arguments->verbose);

        for (guint i = 0; i < session_bus_result->len; i++)
            diag_module_print_list_with_filters(module, (diag_data_t *) g_ptr_array_index(session_bus_result, i));
    }

end:
    if (system_bus_result)
        g_ptr_array_unref(system_bus_result);

    if (session_bus_result)
        g_ptr_array_unref(session_bus_result);

    if (system_diag_tool_object)
        system_info_parser->alterator_ctl_module_info_parser_result_tree_free(system_info_parser,
                                                                              system_diag_tool_object);

    if (session_diag_tool_object)
        session_info_parser->alterator_ctl_module_info_parser_result_tree_free(session_info_parser,
                                                                               session_diag_tool_object);

    g_free(diag_tool_str_id);

    g_free(diag_tool_path);

    return ret;
}

static int diag_module_handle_list_all_results(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                                          = 0;
    GHashTable *system_bus_result_table              = NULL;
    GHashTable *session_bus_result_table             = NULL;
    GPtrArray *system_bus_result                     = NULL;
    GPtrArray *session_bus_result                    = NULL;
    GNode **system_diag_tools_objects                = NULL;
    GNode **session_diag_tools_objects               = NULL;
    gsize system_diag_tools_amount                   = 0;
    gsize session_diag_tools_amount                  = 0;
    gboolean is_root                                 = alterator_ctl_is_root();
    AlteratorCtlModuleInfoParser *system_info_parser = (AlteratorCtlModuleInfoParser *)
                                                           module->gdbus_system_source->info_parser;
    AlteratorCtlModuleInfoParser *session_info_parser = !is_root ? (AlteratorCtlModuleInfoParser *)
                                                                       module->gdbus_session_source->info_parser
                                                                 : NULL;

    if (!module)
    {
        g_printerr(_("Can't process the result of diag list method. The diag module doesn't exist.\n"));
        ERR_EXIT();
    }

    system_diag_tools_objects = system_info_parser
                                    ->alterator_ctl_module_info_parser_get_objects_data(system_info_parser,
                                                                                        module->gdbus_system_source,
                                                                                        DIAG_INTERFACE_NAME,
                                                                                        &system_diag_tools_amount);

    session_diag_tools_objects
        = !is_root
              ? session_info_parser->alterator_ctl_module_info_parser_get_objects_data(session_info_parser,
                                                                                       module->gdbus_session_source,
                                                                                       DIAG_INTERFACE_NAME,
                                                                                       &session_diag_tools_amount)
              : NULL;

    if (!ctx || !(*ctx))
    {
        g_printerr(_("Can't process the result of diag list method. Module data is empty.\n"));
        ERR_EXIT();
    }

    GHashTable *result_table = (*ctx)->results;

    if (g_hash_table_contains(result_table, DIAG_LIST_TOOLS_SYSTEM_RESULT_KEY_NAME))
        system_bus_result_table = g_hash_table_lookup(result_table, DIAG_LIST_TOOLS_SYSTEM_RESULT_KEY_NAME);

    if (g_hash_table_contains(result_table, DIAG_LIST_TOOLS_SESSION_RESULT_KEY_NAME))
        session_bus_result_table = g_hash_table_lookup(result_table, DIAG_LIST_TOOLS_SESSION_RESULT_KEY_NAME);

    if (system_bus_result_table && (system_bus || both_buses))
    {
        system_bus_result = g_ptr_array_new_full(g_hash_table_size(system_bus_result_table),
                                                 (GDestroyNotify) diag_module_data_free);

        g_print(_("<<< Tools on system bus:\n"));

        for (guint i = 0; i < g_hash_table_size(system_bus_result_table); i++)
        {
            alterator_entry_node *diag_data = (alterator_entry_node *) system_diag_tools_objects[i]->data;
            toml_value *diag_name = g_hash_table_lookup(diag_data->toml_pairs, DIAG_ALTERATOR_ENTRY_TOOLNAME_KEY_NAME);

            diag_data_t *data = diag_module_data_new(module->gdbus_system_source,
                                                     diag_name->str_value,
                                                     system_diag_tools_objects[i],
                                                     TOOL);

            g_ptr_array_add(system_bus_result, data);
        }

        g_ptr_array_sort_with_data(system_bus_result,
                                   diag_module_sort_result,
                                   (gpointer) &module->alterator_ctl_app->arguments->verbose);

        for (guint i = 0; i < system_bus_result->len; i++)
            diag_module_print_list_with_filters(module, (diag_data_t *) g_ptr_array_index(system_bus_result, i));
    }

    if (session_bus_result && (session_bus || both_buses))
        g_print("\n");

    if (!is_root && session_bus_result_table && (session_bus || both_buses))
    {
        session_bus_result = g_ptr_array_new_full(g_hash_table_size(session_bus_result_table),
                                                  (GDestroyNotify) diag_module_data_free);

        g_print(_("<<< Tools on session bus:\n"));

        for (guint i = 0; i < g_hash_table_size(session_bus_result_table); i++)
        {
            alterator_entry_node *diag_data = (alterator_entry_node *) session_diag_tools_objects[i]->data;
            toml_value *diag_name = g_hash_table_lookup(diag_data->toml_pairs, DIAG_ALTERATOR_ENTRY_TOOLNAME_KEY_NAME);

            diag_data_t *data = diag_module_data_new(module->gdbus_session_source,
                                                     diag_name->str_value,
                                                     session_diag_tools_objects[i],
                                                     TOOL);

            g_ptr_array_add(session_bus_result, data);
        }

        g_ptr_array_sort_with_data(session_bus_result,
                                   diag_module_sort_result,
                                   (gpointer) &module->alterator_ctl_app->arguments->verbose);

        for (guint i = 0; i < session_bus_result->len; i++)
            diag_module_print_list_with_filters(module, (diag_data_t *) g_ptr_array_index(session_bus_result, i));
    }

end:
    if (system_bus_result)
        g_ptr_array_unref(system_bus_result);

    if (session_bus_result)
        g_ptr_array_unref(session_bus_result);

    if (system_diag_tools_objects)
        system_info_parser->alterator_ctl_module_info_parser_result_trees_free(system_info_parser,
                                                                               system_diag_tools_objects,
                                                                               system_diag_tools_amount);

    if (session_diag_tools_objects)
        system_info_parser->alterator_ctl_module_info_parser_result_trees_free(session_info_parser,
                                                                               session_diag_tools_objects,
                                                                               session_diag_tools_amount);

    return ret;
}

static int diag_module_handle_report_results(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                 = 0;
    GPtrArray *result_array = NULL;
    gchar *file_suffix      = NULL;
    gchar *path             = NULL;
    gchar *file_name        = NULL;
    gchar *param3           = NULL;
    gchar *full_file_name   = NULL;
    GVariant *dbus_answer   = NULL;
    GVariant *suffix        = NULL;
    GVariant *answer_array  = NULL;
    GBytes *report          = NULL;

    if (!module)
    {
        g_printerr(_("Can't process the result of diag report method. The diag module doesn't exist.\n"));
        ERR_EXIT();
    }

    if (!ctx || !(*ctx))
    {
        g_printerr(_("Can't process the result of diag report method. Module data is empty.\n"));
        ERR_EXIT();
    }

    //Get path and filename
    g_variant_get((*ctx)->parameters, "(msmsms)", &path, &file_name, &param3);

    if (!file_name)
    {
        g_printerr(_("Can't process the result of diag report method. Report filename is empty.\n"));
        ERR_EXIT();
    }

    //Here
    result_array = (GPtrArray *) ((*ctx)->results);
    if (!result_array)
    {
        g_printerr(_("Empty result of the diag report method.\n"));
        ERR_EXIT();
    }

    for (gsize i = 0; i < result_array->len; i++)
    {
        dbus_answer = (GVariant *) g_ptr_array_index(result_array, i);

        suffix       = g_variant_get_child_value(dbus_answer, 0);
        answer_array = g_variant_get_child_value(dbus_answer, 1);

        int exit_code = -1;

        g_variant_get(answer_array, "(ayi)", NULL, &exit_code);

        if (exit_code != 0)
        {
            g_printerr(_("Can't save report from %s. Exit code: %i\n"), path, exit_code);
            ERR_EXIT();
        }

        g_variant_get(suffix, "s", &file_suffix);
        if (!file_suffix)
        {
            g_printerr(_("Can't save report from %s. File extension is empty.\n"), path);
            ERR_EXIT();
        }

        report = g_variant_get_data_as_bytes(answer_array);
        if (!report)
        {
            g_printerr(_("Can't save empty report from %s\n"), path);
            ERR_EXIT();
        }

        if (both_buses)
        {
            if (i == 0)
                full_file_name = g_strconcat(file_name, ".", file_suffix, NULL);
            else
                full_file_name = g_strconcat(file_name, "_session", ".", file_suffix, NULL);
        }

        if (result_array->len == 1 && system_bus)
            full_file_name = g_strconcat(file_name, ".", file_suffix, NULL);
        else if (result_array->len == 1 && session_bus && !alterator_ctl_is_root())
            full_file_name = g_strconcat(file_name, "_session", ".", file_suffix, NULL);

        FILE *report_file = fopen(full_file_name, "wb");
        if (!report_file)
        {
            g_printerr(_("Can't open file to write a report: %s\n"), full_file_name);
            ERR_EXIT();
        }

        size_t report_size     = 0;
        gconstpointer data_ptr = g_bytes_get_data(report, &report_size);
        size_t w_bytes         = fwrite(data_ptr, 1, report_size, report_file);
        if (w_bytes != report_size)
        {
            g_printerr(_("Error writing report to file: %s\n"), full_file_name);
            ERR_EXIT();
        }

        fclose(report_file);

        g_print(_("Report %s created\n"), full_file_name);

        g_free(full_file_name);
        full_file_name = NULL;

        g_bytes_unref(report);
        report = NULL;

        g_free(file_suffix);
        file_suffix = NULL;

        g_variant_unref(answer_array);
        answer_array = NULL;

        g_variant_unref(suffix);
        suffix = NULL;

        g_variant_unref(dbus_answer);
        dbus_answer = NULL;
    }

end:
    if (report)
        g_bytes_unref(report);

    if (answer_array)
        g_variant_unref(answer_array);

    if (suffix)
        g_variant_unref(suffix);

    if (dbus_answer)
        g_variant_unref(dbus_answer);

    g_free(full_file_name);

    g_free(file_suffix);

    g_free(param3);

    g_free(file_name);

    g_free(path);

    return ret;
}

static int diag_module_handle_tool_test_results(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx)
{
    int ret = 0;

end:
    return ret;
}

static int diag_module_handle_tool_results(AlteratorCtlDiagModule *module, alteratorctl_ctx_t **ctx)
{
    int ret = 0;

end:
    return ret;
}

static int diag_module_parse_options(AlteratorCtlDiagModule *module,
                                     int *argc,
                                     char **argv,
                                     enum diag_sub_commands command)
{
    int ret                        = 0;
    GOptionContext *option_context = NULL;

    // clang-format off
    static GOptionEntry diag_module_options[]
        = {{"name-only", 'n', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &name_only,
                                                    "Printing only diag tools and tests names",
                                                    "NAME_ONLY"},
           {"no-name", 'N', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &no_name,
                                                    "Printing diag tools and tests tests without names", "NO_NAME"},
           {"path-only", 'p', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &path_only,
                                                    "Printing only diag tools D-Bus paths",
                                                    "PATH_ONLY"},
           {"display-name-only", 'd', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &display_name_only,
                                                    "Printing only diag tools and tests display names",
                                                    "DISPLAY_NAME_ONLY"},
           {"no-display-name", 'D', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &no_display_name,
                                                    "Printing diag tools and tests without display names",
                                                    "NO_DISPLAY_NAME"},
           {"system", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &system_bus,
                                                    "Choise system bus", "SYSTEM_BUS"},
           {"session", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &session_bus,
                                                    "Choise session bus", "SESSION_BUS"},
           {"all", 'a', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &both_buses, "Choise all buses", "BOTH_BUSES"},
           {NULL}};
    // clang-format on

    GError *error  = NULL;
    option_context = g_option_context_new("Diag module options");
    g_option_context_add_main_entries(option_context, diag_module_options, NULL);
    if (!g_option_context_parse(option_context, argc, &argv, &error))
    {
        g_printerr(_("Module diag options parsing failed: %s\n"), error->message);
        g_error_free(error);
        ERR_EXIT();
    }

    if (!system_bus && !session_bus && !both_buses || system_bus && session_bus)
        both_buses = TRUE; // --all - default option for run subcommand

    // Prohibit crossing of opposite options
    if (name_only & path_only)
    {
        g_printerr(_("It is not possible to use options --name-only and --path-only together.\n"));
        ERR_EXIT();
    }
    else if (name_only & display_name_only)
    {
        g_printerr(_("It is not possible to use options --name-only and --display-name-only together.\n"));
        ERR_EXIT();
    }
    else if (display_name_only & path_only)
    {
        g_printerr(_("It is not possible to use options --display-name-only and --path-only together.\n"));
        ERR_EXIT();
    }
    else if (name_only & path_only & display_name_only)
    {
        g_printerr(
            _("It is not possible to use options --name-only and --path-only and --display-name-only together.\n"));
        ERR_EXIT();
    }
    else if (display_name_only & no_display_name)
    {
        g_printerr(_("It is not possible to use options --display-name-only and --no-display-name together.\n"));
        ERR_EXIT();
    }
    else if (module->alterator_ctl_app->arguments->verbose && display_name_only)
    {
        g_printerr(_("It is not possible to use options --verbose and --display-name-only together.\n"));
        ERR_EXIT();
    }
    else if (command != DIAG_RUN && module->alterator_ctl_app->arguments->verbose && name_only)
    {
        g_printerr(_("It is not possible to use options --verbose and --name-only together.\n"));
        ERR_EXIT();
    }
    else if (module->alterator_ctl_app->arguments->verbose && path_only)
    {
        g_printerr(_("It is not possible to use options --verbose and --path-only together.\n"));
        ERR_EXIT();
    }
    else if (no_name && name_only)
    {
        g_printerr(_("It is not possible to use options --no-name and --name-only together.\n"));
        ERR_EXIT();
    }
    else if (no_name && no_display_name)
    {
        g_printerr(_("It is not possible to use options --no-name and --no-display-name together.\n"));
        ERR_EXIT();
    }
    else if (no_name && module->alterator_ctl_app->arguments->verbose)
    {
        g_printerr(_("It is not possible to use options --verbose and --no-name together.\n"));
        ERR_EXIT();
    }

end:
    if (option_context)
        g_option_context_free(option_context);

    return ret;
}

static int diag_module_print_list_with_filters(AlteratorCtlDiagModule *module, diag_data_t *diag_data)
{
    int ret = 0;
    if (diag_data->type == TOOL && module->alterator_ctl_app->arguments->verbose)
    {
        if (!no_display_name)
            g_print("%s (%s : %s)\n", diag_data->display_name, diag_data->name, diag_data->path);
        else
            g_print("%s : %s\n", diag_data->display_name, diag_data->name, diag_data->path);
    }
    else if (path_only && diag_data->type == TOOL)
        g_print("%s\n", diag_data->path);
    else if (display_name_only || no_name)
        g_print("%s\n", diag_data->display_name);
    else if (no_display_name || name_only)
        g_print("%s\n", diag_data->name);
    else
        g_print("%s (%s)\n", diag_data->display_name, diag_data->name);

end:
    return ret;
}

static int diag_module_validate_alterator_entry(AlteratorGDBusSource *source, GNode *alterator_entry_data)
{
    int ret = 0;

    if (!source->info_parser->alterator_ctl_module_info_parser_find_value(source->info_parser,
                                                                          alterator_entry_data,
                                                                          NULL,
                                                                          DIAG_ALTERATOR_ENTRY_TOOLNAME_KEY_NAME,
                                                                          NULL))
    {
        g_printerr(_("Can't get alterator entry data for validation in module diag: field \"%s\" is missing.\n"),
                   DIAG_ALTERATOR_ENTRY_TOOLNAME_KEY_NAME);
        ERR_EXIT();
    }

    if (!source->info_parser->alterator_ctl_module_info_parser_find_table(source->info_parser,
                                                                          alterator_entry_data,
                                                                          NULL,
                                                                          DIAG_ALTERATOR_ENTRY_TESTS_TABLE_NAME,
                                                                          NULL))
    {
        g_printerr(_("Can't get alterator entry data for validation in module diag: table \"%s\" is missing.\n"),
                   DIAG_ALTERATOR_ENTRY_TESTS_TABLE_NAME);
        ERR_EXIT();
    }

end:

    return ret;
}

static int diag_module_validate_object_and_iface(AlteratorGDBusSource *source,
                                                 const gchar *object_str_id,
                                                 const gchar *iface)
{
    int ret          = 0;
    int object_exist = 0;
    int iface_exists = 0;

    //Check object
    if (source->alterator_gdbus_source_check_object_by_path(source, object_str_id, &object_exist) < 0)
        g_printerr(_("The object %s doesn't exits.\n"), object_str_id);

    if (object_exist == 0)
        ERR_EXIT();

    //check interface of the object
    if (source->alterator_gdbus_source_check_object_by_iface(source, object_str_id, iface, &iface_exists) < 0)
        ERR_EXIT();

    if (iface_exists == 0)
        ERR_EXIT();

    ret = 1;

end:
    return ret;
}

static int diag_module_get_list_of_tests(AlteratorCtlDiagModule *module,
                                         AlteratorGDBusSource *source,
                                         const gchar *path,
                                         GHashTable **result)
{
    int ret                = 0;
    dbus_ctx_t *d_ctx      = NULL;
    GVariant *answer_array = NULL;
    GVariant *exit_code    = NULL;

    if (!source || !path || !result)
        ERR_EXIT();

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                          path,
                          DIAG_INTERFACE_NAME,
                          DIAG_LIST_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
        ERR_EXIT();

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    source->call(source, d_ctx, NULL);

    if (!d_ctx)
        ERR_EXIT();

    (*result) = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    if (!(*result))
        ERR_EXIT();

    answer_array = g_variant_get_child_value(d_ctx->result, 0);
    exit_code    = g_variant_get_child_value(d_ctx->result, 1);

    if (g_variant_get_int32(exit_code) != 0)
        ERR_EXIT();

    GVariantIter *iter = NULL;
    gchar *str         = NULL;

    g_variant_get(answer_array, "as", &iter);
    while (g_variant_iter_loop(iter, "s", &str))
    {
        g_hash_table_insert((*result), g_strdup(str), NULL);
    }

    g_variant_iter_free(iter);

end:
    if (answer_array)
        g_variant_unref(answer_array);

    if (exit_code)
        g_variant_unref(exit_code);

    dbus_ctx_free(d_ctx);

    return ret;
}

static void diag_module_run_stdout_signal_handler(GDBusConnection *connection,
                                                  const gchar *sender_name,
                                                  const gchar *object_path,
                                                  const gchar *interface_name,
                                                  const gchar *signal_name,
                                                  GVariant *parameters,
                                                  gpointer user_data)
{
    for (int i = 0; i < g_variant_n_children(parameters); i++)
    {
        GVariant *tmp = g_variant_get_child_value(parameters, i);

        const gchar *str;

        str = g_variant_get_string(tmp, NULL);

        g_print("%s\n", str);

        g_variant_unref(tmp);
    }

    return;
}

static void diag_module_run_stderr_signal_handler(GDBusConnection *connection,
                                                  const gchar *sender_name,
                                                  const gchar *object_path,
                                                  const gchar *interface_name,
                                                  const gchar *signal_name,
                                                  GVariant *parameters,
                                                  gpointer user_data)
{
    for (int i = 0; i < g_variant_n_children(parameters); i++)
    {
        GVariant *tmp = g_variant_get_child_value(parameters, i);

        const gchar *str;

        str = g_variant_get_string(tmp, NULL);

        g_print("%s\n", str);

        g_variant_unref(tmp);
    }

    return;
}

static int diag_module_get_and_run_test(AlteratorCtlDiagModule *module,
                                        AlteratorGDBusSource *source,
                                        const gchar *diag_tool_str_id,
                                        const gchar *test,
                                        int *result)
{
    int ret           = 0;
    GHashTable *tests = NULL;
    int exit_code     = 0;

    // run all tests
    if (!test)
    {
        if (diag_module_get_list_of_tests(module, source, diag_tool_str_id, &tests) < 0)
        {
            g_printerr(_("Can't get list of tests from object %s.\n"), diag_tool_str_id);
            ERR_EXIT();
        }

        if (g_hash_table_size(tests) == 0)
        {
            g_printerr(_("No tests were found in the tool %s\n"), diag_tool_str_id);
            ERR_EXIT();
        }

        GHashTableIter iter;
        gpointer key = NULL, value = NULL;
        g_hash_table_iter_init(&iter, tests);
        while (g_hash_table_iter_next(&iter, &key, &value))
        {
            if (diag_module_run_test(module, source, diag_tool_str_id, (gchar *) key, &exit_code))
                ERR_EXIT();
            if (exit_code != 0)
                *result = exit_code;
        }
        goto end;
    }

    if (diag_module_get_list_of_tests(module, source, diag_tool_str_id, &tests) < 0)
    {
        g_printerr(_("Can't get list of tests from object %s.\n"), diag_tool_str_id);
        ERR_EXIT();
    }

    if (!g_hash_table_contains(tests, test))
    {
        g_printerr(_("No test named %s was found in the tool %s\n"), test, diag_tool_str_id);
        ERR_EXIT();
    }

    if (diag_module_run_test(module, source, diag_tool_str_id, test, &exit_code) < 0)
        ERR_EXIT();

    *result = exit_code;

end:
    if (tests)
        g_hash_table_unref(tests);

    return ret;
}

static int diag_module_run_test(
    AlteratorCtlDiagModule *module, AlteratorGDBusSource *source, const gchar *path, const gchar *test, int *result)
{
    int ret                            = 0;
    GPtrArray *signals                 = NULL;
    subscribe_signals_t *stdout_signal = NULL;
    subscribe_signals_t *stderr_signal = NULL;
    GError *dbus_call_error            = NULL;
    dbus_ctx_t *d_ctx                  = NULL;
    gchar *test_display_name           = NULL;
    gchar *colored_status              = NULL;

    if (!module || !source || !path || !test || !result)
        ERR_EXIT();

    signals = g_ptr_array_new();

    stdout_signal = g_malloc0(sizeof(subscribe_signals_t));
    if (!stdout_signal)
    {
        g_printerr(_("Running test in diag tool failed. Can't allocate memory for stdout signal.\n"));
        ERR_EXIT();
    }

    stderr_signal = g_malloc0(sizeof(subscribe_signals_t));
    if (!stderr_signal)
    {
        g_printerr(_("Running test in diag tool failed. Can't allocate memory for stderr signal.\n"));
        ERR_EXIT();
    }

    stdout_signal->signal_name = DIAG_STDOUT_SIGNAL_NAME;
    stdout_signal->callback    = &diag_module_run_stdout_signal_handler;
    stdout_signal->user_data   = d_ctx;

    stderr_signal->signal_name = DIAG_STDERR_SIGNAL_NAME;
    stderr_signal->callback    = &diag_module_run_stderr_signal_handler;
    stderr_signal->user_data   = d_ctx;

    g_ptr_array_add(signals, stdout_signal);
    g_ptr_array_add(signals, stderr_signal);

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                          path,
                          DIAG_INTERFACE_NAME,
                          DIAG_RUN_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);
    if (!d_ctx)
    {
        g_printerr(_("Running test in diag tool failed. Can't allocate memory for D-Bus context.\n"));
        ERR_EXIT();
    }

    d_ctx->parameters = g_variant_new("(s)", test);

    d_ctx->reply_type = G_VARIANT_TYPE("(i)");

    d_ctx->disable_output = !d_ctx->verbose;

    struct winsize w;
    if (isatty_safe(STDOUT_FILENO) && ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) < 0)
    {
        g_printerr(_("Running test in diag tool failed. Failed to get terminal width: %s\n"), strerror(errno));
        ERR_EXIT();
    }

    const gchar *output_name = NULL;
    test_result test_result  = *result;

    diag_module_get_test_display_name(source, path, test, &test_display_name);

    gboolean should_use_real_name = (name_only || no_display_name);
    if (should_use_real_name)
        output_name = test;
    else
        output_name = test_display_name ? test_display_name : test;

    if (module->alterator_ctl_app->arguments->verbose)
    {
        // Test header with bold title
        gchar *title_border = g_strnfill(isatty_safe(STDOUT_FILENO) == TTY ? w.ws_col - 20 : 80, '#');
        g_print("%s\n", title_border);
        gchar *start_message_format = _("The \"%s\" test is started");
        gchar *start_message        = g_strdup_printf(start_message_format, output_name);
        g_print("\033[1m%s\033[0m\n", start_message);
        g_free(start_message);
        g_print("%s\n", title_border);
        g_free(title_border);
    }

    source->call_with_signals(source, d_ctx, signals, &dbus_call_error);
    if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    g_variant_get(d_ctx->result, "(i)", result);

    const gchar *status_message = test_result == PASS ? _("[PASS]") : (test_result == FAIL ? _("[FAIL]") : _("[WARN]"));
    text_color status_color     = test_result == PASS ? GREEN : (test_result == FAIL ? RED : YELLOW);
    colored_status              = colorize_text(status_message, status_color);

    if (test_result == FAIL)
        g_printerr("%s: %s%s\n", colored_status, output_name, module->alterator_ctl_app->arguments->verbose ? "\n" : "");
    else
        g_print("%s: %s\%s\n", colored_status, output_name, module->alterator_ctl_app->arguments->verbose ? "\n" : "");

end:
    dbus_ctx_free(d_ctx);

    g_free(test_display_name);

    g_free(colored_status);

    if (stdout_signal)
        g_free(stdout_signal);

    if (stderr_signal)
        g_free(stderr_signal);

    if (signals)
        g_ptr_array_unref(signals);

    g_clear_error(&dbus_call_error);

    return ret;
}

static int diag_module_get_file_suffix(AlteratorGDBusSource *source, const gchar *path, gchar **file_suffix)
{
    int ret                            = 0;
    GNode *parsed_tool_alterator_entry = NULL;
    toml_value *suffix                 = NULL;

    if (source->info_parser->alterator_ctl_module_info_parser_get_specified_object_data(source->info_parser,
                                                                                        source,
                                                                                        path,
                                                                                        DIAG_INTERFACE_NAME,
                                                                                        &parsed_tool_alterator_entry)
        < 0)
    {
        g_printerr(_("Error in diag report module: can't get alterator entry data of %s object.\n"), path);
        ERR_EXIT();
    }

    if (diag_module_validate_alterator_entry(source, parsed_tool_alterator_entry) < 0)
        ERR_EXIT();

    if (!source->info_parser->alterator_ctl_module_info_parser_find_value(source->info_parser,
                                                                          parsed_tool_alterator_entry,
                                                                          &suffix,
                                                                          DIAG_ALTERATOR_ENTRY_REPORT_KEY_NAME,
                                                                          NULL))
    {
        g_printerr(_("Error in diag report: ReportSuffix from %s not found\n"), path);
        ERR_EXIT();
    }

    if (suffix->type != TOML_DATA_STRING)
    {
        g_printerr(_("Error in diag report: ReportSuffix from %s data type isn't a string\n"), path);
        ERR_EXIT();
    }

    if (!suffix->str_value)
    {
        g_printerr(_("Error in diag report: ReportSuffix from %s hasn't been initialized\n"), path);
        ERR_EXIT();
    }

    (*file_suffix) = g_strdup(suffix->str_value);

end:
    if (parsed_tool_alterator_entry)
        source->info_parser->alterator_ctl_module_info_parser_result_tree_free(source->info_parser,
                                                                               parsed_tool_alterator_entry);

    return ret;
}

static int diag_module_get_report(AlteratorGDBusSource *source, const gchar *path, GVariant **result)
{
    int ret                 = 0;
    dbus_ctx_t *d_ctx       = NULL;
    GError *dbus_call_error = NULL;
    gchar *file_suffix      = NULL;

    if (!source || !path || !result)
        ERR_EXIT();

    //Get suffix
    if (diag_module_get_file_suffix(source, path, &file_suffix) < 0)
        ERR_EXIT();

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, path, DIAG_INTERFACE_NAME, DIAG_REPORT_METHOD_NAME, source->verbose);

    d_ctx->reply_type = G_VARIANT_TYPE("(ayi)");

    if (!d_ctx)
    {
        g_printerr(_("Internal error of diag report: can't allocate D-Bus context.\n"));
        ERR_EXIT();
    }

    source->call(source, d_ctx, &dbus_call_error);

    if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    (*result) = g_variant_new("(s@(ayi))", file_suffix, d_ctx->result);

end:
    g_free(file_suffix);

    dbus_ctx_free(d_ctx);

    g_clear_error(&dbus_call_error);

    return ret;
}
