/**/

#include "alterator_manager_default_interfaces.h"
#include "alterator_manager_backends.h"
#include "alterator_manager_polkit.h"
#include "alterator_manager_sender_environment.h"

static const gchar interfaces_xml[] =
  "<node>"
  "  <interface name='org.altlinux.alterator.manager'>"
  "    <method name='GetObjects'>"
  "      <arg type='s' name='interface' direction='in'/>"
  "      <arg type='ao' name='objects' direction='out'/>"
  "    </method>"
  "    <method name='GetAllObjects'>"
  "      <arg type='ao' name='objects' direction='out'/>"
  "    </method>"
  "    <method name='GetSignals'>"
  "      <arg type='s' name='object' direction='in'/>"
  "      <arg type='s' name='interface' direction='in'/>"
  "      <arg type='s' name='method' direction='in'/>"
  "      <arg type='as' name='signals' direction='out'/>"
  "    </method>"
  "    <method name='GetInterfaces'>"
  "      <arg type='s' name='object' direction='in'/>"
  "      <arg type='as' name='interfaces' direction='out'/>"
  "    </method>"
  "    <method name='GetAllInterfaces'>"
  "      <arg type='as' name='interfaces' direction='out'/>"
  "    </method>"
  "    <method name='GetEnvValue'>"
  "      <arg type='s' name='name' direction='in'/>"
  "      <arg type='s' name='value' direction='out'/>"
  "    </method>"
  "    <method name='SetEnvValue'>"
  "      <arg type='s' name='name' direction='in'/>"
  "      <arg type='s' name='value' direction='in'/>"
  "    </method>"
  "    <method name='UnsetEnvValue'>"
  "      <arg type='s' name='name' direction='in'/>"
  "    </method>"
  "  </interface>"
  "</node>";

static GList *interface_list = NULL;
static GDBusNodeInfo *introspection_data = NULL;
static GDBusInterfaceInfo *interface_info = NULL;
static PolkitAuthority *polkit_authority = NULL;
static gboolean user_mode = FALSE;

static void method_get_objects(GVariant *parameters,
                               GDBusMethodInvocation *invocation)
{
    GVariantBuilder *builder;
    GHashTableIter nodes_iter;
    GHashTable *interfaces;
    GList *objects = NULL;
    gchar *node_name;
    gchar *param;
    gchar *object_path;

    builder = g_variant_builder_new(G_VARIANT_TYPE ("ao"));
    g_variant_get(parameters, "(&s)", &param);

    GHashTable *data = alterator_manager_backends_get_data();

    g_hash_table_iter_init(&nodes_iter, data);
    while (g_hash_table_iter_next(&nodes_iter,
                                  (gpointer *) &node_name,
                                  (gpointer *) &interfaces))
    {
        if (g_hash_table_lookup(interfaces, param)) {
            object_path = g_strconcat(OBJECT_PATH_PREFIX, node_name, NULL);
            objects = g_list_prepend(objects, object_path);
        }
    }

    objects = g_list_sort(objects, (GCompareFunc) g_strcmp0);
    for (GList *a = objects; a; a = a->next) {
        g_variant_builder_add(builder, "o", (gchar *) a->data);
    }

    g_dbus_method_invocation_return_value(invocation,
                                          g_variant_new("(ao)", builder));

    if (objects) {
        g_list_free_full(objects, g_free);
    }
    g_variant_builder_unref(builder);
}

static void method_get_signals(GVariant *parameters,
                               GDBusMethodInvocation *invocation)
{
    GVariantBuilder *builder;
    GHashTableIter method_data_iter;
    InterfaceObjectInfo *interface_object_info  = NULL;
    MethodObjectInfo *method_object_info = NULL;
    GHashTable *interfaces = NULL;
    GHashTable *method_data = NULL;
    gchar *attr_name = NULL;
    gchar *attr_val = NULL;
    gchar *signal_name = NULL;
    gchar *object_param = NULL;
    gchar *interface_param = NULL;
    gchar *method_param = NULL;
    gchar *node_name = NULL;

    builder = g_variant_builder_new(G_VARIANT_TYPE ("as"));
    g_variant_get(parameters,
                  "(&s&s&s)",
                  &object_param,
                  &interface_param,
                  &method_param);

    GHashTable *data = alterator_manager_backends_get_data();

    if (g_str_has_prefix(object_param, OBJECT_PATH_PREFIX)) {
        node_name = &object_param[g_utf8_strlen(OBJECT_PATH_PREFIX, -1)];
        interfaces = g_hash_table_lookup (data, node_name);
    }

    if (interfaces) {
        interface_object_info = g_hash_table_lookup(interfaces,
                                                    interface_param);

        if (interface_object_info && interface_object_info->methods) {
            method_object_info =
                             g_hash_table_lookup(interface_object_info->methods,
                                                 method_param);

            if (method_object_info && method_object_info->method_data) {
                method_data = method_object_info->method_data;

                g_hash_table_iter_init(&method_data_iter, method_data);
                while (g_hash_table_iter_next(&method_data_iter,
                                              (gpointer *) &attr_name,
                                              (gpointer *) &attr_val))
                {
                    if (g_str_has_suffix(attr_name, SIGNAL_SUFFIX) &&
                        !toml_rtos(attr_val, &signal_name) &&
                        g_dbus_is_member_name(signal_name))
                    {
                        g_variant_builder_add(builder, "s", signal_name);
                    }
                    g_free(signal_name);
                    signal_name = NULL;
                }
            }
        }
    }

    g_dbus_method_invocation_return_value(invocation,
                                          g_variant_new("(as)", builder));

    g_variant_builder_unref(builder);
}

static void method_get_interfaces(GVariant *parameters,
                                  GDBusMethodInvocation *invocation)
{
    GVariantBuilder *builder;
    GHashTable *interfaces = NULL;
    gchar *param;
    gchar *node_name;

    builder = g_variant_builder_new(G_VARIANT_TYPE ("as"));
    g_variant_get(parameters, "(&s)", &param);

    GHashTable *data = alterator_manager_backends_get_data();

    if (g_str_has_prefix(param, OBJECT_PATH_PREFIX)) {

        node_name = &param[g_utf8_strlen(OBJECT_PATH_PREFIX, -1)];
        interfaces = g_hash_table_lookup(data, node_name);

        if (interfaces) {
            GList *keys = g_hash_table_get_keys(interfaces);
            keys = g_list_sort(keys, (GCompareFunc) g_strcmp0);

            for (GList *a = keys; a; a = a->next) {
                g_variant_builder_add(builder, "s", (gchar *)a->data);
            }
            if (keys) {
                g_list_free(keys);
            }
        }
    }

    g_dbus_method_invocation_return_value(invocation,
                                          g_variant_new("(as)", builder));

    g_variant_builder_unref(builder);
}

static void method_get_environment(const gchar* sender,
                                   GVariant *parameters,
                                   GDBusMethodInvocation *invocation)
{
    GError *error = NULL;
    gchar *value = NULL;
    gchar *param;

    GHashTable *table = alterator_manager_sender_environment_get_table(sender);

    g_variant_get(parameters, "(&s)", &param);

    if (!is_valid_environment_name(param)) {
        error = g_error_new_literal(G_DBUS_ERROR,
                                    G_DBUS_ERROR_INVALID_ARGS,
                                    "Invalid environment variable.");
    } else {
        if (table) {
            value = g_hash_table_lookup(table, param);
        }

        if (value == NULL) {
            error = g_error_new_literal(G_DBUS_ERROR,
                                        G_DBUS_ERROR_FAILED,
                                        "Environment variable not found.");
        }
    }

    if (error != NULL) {
        g_dbus_method_invocation_return_gerror(invocation, error);
        g_error_free(error);
    } else {
        g_dbus_method_invocation_return_value(invocation,
                                              g_variant_new("(s)", value));
    }

    g_hash_table_unref(table);
}

static void method_set_environment(const gchar* sender,
                                   GVariant *parameters,
                                   GDBusMethodInvocation *invocation)
{
    GError *error = NULL;
    gchar *param_name;
    gchar *param_value;

    GHashTable *table = alterator_manager_sender_environment_get_table(sender);

    g_variant_get(parameters, "(&s&s)", &param_name, &param_value);

    if (!is_valid_environment_name(param_name)) {
        error = g_error_new_literal(G_DBUS_ERROR,
                                    G_DBUS_ERROR_INVALID_ARGS,
                                    "Invalid environment variable.");
    } else {
        if (table == NULL) {
            error = g_error_new_literal(G_DBUS_ERROR,
                                        G_DBUS_ERROR_IO_ERROR,
                                        "Environment table not found.");
        } else {
            g_hash_table_replace(table,
                                 g_strdup(param_name),
                                 g_strdup(param_value));
        }
    }

    if (error != NULL) {
        g_dbus_method_invocation_return_gerror(invocation, error);
        g_error_free(error);
    } else {
        g_dbus_method_invocation_return_value(invocation,
                                              g_variant_new("()"));
    }

    g_hash_table_unref(table);
}

static void method_unset_environment(const gchar* sender,
                                     GVariant *parameters,
                                     GDBusMethodInvocation *invocation)
{
    GError *error = NULL;
    gchar *param;

    GHashTable *table = alterator_manager_sender_environment_get_table(sender);

    g_variant_get(parameters, "(&s)", &param);

    if (!is_valid_environment_name(param)) {
        error = g_error_new_literal(G_DBUS_ERROR,
                                    G_DBUS_ERROR_INVALID_ARGS,
                                    "Invalid environment variable.");
    } else {
        if (table == NULL) {
            error = g_error_new_literal(G_DBUS_ERROR,
                                        G_DBUS_ERROR_IO_ERROR,
                                        "Environment table not found.");
        } else {
            if (!g_hash_table_remove(table, param)) {
                error = g_error_new_literal(G_DBUS_ERROR,
                                            G_DBUS_ERROR_IO_ERROR,
                                            "Environment variable not found.");
            }
        }
    }

    if (error != NULL) {
        g_dbus_method_invocation_return_gerror(invocation, error);
        g_error_free(error);
    } else {
        g_dbus_method_invocation_return_value(invocation,
                                              g_variant_new("()"));
    }

    g_hash_table_unref(table);
}

static void method_get_all_objects(GVariant *parameters,
                                   GDBusMethodInvocation *invocation)
{
    GVariantBuilder *builder;
    GHashTableIter nodes_iter;
    GList *objects = NULL;
    gchar *node_name;
    gchar *object_path;

    builder = g_variant_builder_new(G_VARIANT_TYPE ("ao"));

    GHashTable *data = alterator_manager_backends_get_data();

    g_hash_table_iter_init(&nodes_iter, data);
    while (g_hash_table_iter_next(&nodes_iter,
                                  (gpointer *) &node_name,
                                  NULL))
    {
        if (node_name) {
            object_path = g_strconcat(OBJECT_PATH_PREFIX, node_name, NULL);
            objects = g_list_prepend(objects, object_path);
        }

    }

    objects = g_list_sort(objects, (GCompareFunc) g_strcmp0);
    for (GList *a = objects; a; a = a->next) {
        g_variant_builder_add(builder, "o", (gchar *) a->data);
    }

    g_dbus_method_invocation_return_value(invocation,
                                          g_variant_new("(ao)", builder));

    if (objects) {
        g_list_free_full(objects, g_free);
    }
    g_variant_builder_unref(builder);
}

static void method_get_all_interfaces(GVariant *parameters,
                                      GDBusMethodInvocation *invocation)
{
    GVariantBuilder *builder;
    GList *list = NULL;
    GHashTableIter interfaces_iter;
    GHashTableIter nodes_iter;
    gchar *interface_name;
    GHashTable *interfaces = NULL;
    gchar *node_name;

    builder = g_variant_builder_new(G_VARIANT_TYPE ("as"));

    GHashTable *data = alterator_manager_backends_get_data();

    g_hash_table_iter_init(&nodes_iter, data);
    while (g_hash_table_iter_next(&nodes_iter,
                                  (gpointer *) &node_name,
                                  (gpointer *) &interfaces))
    {
        if (!node_name || !interfaces) {
            continue;
        }

        g_hash_table_iter_init(&interfaces_iter, interfaces);
        while (g_hash_table_iter_next(&interfaces_iter,
                                      (gpointer *) &interface_name,
                                      NULL))
        {
            if (interface_name) {
                if (g_list_find_custom(list,
                                       interface_name,
                                       (GCompareFunc) g_strcmp0))
                {
                    continue;
                }
                else
                {
                    list = g_list_prepend(list, interface_name);
                }
            }
        }
    }

    list = g_list_sort(list, (GCompareFunc) g_strcmp0);
    for (GList *a = list; a; a = a->next) {
        g_variant_builder_add(builder, "s", (gchar *) a->data);
    }

    g_dbus_method_invocation_return_value(invocation,
                                          g_variant_new("(as)", builder));

    if (list) {
        g_list_free(list);
    }
    g_variant_builder_unref(builder);
}

static gboolean authorization_check(GDBusMethodInvocation *invocation,
                                    gchar *action_id)
{
    gboolean result = FALSE;
    PolkitSubject *subject = NULL;
    PolkitCheckAuthorizationFlags flags =
                        POLKIT_CHECK_AUTHORIZATION_FLAGS_ALLOW_USER_INTERACTION;
    PolkitAuthorizationResult *auth_result = NULL;
    GError *error = NULL;

    /* In user mode, validation is not needed. */
    if (user_mode) {
        return TRUE;
    }

    subject = polkit_system_bus_name_new(
                              g_dbus_method_invocation_get_sender (invocation));

    auth_result = polkit_authority_check_authorization_sync(polkit_authority,
                                                            subject,
                                                            action_id,
                                                            NULL,//details,
                                                            flags,
                                                            NULL, /* GCancellable* */
                                                            &error);

    if (auth_result == NULL) {
        g_warning("Manager: check authorization result == NULL (%s).",
                  error->message);
        g_dbus_method_invocation_return_dbus_error(invocation,
                                                   ERROR_NAME,
                                         "Check authorization result == NULL.");
    } else if (!polkit_authorization_result_get_is_authorized(auth_result)) {
        g_dbus_method_invocation_return_dbus_error(invocation,
                                                   ERROR_NAME,
                                                   "Authorization failed.");
    } else {
        result = TRUE;
    }

    if (error) {
        g_error_free(error);
    }
    g_object_unref(subject);
    if (auth_result) {
        g_object_unref(auth_result);
    }

    return result;
}

/* When adding new methods, don't forget to add action_ids to the
   org.altlinux.alterator.policy file. */
static gpointer handler(gpointer data) {
    ManagersMethodsParam *param = (ManagersMethodsParam *)data;

    if (!g_strcmp0(param->method_name, METHOD_GET_OBJECTS) &&
        authorization_check(param->invocation, METHOD_GET_OBJECTS_ACTION_ID))
    {
        method_get_objects(param->parameters, param->invocation);
    } else if (!g_strcmp0(param->method_name, METHOD_GET_SIGNALS) &&
        authorization_check(param->invocation, METHOD_GET_SIGNALS_ACTION_ID))
    {
        method_get_signals(param->parameters, param->invocation);
    } else if (!g_strcmp0(param->method_name, METHOD_GET_INTERFACES) &&
        authorization_check(param->invocation, METHOD_GET_INTERFACES_ACTION_ID))
    {
        method_get_interfaces(param->parameters, param->invocation);
    } else if (!g_strcmp0(param->method_name, METHOD_GET_ENVIRONMENT) &&
        authorization_check(param->invocation, METHOD_GET_ENVIRONMENT_ACTION_ID))
    {
        method_get_environment(param->sender, param->parameters, param->invocation);
    } else if (!g_strcmp0(param->method_name, METHOD_SET_ENVIRONMENT) &&
        authorization_check(param->invocation, METHOD_SET_ENVIRONMENT_ACTION_ID))
    {
        method_set_environment(param->sender, param->parameters, param->invocation);
    } else if (!g_strcmp0(param->method_name, METHOD_UNSET_ENVIRONMENT) &&
        authorization_check(param->invocation, METHOD_UNSET_ENVIRONMENT_ACTION_ID))
    {
        method_unset_environment(param->sender, param->parameters, param->invocation);
    } else if (!g_strcmp0(param->method_name, METHOD_GET_ALL_OBJECTS) &&
        authorization_check(param->invocation, METHOD_GET_ALL_OBJECTS_ACTION_ID))
    {
        method_get_all_objects(param->parameters, param->invocation);
    } else if (!g_strcmp0(param->method_name, METHOD_GET_ALL_INTERFACES) &&
        authorization_check(param->invocation, METHOD_GET_ALL_INTERFACES_ACTION_ID))
    {
        method_get_all_interfaces(param->parameters, param->invocation);
    }

    /* Do not free members. */
    g_free(param);

    return NULL;
}

static void handle_method_call(GDBusConnection *connection,
                               const gchar *sender,
                               const gchar *object_path,
                               const gchar *interface_name,
                               const gchar *method_name,
                               GVariant *parameters,
                               GDBusMethodInvocation *invocation,
                               gpointer user_data)
{
    if (!g_strcmp0(interface_name, INTERFACE_MANAGER)) {
        ManagersMethodsParam *method_param = g_new(ManagersMethodsParam, 1);
        method_param->sender = sender;
        method_param->parameters = parameters;
        method_param->method_name = method_name;
        method_param->invocation = invocation;

        GThread *thread = g_thread_new("manager", handler, method_param);
        if (thread != NULL) {
            g_thread_unref(thread);
        } else {
            g_free(method_param);
            g_warning("Manager: g_thread_new returned NULL.");
        }
    }
}

const GDBusInterfaceVTable vtable =
{
    handle_method_call,
    NULL,                 /* get_property */
    NULL,                 /* set_property */
    { 0 }
};

void alterator_manager_default_interfaces_init(gboolean is_session) {
    introspection_data = g_dbus_node_info_new_for_xml(interfaces_xml, NULL);

    if (introspection_data == NULL) {
        g_warning("default interfaces, introspection_data == NULL.");
        return;
    }

    interface_info = g_dbus_node_info_lookup_interface(introspection_data,
                                                       INTERFACE_MANAGER);

    if (interface_info == NULL) {
        g_warning("default interfaces, interface_info == NULL "
                  "(org.altlinux.alterator.manager).");
    } else {
        interface_list = g_list_prepend(interface_list, interface_info);
    }

    polkit_authority = alterator_manager_polkit_get_authority();

    user_mode = is_session;
}

GList* alterator_manager_default_interfaces_get_intorspections(void) {
    return interface_list;
}

const GDBusInterfaceVTable* alterator_manager_default_interfaces_get_vtable(void) {
    return &vtable;
}
