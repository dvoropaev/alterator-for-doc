#include "alteratorctlgdbussource.h"

#define ALTERATOR_MANAGER_INTERFACE "org.altlinux.alterator.manager"
#define ALTERATOR_MANAGER_GETOBJECT_METHOD_NAME "GetObjects"
#define ALTERATOR_MANAGER_GETALLOBJECT_METHOD_NAME "GetAllObjects"
#define ALTERATOR_MANAGER_GETINTERFACES_METHOD_NAME "GetInterfaces"
#define ALTERATOR_MANAGER_GETALLINTERFACES_METHOD_NAME "GetAllInterfaces"
#define ALTERATOR_MANAGER_GETSIGNALS_METHOD_NAME "GetSignals"
#define ALTERATOR_MANAGER_SETENVVALUE_METHOD_NAME "SetEnvValue"
#define ALTERATOR_MANAGER_GETENVVALUE_METHOD_NAME "GetEnvValue"

#define FREEDESKTOP_DBUS_SERVICE "org.freedesktop.DBus"
#define FREEDESKTOP_DBUS_OBJECT "/org/freedesktop/DBus"
#define FREEDESKTOP_DBUS_INTERFACE "org.freedesktop.DBus"
#define FREEDESKTOP_DBUS_GET_SERVICES_METHOD_NAME "ListNames"

#define FREEDESKTOP_DBUS_INTROSPECTABLE_INTERFACE "org.freedesktop.DBus.Introspectable"
#define FREEDESKTOP_DBUS_GET_INTROSPECT_METHOD_NAME "Introspect"

#define ALTERATOR_ENTRY_OBJECT_KEY_NAME "name"

static GMainLoop *alterator_dbus_source_main_loop         = NULL;
static gboolean quit_alterator_dbus_source_main_loop_flag = FALSE;
static GMainContext *alterator_dbus_source_context        = NULL;

static GObjectClass *alterator_gdbus_source_parent_class = NULL;
static void alterator_gdbus_source_class_init(AlteratorGDBusSourceClass *klass);
static void alterator_gdbus_source_class_finalize(GObject *klass);

static void alterator_gdbus_source_start_main_loop(gboolean turn_off_output);
static void alterator_gdbus_source_on_manager_bus_get_ready(GObject *source_object,
                                                            GAsyncResult *res,
                                                            gpointer user_data);
static void alterator_gdbus_source_callback(GObject *source_object, GAsyncResult *res, gpointer user_data);

static void alterator_gdbus_source_on_call_proxy_ready(GObject *source_object, GAsyncResult *res, gpointer user_data);
static void alterator_gdbus_source_on_call_get_ready(GObject *source_object, GAsyncResult *res, gpointer user_data);
static int alterator_gdbus_source_subscribe_signal(gpointer self,
                                                   subscribe_signals_t *signal,
                                                   const gchar *interface_name,
                                                   const gchar *path);
static void alterator_gdbus_source_unsubscribe_signal(gpointer self, int subscriber);

static int alterator_gdbus_source_get_all_objects(gpointer self, GHashTable **objects);

static int alterator_gdbus_source_get_all_ifaces(gpointer self, GHashTable **ifaces);

static int alterator_gdbus_source_get_object_ifaces(gpointer self, const gchar *path, GHashTable **ifaces);

static int alterator_gdbus_source_get_iface_objects(gpointer self, const gchar *iface, GHashTable **objects);

static int alterator_gdbus_source_get_signals(
    gpointer self, const gchar *object_str_id, const gchar *interface, const gchar *method, GHashTable **signals);

static int alterator_gdbus_source_set_env_value(gpointer self, const gchar *name, gchar *value);

static int alterator_gdbus_source_get_env_value(gpointer self, const gchar *name, gchar **result);

static int alterator_gdbus_source_check_object_by_path(gpointer self, const gchar *object_str_id, int *result);

static int alterator_gdbus_source_check_object_by_iface(gpointer self,
                                                        const gchar *object_str_id,
                                                        const gchar *iface,
                                                        int *result);

static int alterator_gdbus_source_get_text_of_alterator_entry_by_path(gpointer self,
                                                                      const gchar *object_str_id,
                                                                      const gchar *iface,
                                                                      gchar **alterator_entry);

static int alterator_gdbus_source_getobjets_priv(gpointer *self, const gchar *iface, GVariant **result);

static int alterator_gdbus_source_getifaces_priv(gpointer *self, const gchar *path, GVariant **result);

static int alterator_gdbus_source_getsignals_priv(
    gpointer *self, const gchar *path, const gchar *interface, const gchar *method, GVariant **result);

static int alterator_gdbus_source_get_services_names(gpointer self, GHashTable **result);

static int alterator_gdbus_source_get_introspection(gpointer self,
                                                    const gchar *service,
                                                    const gchar *path,
                                                    gchar **introspection);

static const gchar *alterator_gdbus_source_get_name_by_path(gpointer self, const gchar *path, const gchar *iface);

static const gchar *alterator_gdbus_source_get_path_by_name(gpointer self, const gchar *name, const gchar *iface);

static gboolean is_object_path(const gchar *str_id);

gchar *concat_signal_name_with_connection_name(const gchar *signal, const gchar *connection);

GType alterator_gdbus_source_get_type()
{
    static GType alterator_gdbus_source_type = 0;

    if (!alterator_gdbus_source_type)
    {
        static const GTypeInfo alterator_gdbus_source_info
            = {sizeof(AlteratorGDBusSourceClass),                  /* class structure size */
               NULL,                                               /* base class initializer */
               NULL,                                               /* base class finalizer */
               (GClassInitFunc) alterator_gdbus_source_class_init, /* class initializer */
               NULL,                                               /* class finalizer */
               NULL,                                               /* class data */
               sizeof(AlteratorGDBusSource),                       /* instance structure size */
               1,                                                  /* preallocated instances */
               NULL,                                               /* instance initializers */
               NULL};

        alterator_gdbus_source_type = g_type_register_static(G_TYPE_OBJECT, /* parent class */
                                                             "GDBusSource",
                                                             &alterator_gdbus_source_info,
                                                             0);
    }

    return alterator_gdbus_source_type;
}

static void alterator_gdbus_source_class_finalize(GObject *klass)
{
    AlteratorGDBusSourceClass *obj = (AlteratorGDBusSourceClass *) klass;

    G_OBJECT_CLASS(alterator_gdbus_source_parent_class)->finalize(klass);
}

static void alterator_gdbus_source_class_init(AlteratorGDBusSourceClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->finalize = alterator_gdbus_source_class_finalize;

    alterator_gdbus_source_parent_class = g_type_class_peek_parent(klass);

    return;
}

subscribe_signals_t *subscribe_signals_init(gchar *signal_name, GDBusSignalCallback callback, gpointer user_data)
{
    if (!signal_name || !callback)
        return NULL;

    subscribe_signals_t *signal = g_malloc0(sizeof(subscribe_signals_t));
    if (!signal)
        return NULL;

    signal->signal_name = signal_name;
    signal->callback    = callback;
    signal->user_data   = user_data;

    return signal;
}

void subscribe_signals_free(subscribe_signals_t *signal)
{
    if (!signal)
        return;

    g_free(signal);
}

AlteratorGDBusSource *alterator_gdbus_source_new(gboolean is_verbose, GBusType type)
{
    AlteratorCtlModuleInfoParser *info_parser = alterator_module_info_parser_new();
    if (!info_parser)
    {
        g_printerr(_("Can't create alterator entry info parser in AlteratorGDbusource.\n"));
        return NULL;
    }

    AlteratorGDBusSource *object = g_object_new(TYPE_ALTERATOR_GDBUS_SOURCE, NULL);

    object->info_parser = info_parser;

    //GDBusSourceClass *klass = GDBUS_SOURCE_GET_CLASS(object);

    object->call              = &call;
    object->call_with_signals = &call_with_signals;

    //Initialize functions pointers
    object->alterator_gdbus_source_get_all_objects = &alterator_gdbus_source_get_all_objects;

    object->alterator_gdbus_source_get_all_ifaces = &alterator_gdbus_source_get_all_ifaces;

    object->alterator_gdbus_source_get_object_ifaces = &alterator_gdbus_source_get_object_ifaces;

    object->alterator_gdbus_source_get_iface_objects = &alterator_gdbus_source_get_iface_objects;

    object->alterator_gdbus_source_get_signals = &alterator_gdbus_source_get_signals;

    object->alterator_gdbus_source_check_object_by_path = &alterator_gdbus_source_check_object_by_path;

    object->alterator_gdbus_source_check_object_by_iface = &alterator_gdbus_source_check_object_by_iface;

    object->alterator_gdbus_source_set_env_value = &alterator_gdbus_source_set_env_value;

    object->alterator_gdbus_source_get_env_value = &alterator_gdbus_source_get_env_value;

    object->alterator_gdbus_source_get_text_of_alterator_entry_by_path
        = &alterator_gdbus_source_get_text_of_alterator_entry_by_path;

    object->alterator_gdbus_source_get_services_names = &alterator_gdbus_source_get_services_names;

    object->alterator_gdbus_source_get_introspection = &alterator_gdbus_source_get_introspection;

    object->alterator_gdbus_source_get_name_by_path = &alterator_gdbus_source_get_name_by_path;

    object->alterator_gdbus_source_get_path_by_name = &alterator_gdbus_source_get_path_by_name;

    object->verbose = is_verbose;

    g_bus_get(type, NULL, alterator_gdbus_source_on_manager_bus_get_ready, object);

    alterator_gdbus_source_start_main_loop(FALSE);

    return object;
}

void alterator_gdbus_source_free(AlteratorGDBusSource *gdbus_source)
{
    alterator_module_info_parser_free(gdbus_source->info_parser);
    g_object_unref(gdbus_source);
}

gboolean alterator_ctl_check_object_is_exist(const GError *error)
{
    return !g_error_matches(error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD)
           && !g_error_matches(error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD)
           && !g_error_matches(error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD);
}

void alterator_gdbus_source_on_manager_bus_get_ready(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GError *error = NULL;

    AlteratorGDBusSource *source = (AlteratorGDBusSource *) user_data;

    source->dbus_connection = g_bus_get_finish(res, &error);

    if (error)
    {
        g_printerr(_("DBus error: %s\n"), error->message);
        g_error_free(error);
        quit_alterator_dbus_source_main_loop_flag = TRUE;
    }

    quit_alterator_dbus_source_main_loop_flag = TRUE;

    return;
}

void alterator_gdbus_source_start_main_loop(gboolean turn_off_output)
{
    if (turn_off_output)
        disable_output();

    quit_alterator_dbus_source_main_loop_flag = FALSE;

    alterator_dbus_source_context = g_main_context_default();

    alterator_dbus_source_main_loop = g_main_loop_new(alterator_dbus_source_context, FALSE);

    while (!quit_alterator_dbus_source_main_loop_flag)
        g_main_context_iteration(alterator_dbus_source_context, FALSE);

    g_main_loop_quit(alterator_dbus_source_main_loop);

    g_main_loop_unref(alterator_dbus_source_main_loop);
    alterator_dbus_source_main_loop = NULL;

    if (turn_off_output)
        enable_output();
}

void alterator_gdbus_source_callback(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GVariant *reply = NULL;
    gpointer *data  = (gpointer *) user_data;

    dbus_ctx_t *ctx = (dbus_ctx_t *) data[0];
    GError **error  = (GError **) data[1];

    reply = g_dbus_connection_call_finish(G_DBUS_CONNECTION(source_object), res, error);

    if ((error && *error) || reply == NULL)
    {
        quit_alterator_dbus_source_main_loop_flag = TRUE;
        return;
    }

    ctx->result = reply;

    quit_alterator_dbus_source_main_loop_flag = TRUE;
}

void call(gpointer self, dbus_ctx_t *dbus_ctx, GError **dbus_call_error)
{
    if (!self)
        return;

    AlteratorGDBusSource *source = (AlteratorGDBusSource *) self;

    if (!g_variant_is_object_path(dbus_ctx->path))
    {
        if (!is_object_path(dbus_ctx->path))
        {
            gchar *path = NULL;
            if (!(path = g_strdup(
                      source->alterator_gdbus_source_get_path_by_name(source, dbus_ctx->path, dbus_ctx->interface))))
                return;

            g_free(dbus_ctx->path);
            dbus_ctx->path = path;
        }
        else
            return;
    }

    gpointer user_data[2] = {dbus_ctx, dbus_call_error};
    g_dbus_connection_call(source->dbus_connection,
                           dbus_ctx->service_name,
                           dbus_ctx->path,
                           dbus_ctx->interface,
                           dbus_ctx->method,
                           dbus_ctx->parameters,
                           dbus_ctx->reply_type,
                           G_DBUS_CALL_FLAGS_NONE,
                           dbus_ctx->timeout_msec,
                           NULL,
                           alterator_gdbus_source_callback,
                           user_data);

    alterator_gdbus_source_start_main_loop(FALSE);

    return;
}

void call_with_signals(gpointer self, dbus_ctx_t *dbus_ctx, GPtrArray *signals, GError **dbus_call_error)
{
    if (!self)
        return;

    AlteratorGDBusSource *source = (AlteratorGDBusSource *) self;

    if (!signals)
        return;

    if (!g_variant_is_object_path(dbus_ctx->path))
    {
        if (!is_object_path(dbus_ctx->path))
        {
            gchar *path = NULL;
            if (!(path = g_strdup(
                      source->alterator_gdbus_source_get_path_by_name(source, dbus_ctx->path, dbus_ctx->interface))))
                return;

            g_free(dbus_ctx->path);
            dbus_ctx->path = path;
        }
        else
            return;
    }

    GArray *subscribers = g_array_new(FALSE, FALSE, sizeof(gint));

    //subscribe signals
    for (int i = 0; i < signals->len; i++)
    {
        gint subscriber = alterator_gdbus_source_subscribe_signal(self,
                                                                  g_ptr_array_index(signals, i),
                                                                  dbus_ctx->interface,
                                                                  dbus_ctx->path);
        if (subscriber)
            g_array_append_val(subscribers, subscriber);
    }

    gpointer user_data[2] = {dbus_ctx, dbus_call_error};
    g_dbus_proxy_new(source->dbus_connection,
                     G_DBUS_PROXY_FLAGS_NONE,
                     NULL,
                     dbus_ctx->service_name,
                     dbus_ctx->path,
                     dbus_ctx->interface,
                     NULL,
                     alterator_gdbus_source_on_call_proxy_ready,
                     user_data);

    alterator_gdbus_source_start_main_loop(dbus_ctx->disable_output);

    //unsubscribe signals
    for (int i = 0; i < subscribers->len; i++)
    {
        alterator_gdbus_source_unsubscribe_signal(self, g_array_index(subscribers, gint, i));
    }

end:
    g_array_unref(subscribers);

    return;
}

int alterator_gdbus_source_subscribe_signal(gpointer self,
                                            subscribe_signals_t *signal,
                                            const gchar *interface_name,
                                            const gchar *path)
{
    if (!self)
        return -1;

    AlteratorGDBusSource *source = (AlteratorGDBusSource *) self;

    gchar *signal_name_conn = concat_signal_name_with_connection_name(signal->signal_name,
                                                                      g_dbus_connection_get_unique_name(
                                                                          source->dbus_connection));

    gint subscriber = g_dbus_connection_signal_subscribe(source->dbus_connection,
                                                         NULL,
                                                         interface_name,
                                                         signal_name_conn,
                                                         path,
                                                         NULL,
                                                         G_DBUS_SIGNAL_FLAGS_NO_MATCH_RULE,
                                                         signal->callback,
                                                         signal->user_data,
                                                         NULL);
    g_free(signal_name_conn);
    return subscriber;
}

void alterator_gdbus_source_unsubscribe_signal(gpointer self, int subscriber)
{
    AlteratorGDBusSource *source = (AlteratorGDBusSource *) self;

    g_dbus_connection_signal_unsubscribe(source->dbus_connection, subscriber);
}

void alterator_gdbus_source_on_call_proxy_ready(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    int ret           = 0;
    gpointer *data    = (gpointer *) user_data;
    dbus_ctx_t *ctx   = (dbus_ctx_t *) data[0];
    GError **error    = (GError **) data[1];
    GDBusProxy *proxy = g_dbus_proxy_new_finish(res, error);

    if (error && *error)
    {
        quit_alterator_dbus_source_main_loop_flag = TRUE;
        return;
    }

    g_dbus_proxy_call(proxy,
                      ctx->method,
                      ctx->parameters,
                      G_DBUS_CALL_FLAGS_NONE,
                      ctx->timeout_msec,
                      NULL,
                      alterator_gdbus_source_on_call_get_ready,
                      user_data);

end:

    return;
}

void alterator_gdbus_source_on_call_get_ready(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    int ret = 0;

    gpointer *data   = (gpointer *) user_data;
    dbus_ctx_t *ctx  = (dbus_ctx_t *) data[0];
    GError **error   = (GError **) data[1];
    GVariant *result = g_dbus_proxy_call_finish(G_DBUS_PROXY(source_object), res, error);
    if (error && *error)
    {
        quit_alterator_dbus_source_main_loop_flag = TRUE;
        return;
    }
    ctx->result                               = result;
    quit_alterator_dbus_source_main_loop_flag = TRUE;

end:
    return;
}

static int alterator_gdbus_source_get_all_objects(gpointer self, GHashTable **objects)
{
    int ret          = 0;
    GVariant *result = NULL;

    if (alterator_gdbus_source_getobjets_priv(self, NULL, &result) < 0)
        ERR_EXIT();

    (*objects) = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    if (!result)
        goto end;

    GVariantIter *iter = NULL;
    gchar *str         = NULL;

    g_variant_get(result, "as", &iter);
    while (g_variant_iter_loop(iter, "s", &str))
    {
        g_hash_table_add((*objects), g_strdup(str));
    }

    g_variant_iter_free(iter);

end:
    if (result)
        g_variant_unref(result);

    return ret;
}

static int alterator_gdbus_source_get_all_ifaces(gpointer self, GHashTable **ifaces)
{
    int ret          = 0;
    GVariant *result = NULL;

    if (alterator_gdbus_source_getifaces_priv(self, NULL, &result))
        ERR_EXIT();

    (*ifaces) = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    if (!result)
        goto end;

    GVariantIter *iter = NULL;
    gchar *str         = NULL;

    g_variant_get(result, "as", &iter);
    while (g_variant_iter_loop(iter, "s", &str))
    {
        g_hash_table_add((*ifaces), g_strdup(str));
    }

    g_variant_iter_free(iter);

end:
    if (result)
        g_variant_unref(result);

    return ret;
}

static int alterator_gdbus_source_get_object_ifaces(gpointer self, const gchar *path, GHashTable **ifaces)
{
    int ret          = 0;
    GVariant *result = NULL;

    if (!path)
    {
        g_printerr(_("Interface is null in alterator_gdbus_source_get_object_ifaces.\n"));
        ERR_EXIT();
    }

    if (alterator_gdbus_source_getifaces_priv(self, path, &result))
        ERR_EXIT();

    (*ifaces) = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    if (!result)
        goto end;

    GVariantIter *iter = NULL;
    gchar *str         = NULL;

    g_variant_get(result, "as", &iter);
    while (g_variant_iter_loop(iter, "s", &str))
    {
        g_hash_table_add((*ifaces), g_strdup(str));
    }

    g_variant_iter_free(iter);

end:
    if (result)
        g_variant_unref(result);

    return ret;
}

static int alterator_gdbus_source_get_iface_objects(gpointer self, const gchar *iface, GHashTable **objects)
{
    int ret          = 0;
    GVariant *result = NULL;

    if (!iface)
    {
        g_printerr(_("Interface is null in alterator_gdbus_source_get_iface_objects.\n"));
        ERR_EXIT();
    }

    if (alterator_gdbus_source_getobjets_priv(self, iface, &result) < 0)
        ERR_EXIT();

    (*objects) = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    if (!result)
        goto end;

    GVariantIter *iter = NULL;
    gchar *str         = NULL;

    g_variant_get(result, "as", &iter);
    while (g_variant_iter_loop(iter, "s", &str))
    {
        g_hash_table_add((*objects), g_strdup(str));
    }

    g_variant_iter_free(iter);

end:
    if (result)
        g_variant_unref(result);

    return ret;
}

static int alterator_gdbus_source_get_signals(
    gpointer self, const gchar *object_str_id, const gchar *interface, const gchar *method, GHashTable **signals)
{
    int ret          = 0;
    gchar *path      = NULL;
    GVariant *result = NULL;

    AlteratorGDBusSource *source = (AlteratorGDBusSource *) self;

    if (!object_str_id)
    {
        g_printerr(_("Wrong path or name of object.\n"));
        ERR_EXIT();
    }

    if (!interface)
    {
        g_printerr(_("Wrong interface of object.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_object_path(object_str_id))
        if (!is_object_path(object_str_id))
        {
            if (!(path = g_strdup(source->alterator_gdbus_source_get_path_by_name(source, object_str_id, interface))))
                ERR_EXIT();
        }
        else
            ERR_EXIT();
    else
        path = g_strdup((gchar *) object_str_id);

    if (!method)
    {
        g_printerr(_("Wrong object method.\n"));
        ERR_EXIT();
    }

    if (alterator_gdbus_source_getsignals_priv(self, path, interface, method, &result) < 0)
        ERR_EXIT();

    (*signals) = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    if (!result)
        goto end;

    GVariantIter *iter = NULL;
    gchar *str         = NULL;

    g_variant_get(result, "as", &iter);
    while (g_variant_iter_loop(iter, "s", &str))
    {
        g_hash_table_add((*signals), g_strdup(str));
    }

    g_variant_iter_free(iter);

end:
    g_free(path);

    if (result)
        g_variant_unref(result);

    return ret;
}

static int alterator_gdbus_source_check_object_by_path(gpointer self, const gchar *object_str_id, int *result)
{
    int ret             = 0;
    GHashTable *objects = NULL;
    (*result)           = 0;

    if (!self)
        ERR_EXIT();

    if (!object_str_id)
        ERR_EXIT();

    if (!g_variant_is_object_path(object_str_id))
    {
        if (!is_object_path(object_str_id))
        {
            (*result) = 2;
            return ret;
        }
        else
            ERR_EXIT();
    }

    if (alterator_gdbus_source_get_all_objects(self, &objects) < 0)
        ERR_EXIT();

    if (!objects)
        ERR_EXIT();

    if (g_hash_table_contains(objects, object_str_id))
        (*result) = 1;

end:
    if (objects)
        g_hash_table_unref(objects);

    return ret;
}

static int alterator_gdbus_source_check_object_by_iface(gpointer self,
                                                        const gchar *object_str_id,
                                                        const gchar *iface,
                                                        int *result)
{
    int ret            = 0;
    GHashTable *ifaces = NULL;
    (*result)          = 0;

    if (!self)
        ERR_EXIT();

    AlteratorGDBusSource *source = (AlteratorGDBusSource *) self;

    if (!object_str_id)
        ERR_EXIT();

    if (!iface)
        ERR_EXIT();

    // If not a path, we check for existence by name
    if (!g_variant_is_object_path(object_str_id))
    {
        if (!is_object_path(object_str_id))
        {
            gchar *path = NULL;
            if (!(path = (gchar *) source->alterator_gdbus_source_get_path_by_name(source, object_str_id, iface)))
                ERR_EXIT();

            (*result) = 1;
            return ret;
        }
        else
            ERR_EXIT();
    }

    int objects_exist = 0;
    if (alterator_gdbus_source_check_object_by_path(self, object_str_id, &objects_exist) < 0)
        ERR_EXIT();

    if (objects_exist == 0)
        goto end;

    if (alterator_gdbus_source_get_object_ifaces(self, object_str_id, &ifaces) < 0)
        ERR_EXIT();

    if (g_hash_table_contains(ifaces, iface))
        (*result) = 1;

end:
    if (ifaces)
        g_hash_table_unref(ifaces);

    return ret;
}

static int alterator_gdbus_source_set_env_value(gpointer self, const gchar *name, gchar *value)
{
    int ret                      = 0;
    dbus_ctx_t *dbus_ctx         = NULL;
    AlteratorGDBusSource *source = NULL;

    if (!self)
    {
        g_printerr(_("Internal error in alterator_gdbus_source_set_env_value - AlteratorGDBusSource *source is "
                     "NULL.\n"));
        ERR_EXIT();
    }

    source = (AlteratorGDBusSource *) self;

    if (!name)
    {
        g_printerr(_("Internal error in alterator_gdbus_source_set_env_value - environment variable name is "
                     "NULL.\n"));
        ERR_EXIT();
    }

    if (!value)
    {
        g_printerr(_("Internal error in alterator_gdbus_source_set_env_value - environment variable value is "
                     "NULL.\n"));
        ERR_EXIT();
    }

    dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                             ALTERATOR_MANAGER_PATH,
                             ALTERATOR_MANAGER_INTERFACE,
                             ALTERATOR_MANAGER_SETENVVALUE_METHOD_NAME,
                             source->verbose);

    if (!dbus_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in alterator_gdbus_source_set_env_value\n"));
        ERR_EXIT();
    }

    dbus_ctx->reply_type = NULL;
    dbus_ctx->parameters = g_variant_new("(ss)", name, value);

    source->call(source, dbus_ctx, NULL);

    // Method returns nothing

end:
    if (dbus_ctx)
        dbus_ctx_free(dbus_ctx);

    return 0;
}

static int alterator_gdbus_source_get_env_value(gpointer self, const gchar *name, gchar **result)
{
    int ret                      = 0;
    dbus_ctx_t *dbus_ctx         = NULL;
    AlteratorGDBusSource *source = NULL;
    GVariant *variant_result     = NULL;

    if (!self)
    {
        g_printerr(_("Internal error in alterator_gdbus_source_get_env_value - AlteratorGDBusSource *source is "
                     "NULL.\n"));
        ERR_EXIT();
    }

    if (!result)
    {
        g_printerr(_("Internal error in alterator_gdbus_source_get_env_value - AlteratorGDBusSource **result is "
                     "NULL.\n"));
    }

    source = (AlteratorGDBusSource *) self;

    if (!name)
    {
        g_printerr(_("Internal error in alterator_gdbus_source_get_env_value - environment variable name is "
                     "NULL.\n"));
        ERR_EXIT();
    }

    dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                             ALTERATOR_MANAGER_PATH,
                             ALTERATOR_MANAGER_INTERFACE,
                             ALTERATOR_MANAGER_GETENVVALUE_METHOD_NAME,
                             source->verbose);

    if (!dbus_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in alterator_gdbus_source_get_env_value\n"));
        ERR_EXIT();
    }

    dbus_ctx->reply_type = NULL;
    dbus_ctx->parameters = g_variant_new("(s)", name);

    source->call(source, dbus_ctx, NULL);

    if (!dbus_ctx->result)
        ERR_EXIT();

    variant_result = g_variant_get_child_value(dbus_ctx->result, 0);

    (*result) = g_strdup(g_variant_get_string(variant_result, NULL));

end:
    if (dbus_ctx)
        dbus_ctx_free(dbus_ctx);

    if (variant_result)
        g_variant_unref(variant_result);

    return 0;
}

static int alterator_gdbus_source_get_text_of_alterator_entry_by_path(gpointer self,
                                                                      const gchar *object_str_id,
                                                                      const gchar *iface,
                                                                      gchar **alterator_entry)
{
    int ret                      = 0;
    AlteratorGDBusSource *source = NULL;
    gchar *path                  = NULL;
    int object_exists            = -1;
    int iface_exists             = -1;
    dbus_ctx_t *ctx              = NULL;
    GVariant *info_array         = NULL;

    source = (AlteratorGDBusSource *) self;

    if (!source || !object_str_id || !iface || !alterator_entry)
        ERR_EXIT();

    if (!is_object_path(object_str_id))
        path = g_strdup(source->alterator_gdbus_source_get_path_by_name(self, object_str_id, iface));
    else
        path = g_strdup(object_str_id);

    if (!path)
        ERR_EXIT();

    if (alterator_gdbus_source_check_object_by_path(source, path, &object_exists) < 0)
        ERR_EXIT();

    if (!object_exists)
        ERR_EXIT();

    if (alterator_gdbus_source_check_object_by_iface(source, path, iface, &iface_exists) < 0)
        ERR_EXIT();

    ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME, path, iface, ALTERATOR_ENTRY_GET_METHOD, source->verbose);

    if (!ctx)
        ERR_EXIT();

    ctx->reply_type = G_VARIANT_TYPE("(ayi)");

    source->call(self, ctx, NULL);

    if (!ctx->result)
        ERR_EXIT();

    info_array = g_variant_get_child_value(ctx->result, 0);

    if (!info_array)
        ERR_EXIT();

    gsize array_size;
    gconstpointer gvar_info = g_variant_get_fixed_array(info_array, &array_size, sizeof(guint8));

    (*alterator_entry) = g_malloc0(array_size + 1);
    if (!(*alterator_entry))
        ERR_EXIT();
    memcpy((*alterator_entry), gvar_info, array_size);

end:
    g_free(path);

    if (info_array)
        g_variant_unref(info_array);

    if (ctx)
        dbus_ctx_free(ctx);

    return ret;
}

static int alterator_gdbus_source_getobjets_priv(gpointer *self, const gchar *iface, GVariant **result)
{
    int ret                      = 0;
    AlteratorGDBusSource *source = NULL;
    dbus_ctx_t *dbus_ctx         = NULL;
    GVariantBuilder *builder     = NULL;
    GVariant *array              = NULL;

    if (!self)
    {
        g_printerr(_("Internal error in alterator_gdbus_source_getobjets_priv - AlteratorCtlEditionsModule *module is "
                     "NULL\n"));
        ERR_EXIT();
    }

    source = (AlteratorGDBusSource *) self;

    if (iface)
    {
        dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                                 ALTERATOR_MANAGER_PATH,
                                 ALTERATOR_MANAGER_INTERFACE,
                                 ALTERATOR_MANAGER_GETOBJECT_METHOD_NAME,
                                 source->verbose);

        if (!dbus_ctx)
        {
            g_printerr(_("Can't allocate dbus_ctx_t in alterator_gdbus_source_getobjets_priv\n"));
            ERR_EXIT();
        }

        dbus_ctx->parameters = g_variant_new("(s)", iface);
    }
    else
    {
        dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                                 ALTERATOR_MANAGER_PATH,
                                 ALTERATOR_MANAGER_INTERFACE,
                                 ALTERATOR_MANAGER_GETALLOBJECT_METHOD_NAME,
                                 source->verbose);

        if (!dbus_ctx)
        {
            g_printerr(_("Can't allocate dbus_ctx_t in alterator_gdbus_source_getobjets_priv\n"));
            ERR_EXIT();
        }

        dbus_ctx->parameters = NULL;
    }

    dbus_ctx->reply_type = G_VARIANT_TYPE("(ao)");

    source->call(self, dbus_ctx, NULL);

    if (!dbus_ctx->result && iface)
    {
        g_printerr(_("DBus error in alterator_gdbus_source_getobjets_priv while calling GetObjects() at the object %s: "
                     "failed to produce a result.\n"),
                   iface);
        ERR_EXIT();
    }
    else if (!dbus_ctx->result && !iface)
    {
        g_printerr(
            _("D-Bus error in alterator_gdbus_source_getobjets_priv while calling GetObjects(): failed to produce a "
              "result.\n"));
        ERR_EXIT();
    }

    array = g_variant_get_child_value(dbus_ctx->result, 0);

    builder = g_variant_builder_new(G_VARIANT_TYPE("as"));

    GVariantIter *iter = NULL;
    gchar *str         = NULL;

    g_variant_get(array, "ao", &iter);
    while (g_variant_iter_loop(iter, "o", &str))
    {
        GVariant *string = g_variant_new_take_string(g_strdup(str));
        g_variant_builder_add_value(builder, string);
    }
    (*result) = g_variant_new("as", builder);

    g_variant_iter_free(iter);

end:
    if (builder)
        g_variant_builder_unref(builder);

    if (array)
        g_variant_unref(array);

    dbus_ctx_free(dbus_ctx);

    return ret;
}

static int alterator_gdbus_source_getifaces_priv(gpointer *self, const gchar *path, GVariant **result)
{
    int ret                      = 0;
    AlteratorGDBusSource *source = NULL;
    dbus_ctx_t *dbus_ctx         = NULL;
    GVariantBuilder *builder     = NULL;
    GVariant *array              = NULL;

    if (!self)
    {
        g_printerr(_("Internal error in alterator_gdbus_source_getifaces_priv - AlteratorGDBusSource *module is "
                     "NULL.\n"));
        ERR_EXIT();
    }

    source = (AlteratorGDBusSource *) self;

    if (path)
    {
        dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                                 ALTERATOR_MANAGER_PATH,
                                 ALTERATOR_MANAGER_INTERFACE,
                                 ALTERATOR_MANAGER_GETINTERFACES_METHOD_NAME,
                                 source->verbose);
        if (!g_variant_is_object_path(path))
        {
            g_printerr(_("Object path: %s isn't valid.\n"), path);
            ERR_EXIT();
        }

        if (!dbus_ctx)
        {
            g_printerr(_("Can't allocate dbus_ctx_t in alterator_gdbus_source_getifaces_priv\n"));
            ERR_EXIT();
        }

        dbus_ctx->parameters = g_variant_new("(s)", path);
    }
    else
    {
        dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                                 ALTERATOR_MANAGER_PATH,
                                 ALTERATOR_MANAGER_INTERFACE,
                                 ALTERATOR_MANAGER_GETALLINTERFACES_METHOD_NAME,
                                 source->verbose);

        if (!dbus_ctx)
        {
            g_printerr(_("Can't allocate dbus_ctx_t in alterator_gdbus_source_getifaces_priv\n"));
            ERR_EXIT();
        }

        dbus_ctx->parameters = NULL;
    }

    dbus_ctx->reply_type = G_VARIANT_TYPE("(as)");

    source->call(self, dbus_ctx, NULL);

    if (!dbus_ctx->result && path)
    {
        g_printerr(
            _("D-Bus error in alterator_gdbus_source_getifaces_priv while calling GetObjects() at the object %s: "
              "failed to produce a result.\n"),
            path);
        ERR_EXIT();
    }
    else if (!dbus_ctx->result && !path)
    {
        g_printerr(
            _("D-Bus error in alterator_gdbus_source_getifaces_priv while calling GetObjects(): failed to produce a "
              "result.\n"));
        ERR_EXIT();
    }

    array = g_variant_get_child_value(dbus_ctx->result, 0);

    (*result) = g_variant_ref(array);

end:
    if (array)
        g_variant_unref(array);

    dbus_ctx_free(dbus_ctx);

    return ret;
}

static int alterator_gdbus_source_getsignals_priv(
    gpointer *self, const gchar *path, const gchar *interface, const gchar *method, GVariant **result)
{
    int ret                      = 0;
    AlteratorGDBusSource *source = NULL;
    dbus_ctx_t *dbus_ctx         = NULL;
    GVariant *array              = NULL;

    if (!self)
    {
        g_printerr(_("Internal error in alterator_gdbus_source_getsignals_priv - AlteratorGDBusSource *module is "
                     "NULL.\n"));
        ERR_EXIT();
    }

    source = (AlteratorGDBusSource *) self;

    if (!path || !interface || !method)
    {
        g_printerr(_("Wrong arguments in alterator_gdbus_source_getsignals_priv.\n"));
        ERR_EXIT();
    }

    if (!g_variant_is_object_path(path))
    {
        g_printerr(_("Wrong path: %s in alterator_gdbus_source_getsignals_priv.\n"), path);
        ERR_EXIT();
    }

    dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                             ALTERATOR_MANAGER_PATH,
                             ALTERATOR_MANAGER_INTERFACE,
                             ALTERATOR_MANAGER_GETSIGNALS_METHOD_NAME,
                             source->verbose);

    if (!dbus_ctx)
    {
        g_printerr(_("Can't initialize dbus_ctx in alterator_gdbus_source_getsignals_priv.\n"));
        ERR_EXIT();
    }

    dbus_ctx->parameters = g_variant_new("(sss)", path, interface, method);
    dbus_ctx->reply_type = G_VARIANT_TYPE("(as)");

    source->call(self, dbus_ctx, NULL);

    if (!dbus_ctx->result)
    {
        g_printerr(
            _("D-Bus error in manager module while calling GetSignals() at the object and interface %s %s and method "
              "%s: failed to produce a result.\n"),
            path,
            interface,
            method);
        ERR_EXIT();
    }

    array     = g_variant_get_child_value(dbus_ctx->result, 0);
    (*result) = g_variant_ref(array);

end:
    if (array)
        g_variant_unref(array);

    dbus_ctx_free(dbus_ctx);

    return ret;
}

static int alterator_gdbus_source_get_services_names(gpointer self, GHashTable **result)
{
    int ret              = 0;
    GVariant *array      = NULL;
    dbus_ctx_t *dbus_ctx = NULL;

    if (!self)
    {
        g_printerr(_("Internal error in alterator_gdbus_source_get_services_names - AlteratorGDBusSource *module is "
                     "NULL.\n"));
        ERR_EXIT();
    }

    AlteratorGDBusSource *source = (AlteratorGDBusSource *) self;

    dbus_ctx = dbus_ctx_init(FREEDESKTOP_DBUS_SERVICE,
                             FREEDESKTOP_DBUS_OBJECT,
                             FREEDESKTOP_DBUS_INTERFACE,
                             FREEDESKTOP_DBUS_GET_SERVICES_METHOD_NAME,
                             source->verbose);

    if (!dbus_ctx)
    {
        g_printerr(_("Can't initialize dbus_ctx in alterator_gdbus_source_get_services_names\n"));
        ERR_EXIT();
    }

    dbus_ctx->parameters = NULL;
    dbus_ctx->reply_type = G_VARIANT_TYPE("(as)");

    source->call(self, dbus_ctx, NULL);

    if (!dbus_ctx->result)
    {
        g_printerr(_("D-Bus error in while calling ListNames() at service, the object, interface %s %s %s and method "
                     "%s: failed to produce a result.\n"),
                   FREEDESKTOP_DBUS_SERVICE,
                   FREEDESKTOP_DBUS_OBJECT,
                   FREEDESKTOP_DBUS_INTERFACE,
                   FREEDESKTOP_DBUS_GET_SERVICES_METHOD_NAME);
        ERR_EXIT();
    }

    array = g_variant_get_child_value(dbus_ctx->result, 0);

    (*result) = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    GVariantIter *iter = NULL;
    gchar *service     = NULL;

    g_variant_get(array, "as", &iter);
    while (g_variant_iter_loop(iter, "s", &service))
    {
        g_hash_table_add(*result, g_strdup(service));
    }

    g_variant_iter_free(iter);

end:
    if (array)
        g_variant_unref(array);

    dbus_ctx_free(dbus_ctx);

    return ret;
}

static int alterator_gdbus_source_get_introspection(gpointer self,
                                                    const gchar *service,
                                                    const gchar *path,
                                                    gchar **introspection)
{
    int ret              = 0;
    GVariant *result     = NULL;
    dbus_ctx_t *dbus_ctx = NULL;

    if (!self)
    {
        g_printerr(_("Internal error in alterator_gdbus_source_get_introspection - AlteratorGDBusSource *module is "
                     "NULL.\n"));
        ERR_EXIT();
    }

    AlteratorGDBusSource *source = (AlteratorGDBusSource *) self;

    dbus_ctx = dbus_ctx_init(service,
                             path,
                             FREEDESKTOP_DBUS_INTROSPECTABLE_INTERFACE,
                             FREEDESKTOP_DBUS_GET_INTROSPECT_METHOD_NAME,
                             source->verbose);

    if (!dbus_ctx)
    {
        g_printerr(_("Can't initialize dbus_ctx in alterator_gdbus_source_get_introspection\n"));
        ERR_EXIT();
    }

    dbus_ctx->parameters = NULL;
    dbus_ctx->reply_type = G_VARIANT_TYPE("(s)");

    source->call(self, dbus_ctx, NULL);

    if (!dbus_ctx->result)
    {
        g_printerr(_("D-Bus error in while calling Introspect() at service, the object, interface %s %s %s and method "
                     "%s: failed to produce a result.\n"),
                   service,
                   path,
                   FREEDESKTOP_DBUS_INTROSPECTABLE_INTERFACE,
                   FREEDESKTOP_DBUS_GET_INTROSPECT_METHOD_NAME);
        ERR_EXIT();
    }

    result           = g_variant_get_child_value(dbus_ctx->result, 0);
    (*introspection) = g_strdup(g_variant_get_string(result, NULL));

end:
    if (dbus_ctx)
        dbus_ctx_free(dbus_ctx);

    if (result)
        g_variant_unref(result);

    return ret;
}

static const gchar *alterator_gdbus_source_get_name_by_path(gpointer self, const gchar *path, const gchar *iface)
{
    AlteratorGDBusSource *source = (AlteratorGDBusSource *) self;
    if (!source)
    {
        g_printerr(_("Internal error in alterator_gdbus_source_get_name_by_path - AlteratorGDBusSource *module is "
                     "NULL.\n"));
        return NULL;
    }

    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) source->info_parser;
    if (!info_parser)
    {
        g_printerr(_("Module info parser object wasn't created.\n"));
        return NULL;
    }

    if (info_parser->names_by_dbus_paths
        && info_parser->alterator_ctl_module_info_parser_create_names_by_dbus_paths_table(info_parser, self, iface) < 0)
        return NULL;

    return (const gchar *) g_hash_table_lookup(info_parser->names_by_dbus_paths, ALTERATOR_ENTRY_OBJECT_KEY_NAME);
}

static const gchar *alterator_gdbus_source_get_path_by_name(gpointer self, const gchar *name, const gchar *iface)
{
    AlteratorGDBusSource *source = (AlteratorGDBusSource *) self;
    if (!source)
    {
        g_printerr(_("Internal error in alterator_gdbus_source_get_path_by_name - AlteratorGDBusSource *module is "
                     "NULL.\n"));
        return NULL;
    }

    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) source->info_parser;
    if (!info_parser)
    {
        g_printerr(_("Module info parser object wasn't created.\n"));
        return NULL;
    }

    if (info_parser->alterator_ctl_module_info_parser_create_names_by_dbus_paths_table(info_parser, self, iface) < 0)
        return NULL;

    if (!info_parser->names_by_dbus_paths)
        return NULL;

    return g_hash_table_lookup(info_parser->names_by_dbus_paths, name);
}

static gboolean is_object_path(const gchar *str_id)
{
    return str_id[0] == '/';
}

gchar *concat_signal_name_with_connection_name(const gchar *signal, const gchar *connection)
{
    if (!signal || !connection)
        return NULL;

    size_t signal_size     = strlen(signal);
    size_t connection_size = strlen(connection);

    char fixed_conn[255] = {0};
    strcpy(fixed_conn, connection);

    char *substring_ptr = strstr(fixed_conn, ":");
    if (substring_ptr)
    {
        size_t position      = substring_ptr - fixed_conn;
        fixed_conn[position] = '_';
    }

    substring_ptr = strstr(fixed_conn, ".");
    if (substring_ptr)
    {
        size_t position      = substring_ptr - fixed_conn;
        fixed_conn[position] = '_';
    }

    gchar *result = g_malloc0(signal_size + connection_size + 1);

    strcat(result, signal);
    strcat(result, fixed_conn);

    return result;
}
