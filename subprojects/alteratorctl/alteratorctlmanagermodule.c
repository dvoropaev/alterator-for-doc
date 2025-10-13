#include "alteratorctlmanagermodule.h"

#include <gio/gioenums.h>

#define ALTERATOR_MANAGER_INTERFACE "org.altlinux.alterator.manager"
#define ALTERATOR_MANAGER_GETOBJECT_METHOD_NAME "GetObjects"
#define ALTERATOR_MANAGER_GETALLOBJECT_METHOD_NAME "GetAllObjects"
#define ALTERATOR_MANAGER_GETINTERFACES_METHOD_NAME "GetInterfaces"
#define ALTERATOR_MANAGER_GETALLINTERFACES_METHOD_NAME "GetAllInterfaces"
#define ALTERATOR_MANAGER_GETSIGNALS_METHOD_NAME "GetSignals"

#define ALTERATOR_MANAGER_SYSTEM_BUS_PARAMETER "--system"
#define ALTERATOR_MANAGER_SESSION_BUS_PARAMETER "--session"

typedef struct
{
    char *subcommand;
    enum manager_sub_commands id;
} manager_module_subcommands_t;

static manager_module_subcommands_t manager_module_subcommands_list[] = {{"getobjects", MANAGER_GETOBJECTS},
                                                                         {"getifaces", MANAGER_GETIFACES},
                                                                         {"getsignals", MANAGER_GETSIGNALS}};

static GObjectClass *manager_module_parent_class = NULL;
static alterator_ctl_module_t manager_module     = {0};

static void manager_module_class_init(AlteratorCtlManagerModuleClass *klass);
static void manager_ctl_class_finalize(GObject *klass);

static void manager_module_alterator_interface_init(gpointer iface, gpointer iface_data);
static void manager_module_alterator_interface_finalize(gpointer iface, gpointer iface_data);

AlteratorCtlManagerModule *manager_module_new(gpointer app);
void manager_module_free(AlteratorCtlManagerModule *module);

static void fill_command_hash_table(GHashTable *command);

//Subcommand functions
static int manager_module_run_subcommand_getobjects(AlteratorCtlManagerModule *module, alteratorctl_ctx_t **ctx);
static int manager_module_run_subcommand_getifaces(AlteratorCtlManagerModule *module, alteratorctl_ctx_t **ctx);
static int manager_module_run_subcommand_getsignals(AlteratorCtlManagerModule *module, alteratorctl_ctx_t **ctx);
static int manager_module_help_subcommand(AlteratorCtlManagerModule *module, alteratorctl_ctx_t **ctx);

static int manager_module_parse_arguments(AlteratorCtlManagerModule *module,
                                          int argc,
                                          char **argv,
                                          alteratorctl_ctx_t **ctx);

static gint manager_module_sort_result(gconstpointer a, gconstpointer b);

//Manager module handle result functions
static int manager_module_handle_results(AlteratorCtlManagerModule *module, alteratorctl_ctx_t **ctx);
static int manager_module_handle_getobjects_results(AlteratorCtlManagerModule *module, alteratorctl_ctx_t **ctx);
static int manager_module_handle_getifaces_results(AlteratorCtlManagerModule *module, alteratorctl_ctx_t **ctx);
static int manager_module_handle_getsignals_results(AlteratorCtlManagerModule *module, alteratorctl_ctx_t **ctx);

static int manager_module_initialize_getobject_ctx(alteratorctl_ctx_t **ctx, int subcommand, int argc, char **argv);
static int manager_module_initialize_getiface_ctx(alteratorctl_ctx_t **ctx, int subcommand, int argc, char **argv);
static int manager_module_initialize_getsignals_ctx(alteratorctl_ctx_t **ctx, int subcommand, int argc, char **argv);

int manager_module_print_help(gpointer self);

GType alterator_ctl_manager_module_get_type(void)
{
    static GType manager_module_type = 0;

    if (!manager_module_type)
    {
        static const GTypeInfo manager_module_info = {sizeof(AlteratorCtlManagerModuleClass), /* class structure size */
                                                      NULL, /* base class initializer */
                                                      NULL, /* base class finalizer */
                                                      (GClassInitFunc) manager_module_class_init, /* class initializer */
                                                      NULL,                                       /* class finalizer */
                                                      NULL,                                       /* class data */
                                                      sizeof(AlteratorCtlManagerModule), /* instance structure size */
                                                      1,                                 /* preallocated instances */
                                                      NULL,                              /* instance initializers */
                                                      NULL};

        const GInterfaceInfo alterator_module_interface_info = {
            (GInterfaceInitFunc) manager_module_alterator_interface_init,         /* interface_init */
            (GInterfaceFinalizeFunc) manager_module_alterator_interface_finalize, /* interface_finalize */
            NULL                                                                  /* interface_data */
        };

        manager_module_type = g_type_register_static(G_TYPE_OBJECT, /* parent class */
                                                     "AlteratorCtlManagerModule",
                                                     &manager_module_info,
                                                     0);

        g_type_add_interface_static(manager_module_type, TYPE_ALTERATOR_CTL_MODULE, &alterator_module_interface_info);
    }

    return manager_module_type;
}

static void manager_module_class_init(AlteratorCtlManagerModuleClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->finalize = manager_ctl_class_finalize;

    manager_module_parent_class = g_type_class_peek_parent(klass);
}

static void manager_ctl_class_finalize(GObject *klass)
{
    AlteratorCtlManagerModuleClass *obj = (AlteratorCtlManagerModuleClass *) klass;

    G_OBJECT_CLASS(manager_module_parent_class)->finalize(klass);
}

static void manager_module_alterator_interface_init(gpointer iface, gpointer iface_data)
{
    AlteratorCtlModuleInterface *interface = iface;

    interface->run_with_args = manager_module_run_with_args;

    interface->run = manager_module_run;

    interface->print_help = manager_module_print_help;
}

static void manager_module_alterator_interface_finalize(gpointer iface, gpointer iface_data) {}

alterator_ctl_module_t *get_manager_module()
{
    int ret                              = 0;
    static gsize manager_ctl_module_init = 0;
    if (g_once_init_enter(&manager_ctl_module_init))
    {
        gsize module_id_size = g_strlcpy(manager_module.id,
                                         ALTERATOR_CTL_MANAGER_MODULE_NAME,
                                         strlen(ALTERATOR_CTL_MANAGER_MODULE_NAME) + 1);

        if (module_id_size != strlen(ALTERATOR_CTL_MANAGER_MODULE_NAME))
        {
            g_printerr(_("Internal error in get_manager_module: unvaliable id of manager module.\n"));
            ERR_EXIT();
        }

        manager_module.new_object_func  = (gpointer) manager_module_new;
        manager_module.free_object_func = (gpointer) manager_module_free;

        gsize tmp = 42;

        g_once_init_leave(&manager_ctl_module_init, tmp);
    }

    return &manager_module;

end:
    return NULL;
}

AlteratorCtlManagerModule *manager_module_new(gpointer app)
{
    AlteratorCtlManagerModule *object = g_object_new(TYPE_ALTERATOR_CTL_MANAGER_MODULE, NULL);

    object->commands = g_hash_table_new(g_str_hash, g_str_equal);
    fill_command_hash_table(object->commands);

    object->alterator_ctl_app = (AlteratorCtlApp *) app;

    object->gdbus_system_bus_source = alterator_gdbus_source_new(object->alterator_ctl_app->arguments->verbose,
                                                                 G_BUS_TYPE_SYSTEM);

    object->gdbus_session_bus_source = alterator_gdbus_source_new(object->alterator_ctl_app->arguments->verbose,
                                                                  G_BUS_TYPE_SESSION);

    return object;
}

void manager_module_free(AlteratorCtlManagerModule *module)
{
    g_hash_table_destroy(module->commands);

    if (module->gdbus_system_bus_source)
    {
        alterator_gdbus_source_free(module->gdbus_system_bus_source);
    }

    if (module->gdbus_session_bus_source)
    {
        alterator_gdbus_source_free(module->gdbus_session_bus_source);
    }

    g_object_unref(module);
}

static void fill_command_hash_table(GHashTable *command)
{
    for (int i = 0; i < sizeof(manager_module_subcommands_list) / sizeof(manager_module_subcommands_t); i++)
        g_hash_table_insert(command,
                            manager_module_subcommands_list[i].subcommand,
                            &manager_module_subcommands_list[i].id);
}

int manager_module_run(gpointer self, gpointer data)
{
    int ret = 0;

    AlteratorCtlModuleInterface *iface = GET_ALTERATOR_CTL_MODULE_INTERFACE(self);
    AlteratorCtlManagerModule *module  = ALTERATOR_CTL_MANAGER_MODULE(self);

    if (!self || !iface)
    {
        g_printerr(_("Internal error in manager module run: *module or *interface is NULL.\n"));
        ERR_EXIT();
    }

    if (module->alterator_ctl_app->arguments->module_help)
        goto end;

    if (!data)
    {
        g_printerr(_("Internal error in manager module run: *context is NULL.\n"));
        ERR_EXIT();
    }

    alteratorctl_ctx_t *ctx = (alteratorctl_ctx_t *) data;

    switch (g_variant_get_int32(ctx->subcommands_ids))
    {
    case MANAGER_GETOBJECTS:
        if (manager_module_run_subcommand_getobjects(module, &ctx))
            ERR_EXIT();
        break;

    case MANAGER_GETIFACES:
        if (manager_module_run_subcommand_getifaces(module, &ctx))
            ERR_EXIT();
        break;

    case MANAGER_GETSIGNALS:
        if (manager_module_run_subcommand_getsignals(module, &ctx))
            ERR_EXIT();
        break;

    default:
        g_printerr(_("Try to run unknown manager subcommand.\n"));
        ERR_EXIT();
    }

end:
    return ret;
}

int manager_module_run_subcommand_getobjects(AlteratorCtlManagerModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                        = 0;
    GHashTable *system_bus_result  = NULL;
    GHashTable *session_bus_result = NULL;
    gchar *interface               = NULL;
    gchar *bus                     = NULL;
    GPtrArray *result              = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in manager module - manager *module is NULL in \"manager "
                     "getobjects\".\n"));
        ERR_EXIT();
    }

    if ((*ctx)->parameters)
        g_variant_get((*ctx)->parameters, "(msmsmsms)", &interface, &bus, NULL, NULL);

    //if no bus type calling system bus and session bus
    if (!bus)
    {
        if (interface)
        {
            //calling with specified iface
            module->gdbus_session_bus_source
                ->alterator_gdbus_source_get_iface_objects((gpointer) module->gdbus_session_bus_source,
                                                           interface,
                                                           &session_bus_result);
            module->gdbus_system_bus_source
                ->alterator_gdbus_source_get_iface_objects((gpointer) module->gdbus_system_bus_source,
                                                           interface,
                                                           &system_bus_result);
        }
        else
        {
            //calling to get all objects
            module->gdbus_session_bus_source
                ->alterator_gdbus_source_get_all_objects((gpointer) module->gdbus_session_bus_source,
                                                         &session_bus_result);
            module->gdbus_system_bus_source->alterator_gdbus_source_get_all_objects((gpointer)
                                                                                        module->gdbus_system_bus_source,
                                                                                    &system_bus_result);
        }
    }
    else
    {
        //Call system bus
        if (strcmp(bus, ALTERATOR_MANAGER_SYSTEM_BUS_PARAMETER) == 0)
        {
            if (interface)
            {
                //calling with specified iface
                module->gdbus_system_bus_source
                    ->alterator_gdbus_source_get_iface_objects((gpointer) module->gdbus_system_bus_source,
                                                               interface,
                                                               &system_bus_result);
            }
            else
            {
                //calling to get all objects
                module->gdbus_system_bus_source
                    ->alterator_gdbus_source_get_all_objects((gpointer) module->gdbus_system_bus_source,
                                                             &system_bus_result);
            }
        }

        //Call session bus
        if (strcmp(bus, ALTERATOR_MANAGER_SESSION_BUS_PARAMETER) == 0)
        {
            if (interface)
            {
                //calling with specified iface
                module->gdbus_session_bus_source
                    ->alterator_gdbus_source_get_iface_objects((gpointer) module->gdbus_session_bus_source,
                                                               interface,
                                                               &session_bus_result);
            }
            else
            {
                //calling to get all objects
                module->gdbus_session_bus_source
                    ->alterator_gdbus_source_get_all_objects((gpointer) module->gdbus_session_bus_source,
                                                             &session_bus_result);
            }
        }
    }

    result = g_ptr_array_new_full(100, (GDestroyNotify) g_free);

    GPtrArray *system_bus_array = g_ptr_array_new_full(100, (GDestroyNotify) g_free);
    //Add result to array
    if (system_bus_result)
    {
        GList *system_results = g_hash_table_get_keys(system_bus_result);

        system_results = g_list_first(system_results);
        for (GList *elem = system_results; elem != NULL; elem = elem->next)
        {
            g_ptr_array_add(system_bus_array, g_strdup((gchar *) g_hash_table_lookup(system_bus_result, elem->data)));
        }

        g_list_free(system_results);

        if (system_bus_array->len > 0)
        {
            g_ptr_array_sort(system_bus_array, manager_module_sort_result);
            g_ptr_array_insert(system_bus_array, 0, g_strdup(_("<<< Objects on system bus:")));
        }
    }

    GPtrArray *session_bus_array = g_ptr_array_new_full(100, (GDestroyNotify) g_free);
    if (session_bus_result)
    {
        GList *session_results = g_hash_table_get_keys(session_bus_result);

        session_results = g_list_first(session_results);
        for (GList *elem = session_results; elem != NULL; elem = elem->next)
        {
            g_ptr_array_add(session_bus_array, g_strdup((gchar *) g_hash_table_lookup(session_bus_result, elem->data)));
        }

        g_list_free(session_results);

        if (session_bus_array->len > 0)
        {
            g_ptr_array_sort(session_bus_array, manager_module_sort_result);
            g_ptr_array_insert(session_bus_array, 0, g_strdup(_("<<< Objects on session bus:")));
        }
    }

    for (gsize i = 0; i < system_bus_array->len; i++)
        g_ptr_array_add(result, g_strdup(g_ptr_array_index(system_bus_array, i)));
    g_ptr_array_unref(system_bus_array);

    for (gsize i = 0; i < session_bus_array->len; i++)
        g_ptr_array_add(result, g_strdup(g_ptr_array_index(session_bus_array, i)));
    g_ptr_array_unref(session_bus_array);

    if (result->len == 0)
    {
        if (!bus)
            g_ptr_array_add(result, g_strdup(_("No objects found on system & session bus.")));
        else if (strcmp(bus, ALTERATOR_MANAGER_SYSTEM_BUS_PARAMETER) == 0)
            g_ptr_array_add(result, g_strdup(_("No objects found on system bus.")));
        else if (strcmp(bus, ALTERATOR_MANAGER_SESSION_BUS_PARAMETER) == 0)
            g_ptr_array_add(result, g_strdup(_("No objects found on session bus.")));
    }

    (*ctx)->results      = result;
    (*ctx)->free_results = (void (*)(gpointer results)) g_ptr_array_unref;

end:
    g_free((gpointer) interface);
    g_free((gpointer) bus);

    if (system_bus_result)
        g_hash_table_unref(system_bus_result);

    if (session_bus_result)
        g_hash_table_unref(session_bus_result);

    return ret;
}

int manager_module_run_subcommand_getifaces(AlteratorCtlManagerModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                        = 0;
    GHashTable *system_bus_result  = NULL;
    GHashTable *session_bus_result = NULL;
    gchar *object                  = NULL;
    gchar *bus                     = NULL;
    GPtrArray *result              = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in manager module - AlteratorCtlManagerModule *module is NULL in \"manager "
                     "getifaces\".\n"));
        ERR_EXIT();
    }

    if ((*ctx)->parameters)
        g_variant_get((*ctx)->parameters, "(msmsmsms)", &object, &bus, NULL, NULL);

    if (!bus)
    {
        if (object)
        {
            module->gdbus_session_bus_source
                ->alterator_gdbus_source_get_object_ifaces((gpointer) module->gdbus_session_bus_source,
                                                           object,
                                                           &session_bus_result);

            module->gdbus_system_bus_source
                ->alterator_gdbus_source_get_object_ifaces((gpointer) module->gdbus_system_bus_source,
                                                           object,
                                                           &system_bus_result);
        }
        else
        {
            module->gdbus_session_bus_source
                ->alterator_gdbus_source_get_all_ifaces((gpointer) module->gdbus_session_bus_source,
                                                        &session_bus_result);

            module->gdbus_system_bus_source
                ->alterator_gdbus_source_get_all_ifaces((gpointer) module->gdbus_system_bus_source, &system_bus_result);
        }
    }
    else
    {
        if (strcmp(bus, ALTERATOR_MANAGER_SYSTEM_BUS_PARAMETER) == 0)
        {
            if (object)
            {
                module->gdbus_system_bus_source
                    ->alterator_gdbus_source_get_object_ifaces((gpointer) module->gdbus_system_bus_source,
                                                               object,
                                                               &system_bus_result);
            }
            else
            {
                module->gdbus_system_bus_source
                    ->alterator_gdbus_source_get_all_ifaces((gpointer) module->gdbus_system_bus_source,
                                                            &system_bus_result);
            }
        }
        else
        {
            if (object)
            {
                module->gdbus_session_bus_source
                    ->alterator_gdbus_source_get_object_ifaces((gpointer) module->gdbus_session_bus_source,
                                                               object,
                                                               &session_bus_result);
            }
            else
            {
                module->gdbus_session_bus_source
                    ->alterator_gdbus_source_get_all_ifaces((gpointer) module->gdbus_session_bus_source,
                                                            &session_bus_result);
            }
        }
    }

    result = g_ptr_array_new_full(100, (GDestroyNotify) g_free);

    GPtrArray *system_bus_array = g_ptr_array_new_full(100, (GDestroyNotify) g_free);
    //Add result to array
    if (system_bus_result)
    {
        GList *system_results = g_hash_table_get_keys(system_bus_result);

        system_results = g_list_first(system_results);
        for (GList *elem = system_results; elem != NULL; elem = elem->next)
        {
            g_ptr_array_add(system_bus_array, g_strdup((gchar *) g_hash_table_lookup(system_bus_result, elem->data)));
        }

        g_list_free(system_results);

        if (system_bus_array->len > 0)
        {
            g_ptr_array_sort(system_bus_array, manager_module_sort_result);
            g_ptr_array_insert(system_bus_array, 0, g_strdup(_("<<< Interfaces on system bus:")));
        }
    }

    GPtrArray *session_bus_array = g_ptr_array_new_full(100, (GDestroyNotify) g_free);
    if (session_bus_result)
    {
        GList *session_results = g_hash_table_get_keys(session_bus_result);

        session_results = g_list_first(session_results);
        for (GList *elem = session_results; elem != NULL; elem = elem->next)
        {
            g_ptr_array_add(session_bus_array, g_strdup((gchar *) g_hash_table_lookup(session_bus_result, elem->data)));
        }

        g_list_free(session_results);

        if (session_bus_array->len > 0)
        {
            g_ptr_array_sort(session_bus_array, manager_module_sort_result);
            g_ptr_array_insert(session_bus_array, 0, g_strdup(_("<<< Interfaces on session bus:")));
        }
    }

    for (gsize i = 0; i < system_bus_array->len; i++)
        g_ptr_array_add(result, g_strdup(g_ptr_array_index(system_bus_array, i)));
    g_ptr_array_unref(system_bus_array);

    for (gsize i = 0; i < session_bus_array->len; i++)
        g_ptr_array_add(result, g_strdup(g_ptr_array_index(session_bus_array, i)));
    g_ptr_array_unref(session_bus_array);

    if (result->len == 0)
    {
        if (!bus)
            g_ptr_array_add(result, g_strdup(_("No interfaces found on system & session bus.")));
        else if (strcmp(bus, ALTERATOR_MANAGER_SYSTEM_BUS_PARAMETER) == 0)
            g_ptr_array_add(result, g_strdup(_("No interfaces found on system bus.")));
        else if (strcmp(bus, ALTERATOR_MANAGER_SESSION_BUS_PARAMETER) == 0)
            g_ptr_array_add(result, g_strdup(_("No interfaces found on session bus.")));
    }

    (*ctx)->results      = result;
    (*ctx)->free_results = (void (*)(gpointer results)) g_ptr_array_unref;

end:
    g_free((gpointer) object);
    g_free((gpointer) bus);

    if (system_bus_result)
        g_hash_table_unref(system_bus_result);

    if (session_bus_result)
        g_hash_table_unref(session_bus_result);

    return ret;
}

int manager_module_run_subcommand_getsignals(AlteratorCtlManagerModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                      = 0;
    gchar *object                = NULL;
    gchar *interface             = NULL;
    gchar *signal                = NULL;
    gchar *bus                   = NULL;
    AlteratorGDBusSource *source = NULL;
    GHashTable *result           = NULL;

    if (!module || !(*ctx))
    {
        g_printerr(
            _("Internal error in manager module - AlteratorCtlManagerModule *module or context is NULL in \"manager "
              "getsignals\".\n"));
        ERR_EXIT();
    }

    if ((*ctx)->parameters)
        g_variant_get((*ctx)->parameters, "(msmsmsms)", &object, &interface, &signal, &bus);

    if (!object || !interface || !signal || !bus)
    {
        g_printerr(_("Wrong arguments in manager module method GetSignals().\n"));
        AlteratorCtlModuleInterface *iface = GET_ALTERATOR_CTL_MODULE_INTERFACE((void *) module);
        iface->print_help(module);
        ERR_EXIT();
    }

    if (strcmp(bus, ALTERATOR_MANAGER_SYSTEM_BUS_PARAMETER) == 0)
        source = module->gdbus_system_bus_source;
    else
        source = module->gdbus_session_bus_source;

    //Check object
    int object_exist = 0;
    if (module->gdbus_system_bus_source->alterator_gdbus_source_check_object_by_path(source, object, &object_exist) < 0)
    {
        g_printerr(_("Error when checking the presence of an object %s on D-Bus.\n"), object);
        ERR_EXIT();
    }

    if (object_exist == 0)
    {
        g_printerr(_("Object %s isn't found on D-Bus.\n"), object);
        ERR_EXIT();
    }

    //check interface of the object
    int iface_exists = 0;
    if (module->gdbus_system_bus_source->alterator_gdbus_source_check_object_by_iface(source,
                                                                                      object,
                                                                                      interface,
                                                                                      &iface_exists)
        < 0)
    {
        g_printerr(_("Error when checking if an object: %s has an interface %s.\n"), object, interface);
        ERR_EXIT();
    }

    if (iface_exists == 0)
    {
        g_printerr(_("Object %s has no interface %s.\n"), object, interface);
        ERR_EXIT();
    }

    source->alterator_gdbus_source_get_signals((gpointer) source, object, interface, signal, &result);

    (*ctx)->results      = result;
    (*ctx)->free_results = (void (*)(gpointer results)) g_hash_table_unref;

end:
    g_free(object);
    g_free(interface);
    g_free(signal);
    g_free(bus);

    return ret;
}

static int manager_module_help_subcommand(AlteratorCtlManagerModule *module, alteratorctl_ctx_t **ctx)
{
    int ret = 0;

    AlteratorCtlModuleInterface *iface = GET_ALTERATOR_CTL_MODULE_INTERFACE(module);

    iface->print_help(module);

end:
    return ret;
}

int manager_module_run_with_args(gpointer self, int argc, char **argv)
{
    int ret                           = 0;
    alteratorctl_ctx_t *ctx           = NULL;
    AlteratorCtlManagerModule *module = ALTERATOR_CTL_MANAGER_MODULE(self);

    if (!module)
    {
        g_printerr(_("Internal data error in manager module with args: AlteratorCtlDiagModule *module is NULL.\n"));
        ERR_EXIT();
    }

    if (manager_module_parse_arguments(module, argc, argv, &ctx) < 0)
        ERR_EXIT();

    if (manager_module_run(module, ctx) < 0)
        ERR_EXIT();

    if (manager_module_handle_results(module, &ctx) < 0)
        ERR_EXIT();

end:
    if (ctx)
        alteratorctl_ctx_free(ctx);
    return ret;
}

static int manager_module_handle_results(AlteratorCtlManagerModule *module, alteratorctl_ctx_t **ctx)
{
    int ret = 0;
    if (module->alterator_ctl_app->arguments->module_help)
        goto end;

    if (!module || !(*ctx))
    {
        g_printerr(_("Internal error in manager module when handle results: *module or *context is NULL.\n"));
        ERR_EXIT();
    }

    switch (g_variant_get_int32((*ctx)->subcommands_ids))
    {
    case MANAGER_GETOBJECTS:
        if (manager_module_handle_getobjects_results(module, ctx) < 0)
            ERR_EXIT();

        break;

    case MANAGER_GETIFACES:
        if (manager_module_handle_getifaces_results(module, ctx) < 0)
            ERR_EXIT();

        break;

    case MANAGER_GETSIGNALS:
        if (manager_module_handle_getsignals_results(module, ctx) < 0)
            ERR_EXIT();

        break;

    default:
        break;
    }

end:
    return ret;
}

static int manager_module_handle_getobjects_results(AlteratorCtlManagerModule *module, alteratorctl_ctx_t **ctx)
{
    int ret            = 0;
    GPtrArray *objects = NULL;
    GString *output    = g_string_new("");

    if (!(*ctx)->results)
    {
        g_printerr(_("D-Bus error in manager getobjects handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    objects = (*ctx)->results;

    for (guint i = 0; i < objects->len; i++)
        g_string_append_printf(output, "%s\n", (gchar *) g_ptr_array_index(objects, i));

    print_with_pager(output->str);

end:
    if (output)
        g_string_free(output, TRUE);

    return ret;
}

static int manager_module_handle_getifaces_results(AlteratorCtlManagerModule *module, alteratorctl_ctx_t **ctx)
{
    int ret           = 0;
    GPtrArray *ifaces = NULL;
    GString *output   = g_string_new("");

    if (!(*ctx)->results)
    {
        g_printerr(_("D-Bus error in manager getifaces handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    ifaces = (*ctx)->results;

    for (guint i = 0; i < ifaces->len; i++)
        g_string_append_printf(output, "%s\n", (gchar *) g_ptr_array_index(ifaces, i));

    print_with_pager(output->str);

end:
    if (output)
        g_string_free(output, TRUE);

    return ret;
}

static int manager_module_handle_getsignals_results(AlteratorCtlManagerModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                   = 0;
    GHashTable *signals_table = NULL;
    GPtrArray *signals        = NULL;
    GString *output           = g_string_new("");

    if (!(*ctx)->results)
    {
        g_printerr(_("D-Bus error in manager getsignals handle result: failed to produce a result.\n"));
        ERR_EXIT();
    }

    signals_table = (*ctx)->results;

    if (g_hash_table_size(signals_table) == 0)
    {
        gchar *object = NULL, *interface = NULL, *signal = NULL, *bus = NULL;
        if ((*ctx)->parameters)
            g_variant_get((*ctx)->parameters, "(msmsmsms)", &object, &interface, &signal, &bus);

        if (bus && strcmp(bus, ALTERATOR_MANAGER_SYSTEM_BUS_PARAMETER) == 0)
            g_string_append(output, _("No signals found on system bus."));
        else if (bus && strcmp(bus, ALTERATOR_MANAGER_SESSION_BUS_PARAMETER) == 0)
            g_string_append(output, _("No signals found on session bus."));
        else
            g_string_append(output, _("No signals found."));

        print_with_pager(output->str);

        if (object)
            g_free(object);
        if (interface)
            g_free(interface);
        if (signal)
            g_free(signal);
        if (bus)
            g_free(bus);

        goto end;
    }

    signals = g_ptr_array_new_full(g_hash_table_size(signals_table), (GDestroyNotify) g_free);

    GHashTableIter iter;
    g_hash_table_iter_init(&iter, signals_table);
    gpointer key = NULL;
    while (g_hash_table_iter_next(&iter, &key, NULL))
        g_ptr_array_add(signals, g_strdup((gchar *) key));

    g_ptr_array_sort(signals, manager_module_sort_result);

    for (guint i = 0; i < signals->len; i++)
        g_string_append_printf(output, "%s\n", (gchar *) g_ptr_array_index(signals, i));

    print_with_pager(output->str);

end:
    if (output)
        g_string_free(output, TRUE);

    if (signals)
        g_ptr_array_unref(signals);

    return ret;
}

int manager_module_print_help(gpointer self)
{
    g_print(_("Usage:\n"));
    g_print(_("  alteratorctl manager <COMMAND [arguments]> [OPTIONS]\n\n"));
    g_print(_("Commands:\n"));
    g_print(_("  getifaces [path] [--session | --system]\n"
              "                              Gets interfaces of specified object on system\n"
              "                              or/and session bus\n"));
    g_print(_("  getobjects [interface] [--session | --system]\n"
              "                              Gets objects paths that implements specified\n"
              "                              interface on system or/and session bus\n"));
    g_print(_("  getsignals <path> <interface> <method name> <--session | --system>\n"
              "                              Gets signals names of specified method on\n"
              "                              implemented interface of specifiead object on\n"
              "                              specified bus\n\n"));
    g_print(_("Options:\n"));
    g_print(_("  --session                   D-Bus session bus type\n"));
    g_print(_("  --system                    D-Bus system bus type\n"));
    g_print(_("  -h, --help                  Show manager module usage\n\n"));

    return 0;
}

int manager_module_parse_arguments(AlteratorCtlManagerModule *module, int argc, char **argv, alteratorctl_ctx_t **ctx)
{
    int ret = 0;

    AlteratorCtlModuleInterface *iface = GET_ALTERATOR_CTL_MODULE_INTERFACE((void *) module);

    if (!iface)
    {
        g_printerr(_("Internal error in manager module while parsing arguments: *iface is NULL.\n"));
        ERR_EXIT();
    }

    if (module->alterator_ctl_app->arguments->module_help)
    {
        iface->print_help(module);
        goto end;
    }

    if ((argc < 3) || (argc > 7))
    {
        g_printerr(_("Wrong arguments in manager module.\n"));
        iface->print_help(module);
        ERR_EXIT();
    }

    int selected_subcommand = iface->get_command_id(module->commands, argv[2]);
    switch (selected_subcommand)
    {
    case MANAGER_GETOBJECTS:
        if (manager_module_initialize_getobject_ctx(ctx, selected_subcommand, argc, argv) < 0)
        {
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    case MANAGER_GETIFACES:
        if (manager_module_initialize_getiface_ctx(ctx, selected_subcommand, argc, argv) < 0)
        {
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    case MANAGER_GETSIGNALS:
        if (manager_module_initialize_getsignals_ctx(ctx, selected_subcommand, argc, argv) < 0)
        {
            {
                iface->print_help(module);
                ERR_EXIT();
            }
        }
        break;

    default:
        g_printerr(_("Unknown manager module command.\n"));
        iface->print_help(module);
        ERR_EXIT();
    }
end:
    return ret;
}

static gint manager_module_sort_result(gconstpointer a, gconstpointer b)
{
    return g_utf8_collate((const gchar *) ((GPtrArray *) a)->pdata, (const gchar *) ((GPtrArray *) b)->pdata);
}

static int manager_module_initialize_getobject_ctx(alteratorctl_ctx_t **ctx, int subcommand, int argc, char **argv)
{
    int ret    = 0;
    char *bus  = NULL;
    char *path = NULL;

    if (argc > 5)
    {
        g_printerr(_("To many arguments in getobjects manager module subcommand\n"));
        ERR_EXIT();
    }

    if (argc == 3) //Only getobjects command
        (*ctx) = alteratorctl_ctx_init_manager(subcommand, NULL, NULL, NULL, NULL, NULL, NULL);

    //Get bus and interface
    for (int i = 3; i < argc; i++)
    {
        if (g_str_has_prefix(argv[i], "--"))
            if (bus)
            {
                g_printerr(_("The bus type is specified repeatedly.\n"));
                ERR_EXIT();
            }
            else
                bus = argv[i]; // this is the bus
        else
            path = argv[i];
    }

    //validate the bus
    if (bus)
    {
        if (strcmp(bus, ALTERATOR_MANAGER_SYSTEM_BUS_PARAMETER) != 0
            && strcmp(bus, ALTERATOR_MANAGER_SESSION_BUS_PARAMETER) != 0)
        {
            g_printerr(_("Wrong bus type in getobjects manager module subcommand.\n"));
            ERR_EXIT();
        }
    }

    (*ctx) = !(*ctx) ? alteratorctl_ctx_init_manager(subcommand, path, bus, NULL, NULL, NULL, NULL) : (*ctx);
    if (!(*ctx))
    {
        g_printerr(_("Can't initialize context in manager module getobjects subcommand.\n"));
        ERR_EXIT();
    }

end:
    return ret;
}

static int manager_module_initialize_getiface_ctx(alteratorctl_ctx_t **ctx, int subcommand, int argc, char **argv)
{
    int ret     = 0;
    char *bus   = NULL;
    char *iface = NULL;

    if (argc > 5)
    {
        g_printerr(_("To many arguments in getifaces manager module subcommand.\n"));
        ERR_EXIT();
    }

    if (argc == 3) //Only getobjects command
        (*ctx) = alteratorctl_ctx_init_manager(subcommand, NULL, NULL, NULL, NULL, NULL, NULL);

    //Get bus and interface
    for (int i = 3; i < argc; i++)
    {
        if (g_str_has_prefix(argv[i], "--"))
            bus = argv[i]; // this is the bus
        else
            iface = argv[i];
    }

    //validate the bus
    if (bus)
    {
        if (strcmp(bus, ALTERATOR_MANAGER_SYSTEM_BUS_PARAMETER) != 0
            && strcmp(bus, ALTERATOR_MANAGER_SESSION_BUS_PARAMETER) != 0)
        {
            g_printerr(_("Wrong bus type in getifaces manager module subcommand.\n"));
            ERR_EXIT();
        }
    }

    (*ctx) = alteratorctl_ctx_init_manager(subcommand, iface, bus, NULL, NULL, NULL, NULL);
    if (!(*ctx))
    {
        g_printerr(_("Can't initialize context in manager module getifaces subcommand.\n"));
        ERR_EXIT();
    }

end:
    return ret;
}

static int manager_module_initialize_getsignals_ctx(alteratorctl_ctx_t **ctx, int subcommand, int argc, char **argv)
{
    int ret   = 0;
    char *bus = NULL;

    if (argc != 7)
    {
        g_printerr(_("Wrong argument to manager module getsignals subcommand.\n"));
        ERR_EXIT();
    }

    //Get bus
    for (int i = 3; i < argc; i++)
    {
        if (g_str_has_prefix(argv[i], "--"))
            bus = argv[i]; // this is the bus
    }

    //validate the bus
    if (bus)
    {
        if (strcmp(bus, ALTERATOR_MANAGER_SYSTEM_BUS_PARAMETER) != 0
            && strcmp(bus, ALTERATOR_MANAGER_SESSION_BUS_PARAMETER) != 0)
        {
            g_printerr(_("Wrong bus type in getifaces manager module subcommand.\n"));
            ERR_EXIT();
        }
    }
    else
    {
        g_printerr(_("Can't find bus in arguments.\n"));
        ERR_EXIT();
    }

    (*ctx) = alteratorctl_ctx_init_manager(subcommand, argv[3], argv[4], argv[5], bus, NULL, NULL);
    if (!(*ctx))
    {
        g_printerr(_("Can't initialize context in manager module getsignals subcommand.\n"));
        ERR_EXIT();
    }

end:
    return ret;
}
