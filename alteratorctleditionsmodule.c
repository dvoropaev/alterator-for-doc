#include "alteratorctleditionsmodule.h"
#include "alteratorctlapp.h"

#include <gio/gioenums.h>

#define CURRENT_EDITION_INTERFACE_NAME "org.altlinux.alterator.current_edition1"
#define CURRENT_EDITION_DESCRIPTION_METHOD_NAME "Description"
#define CURRENT_EDITION_LICENSE_METHOD_NAME "License"
#define CURRENT_EDITION_LIST_METHOD_NAME "List"
#define CURRENT_EDITION_INFO_METHOD_NAME "Info"
#define CURRENT_EDITION_SET_CURRENT_METHOD_NAME "Set"
#define CURRENT_EDITION_GET_CURRENT_METHOD_NAME "Get"

#define EDITIONS_INTERFACE_NAME "org.altlinux.alterator.edition1"

#define EDITIONS_ALTERATOR_ENTRY_KEY_NAME "name"
#define EDITIONS_ALTERATOR_ENTRY_DISPLAY_NAME_TABLE_NAME "display_name"

typedef struct editions_module_subcommands_t
{
    char *subcommand;
    enum editions_sub_commands id;
} editions_module_subcommands_t;

typedef struct edition_data_t
{
    gchar *name;
    gchar *display_name;
    gchar *path;
} edition_data_t;

static editions_module_subcommands_t editions_module_subcommands_list[] = {{"list", EDITIONS_LIST},
                                                                           {"description", EDITIONS_DESCRIPTION},
                                                                           {"license", EDITIONS_LICENSE},
                                                                           {"info", EDITIONS_INFO},
                                                                           {"set", EDITIONS_SET},
                                                                           {"get", EDITIONS_GET}};

static GObjectClass *editions_module_parent_class = NULL;
static alterator_ctl_module_t editions_module     = {0};
static gboolean is_dbus_call_error                = FALSE;

static gboolean name_only              = FALSE;
static gboolean no_name                = FALSE;
static gboolean path_only              = FALSE;
static gboolean display_name_only      = FALSE;
static gboolean no_display_name        = FALSE;
static gboolean hide_installed_markers = FALSE;

static void editions_module_class_init(AlteratorCtlEditionsModuleClass *klass);
static void editions_ctl_class_finalize(GObject *klass);

static void editions_module_alterator_interface_init(gpointer iface, gpointer iface_data);
static void editions_module_alterator_interface_finalize(gpointer iface, gpointer iface_data);

AlteratorCtlEditionsModule *editions_module_new(gpointer app);
void editions_module_free(AlteratorCtlEditionsModule *module);

static void fill_command_hash_table(GHashTable *command);

static int editions_module_parse_options(AlteratorCtlEditionsModule *module, int *argc, char **argv);
static int editions_module_parse_arguments(AlteratorCtlEditionsModule *module,
                                           int argc,
                                           char **argv,
                                           alteratorctl_ctx_t **ctx);

static int editions_module_get_display_name(AlteratorCtlEditionsModule *module,
                                            const gchar *edition_str_id,
                                            GNode *data,
                                            gchar **result);
static edition_data_t *editions_module_edition_data_new(AlteratorCtlEditionsModule *module,
                                                        const gchar *edition_str_id,
                                                        GNode *edition_node);
static int editions_module_edition_data_free(edition_data_t *data);
static gint editions_module_sort_result(gconstpointer a, gconstpointer b, gpointer user_data);

static int editions_module_description_subcommand(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx);
static int editions_module_license_subcommand(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx);
static int editions_module_list_subcommand(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx);
static int editions_module_info_subcommand(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx);
static int editions_module_set_current_subcommand(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx);
static int editions_module_get_current_subcommand(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx);

static int editions_module_handle_results(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx);
static int editions_module_handle_description_results(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx);
static int editions_module_handle_license_results(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx);
static int editions_module_handle_list_results(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx);
static int editions_module_handle_info_results(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx);
static int editions_module_handle_set_current_results(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx);
static int editions_module_handle_get_current_results(AlteratorCtlEditionsModule *module,
                                                      alteratorctl_ctx_t **ctx,
                                                      gchar **result_name);

static int editions_get_current(AlteratorCtlEditionsModule *module, gchar **result);

static int editions_module_print_list_with_filters(AlteratorCtlEditionsModule *module, edition_data_t *edition_data);

static int editions_module_validate_object_and_iface(AlteratorCtlEditionsModule *module,
                                                     const gchar *object,
                                                     const gchar *iface);

GType alterator_ctl_editions_module_get_type(void)
{
    static GType editions_module_type = 0;

    if (!editions_module_type)
    {
        static const GTypeInfo editions_module_info
            = {sizeof(AlteratorCtlEditionsModuleClass),     /* class structure size */
               NULL,                                        /* base class initializer */
               NULL,                                        /* base class finalizer */
               (GClassInitFunc) editions_module_class_init, /* class initializer */
               NULL,                                        /* class finalizer */
               NULL,                                        /* class data */
               sizeof(AlteratorCtlEditionsModule),          /* instance structure size */
               1,                                           /* preallocated instances */
               NULL,                                        /* instance initializers */
               NULL};

        const GInterfaceInfo alterator_module_interface_info = {
            (GInterfaceInitFunc) editions_module_alterator_interface_init,         /* interface_init */
            (GInterfaceFinalizeFunc) editions_module_alterator_interface_finalize, /* interface_finalize */
            NULL                                                                   /* interface_data */
        };

        editions_module_type = g_type_register_static(G_TYPE_OBJECT, /* parent class */
                                                      "AlteratorCtlEditionsModule",
                                                      &editions_module_info,
                                                      0);

        g_type_add_interface_static(editions_module_type, TYPE_ALTERATOR_CTL_MODULE, &alterator_module_interface_info);
    }

    return editions_module_type;
}

static void editions_module_class_init(AlteratorCtlEditionsModuleClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->finalize = editions_ctl_class_finalize;

    editions_module_parent_class = g_type_class_peek_parent(klass);
}

static void editions_ctl_class_finalize(GObject *klass)
{
    AlteratorCtlEditionsModuleClass *obj = (AlteratorCtlEditionsModuleClass *) klass;

    G_OBJECT_CLASS(editions_module_parent_class)->finalize(klass);
}

static void editions_module_alterator_interface_init(gpointer iface, gpointer iface_data)
{
    AlteratorCtlModuleInterface *interface = iface;

    interface->run_with_args = editions_module_run_with_args;

    interface->run = editions_module_run;

    interface->print_help = editions_module_print_help;
}

static void editions_module_alterator_interface_finalize(gpointer iface, gpointer iface_data) {}

alterator_ctl_module_t *get_editions_module()
{
    int ret                               = 0;
    static gsize editions_ctl_module_init = 0;
    if (g_once_init_enter(&editions_ctl_module_init))
    {
        gsize module_id_size = g_strlcpy(editions_module.id,
                                         ALTERATOR_CTL_EDITIONS_MODULE_NAME,
                                         strlen(ALTERATOR_CTL_EDITIONS_MODULE_NAME) + 1);

        if (module_id_size != strlen(ALTERATOR_CTL_EDITIONS_MODULE_NAME))
        {
            g_printerr(_("Internal error in get_editions_module: unvaliable id of editions module.\n"));
            ERR_EXIT();
        }

        editions_module.new_object_func  = (gpointer) editions_module_new;
        editions_module.free_object_func = (gpointer) editions_module_free;

        gsize tmp = 42;

        g_once_init_leave(&editions_ctl_module_init, tmp);
    }

    return &editions_module;

end:
    return NULL;
}

AlteratorCtlEditionsModule *editions_module_new(gpointer app)
{
    AlteratorCtlEditionsModule *object = g_object_new(TYPE_ALTERATOR_CTL_EDITIONS_MODULE, NULL);

    object->commands = g_hash_table_new(g_str_hash, g_str_equal);
    fill_command_hash_table(object->commands);

    object->alterator_ctl_app = (AlteratorCtlApp *) app;

    object->gdbus_source = alterator_gdbus_source_new(object->alterator_ctl_app->arguments->verbose, G_BUS_TYPE_SYSTEM);

    return object;
}

void editions_module_free(AlteratorCtlEditionsModule *module)
{
    g_hash_table_destroy(module->commands);

    if (module->gdbus_source)
    {
        alterator_gdbus_source_free(module->gdbus_source);
    }

    g_object_unref(module);
}

static void fill_command_hash_table(GHashTable *command)
{
    for (int i = 0; i < sizeof(editions_module_subcommands_list) / sizeof(editions_module_subcommands_t); i++)
        g_hash_table_insert(command,
                            editions_module_subcommands_list[i].subcommand,
                            &editions_module_subcommands_list[i].id);
}

static int editions_module_description_subcommand(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                     = 0;
    const gchar *edition_str_id = NULL;
    GError *dbus_call_error     = NULL;
    gchar *locale               = NULL;
    dbus_ctx_t *d_ctx           = NULL;

    if (!module)
    {
        g_printerr(_("The call to the editions description method failed. The module doesn't exist.\n"));
        ERR_EXIT();
    }

    g_variant_get((*ctx)->parameters, "(ms)", &edition_str_id);
    if (!edition_str_id)
    {
        alteratorctl_ctx_t *e_ctx = alteratorctl_ctx_init_editions(EDITIONS_GET, NULL, NULL, NULL);

        if (editions_module_get_current_subcommand(module, &e_ctx))
            ERR_EXIT();

        if (editions_module_handle_get_current_results(module, &e_ctx, (gchar **) &edition_str_id) < 0)
            ERR_EXIT();

        alteratorctl_ctx_free(e_ctx);
    }

    int is_exist = 0;
    if (module->gdbus_source->alterator_gdbus_source_check_object_by_iface(module->gdbus_source,
                                                                           edition_str_id,
                                                                           EDITIONS_INTERFACE_NAME,
                                                                           &is_exist)
        < 0)
    {
        if (edition_str_id)
            g_printerr(_("Fail to check existance of edition \"%s\"\n"), edition_str_id);
        else
            g_printerr(_("Fail to check existance of unspecified edition.\n"));
        ERR_EXIT();
    }

    if (is_exist == 0)
    {
        if (edition_str_id)
            g_printerr(_("Edition \"%s\" object not defined on D-Bus.\n"), edition_str_id);
        else
            g_printerr(_("The current edition doesn't exist.\n"));
    }

    if (!(locale = alterator_ctl_get_effective_locale()))
        ERR_EXIT();

    if (module->gdbus_source->alterator_gdbus_source_set_env_value(module->gdbus_source, "LC_ALL", locale) < 0)
        ERR_EXIT();

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                          edition_str_id && strlen(edition_str_id) ? edition_str_id : ALTERATOR_GLOBAL_PATH,
                          edition_str_id && strlen(edition_str_id) ? EDITIONS_INTERFACE_NAME
                                                                   : CURRENT_EDITION_INTERFACE_NAME,
                          CURRENT_EDITION_DESCRIPTION_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in editions_module_description_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(ayi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (d_ctx->result)
        (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);

end:
    if (d_ctx)
        dbus_ctx_free(d_ctx);

    if (edition_str_id)
        g_free((gpointer) edition_str_id);

    g_free(locale);

    g_clear_error(&dbus_call_error);

    return ret;
}

static int editions_module_license_subcommand(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                     = 0;
    const gchar *edition_str_id = NULL;
    GError *dbus_call_error     = NULL;
    gchar *locale               = NULL;
    dbus_ctx_t *d_ctx           = NULL;

    if (!module)
    {
        g_printerr(_("The call to the editions license method failed. The module doesn't exist.\n"));
        ERR_EXIT();
    }

    g_variant_get((*ctx)->parameters, "(ms)", &edition_str_id);
    if (!edition_str_id)
    {
        alteratorctl_ctx_t *e_ctx = alteratorctl_ctx_init_editions(EDITIONS_GET, NULL, NULL, NULL);

        if (editions_module_get_current_subcommand(module, &e_ctx))
            ERR_EXIT();

        if (editions_module_handle_get_current_results(module, &e_ctx, (gchar **) &edition_str_id) < 0)
            ERR_EXIT();

        alteratorctl_ctx_free(e_ctx);
    }

    int is_exist = 0;
    if (module->gdbus_source->alterator_gdbus_source_check_object_by_iface(module->gdbus_source,
                                                                           edition_str_id,
                                                                           EDITIONS_INTERFACE_NAME,
                                                                           &is_exist)
        < 0)
    {
        if (edition_str_id)
            g_printerr(_("Fail to check existance of edition \"%s\"\n"), edition_str_id);
        else
            g_printerr(_("Fail to check existance of unspecified edition.\n"));
        ERR_EXIT();
    }

    if (is_exist == 0)
    {
        if (edition_str_id)
            g_printerr(_("Edition \"%s\" object not defined on D-Bus.\n"), edition_str_id);
        else
            g_printerr(_("The current edition doesn't exist.\n"));
    }

    if (!(locale = alterator_ctl_get_effective_locale()))
        ERR_EXIT();

    if (module->gdbus_source->alterator_gdbus_source_set_env_value(module->gdbus_source, "LC_ALL", locale) < 0)
        ERR_EXIT();

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                          edition_str_id && strlen(edition_str_id) ? edition_str_id : ALTERATOR_GLOBAL_PATH,
                          edition_str_id && strlen(edition_str_id) ? EDITIONS_INTERFACE_NAME
                                                                   : CURRENT_EDITION_INTERFACE_NAME,
                          CURRENT_EDITION_LICENSE_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in editions_module_license_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(ayi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, NULL);

    if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (d_ctx->result)
        (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);

end:
    if (d_ctx)
        dbus_ctx_free(d_ctx);

    if (edition_str_id)
        g_free((gpointer) edition_str_id);

    g_free(locale);

    g_clear_error(&dbus_call_error);

    return ret;
}

static int editions_module_list_subcommand(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                                   = 0;
    gsize editions_amount                     = 0;
    GNode **editions_parsed_objects           = NULL;
    GError *dbus_call_error                   = NULL;
    GPtrArray *result                         = NULL;
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    if (!module)
    {
        g_printerr(_("Internal error in editions module - AlteratorCtlEditionsModule *module is NULL in \"editions "
                     "list\".\n"));
        ERR_EXIT();
    }

    //Check object
    if (editions_module_validate_object_and_iface(module, ALTERATOR_GLOBAL_PATH, CURRENT_EDITION_INTERFACE_NAME) < 0)
        ERR_EXIT();

    if (!(editions_parsed_objects = info_parser->alterator_ctl_module_info_parser_get_objects_data(
              info_parser, module->gdbus_source, EDITIONS_INTERFACE_NAME, &editions_amount)))
    {
        if (!editions_amount)
            g_printerr(_("Editions list is empty.\n"));
        ERR_EXIT();
    }

    result = g_ptr_array_new_full(editions_amount, (GDestroyNotify) editions_module_edition_data_free);

    for (guint i = 0; i < editions_amount; i++)
    {
        alterator_entry_node *edition_data = (alterator_entry_node *) editions_parsed_objects[i]->data;
        toml_value *edition_name = g_hash_table_lookup(edition_data->toml_pairs, EDITIONS_ALTERATOR_ENTRY_KEY_NAME);

        edition_data_t *data = editions_module_edition_data_new(module,
                                                                edition_name->str_value,
                                                                editions_parsed_objects[i]);
        g_ptr_array_add(result, data);
    }

    g_ptr_array_sort_with_data(result,
                               editions_module_sort_result,
                               (gpointer) &module->alterator_ctl_app->arguments->verbose);

    for (guint i = 0; i < result->len; i++)
        editions_module_print_list_with_filters(module, g_ptr_array_index(result, i));

end:
    if (editions_parsed_objects)
        info_parser->alterator_ctl_module_info_parser_result_trees_free(info_parser,
                                                                        editions_parsed_objects,
                                                                        editions_amount);

    if (result)
        g_ptr_array_unref(result);

    g_clear_error(&dbus_call_error);

    return ret;
}

static int editions_module_info_subcommand(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                 = 0;
    gchar *edition_str_id   = NULL;
    GError *dbus_call_error = NULL;
    dbus_ctx_t *d_ctx       = NULL;

    if (!module)
    {
        g_printerr(_("The call to the editions info method failed. The module doesn't exist.\n"));
        ERR_EXIT();
    }

    g_variant_get((*ctx)->parameters, "(ms)", &edition_str_id);
    if (!edition_str_id)
    {
        alteratorctl_ctx_t *e_ctx = alteratorctl_ctx_init_editions(EDITIONS_GET, NULL, NULL, NULL);

        if (editions_module_get_current_subcommand(module, &e_ctx))
            ERR_EXIT();

        if (editions_module_handle_get_current_results(module, &e_ctx, &edition_str_id) < 0)
            ERR_EXIT();

        alteratorctl_ctx_free(e_ctx);
    }

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                          edition_str_id && strlen(edition_str_id) ? edition_str_id : ALTERATOR_GLOBAL_PATH,
                          edition_str_id && strlen(edition_str_id) ? EDITIONS_INTERFACE_NAME
                                                                   : CURRENT_EDITION_INTERFACE_NAME,
                          CURRENT_EDITION_INFO_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in editions_module_info_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(ayi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, NULL);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Current edition is not set.\n"));
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (d_ctx->result)
        (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);

end:
    if (d_ctx)
        dbus_ctx_free(d_ctx);

    g_free(edition_str_id);

    g_clear_error(&dbus_call_error);

    return ret;
}

static int editions_module_set_current_subcommand(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                     = 0;
    const gchar *edition_str_id = NULL;
    gchar *edition_name         = NULL;
    GError *dbus_call_error     = NULL;
    dbus_ctx_t *d_ctx           = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in editions module - AlteratorCtlEditionsModule *module is NULL in \"editions "
                     "set\".\n"));
        ERR_EXIT();
    }

    g_variant_get((*ctx)->parameters, "(ms)", &edition_str_id);
    if (!edition_str_id)
    {
        g_printerr(_("Internal error in editions module: edition_str_id is NULL.\n"));
        ERR_EXIT();
    }

    edition_name = edition_str_id[0] == '/'
                       ? g_strdup(module->gdbus_source->alterator_gdbus_source_get_name_by_path(module->gdbus_source,
                                                                                                edition_str_id,
                                                                                                EDITIONS_INTERFACE_NAME))
                       : g_strdup(edition_str_id);

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                          ALTERATOR_GLOBAL_PATH,
                          CURRENT_EDITION_INTERFACE_NAME,
                          CURRENT_EDITION_SET_CURRENT_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in editions_module_set_current_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->parameters = g_variant_new("(s)", edition_name);
    d_ctx->reply_type = G_VARIANT_TYPE("(i)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Can't set a non-existent edition.\n"));
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    if (dbus_call_error)
    {
        g_printerr("%s\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (d_ctx->result)
        (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);

end:
    if (d_ctx)
        dbus_ctx_free(d_ctx);

    if (edition_str_id)
        g_free((gpointer) edition_str_id);

    if (edition_name)
        g_free(edition_name);

    g_clear_error(&dbus_call_error);

    return ret;
}

static int editions_module_get_current_subcommand(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                 = 0;
    dbus_ctx_t *d_ctx       = NULL;
    GError *dbus_call_error = NULL;
    if (!module)
    {
        g_printerr(_("Internal error in editions module - AlteratorCtlEditionsModule *module is NULL in \"editions "
                     "get\".\n"));
        ERR_EXIT();
    }

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                          ALTERATOR_GLOBAL_PATH,
                          CURRENT_EDITION_INTERFACE_NAME,
                          CURRENT_EDITION_GET_CURRENT_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in editions_module_get_current_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Current edition is not set.\n"));
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (d_ctx->result)
        (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);

end:
    if (d_ctx)
        dbus_ctx_free(d_ctx);

    g_clear_error(&dbus_call_error);

    return ret;
}

int editions_module_run_with_args(gpointer self, int argc, char **argv)
{
    int ret                            = 0;
    alteratorctl_ctx_t *ctx            = NULL;
    AlteratorCtlEditionsModule *module = ALTERATOR_CTL_EDITIONS_MODULE(self);

    if (!module)
    {
        g_printerr(
            _("Internal data error in editions module with args: AlteratorCtlEditionsModule *module is NULL.\n"));
        ERR_EXIT();
    }

    if (editions_module_parse_arguments(module, argc, argv, &ctx) < 0)
        ERR_EXIT();

    if (editions_module_run(module, ctx) < 0)
        ERR_EXIT();

    if (!is_dbus_call_error && editions_module_handle_results(module, &ctx) < 0)
        ERR_EXIT();

end:
    if (ctx)
        alteratorctl_ctx_free(ctx);
    return ret;
}

int editions_module_run(gpointer self, gpointer data)
{
    int ret                            = 0;
    AlteratorCtlModuleInterface *iface = GET_ALTERATOR_CTL_MODULE_INTERFACE(self);
    AlteratorCtlEditionsModule *module = ALTERATOR_CTL_EDITIONS_MODULE(self);

    if (!self)
    {
        g_printerr(_("Internal error in editions module run: *module is NULL.\n"));
        ERR_EXIT();
    }

    if (module->alterator_ctl_app->arguments->module_help)
        goto end;

    if (!data)
    {
        g_printerr(_("Internal error in editions module run: *context is NULL.\n"));
        ERR_EXIT();
    }

    alteratorctl_ctx_t *ctx = (alteratorctl_ctx_t *) data;

    int subcommand_id = g_variant_get_int32(ctx->subcommands_ids);

    switch (subcommand_id)
    {
    case EDITIONS_DESCRIPTION:
        if (editions_module_description_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case EDITIONS_LICENSE:
        if (editions_module_license_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case EDITIONS_LIST:
        if (editions_module_list_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case EDITIONS_LIST_PRIV:
        if (editions_module_list_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case EDITIONS_INFO:
        if (editions_module_info_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case EDITIONS_SET:
        if (editions_module_set_current_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case EDITIONS_GET:
        if (editions_module_get_current_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    default:
        g_printerr(_("Try to run unknown editions subcommand.\n"));
        ERR_EXIT();
    }

end:
    return ret;
}

static int editions_module_handle_results(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret = 0;
    if (!module)
    {
        g_printerr(_("Internal error in editions module when handle result: module is NULL.\n"));
        ERR_EXIT();
    }

    if (module->alterator_ctl_app->arguments->module_help)
        goto end;

    if (!(*ctx))
    {
        g_printerr(_("Internal error in editions module when handle results: *ctx is NULL.\n"));
        ERR_EXIT();
    }

    switch (g_variant_get_int32((*ctx)->subcommands_ids))
    {
    case EDITIONS_DESCRIPTION:
        if (editions_module_handle_description_results(module, ctx) < 0)
            ERR_EXIT();

        break;

    case EDITIONS_LICENSE:
        if (editions_module_handle_license_results(module, ctx) < 0)
            ERR_EXIT();

        break;

    case EDITIONS_LIST:
        if (editions_module_handle_list_results(module, ctx) < 0)
            ERR_EXIT();

    case EDITIONS_LIST_PRIV:
        if (editions_module_handle_list_results(module, ctx) < 0)
            ERR_EXIT();

        break;

    case EDITIONS_INFO:
        if (editions_module_handle_info_results(module, ctx) < 0)
            ERR_EXIT();

        break;

    case EDITIONS_SET:
        if (editions_module_handle_set_current_results(module, ctx) < 0)
            ERR_EXIT();

        break;

    case EDITIONS_GET:
        if (editions_module_handle_get_current_results(module, ctx, NULL) < 0)
            ERR_EXIT();

        break;

    default:
        g_printerr(_("Can't handle result of unknown command.\n"));
        ERR_EXIT();
        break;
    }

end:
    return ret;
}

static int editions_module_handle_description_results(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                 = 0;
    gchar *edition_str_id   = NULL;
    gchar *dbus_result_text = NULL;
    gchar *result           = NULL;

    if (!(*ctx)->results)
    {
        g_printerr(_("Edition description is empty.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(ayi)")))
    {
        g_printerr(_("Wrong answer type in edition description.\n"));
        ERR_EXIT();
    }

    GVariant *answer_array = g_variant_get_child_value((*ctx)->results, 0);
    GVariant *exit_code    = g_variant_get_child_value((*ctx)->results, 1);

    if (!exit_code)
    {
        g_printerr(_("Handling of edition description failed: no return code.\n"));
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
    g_free(edition_str_id);

    g_free(dbus_result_text);

    g_free(result);

    return ret;
}

static int editions_module_handle_license_results(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                 = 0;
    gchar *edition_str_id   = NULL;
    gchar *dbus_result_text = NULL;
    gchar *result           = NULL;

    if (!(*ctx)->results)
    {
        g_printerr(_("Current edition license is empty.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(ayi)")))
    {
        g_printerr(_("Wrong answer type in edition license.\n"));
        ERR_EXIT();
    }

    GVariant *answer_array = g_variant_get_child_value((*ctx)->results, 0);
    GVariant *exit_code    = g_variant_get_child_value((*ctx)->results, 1);

    if (!exit_code)
    {
        g_printerr(_("Handling of edition license failed: empty exit code.\n"));
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
    g_free(edition_str_id);

    g_free(dbus_result_text);

    g_free(result);

    return ret;
}

static int editions_module_handle_list_results(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret = 0;

end:
    return ret;
}

static int editions_module_handle_info_results(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                = 0;
    GVariant *exit_code    = NULL;
    GVariant *info_array   = NULL;
    gchar *alterator_entry = NULL;

    if (!(*ctx)->results)
    {
        g_printerr(_("D-Bus error in components info.\n"));
        ERR_EXIT();
    }

    info_array = g_variant_get_child_value((*ctx)->results, 0);
    exit_code  = g_variant_get_child_value((*ctx)->results, 1);

    if (!exit_code)
    {
        g_printerr(_("Can't get editions info result: exit code is empty.\n"));
        ERR_EXIT();
    }
    if (!info_array)
        ERR_EXIT();

    gsize array_size;
    gconstpointer gvar_info = g_variant_get_fixed_array(info_array, &array_size, sizeof(guint8));

    alterator_entry = g_malloc0(array_size + 1);
    if (!alterator_entry)
        ERR_EXIT();
    memcpy(alterator_entry, gvar_info, array_size);

    print_with_pager(alterator_entry);

end:
    g_free(alterator_entry);

    return ret;
}

static int editions_module_handle_set_current_results(AlteratorCtlEditionsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret             = 0;
    GVariant *exit_code = NULL;

    if (!(*ctx)->results)
    {
        g_printerr(_("D-Bus error in editions set handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(i)")))
    {
        g_printerr(_("Wrong answer type in editions set.\n"));
        ERR_EXIT();
    }

    exit_code = g_variant_get_child_value((*ctx)->results, 0);

    if (!exit_code)
    {
        const gchar *parameter1 = g_variant_get_string((*ctx)->parameters, NULL);
        g_printerr(_("Error while running Current() method in %s: exit_code is NULL.\n"), parameter1);
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);
    if (ret)
    {
        g_printerr(_("Editions set failed: exit code is %i.\n"), ret);
        ERR_EXIT();
    }

end:
    if (exit_code)
        g_variant_unref(exit_code);

    return ret;
}

static int editions_module_handle_get_current_results(AlteratorCtlEditionsModule *module,
                                                      alteratorctl_ctx_t **ctx,
                                                      gchar **result_name)
{
    int ret                = 0;
    GVariant *answer_array = NULL;
    GVariant *exit_code    = NULL;

    if (!(*ctx)->results)
    {
        g_printerr(_("D-Bus error in editions get handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(asi)")))
    {
        g_printerr(_("Wrong answer type in editions get.\n"));
        ERR_EXIT();
    }

    answer_array = g_variant_get_child_value((*ctx)->results, 0);
    exit_code    = g_variant_get_child_value((*ctx)->results, 1);

    if (!exit_code)
    {
        const gchar *parameter1 = g_variant_get_string((*ctx)->parameters, NULL);
        g_printerr(_("Error while running Current() method in %s: exit_code is NULL.\n"), parameter1);
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);

    GVariantIter *iter = NULL;
    gchar *str         = NULL;

    g_variant_get(answer_array, "as", &iter);
    while (g_variant_iter_loop(iter, "s", &str))
    {
        if (!result_name)
            g_print("%s\n", str);
        else
            (*result_name) = g_strdup(str);
    }

    g_variant_iter_free(iter);

end:
    if (exit_code)
        g_variant_unref(exit_code);

    if (answer_array)
        g_variant_unref(answer_array);

    return ret;
}

static int editions_get_current(AlteratorCtlEditionsModule *module, gchar **result)
{
    int ret                = 0;
    dbus_ctx_t *d_ctx      = NULL;
    GVariant *answer_array = NULL;
    GVariant *exit_code    = NULL;
    if (!module)
    {
        g_printerr(_("Internal error in editions module - AlteratorCtlEditionsModule *module is NULL in \"editions "
                     "get\".\n"));
        ERR_EXIT();
    }

    //Check object
    if (editions_module_validate_object_and_iface(module, ALTERATOR_GLOBAL_PATH, CURRENT_EDITION_INTERFACE_NAME) < 0)
        ERR_EXIT();

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                          ALTERATOR_GLOBAL_PATH,
                          CURRENT_EDITION_INTERFACE_NAME,
                          CURRENT_EDITION_GET_CURRENT_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in editions_module_get_current_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    module->gdbus_source->call(module->gdbus_source, d_ctx, NULL);

    if (!d_ctx->result)
    {
        g_printerr(_("D-Bus error in editions module while calling Current(): failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type(d_ctx->result, G_VARIANT_TYPE("(asi)")))
    {
        g_printerr(_("Wrong answer type in editions get.\n"));
        ERR_EXIT();
    }

    answer_array = g_variant_get_child_value(d_ctx->result, 0);
    exit_code    = g_variant_get_child_value(d_ctx->result, 1);

    if (!exit_code)
    {
        g_printerr(_("Error while running Current() method in: exit_code is NULL.\n"));
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);

    GVariantIter *iter = NULL;
    gchar *str         = NULL;

    g_variant_get(answer_array, "as", &iter);
    while (g_variant_iter_loop(iter, "s", &str))
        (*result) = g_strdup(str);

    g_variant_iter_free(iter);

end:
    if (d_ctx)
        dbus_ctx_free(d_ctx);

    if (exit_code)
        g_variant_unref(exit_code);

    if (answer_array)
        g_variant_unref(answer_array);

    return ret;
}

static int editions_module_parse_options(AlteratorCtlEditionsModule *module, int *argc, char **argv)
{
    int ret                        = 0;
    GOptionContext *option_context = NULL;

    // clang-format off
    static GOptionEntry editions_module_options[]
        = {{"name-only", 'n', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &name_only,
                                                    "Printing only editions names",
                                                    "NAME_ONLY"},
           {"no-name", 'N', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &no_name,
                                                    "Printing editions without names", "NO_NAME"},
           {"path-only", 'p', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &path_only,
                                                    "Printing only editions D-Bus paths",
                                                    "PATH_ONLY"},
           {"display-name-only", 'd', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &display_name_only,
                                                    "Printing only editions display names",
                                                    "DISPLAY_NAME_ONLY"},
           {"no-display-name", 'D', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &no_display_name,
                                                    "Printing editions without display names",
                                                    "NO_DISPLAY_NAME"},
           {"hide-installation-markers", 'H', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &hide_installed_markers,
                                                    "Hide installation status markers", "HIDE_INSTALLATION_MARKERS"},
           {NULL}};
    // clang-format on

    GError *error  = NULL;
    option_context = g_option_context_new("Editions module options");
    g_option_context_add_main_entries(option_context, editions_module_options, NULL);
    if (!g_option_context_parse(option_context, argc, &argv, &error))
    {
        g_printerr(_("Module editions options parsing failed: %s\n"), error->message);
        g_error_free(error);
        ERR_EXIT();
    }

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
    else if (module->alterator_ctl_app->arguments->verbose && name_only)
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

static int editions_module_parse_arguments(AlteratorCtlEditionsModule *module,
                                           int argc,
                                           char **argv,
                                           alteratorctl_ctx_t **ctx)
{
    int ret                            = 0;
    AlteratorCtlModuleInterface *iface = GET_ALTERATOR_CTL_MODULE_INTERFACE((void *) module);

    if (!iface)
    {
        g_printerr(_("Internal error in editions module while parsing arguments: *iface is NULL.\n"));
        ERR_EXIT();
    }

    if (module->alterator_ctl_app->arguments->module_help)
    {
        iface->print_help(module);
        goto end;
    }

    if (editions_module_parse_options(module, &argc, argv) < 0)
        ERR_EXIT();

    if ((argc < 2) || (argc > 4))
    {
        g_printerr(_("Wrong arguments in editions module.\n"));
        iface->print_help(module);
        ERR_EXIT();
    }

    int selected_subcommand = iface->get_command_id(module->commands, argv[2]);
    switch (selected_subcommand)
    {
    case EDITIONS_LIST:
        if (argc == 3)
            (*ctx) = alteratorctl_ctx_init_editions(EDITIONS_LIST, NULL, NULL, NULL);
        else
        {
            g_printerr(_("Wrong arguments to list module command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    case EDITIONS_DESCRIPTION:
        if (argc == 4)
            (*ctx) = alteratorctl_ctx_init_editions(EDITIONS_DESCRIPTION, argv[3], NULL, NULL);
        else if (argc == 3)
            (*ctx) = alteratorctl_ctx_init_editions(EDITIONS_DESCRIPTION, NULL, NULL, NULL);
        else
        {
            g_printerr(_("Wrong arguments to description module command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    case EDITIONS_LICENSE:
        if (argc == 4)
            (*ctx) = alteratorctl_ctx_init_editions(EDITIONS_LICENSE, argv[3], NULL, NULL);
        else if (argc == 3)
            (*ctx) = alteratorctl_ctx_init_editions(EDITIONS_LICENSE, NULL, NULL, NULL);
        else
        {
            g_printerr(_("Wrong arguments to license module command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    case EDITIONS_INFO:
        if (argc == 4)
            (*ctx) = alteratorctl_ctx_init_editions(EDITIONS_INFO, argv[3], NULL, NULL);
        else if (argc == 3)
            (*ctx) = alteratorctl_ctx_init_editions(EDITIONS_INFO, NULL, NULL, NULL);
        else
        {
            g_printerr(_("Wrong arguments to info module command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    case EDITIONS_SET:
        if (argc == 4)
            (*ctx) = alteratorctl_ctx_init_editions(EDITIONS_SET, argv[3], NULL, NULL);
        else
        {
            g_printerr(_("Wrong arguments to set module command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    case EDITIONS_GET:
        if (argc == 3)
            (*ctx) = alteratorctl_ctx_init_editions(EDITIONS_GET, NULL, NULL, NULL);
        else
        {
            g_printerr(_("Wrong arguments to get module command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    default:

        if (argc == 2)
            (*ctx) = alteratorctl_ctx_init_editions(EDITIONS_LIST_PRIV, NULL, NULL, NULL);
        else
        {
            g_printerr(_("Unknown module editions command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;
    }

end:
    return ret;
}

static int editions_module_print_list_with_filters(AlteratorCtlEditionsModule *module, edition_data_t *edition_data)
{
    int ret                 = 0;
    gchar *current_edition  = NULL;
    gchar *installed_marker = NULL;

    if (!hide_installed_markers)
    {
        editions_get_current(module, &current_edition);
        installed_marker = g_strcmp0(edition_data->name, current_edition) == 0 ? g_strdup("* ") : g_strdup("  ");
    }
    else
        installed_marker = g_strdup("");

    if (module->alterator_ctl_app->arguments->verbose)
    {
        if (!no_display_name)
            g_print("%s%s (%s : %s)\n",
                    installed_marker,
                    edition_data->display_name,
                    edition_data->name,
                    edition_data->path);
        else
            g_print("%s%s : %s\n", installed_marker, edition_data->display_name, edition_data->name, edition_data->path);
    }
    else if (path_only)
        g_print("%s%s\n", installed_marker, edition_data->path);
    else if (display_name_only || no_name)
        g_print("%s%s\n", installed_marker, edition_data->display_name);
    else if (no_display_name || name_only)
        g_print("%s%s\n", installed_marker, edition_data->name);
    else if (!display_name_only && !path_only && !name_only)
        g_print("%s%s (%s)\n", installed_marker, edition_data->display_name, edition_data->name);

end:
    g_free(installed_marker);

    g_free(current_edition);

    return ret;
}

static int editions_module_get_display_name(AlteratorCtlEditionsModule *module,
                                            const gchar *edition_str_id,
                                            GNode *data,
                                            gchar **result)
{
    int ret                                   = 0;
    gchar *locale                             = NULL;
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    if (!(locale = alterator_ctl_get_effective_language()))
        ERR_EXIT();

    GHashTable *display_name = NULL;
    if (!info_parser->alterator_ctl_module_info_parser_find_table(info_parser,
                                                                  data,
                                                                  &display_name,
                                                                  -1,
                                                                  EDITIONS_ALTERATOR_ENTRY_DISPLAY_NAME_TABLE_NAME,
                                                                  NULL))
    {
        g_printerr(_("Can't get display name of edition %s. Display name data by key %s is empty.\n"),
                   edition_str_id,
                   EDITIONS_ALTERATOR_ENTRY_DISPLAY_NAME_TABLE_NAME);
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

static edition_data_t *editions_module_edition_data_new(AlteratorCtlEditionsModule *module,
                                                        const gchar *edition_str_id,
                                                        GNode *edition_node)
{
    edition_data_t *result = g_malloc0(sizeof(edition_data_t));
    result->name           = g_strdup(edition_str_id);
    editions_module_get_display_name(module, edition_str_id, edition_node, &result->display_name);
    result->path = g_strdup(module->gdbus_source->alterator_gdbus_source_get_path_by_name(module->gdbus_source,
                                                                                          edition_str_id,
                                                                                          EDITIONS_INTERFACE_NAME));

    return result;
}

static int editions_module_edition_data_free(edition_data_t *data)
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

static gint editions_module_sort_result(gconstpointer a, gconstpointer b, gpointer user_data)
{
    edition_data_t *edition_data_first  = (edition_data_t *) ((GPtrArray *) a)->pdata;
    edition_data_t *edition_data_second = (edition_data_t *) ((GPtrArray *) b)->pdata;

    const gchar *first_comparable_data  = NULL;
    const gchar *second_comparable_data = NULL;
    gboolean is_verbose                 = *((gboolean *) user_data);

    if ((display_name_only || no_name) && !no_display_name && !path_only && !name_only)
    {
        first_comparable_data  = g_strdup((const gchar *) edition_data_first->display_name);
        second_comparable_data = g_strdup((const gchar *) edition_data_second->display_name);
    }
    else if ((no_display_name || name_only) && !path_only && !display_name_only)
    {
        first_comparable_data  = g_strdup((const gchar *) edition_data_first->name);
        second_comparable_data = g_strdup((const gchar *) edition_data_second->name);
    }
    else if ((no_display_name || (is_verbose && no_name) || path_only) && !name_only && !display_name_only)
    {
        first_comparable_data  = g_strdup(edition_data_first->path);
        second_comparable_data = g_strdup(edition_data_second->path);
    }
    else
    {
        // Default
        first_comparable_data  = edition_data_first->display_name;
        second_comparable_data = edition_data_second->display_name;
    }

    return g_utf8_collate(first_comparable_data, second_comparable_data);
}

int editions_module_print_help(gpointer self)
{
    int ret = 0;
    g_print(_("Usage:\n"));
    g_print(_("  alteratorctl editions [[COMMAND] [edition]] [OPTIONS]\n"));
    g_print(_("  alteratorctl editions [OPTIONS]\n"
              "                              List editions\n\n"));
    g_print(_("Commands:\n"));
    g_print(_("  description [edition]       Get description of specified or current edition\n"));
    g_print(_("  get                         Get current edition name\n"));
    g_print(_("  info [edition]              Get info about specified or current edition\n"));
    g_print(_("  license [edition]           Get license of specified or current edition\n"));
    g_print(_("  list                        List editions\n"));
    g_print(_("  set <edition>               Choose the specified edition\n\n"));
    g_print(_("Options:\n"));
    g_print(_("  -v, --verbose               Add editions paths to editions list output\n"));
    g_print(_("  -d, --display-name-only     Show only editions display names\n"));
    g_print(_("  -D, --no-display-name       Show editions without display names\n"));
    g_print(_("  -p, --path-only             Show only editions objects paths on D-Bus\n"));
    g_print(_("  -n, --name-only             Show only editions objects names\n"));
    g_print(_("  -N, --no-name               Hide editions names\n"));
    g_print(_("  -H, --hide-installation-markers\n"
              "                              Hide the installed edition marker\n"));
    g_print(_("  -h, --help                  Show module editions usage\n\n"));

end:
    return ret;
}

static int editions_module_validate_object_and_iface(AlteratorCtlEditionsModule *module,
                                                     const gchar *object,
                                                     const gchar *iface)
{
    int ret          = 0;
    int object_exist = 0;
    int iface_exists = 0;

    //Check object
    if (module->gdbus_source->alterator_gdbus_source_check_object_by_path(module->gdbus_source, object, &object_exist)
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

    //check interface of the object
    if (module->gdbus_source->alterator_gdbus_source_check_object_by_iface(module->gdbus_source,
                                                                           object,
                                                                           iface,
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
