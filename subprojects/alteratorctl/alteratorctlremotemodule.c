#include "alteratorctlremotemodule.h"

#define REMOTE_INTERFACE_NAME "org.altlinux.alterator.remote"
#define REMOTE_CONNECT_METHOD_NAME "Connect"
#define REMOTE_DISCONNECT_METHOD_NAME "Disconnect"
#define REMOTE_LIST_METHOD_NAME "GetConnections"

#define NAME_IP_ADDRESS_DELIM "@"

#define PASSWORD_AGENT_SERVICE "org.altlinux.alterator.password"
#define PASSWORD_AGENT_OBJECT "/org/altlinux/PasswordAgent"
#define PASSWORD_AGENT_INTERFACE "org.altlinux.PasswordAgent"

typedef struct remote_module_subcommands_t
{
    char *subcommand;
    enum remote_sub_commands id;
} remote_module_subcommands_t;

static remote_module_subcommands_t remote_module_subcommands_list[] = {{"connect", REMOTE_CONNECT},
                                                                       {"disconnect", REMOTE_DISCONNECT},
                                                                       {"list", REMOTE_LIST}};

static GObjectClass *remote_module_parent_class = NULL;
static alterator_ctl_module_t remote_module     = {0};

static void remote_module_class_init(AlteratorCtlRemoteModuleClass *klass);
static void remote_ctl_class_finalize(GObject *klass);

static void remote_module_alterator_interface_init(gpointer iface, gpointer iface_data);
static void remote_module_alterator_interface_finalize(gpointer iface, gpointer iface_data);

AlteratorCtlRemoteModule *remote_module_new(gpointer app);
void remote_module_free(AlteratorCtlRemoteModule *module);

static void fill_command_hash_table(GHashTable *command);

static int remote_module_parse_arguments(AlteratorCtlRemoteModule *module,
                                         int argc,
                                         char **argv,
                                         alteratorctl_ctx_t **ctx);

static int remote_module_connect_subcommand(AlteratorCtlRemoteModule *module, alteratorctl_ctx_t **ctx);
static int remote_module_disconnect_subcommand(AlteratorCtlRemoteModule *module, alteratorctl_ctx_t **ctx);
static int remote_module_list_subcommand(AlteratorCtlRemoteModule *module, alteratorctl_ctx_t **ctx);

static int remote_module_handle_results(AlteratorCtlRemoteModule *module, alteratorctl_ctx_t **ctx);
static int remote_module_connect_handle_result(AlteratorCtlRemoteModule *module, alteratorctl_ctx_t **ctx);
static int remote_module_disconnect_handle_result(AlteratorCtlRemoteModule *module, alteratorctl_ctx_t **ctx);
static int remote_module_list_handle_result(AlteratorCtlRemoteModule *module, alteratorctl_ctx_t **ctx);

static int remote_module_validate_object_and_iface(AlteratorCtlRemoteModule *module,
                                                   const gchar *object,
                                                   const gchar *iface);

static int remote_module_validation_connection_address(const gchar *connection_address);

static int remote_module_password_agent_validation(AlteratorGDBusSource *source,
                                                   const gchar *service,
                                                   const gchar *path,
                                                   const gchar *iface);

GType alterator_ctl_remote_module_get_type(void)
{
    static GType remote_module_type = 0;

    if (!remote_module_type)
    {
        static const GTypeInfo remote_module_info         //
            = {sizeof(AlteratorCtlRemoteModuleClass),     /* class structure size */
               NULL,                                      /* base class initializer */
               NULL,                                      /* base class finalizer */
               (GClassInitFunc) remote_module_class_init, /* class initializer */
               NULL,                                      /* class finalizer */
               NULL,                                      /* class data */
               sizeof(AlteratorCtlRemoteModule),          /* instance structure size */
               1,                                         /* preallocated instances */
               NULL,                                      /* instance initializers */
               NULL};

        const GInterfaceInfo alterator_module_interface_info = {
            (GInterfaceInitFunc) remote_module_alterator_interface_init,         /* interface_init */
            (GInterfaceFinalizeFunc) remote_module_alterator_interface_finalize, /* interface_finalize */
            NULL                                                                 /* interface_data */
        };

        remote_module_type = g_type_register_static(G_TYPE_OBJECT, /* parent class */
                                                    "AlteratorCtlRemoteModule",
                                                    &remote_module_info,
                                                    0);

        g_type_add_interface_static(remote_module_type, TYPE_ALTERATOR_CTL_MODULE, &alterator_module_interface_info);
    }

    return remote_module_type;
}

static void remote_module_class_init(AlteratorCtlRemoteModuleClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->finalize = remote_ctl_class_finalize;

    remote_module_parent_class = g_type_class_peek_parent(klass);
}

static void remote_ctl_class_finalize(GObject *klass)
{
    AlteratorCtlRemoteModuleClass *obj = (AlteratorCtlRemoteModuleClass *) klass;

    G_OBJECT_CLASS(remote_module_parent_class)->finalize(klass);
}

static void remote_module_alterator_interface_init(gpointer iface, gpointer iface_data)
{
    AlteratorCtlModuleInterface *interface = iface;

    interface->run_with_args = remote_module_run_with_args;

    interface->run = remote_module_run;

    interface->print_help = remote_module_print_help;
}

alterator_ctl_module_t *get_remote_module()
{
    int ret                             = 0;
    static gsize remote_ctl_module_init = 0;
    if (g_once_init_enter(&remote_ctl_module_init))
    {
        gsize module_id_size = g_strlcpy(remote_module.id,
                                         ALTERATOR_CTL_REMOTE_MODULE_NAME,
                                         strlen(ALTERATOR_CTL_REMOTE_MODULE_NAME) + 1);

        if (module_id_size != strlen(ALTERATOR_CTL_REMOTE_MODULE_NAME))
        {
            g_printerr(_("Internal error in get_remote_module: unvaliable id of remote module.\n"));
            ERR_EXIT();
        }

        remote_module.new_object_func  = (gpointer) remote_module_new;
        remote_module.free_object_func = (gpointer) remote_module_free;

        gsize tmp = 42;

        g_once_init_leave(&remote_ctl_module_init, tmp);
    }

    return &remote_module;

end:
    return NULL;
}

AlteratorCtlRemoteModule *remote_module_new(gpointer app)
{
    AlteratorCtlRemoteModule *object = g_object_new(TYPE_ALTERATOR_CTL_REMOTE_MODULE, NULL);

    object->commands = g_hash_table_new(g_str_hash, g_str_equal);
    fill_command_hash_table(object->commands);

    object->alterator_ctl_app = (AlteratorCtlApp *) app;

    object->gdbus_source = alterator_gdbus_source_new(object->alterator_ctl_app->arguments->verbose, G_BUS_TYPE_SESSION);

    return object;
}

void remote_module_free(AlteratorCtlRemoteModule *module)
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
    for (int i = 0; i < sizeof(remote_module_subcommands_list) / sizeof(remote_module_subcommands_t); i++)
        g_hash_table_insert(command,
                            remote_module_subcommands_list[i].subcommand,
                            &remote_module_subcommands_list[i].id);
}

static void remote_module_alterator_interface_finalize(gpointer iface, gpointer iface_data) {}

static int remote_module_parse_arguments(AlteratorCtlRemoteModule *module,
                                         int argc,
                                         char **argv,
                                         alteratorctl_ctx_t **ctx)
{
    int ret                            = 0;
    AlteratorCtlModuleInterface *iface = GET_ALTERATOR_CTL_MODULE_INTERFACE((void *) module);

    if (!iface)
    {
        g_printerr(_("Internal error in remote module while parsing arguments: *iface is NULL.\n"));
        ERR_EXIT();
    }

    if (module->alterator_ctl_app->arguments->module_help)
    {
        iface->print_help(module);
        goto end;
    }

    if (argc < 3 || argc > 7)
    {
        g_printerr(_("Wrong arguments in remote module.\n"));
        iface->print_help(module);
        ERR_EXIT();
    }

    int selected_subcommand = iface->get_command_id(module->commands, argv[2]);
    if (selected_subcommand < 0)
    {
        g_printerr(_("Wrong remote module command.\n"));
        iface->print_help(module);
        ERR_EXIT();
    }

    switch (selected_subcommand)
    {
    case REMOTE_CONNECT:
        if (argc == 7)
            (*ctx) = alteratorctl_ctx_init_remote(REMOTE_CONNECT, argv[3], argv[4], argv[5], argv[6], NULL, NULL);
        else
        {
            g_printerr(_("Wrong arguments to connect module command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    case REMOTE_DISCONNECT:
        if (argc == 4)
            (*ctx) = alteratorctl_ctx_init_remote(REMOTE_DISCONNECT, argv[3], NULL, NULL, NULL, NULL, NULL);
        else
        {
            g_printerr(_("Wrong arguments to disconnect module command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    case REMOTE_LIST:
        if (argc == 3)
            (*ctx) = alteratorctl_ctx_init_remote(REMOTE_LIST, NULL, NULL, NULL, NULL, NULL, NULL);
        else
        {
            g_printerr(_("Wrong arguments to list module command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    default:
        g_printerr(_("Unknown module remote command.\n"));
        iface->print_help(module);
        ERR_EXIT();
        break;
    }

end:
    return ret;
}

int remote_module_run_with_args(gpointer self, int argc, char **argv)
{
    int ret                          = 0;
    alteratorctl_ctx_t *ctx          = NULL;
    AlteratorCtlRemoteModule *module = ALTERATOR_CTL_REMOTE_MODULE(self);

    if (!module)
    {
        g_printerr(_("Internal data error in remote module with args: AlteratorCtlRemoteModule *module is NULL.\n"));
        ERR_EXIT();
    }

    if (remote_module_parse_arguments(module, argc, argv, &ctx) < 0)
        ERR_EXIT();

    if (remote_module_run(module, ctx) < 0)
        ERR_EXIT();

    if (remote_module_handle_results(module, &ctx) < 0)
        ERR_EXIT();

end:
    if (ctx)
        alteratorctl_ctx_free(ctx);
    return ret;
}

int remote_module_run(gpointer self, gpointer data)
{
    int ret                            = 0;
    AlteratorCtlModuleInterface *iface = GET_ALTERATOR_CTL_MODULE_INTERFACE(self);
    AlteratorCtlRemoteModule *module   = ALTERATOR_CTL_REMOTE_MODULE(self);

    if (module->alterator_ctl_app->arguments->module_help)
        goto end;

    if (!self || !data)
    {
        g_printerr(_("Internal error in remote module run: *module or *context is NULL.\n"));
        ERR_EXIT();
    }

    alteratorctl_ctx_t *ctx = (alteratorctl_ctx_t *) data;

    switch (g_variant_get_int32(ctx->subcommands_ids))
    {
    case REMOTE_CONNECT:
        if (remote_module_connect_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case REMOTE_DISCONNECT:
        if (remote_module_disconnect_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    case REMOTE_LIST:
        if (remote_module_list_subcommand(module, &ctx))
            ERR_EXIT();
        break;

    default:
        g_printerr(_("Try to run unknown remote subcommand.\n"));
        ERR_EXIT();
    }

end:
    return ret;
}

static int remote_module_connect_subcommand(AlteratorCtlRemoteModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                = 0;
    gchar *remote_address  = NULL;
    gchar *connection_name = NULL;
    gchar *agent_bus_name  = NULL;
    gchar *pty             = NULL;

    dbus_ctx_t *d_ctx = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in remote module - AlteratorCtlRemoteModule *module is NULL in \"remote "
                     "connect\".\n"));
        ERR_EXIT();
    }

    g_variant_get((*ctx)->parameters, "(msmsmsms)", &remote_address, &connection_name, &agent_bus_name, &pty);
    if (!remote_address | !connection_name | !agent_bus_name | !pty)
    {
        g_printerr(_("Not enough parameters for remote connect subcommand.\n"));
        remote_module_print_help((gpointer) module);
        ERR_EXIT();
    }

    if (remote_module_validation_connection_address(remote_address) < 0)
        ERR_EXIT();

    //Check remote object
    if (remote_module_validate_object_and_iface(module, ALTERATOR_REMOTE_PATH, REMOTE_INTERFACE_NAME) < 0)
        ERR_EXIT();

    // password agent validation
    if (remote_module_password_agent_validation(module->gdbus_source,
                                                PASSWORD_AGENT_SERVICE,
                                                PASSWORD_AGENT_OBJECT,
                                                PASSWORD_AGENT_INTERFACE)
        < 0)
        ERR_EXIT();

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                          ALTERATOR_REMOTE_PATH,
                          REMOTE_INTERFACE_NAME,
                          REMOTE_CONNECT_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in remote_module_connect_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->parameters = g_variant_new("(ssss)", remote_address, connection_name, agent_bus_name, pty);
    d_ctx->reply_type = G_VARIANT_TYPE("(bs)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, NULL);

    if (d_ctx->result)
        (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);
    else
    {
        g_printerr(_("D-Bus error in remote module while calling Connect(): failed to produce a result.\n"));
        ERR_EXIT();
    }

end:
    if (d_ctx)
        dbus_ctx_free(d_ctx);

    if (remote_address)
        g_free(remote_address);

    if (connection_name)
        g_free(connection_name);

    if (agent_bus_name)
        g_free(agent_bus_name);

    if (pty)
        g_free(pty);

    return ret;
}

static int remote_module_disconnect_subcommand(AlteratorCtlRemoteModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                = 0;
    gchar *connection_name = NULL;
    dbus_ctx_t *d_ctx      = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in remote module - AlteratorCtlRemoteModule *module is NULL in \"remote "
                     "disconnect\".\n"));
        ERR_EXIT();
    }

    g_variant_get((*ctx)->parameters, "(msmsmsms)", &connection_name, NULL, NULL, NULL);
    if (!connection_name)
    {
        g_printerr(_("Not enough parameters for remote disconnect subcommand.\n"));
        remote_module_print_help((gpointer) module);
        ERR_EXIT();
    }

    //Check remote object
    if (remote_module_validate_object_and_iface(module, ALTERATOR_REMOTE_PATH, REMOTE_INTERFACE_NAME) < 0)
        ERR_EXIT();

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                          ALTERATOR_REMOTE_PATH,
                          REMOTE_INTERFACE_NAME,
                          REMOTE_DISCONNECT_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in remote_module_disconnect_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->parameters = g_variant_new("(s)", connection_name);
    d_ctx->reply_type = G_VARIANT_TYPE("(b)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, NULL);

    if (d_ctx->result)
        (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);
    else
    {
        g_printerr(_("D-Bus error in remote module while calling Disconnect(): failed to produce a result.\n"));
        ERR_EXIT();
    }

end:
    if (d_ctx)
        dbus_ctx_free(d_ctx);

    if (connection_name)
        g_free(connection_name);

    return ret;
}

static int remote_module_list_subcommand(AlteratorCtlRemoteModule *module, alteratorctl_ctx_t **ctx)
{
    int ret           = 0;
    dbus_ctx_t *d_ctx = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in remote module - AlteratorCtlRemoteModule *module is NULL in \"remote "
                     "list\".\n"));
        ERR_EXIT();
    }

    //Check remote object
    if (remote_module_validate_object_and_iface(module, ALTERATOR_REMOTE_PATH, REMOTE_INTERFACE_NAME) < 0)
        ERR_EXIT();

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                          ALTERATOR_REMOTE_PATH,
                          REMOTE_INTERFACE_NAME,
                          REMOTE_LIST_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in remote_module_list_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(as)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, NULL);

    if (d_ctx->result)
        (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);
    else
    {
        g_printerr(_("D-Bus error in remote module while calling GetConnections(): failed to produce a result.\n"));
        ERR_EXIT();
    }

end:
    if (d_ctx)
        dbus_ctx_free(d_ctx);

    return ret;
}

static int remote_module_handle_results(AlteratorCtlRemoteModule *module, alteratorctl_ctx_t **ctx)
{
    int ret = 0;
    if (!module)
    {
        g_printerr(_("Internal error in remote module when handle results: *module is NULL.\n"));
        ERR_EXIT();
    }

    if (module->alterator_ctl_app->arguments->module_help)
        goto end;

    if (!(*ctx))
    {
        g_printerr(_("Internal error in remote module when handle results: *ctx is NULL.\n"));
        ERR_EXIT();
    }

    switch (g_variant_get_int32((*ctx)->subcommands_ids))
    {
    case REMOTE_CONNECT:
        if (remote_module_connect_handle_result(module, ctx) < 0)
            ERR_EXIT();
        break;

    case REMOTE_DISCONNECT:
        if (remote_module_disconnect_handle_result(module, ctx) < 0)
            ERR_EXIT();
        break;

    case REMOTE_LIST:
        if (remote_module_list_handle_result(module, ctx) < 0)
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

static int remote_module_connect_handle_result(AlteratorCtlRemoteModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                      = 0;
    GVariant *is_success         = NULL;
    GVariant *remote_object_path = NULL;

    if (!(*ctx)->results)
    {
        g_printerr(_("D-Bus error in remote connect handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(bs)")))
    {
        g_printerr(_("Wrong answer type in remote connect\n"));
        ERR_EXIT();
    }

    is_success         = g_variant_get_child_value((*ctx)->results, 0);
    remote_object_path = g_variant_get_child_value((*ctx)->results, 1);

    ret = (int) (!g_variant_get_boolean(is_success));
    if (ret)
    {
        g_printerr(_("Failed to create a connection\n"));
    }

    g_print(_("Remote object path: %s\n"), g_variant_get_string(remote_object_path, NULL));

end:
    if (remote_object_path)
        g_variant_unref(remote_object_path);

    if (is_success)
        g_variant_unref(is_success);

    return ret;
}

static int remote_module_disconnect_handle_result(AlteratorCtlRemoteModule *module, alteratorctl_ctx_t **ctx)
{
    int ret              = 0;
    GVariant *is_success = NULL;

    if (!(*ctx)->results)
    {
        g_printerr(_("D-Bus error in remote disconnect handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(b)")))
    {
        g_printerr(_("Wrong answer type in remote disconnect.\n"));
        ERR_EXIT();
    }

    is_success = g_variant_get_child_value((*ctx)->results, 0);

    ret = (int) (!g_variant_get_boolean(is_success));
    if (ret)
    {
        g_printerr(_("Failed to disconnect\n"));
    }

end:
    if (is_success)
        g_variant_unref(is_success);

    return ret;
}

static int remote_module_list_handle_result(AlteratorCtlRemoteModule *module, alteratorctl_ctx_t **ctx)
{
    int ret = 0;

    GVariant *answer_array = NULL;

    if (!(*ctx)->results)
    {
        g_printerr(_("D-Bus error in remote list handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(as)")))
    {
        g_printerr(_("Wrong answer type in remote list.\n"));
        ERR_EXIT();
    }

    answer_array = g_variant_get_child_value((*ctx)->results, 0);

    GVariantIter *iter = NULL;
    gchar *str         = NULL;

    g_variant_get(answer_array, "as", &iter);
    while (g_variant_iter_loop(iter, "s", &str))
    {
        g_print("%s\n", str);
    }

    g_variant_iter_free(iter);

end:
    if (answer_array)
        g_variant_unref(answer_array);

    return ret;
}

static int remote_module_validation_connection_address(const gchar *connection_address)
{
    int ret                          = 0;
    gchar **connection_address_parts = NULL;
    GRegex *ipv4_regex               = NULL;
    GMatchInfo *ipv4_match_info      = NULL;

    if (!g_strrstr(connection_address, NAME_IP_ADDRESS_DELIM))
    {
        g_printerr(_("Invalid connection address: an '@' delimiter between name and IP is required.\n"));
        ERR_EXIT();
    }

    connection_address_parts = g_strsplit(connection_address, NAME_IP_ADDRESS_DELIM, 2);
    if (!connection_address_parts)
    {
        g_printerr(_("Invalid connection address: can't split connection address to username and IP.\n"));
        ERR_EXIT();
    }

    if (!strlen(connection_address_parts[0]))
    {
        g_printerr(_("Invalid connection address: username is empty.\n"));
        ERR_EXIT();
    }

    ipv4_regex
        = g_regex_new("^(\\b25[0-5]|\\b2[0-4][0-9]|\\b[01]?[0-9][0-9]?)(\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}$",
                      0,
                      0,
                      NULL);

    g_regex_match(ipv4_regex, connection_address_parts[1], 0, &ipv4_match_info);
    if (!g_match_info_matches(ipv4_match_info))
    {
        g_printerr(_("Invalid connection address: invalid IP.\n"));
        ERR_EXIT();
    }

end:
    if (ipv4_match_info)
        g_match_info_free(ipv4_match_info);

    if (ipv4_regex)
        g_regex_unref(ipv4_regex);

    if (connection_address_parts)
        g_strfreev(connection_address_parts);

    return ret;
}

int remote_module_print_help(gpointer self)
{
    int ret = 0;

    g_print(_("Usage:\n"));
    g_print(_("  alteratorctl remote <COMMAND> [arguments] [OPTIONS]\n\n"));
    g_print(_("Commands:\n"));
    g_print(_("  connect <remote address> <connection name> <agent bus name> <pty> Connect to a\n"
              "                              remote machine via ssh\n"));
    g_print(_("  disconnect <connection name> Disconnect from the remote machine\n"));
    g_print(_("  list                        Get the list of names of installed ssh connections\n\n"));
    g_print(_("Options:\n"));
    g_print(_("  -h, --help                  Show module remote usage\n\n"));

end:
    return ret;
}

static int remote_module_validate_object_and_iface(AlteratorCtlRemoteModule *module,
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

static int remote_module_password_agent_validation(AlteratorGDBusSource *source,
                                                   const gchar *service,
                                                   const gchar *path,
                                                   const gchar *iface)
{
    int ret                      = 0;
    GHashTable *services         = NULL;
    gchar *introspection         = NULL;
    GRegex *iface_regex          = NULL;
    GMatchInfo *iface_match_info = NULL;

    if (!service | !path | !iface)
    {
        g_printerr(_("Can't validate password agent: wrong password agent service, path or iface.\n"));
        ERR_EXIT();
    }

    //Get services names
    if (source->alterator_gdbus_source_get_services_names(source, &services) < 0)
    {
        g_printerr(_("Can't get services names in D-Bus system bus\n"));
        ERR_EXIT();
    }

    // Check remote password service
    if (!g_hash_table_lookup(services, service))
    {
        g_printerr(_("%s service not found.\n"), service);
        ERR_EXIT();
    }

    source->alterator_gdbus_source_get_introspection(source, service, path, &introspection);
    if (!introspection)
    {
        g_printerr(_("Can't get introspection of password agent by path: %s in service: %s\n"), path, service);
        ERR_EXIT();
    }

    iface_regex = g_regex_new(iface, 0, 0, NULL);
    g_regex_match(iface_regex, introspection, 0, &iface_match_info);
    if (!g_match_info_matches(iface_match_info))
    {
        g_printerr(_("Password agent with iface %s doesn't exists.\n"), iface);
        ERR_EXIT();
    }

end:
    if (services)
        g_hash_table_unref(services);

    if (introspection)
        g_free(introspection);

    if (iface_regex)
        g_regex_unref(iface_regex);

    if (iface_match_info)
        g_match_info_unref(iface_match_info);

    return ret;
}
