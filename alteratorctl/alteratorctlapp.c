#include "alteratorctlapp.h"
#include "alteratorctlcommon.h"
#include "version.h"

#include <glib-object.h>
#include <string.h>

typedef struct
{
    gboolean flag;
    gint index;
    gint module_arg_offset; //Index relative to the module excluding options
                            //Example: alteratorctl manager -v --help. -v - 1, --help - 2
    int argc;
    char **argv;
} flag_callback_ctx_t;

static flag_callback_ctx_t help = {FALSE, -1, -1, 0, NULL};

alteratorctl_arguments_t *alteratorctl_arguments_init();
void alteratorctl_add_module(alteratorctl_arguments_t *args, gchar *module);
void alteratorctl_arguments_free(alteratorctl_arguments_t *args);

static GObjectClass *alterator_ctl_app_parent_class = NULL;
static void alterator_ctl_app_class_init(AlteratorCtlAppClass *klass);
static void alterator_ctl_app_class_finalize(GObject *klass);

static alterator_ctl_module_t *alterator_ctl_app_find_module(AlteratorCtlApp *app, gchar *module);
static int alterator_ctl_app_list_modules(AlteratorCtlApp *app);
static int alterator_ctl_app_parse_args(AlteratorCtlApp *alterator_ctl, int *argc, char **argv);
static gboolean alterator_ctl_app_get_flag_first_index(const gchar *option_name,
                                                       const gchar *value,
                                                       gpointer data,
                                                       GError **error);

static void alterator_ctl_print_help();

alteratorctl_arguments_t *alteratorctl_arguments_init()
{
    alteratorctl_arguments_t *args = g_malloc0(sizeof(alteratorctl_arguments_t));
    if (!args)
        return NULL;

    return args;
}

void alteratorctl_add_module(alteratorctl_arguments_t *args, gchar *module)
{
    if (!args || !module)
        return;

    args->module = strdup(module);
}

void alteratorctl_arguments_free(alteratorctl_arguments_t *args)
{
    if (args->module)
        g_free(args->module);

    g_free(args);
}

GType alterator_ctl_app_get_type(void)
{
    static GType alterator_ctl_app_type = 0;

    if (!alterator_ctl_app_type)
    {
        static const GTypeInfo alterator_ctl_app_info = {sizeof(AlteratorCtlAppClass), /* class structure size */
                                                         NULL,                         /* base class initializer */
                                                         NULL,                         /* base class finalizer */
                                                         (GClassInitFunc)
                                                             alterator_ctl_app_class_init, /* class initializer */
                                                         NULL,                             /* class finalizer */
                                                         NULL,                             /* class data */
                                                         sizeof(AlteratorCtlApp),          /* instance structure size */
                                                         1,                                /* preallocated instances */
                                                         NULL,                             /* instance initializers */
                                                         NULL};

        alterator_ctl_app_type = g_type_register_static(G_TYPE_OBJECT, /* parent class */
                                                        "AlteratorCtlApp",
                                                        &alterator_ctl_app_info,
                                                        0);
    }

    return alterator_ctl_app_type;
}

static void alterator_ctl_app_class_finalize(GObject *klass)
{
    AlteratorCtlAppClass *obj = (AlteratorCtlAppClass *) klass;

    G_OBJECT_CLASS(alterator_ctl_app_parent_class)->finalize(klass);
}

static void alterator_ctl_app_class_init(AlteratorCtlAppClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->finalize = alterator_ctl_app_class_finalize;

    alterator_ctl_app_parent_class = g_type_class_peek_parent(klass);
}

AlteratorCtlApp *alterator_ctl_app_new(void)
{
    AlteratorCtlApp *object = g_object_new(TYPE_ALTERATOR_CTL_APP, NULL);

    object->modules = g_hash_table_new(g_str_hash, g_str_equal);

    object->arguments = alteratorctl_arguments_init();

    object->local_polkit_agent = NULL;
    object->local_agent_handle = NULL;

    if (isatty_safe(STDIN_FILENO) != TTY && isatty_safe(STDOUT_FILENO) != TTY && isatty_safe(STDERR_FILENO) != TTY)
        return object;

    PolkitSubject *subject = NULL;

    subject = polkit_unix_process_new_for_owner(getpid(), 0, getuid());

    GError *error = NULL;
    /* this will fail if we can't find a controlling terminal */
    object->local_polkit_agent = polkit_agent_text_listener_new(NULL, &error);
    if (subject && object->local_polkit_agent)
    {
        GVariantBuilder *options_builder;
        GVariant *options;

        options_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
        g_variant_builder_add(options_builder, "{sv}", "fallback", g_variant_new_boolean(TRUE));
        options = g_variant_builder_end(options_builder);

        object->local_agent_handle
            = polkit_agent_listener_register_with_options(object->local_polkit_agent,
                                                          POLKIT_AGENT_REGISTER_FLAGS_RUN_IN_THREAD,
                                                          subject,
                                                          NULL, /* object_path */
                                                          options,
                                                          NULL, /* GCancellable */
                                                          &error);
        g_variant_builder_unref(options_builder);

        if (!object->local_agent_handle)
        {
            g_printerr(_("Error registering local authentication agent: %s (%s, %d)\n"),
                       error->message,
                       g_quark_to_string(error->domain),
                       error->code);
            g_clear_error(&error);
        }
    }
    else
    {
        g_printerr(_("Error creating textual authentication agent: %s (%s, %d)\n"),
                   error->message,
                   g_quark_to_string(error->domain),
                   error->code);
        g_clear_error(&error);
    }

    if (subject)
    {
        g_object_unref(subject);
    }

    return object;
}

void alterator_ctl_app_free(AlteratorCtlApp *alterator_ctl)
{
    alteratorctl_arguments_free(alterator_ctl->arguments);

    g_hash_table_destroy(alterator_ctl->modules);

    if (alterator_ctl->local_agent_handle)
        polkit_agent_listener_unregister(alterator_ctl->local_agent_handle);
    if (alterator_ctl->local_polkit_agent)
        g_object_unref(alterator_ctl->local_polkit_agent);

    g_object_unref(alterator_ctl);
}

void alterator_ctl_register_module(AlteratorCtlApp *app, alterator_ctl_module_t *module)
{
    if (g_hash_table_contains(app->modules, module->id))
        return;

    g_hash_table_insert(app->modules, module->id, module);
}

static int alterator_ctl_app_parse_args(AlteratorCtlApp *alterator_ctl, int *argc, char **argv)
{
    int ret = 0;

    GError *error           = NULL;
    GOptionContext *context = NULL;

    gboolean verb    = FALSE;
    gboolean version = FALSE;
    gboolean modules = FALSE;

    if (*argc < 2)
    {
        g_printerr(_("Not enough arguments.\n"));

        alterator_ctl_print_help();

        ERR_EXIT();
    }

    help.argc = *argc;
    help.argv = g_strdupv(argv);

    // clang-format off
    GOptionEntry entries[]
        = {{"verbose", 'v', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &verb, "Be verbose", NULL},
           {"version", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &version, "Version", NULL},
           {"modules", 'm', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &modules, "Lists registered modules", NULL},
           {"help", 'h', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, &alterator_ctl_app_get_flag_first_index, "Show help", NULL},
           {NULL}};
    // clang-format on

    context = g_option_context_new("[subcommand]");

    g_option_context_add_main_entries(context, entries, NULL);
    g_option_context_set_ignore_unknown_options(context, TRUE);
    g_option_context_set_help_enabled(context, FALSE);

    if (!g_option_context_parse(context, argc, &argv, &error))
    {
        g_printerr(_("Can't parse aruments\n"));

        g_error_free(error);

        alterator_ctl_print_help();

        ERR_EXIT();
    }

    if (help.flag && help.index == 1)
    {
        alterator_ctl_print_help();
        goto end;
    }

    help.module_arg_offset = help.index - (help.argc - *argc) + 1 * ((help.argc - *argc) / 2);
    if (help.flag && help.module_arg_offset == 0)
        alterator_ctl->arguments->help_type = ALTERATORCTL_NONE_HELP;
    else if (help.flag && help.module_arg_offset > 0)
        alterator_ctl->arguments->help_type = ALTERATORCTL_MODULE_HELP;

    if (version)
    {
        alterator_ctl->arguments->command = ALTERATORCTL_GET_VERSION;
        goto end;
    }

    if (modules)
    {
        alterator_ctl->arguments->command = ALTERATORCTL_LIST_MODULES;
        goto end;
    }

    if (*argc >= 2)
    {
        alteratorctl_add_module(alterator_ctl->arguments, argv[1]);
        alterator_ctl->arguments->command = ALTERATORCTL_RUN_MODULE;

        if (help.flag && help.module_arg_offset > 1)
            alterator_ctl->arguments->help_type = ALTERATORCTL_SUBMODULE_HELP;

        alterator_ctl->arguments->verbose = verb;
    }

end:
    if (help.argv)
        g_strfreev(help.argv);

    if (context)
        g_option_context_free(context);

    return ret;
}

static gboolean alterator_ctl_app_get_flag_first_index(const gchar *option_name,
                                                       const gchar *value,
                                                       gpointer data,
                                                       GError **error)
{
    if (!help.flag)
        for (guint i = 0; i < help.argc; i++)
            if (g_strcmp0(option_name, help.argv[i]) == 0)
            {
                help.index = i;
                help.flag  = TRUE;
                break;
            }

    return TRUE;
}

int alterator_ctl_app_run(AlteratorCtlApp *app, int argc, char **argv)
{
    int ret                             = 0;
    alterator_ctl_module_t *module_info = NULL;
    void *module                        = NULL;

    if (alterator_ctl_app_parse_args(app, &argc, argv) < 0)
        ERR_EXIT();

    switch (app->arguments->command)
    {
    case ALTERATORCTL_LIST_MODULES:
        if (alterator_ctl_app_list_modules(app) < 0)
            ERR_EXIT();
        break;

    case ALTERATORCTL_GET_VERSION:
        g_print("alteratorctl %s\n", ALTERATORCTL_VERSION);
        break;

    case ALTERATORCTL_RUN_MODULE:
        //TODO say, what module isn't found. List all registered modules??
        app->arguments->module_help = help.flag;
        if (!(module_info = alterator_ctl_app_find_module(app, app->arguments->module)))
        {
            g_printerr(_("Module %s isn't found.\n"), app->arguments->module);
            alterator_ctl_print_help();
            ERR_EXIT();
        }

        module = module_info->new_object_func(app);

        AlteratorCtlModuleInterface *alterator_ctl_module = GET_ALTERATOR_CTL_MODULE_INTERFACE(module);

        ret = alterator_ctl_module->run_with_args(module, argc, argv);

        module_info->free_object_func(module);

        break;
    }

end:
    return ret;
}

static alterator_ctl_module_t *alterator_ctl_app_find_module(AlteratorCtlApp *app, gchar *module)
{
    if (g_hash_table_contains(app->modules, module))
    {
        alterator_ctl_module_t *mod = (alterator_ctl_module_t *) g_hash_table_lookup(app->modules, module);

        return mod;
    }

    return NULL;
}
static void alterator_ctl_print_help()
{
    g_print(_("Usage:\n"));
    g_print(_("  alteratorctl [OPTIONS] \n"));
    g_print(_("  alteratorctl <MODULE> [COMMAND] [OPTIONS]\n"));
    g_print(_("  alteratorctl <MODULE> <SUBMODULE> [COMMAND] [OPTIONS]\n\n"));
    g_print(_("Options:\n"));
    g_print(_("  -h, --help                  Show usage help\n"));
    g_print(_("  -v, --verbose               Be verbose\n"));
    g_print(_("  --version                   Get alteratorctl version\n"));
    g_print(_("  -m, --modules               Lists registered client modules\n\n"));
    g_print(_("For more information on a specific module and its subcommands, use:\n"
              "  alteratorctl <MODULE> (-h | --help)\n\n"));

    return;
}

static int alterator_ctl_app_list_modules(AlteratorCtlApp *app)
{
    int ret = 0;

    if (!app)
    {
        g_printerr(_("Internal error, app is NULL.\n"));
        ERR_EXIT();
    }

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, app->modules);
    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        g_print("%s\n", (gchar *) key);
    }

end:
    return ret;
}

int alterator_ctl_get_registered_module(AlteratorCtlApp *app, gchar *module_name, AlteratorCtlModule **module)
{
    int ret = 0;

    alterator_ctl_module_t *module_info = alterator_ctl_app_find_module(app, module_name);

    if (!module_info)
    {
        (*module) = NULL;
        goto end;
    }

    (*module) = g_malloc0(sizeof(AlteratorCtlModule));

    if (!(*module))
    {
        g_printerr("AlteratorCtlGetRegisteredModule : g_malloc0(sizeof(AlteratorCtlModule))\n");
        ERR_EXIT();
    }

    (*module)->module_instance = module_info->new_object_func(app);

    (*module)->module_iface = GET_ALTERATOR_CTL_MODULE_INTERFACE((*module)->module_instance);

    if (!(*module)->module_instance)
    {
        g_printerr("AlteratorCtlGetRegisteredModule : !founded_module->module\n");
        ERR_EXIT();
    }

    (*module)->free_module_func = module_info->free_object_func;

end:
    return ret;
}
