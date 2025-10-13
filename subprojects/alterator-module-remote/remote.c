/*Module for alterator-manager (remote).*/

#include "remote.h"

static const gchar xml_string[] =
  "<node>"
  "  <interface name='org.altlinux.alterator.remote'>"
  "    <method name='Connect'>"
  "      <arg type='s' name='remote_address' direction='in'/>"
  "      <arg type='s' name='connection_name' direction='in'/>"
  "      <arg type='s' name='agent_bus_name' direction='in'/>"
  "      <arg type='s' name='pty' direction='in'/>"
  "      <arg type='b' name='response' direction='out'/>"
  "      <arg type='s' name='object_path' direction='out'/>"
  "    </method>"
  "    <method name='Disconnect'>"
  "      <arg type='s' name='connection_name' direction='in'/>"
  "      <arg type='b' name='response' direction='out'/>"
  "    </method>"
  "    <method name='GetConnections'>"
  "      <arg type='as' name='connections' direction='out'/>"
  "    </method>"
  "  </interface>"
  "</node>";

static AlteratorManagerInterface *manager_interface = NULL;
/* object_path -> SubtreeInfo */
static GHashTable *subtrees_info_table = NULL;
static GMutex mutex;
static ManagerData *local_manager_data = NULL;

static void handlers_data_free(void *data) {
    HandlersData *handlers_data = (HandlersData *)data;

    if (handlers_data == NULL) {
        return;
    }

    if (handlers_data->object_path) {
        g_free(handlers_data->object_path);
    }

    if (handlers_data->agent_bus_name) {
        g_free(handlers_data->agent_bus_name);
    }

    if (handlers_data->connection_name) {
        g_free(handlers_data->connection_name);
    }

    if (handlers_data->pty) {
        g_free(handlers_data->pty);
    }

    /* Connection to remote machine. */
    if (handlers_data->connection) {
        g_object_unref(handlers_data->connection);
    }

    if (handlers_data->password) {
        memset(handlers_data->password, 0, strlen(handlers_data->password));
        g_free(handlers_data->password);
    }

    /* Do not free invocation, it's form handle_method_call(). */

    g_free(handlers_data);
}

static void interface_info_free(void *interface_info) {
    InterfaceObjectInfo *info = (InterfaceObjectInfo *)interface_info;

    if (info == NULL) {
        return;
    }

    if (info->module_name) {
        g_free(info->module_name);
    }

    if (info->action_id) {
        g_free(info->action_id);
    }

    if (info->interface_introspection) {
        g_dbus_interface_info_unref(info->interface_introspection);
    }

    if (info->methods != NULL) {
        g_hash_table_unref(info->methods);
    }
    //GDBusInterfaceVTable *interface_vtable; //belongs to the module
    g_free(info);
}

static gint safe_dup2(gint fd1, gint fd2) {
    gint ret;

    do
        ret = dup2(fd1, fd2);
    while (ret < 0 && (errno == EINTR || errno == EBUSY));

    return ret;
}

static void unixexec_prepare_child(gpointer data) {
    int fd;
    gint *s = (gint *)data;

   /* We want to keep socket pair alive. Therefore we don't set CLOEXEC on it */
    fd = safe_dup2(s[1], STDIN_FILENO);
    if (fd != STDIN_FILENO) {
        g_warning("Remote: safe_dup2 error.");
    }
    fd = safe_dup2(s[1], STDOUT_FILENO);
    if (fd != STDOUT_FILENO) {
        g_warning("Remote: safe_dup2 error.");
    }
}

//rename (g_)
static gboolean g_socketpair(gint domain, gint type, gint protocol, gint *s) {
    int r;
    int errsv;

    r = socketpair(domain, type | SOCK_CLOEXEC, protocol, s);
    errsv = errno;

    /* It's possible that libc has SOCK_CLOEXEC but the kernel does not */
    if (r == -1 && (errsv == EINVAL || errsv == EPROTOTYPE)) {
        r = socketpair(domain, type, protocol, s);
    }

    if (r == -1) {
        errsv = errno;

        g_warning("Remote: unable to create socketpair: %s.",
                  g_strerror(errsv));

        return FALSE;
    }

    if (fcntl(s[0], F_SETFD, FD_CLOEXEC) < 0 ||
        fcntl(s[1], F_SETFD, FD_CLOEXEC) < 0)
    {
        errsv = errno;
        g_close(s[0], NULL);
        g_close(s[1], NULL);
        s[0] = -1;
        s[1] = -1;
        g_warning("Remote: failed to call fcntl: %s.", g_strerror(errsv));
        return FALSE;
    }

    return TRUE;
}

/* Maybe you need to add GPid to the parameters ??? */
static GIOStream *spawn_stream_to_remote(gchar *address) {
    GPid child_pid;
    GIOStream *stream = NULL;
    GStrv argv;
    GStrvBuilder *builder = NULL;
    gint s[2];
    GSocket *fdsocket;
    GError *error = NULL;

    builder = g_strv_builder_new();
    g_strv_builder_add(builder, "ssh");
    g_strv_builder_add(builder, address);
    g_strv_builder_add(builder, DBUS_BRIDGE);
    argv = g_strv_builder_end(builder);
    g_strv_builder_unref(builder);

    if (!g_socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, s)) {
        g_warning("Remote: g_socketpair returned FALSE.");
        return stream;
    }

    g_spawn_async_with_pipes(NULL,
                             argv,
                             NULL,
                             G_SPAWN_SEARCH_PATH |
//?                             G_SPAWN_DO_NOT_REAP_CHILD |
                             G_SPAWN_CHILD_INHERITS_STDIN,
                             unixexec_prepare_child,
                             s,
                             &child_pid,
                             NULL,
                             NULL,
                             NULL,
                             &error);


    if (error) {
        g_warning("Remote: error spawning a process for GIOStream: %s.",
                  error->message);
        g_close (s[0], NULL);
        g_close (s[1], NULL);
        s[0] = -1;
        s[1] = -1;
        g_error_free(error);
        g_strfreev(argv);
        return stream;
    }

    g_close (s[1], NULL);

    fdsocket = g_socket_new_from_fd(s[0], NULL);
    if (fdsocket == NULL) {
        g_warning("Remote: g_socket_new_from_fd returned NULL.");
        g_close (s[0], NULL);
        g_spawn_close_pid(child_pid);
        g_strfreev(argv);
        return stream;
    }

    stream = (GIOStream *)
                        g_socket_connection_factory_create_connection(fdsocket);

    if (!stream) {
        g_warning("Remote: error creating GIOStream.");
        g_object_unref(fdsocket);
        g_close (s[0], NULL);
        g_spawn_close_pid(child_pid);
        g_strfreev(argv);
    }

    g_object_unref(fdsocket);
    g_strfreev(argv);
    return stream;
}

/*  */
static GPid spawn_polkit_agent_on_remote(guint pid,
                                         const gchar *address,
                                         gint *pkstdin,
                                         gint *pkstdout)
{
    GPid child_pid = 0;
    GStrv argv;
    gchar *p = NULL;
    GStrvBuilder *builder = NULL;
    GError *error = NULL;

    p = g_strdup_printf("%u", pid);
    builder = g_strv_builder_new();
    g_strv_builder_add(builder, "ssh");
    g_strv_builder_add(builder, address);
    g_strv_builder_add(builder, POLKIT_AGENT_NAME);
    g_strv_builder_add(builder, "-p");
    g_strv_builder_add(builder, p);
    argv = g_strv_builder_end(builder);
    g_strv_builder_unref(builder);

    g_spawn_async_with_pipes(NULL,
                             argv,
                             NULL,
                             G_SPAWN_SEARCH_PATH,
                             NULL,
                             NULL,
                             &child_pid,
                             pkstdin,
                             pkstdout,
                             NULL,
                             &error);

    if (error) {
        g_warning("Remote: error spawning a process for a polkit agent: %s.",
                  error->message);
        child_pid = 0;
        g_error_free(error);
    }

    g_strfreev(argv);
    g_free(p);

    return child_pid;
}

/* Receiving an introspection of an object from a remote machine. */
/* Don't forget to free the return value. */
static gchar *receive_object_introspection(GDBusConnection *connection,
                                           const gchar *object)
{
    gchar *result = NULL;
    GVariant *answer = NULL;
    GError *error = NULL;

    answer = g_dbus_connection_call_sync(connection,
                                         DBUS_NAME,
                                         object,
                                         INTERFACE_INTROSPECTABLE,
                                         METHOD_INTROSPECT,
                                         NULL,
                                         G_VARIANT_TYPE ("(s)"),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (error) {
        g_warning("Remote: error getting introspection for %s (%s).",
                  object, error->message);

        g_error_free(error);
        return NULL;
    }

    g_variant_get(answer, "(s)", &result);
    g_variant_unref(answer);

    return result;
}

/* Receiving introspections of objects from a remote machine. */
/* Don't forget to free the return value. */
static GHashTable *receive_introspections(GDBusConnection *connection,
                                          const gchar *remote_address)
{
    GHashTable *xmls = NULL;
    GVariant *answer = NULL;
    gchar *object_path = NULL;
    gchar *xml = NULL;
    GVariantIter *iter;
    GError *error = NULL;

    /* Getting introspection of org.altlinux.alterator.manager */
    xml = receive_object_introspection(connection, OBJECT_ALTERATOR);
    if (xml) {
        xmls = g_hash_table_new_full(g_str_hash,
                                     g_str_equal,
                                     g_free,
                                     g_free);

        g_hash_table_replace(xmls, g_strdup(ROOT_NODE), xml);
    } else {
        g_warning("Remote: error getting introspection for %s.",
                  OBJECT_ALTERATOR);
        return NULL;
    }

    /* Getting the names of all objects. */
    answer = g_dbus_connection_call_sync(connection,
                                         DBUS_NAME,
                                         OBJECT_ALTERATOR,
                                         INTERFACE_MANAGER,
                                         METHOD_GETALLOBJECTS,
                                         NULL,
                                         G_VARIANT_TYPE ("(ao)"),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (error) {
        g_warning("Remote: error getting the names of objects (%s).",
                  error->message);

        g_error_free(error);
        return xmls;
    }

    g_variant_get(answer, "(ao)", &iter);
    while (g_variant_iter_loop(iter, "o", &object_path)) {
        gchar *xml = receive_object_introspection(connection, object_path);
        if (xml) {
            /* The table keys are the node names. We're just dropping
               the prefix "/org/altlinux/alterator/" here.
               Because the subtree cannot have a hierarchy of objects. */
            gchar **array = g_strsplit(object_path, "/", -1);
            if (array) {
                guint len = g_strv_length(array);
                if (array[len - 1][0] != '\0') {
                    g_hash_table_replace(xmls,
                                         g_strdup(array[len - 1]),
                                         g_strdup(xml));
                }
                g_strfreev(array);
            }
            g_free(xml);
        }
    }
    g_variant_iter_free(iter);
    g_variant_unref(answer);

    return xmls;
}

/* Does the subtrees_info_table have a key connection_name? */
static gboolean connection_name_is_exist_in_table(const gchar *connection_name)
{
    gconstpointer res;
    if (subtrees_info_table == NULL ) {
        return FALSE;
    }

    g_mutex_lock(&mutex);
    res = g_hash_table_lookup(subtrees_info_table, connection_name);
    g_mutex_unlock(&mutex);

    if (res == NULL) {
        return FALSE;
    } else {
        return TRUE;
    }
}

/* Add a new key-value pair to the subtrees_info_table. */
static void subtrees_info_table_add_info(gchar *connection_name,
                                         SubtreeInfo *info)
{
    g_mutex_lock(&mutex);
    g_hash_table_replace(subtrees_info_table, connection_name, info);
    g_mutex_unlock(&mutex);
}

/* Kill remote-polkit-agent. */
static gboolean subtrees_info_table_kill_pkagent(const gchar *connection_name) {
    SubtreeInfo *info = NULL;
    gboolean res = TRUE;

    g_mutex_lock(&mutex);
    info = (SubtreeInfo *)g_hash_table_lookup(subtrees_info_table,
                                              connection_name);
    if (info) {
        g_spawn_close_pid(info->pid_pkagent);
        int sig = 0;
        sig = kill(info->pid_pkagent, 9);
        if (sig == -1) {
            int err = errno;
            g_warning("Remote: error sending signal to remote-polkit-agent "
                      "(errno = %d).", err);
            res = FALSE;
        }
    } else {
        g_warning("Remote: error sending signal to remote-polkit-agent, "
                  "there is no such connection name in the table (%s).",
                  connection_name);
        res = FALSE;
    }
    g_mutex_unlock(&mutex);

    return res;
}

/* Stop the loop. */
static void subtrees_info_table_stop_loop(const gchar *connection_name) {
    SubtreeInfo *info = NULL;

    if (connection_name == NULL) {
        return;
    }

    g_mutex_lock(&mutex);
    info = (SubtreeInfo *)g_hash_table_lookup(subtrees_info_table,
                                              connection_name);
    if (info && info->loop) {
        g_main_loop_quit(info->loop);
    }
    g_mutex_unlock(&mutex);
}

/* Is the loop running? */
/*static gboolean subtrees_info_table_is_loop_running
                                                  (const gchar *connection_name)
{
    gboolean res = FALSE;
    SubtreeInfo *info = NULL;

    if (connection_name == NULL) {
        return FALSE;
    }

    g_mutex_lock(&mutex);
    info = (SubtreeInfo *)g_hash_table_lookup(subtrees_info_table,
                                              connection_name);
    if (info && info->loop) {
        res = g_main_loop_is_running(info->loop);
    }
    g_mutex_unlock(&mutex);

    return res;
}*/

/* Remove a key-value pair from the subtrees_info_table. */
static void subtrees_info_table_remove_info(const gchar *connection_name) {
    g_mutex_lock(&mutex);
    if (!g_hash_table_remove(subtrees_info_table, connection_name)) {
        g_warning("Remote: error deleting from subtrees_info_table (%s).",
                  connection_name);
    }
    g_mutex_unlock(&mutex);
}

/* It returns a newly allocated array of strings for node names. The array is
   NULL-terminated. */
static gchar **subtrees_info_table_get_node_names(const gchar *connection_name) {
    gchar **nodes = NULL;
    SubtreeInfo *info = NULL;

    if (connection_name == NULL) {
        return NULL;
    }

    g_mutex_lock(&mutex);
    info = (SubtreeInfo *)g_hash_table_lookup(subtrees_info_table,
                                              connection_name);
    if (info && info->nodes_info) {
        GList *list = g_hash_table_get_keys(info->nodes_info);

        if (list) {
            GPtrArray *p = g_ptr_array_new();

            for (GList *a = list; a; a = a->next) {
                gchar *node = (gchar*) a->data;
                /* We do not give out the root node. */
                if (g_strcmp0(node, ROOT_NODE)) {
                    g_ptr_array_add(p, g_strdup(node));
                }
            }
            g_ptr_array_add(p, NULL);
            nodes = (gchar **) g_ptr_array_free(p, FALSE);
            g_list_free(list);
        }
    }
    g_mutex_unlock(&mutex);

    return nodes;
}

/* Is the interface name standart D-Bus interface? */
static gboolean is_standard_interface(const gchar *name) {
    if (!g_strcmp0(name, INTERFACE_PEER)) {
        return TRUE;
    }

    if (!g_strcmp0(name, INTERFACE_INTROSPECTABLE)) {
        return TRUE;
    }

    if (!g_strcmp0(name, INTERFACE_PROPERTIES)) {
        return TRUE;
    }

    return FALSE;
}

/* It returns a newly allocated array of GDBusInterfaceInfo for node name.
   The array is NULL-terminated. */
static GDBusInterfaceInfo **
subtrees_info_table_get_node_interfaces(const gchar *connection_name,
                                        const gchar *node_name)
{
    SubtreeInfo *subtree_info = NULL;
    GDBusNodeInfo *node_info = NULL;
    GPtrArray *p = NULL;

    if (connection_name == NULL) {
        return NULL;
    }

    if (node_name == NULL) {
        node_name = ROOT_NODE;
    }

    p = g_ptr_array_new();

    g_mutex_lock(&mutex);
    subtree_info = (SubtreeInfo *)g_hash_table_lookup(subtrees_info_table,
                                                      connection_name);
    if (subtree_info && subtree_info->nodes_info) {
        node_info =
                  (GDBusNodeInfo *)g_hash_table_lookup(subtree_info->nodes_info,
                                                       node_name);
        if (node_info) {
            for (GDBusInterfaceInfo **i = node_info->interfaces;
                 *i != NULL;
                 i++)
            {
                if (!is_standard_interface((*i)->name)) {
                    g_ptr_array_add(p, g_dbus_interface_info_ref(*i));
                }
            }
        } else {
            g_warning("Remote: there are no entries in the table for %s node.",
                      connection_name);
        }
    } else {
        g_warning("Remote: there are no entries in subtrees_info_table for %s.",
                  connection_name);
    }
    g_mutex_unlock(&mutex);

    g_ptr_array_add(p, NULL);

    return (GDBusInterfaceInfo **) g_ptr_array_free(p, FALSE);
}

/* -------------------------------------------------------------------------- */

static void remote_method_call_cb(GDBusConnection *connection,
                                  GAsyncResult *res,
                                  gpointer user_data)
{
    GDBusMethodInvocation *invocation = user_data;
    GVariant *answer = NULL;
    GError *error = NULL;

    answer = g_dbus_connection_call_finish(connection,
                                           res,
                                           &error);
    if (error) {
        g_warning("Remote: error remote handle method call (%s).",
                  error->message);

        g_dbus_method_invocation_return_dbus_error(invocation,
                                                   INTERFACE_REMOTE,
                                                   "Error remote handle method"
                                                   " call.");

        g_error_free(error);
    } else {
        g_dbus_method_invocation_return_value(invocation, answer);
        g_variant_unref(answer);
    }
}

static void remote_handle_method_call(GDBusConnection *connection,
                                      const gchar *sender,
                                      const gchar *object_path,
                                      const gchar *interface_name,
                                      const gchar *method_name,
                                      GVariant *parameters,
                                      GDBusMethodInvocation *invocation,
                                      gpointer user_data)
{
    gchar *path = NULL;
    HandlersData *data = (HandlersData *)user_data;

    if (!data) {
        g_warning("Remote: remote_handle_method_call data is null. "
                  "object_path: %s, interface_name: %s, method_name: %s.",
                  object_path, interface_name, method_name);

        g_dbus_method_invocation_return_dbus_error(invocation,
                                                   INTERFACE_REMOTE,
                                                   "Error: user_data in "
                                                   "remote_handle_method_call()"
                                                   " is NULL.");
        return;
    }

    /* Remove remote_name form object_path
       /org/altlinux/alterator/connection/example =>
       => /org/altlinux/alterator/example
       (make object_path for remote machine). */
    gchar **parts = g_strsplit(object_path, data->object_path, -1);
    gchar *last_part = NULL;
    for (gchar **a = parts; *a != NULL; a++) {
        last_part = *a;
    }
    path = g_strconcat(OBJECT_ALTERATOR, last_part, NULL);
    g_strfreev(parts);

    g_dbus_connection_call(data->connection,
                           DBUS_NAME,
                           path,
                           interface_name,
                           method_name,
                           parameters,
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) remote_method_call_cb,
                           invocation);

    g_free(path);
}

static const GDBusInterfaceVTable remote_interface_vtable =
{
    remote_handle_method_call,
    NULL,
    NULL,
    { 0 }
};

/* -------------------------------------------------------------------------- */

static gchar **subtree_enumerate(GDBusConnection       *connection,
                                 const gchar           *sender,
                                 const gchar           *object_path,
                                 gpointer               user_data)
{
    HandlersData *data = (HandlersData *)user_data;
    return subtrees_info_table_get_node_names(data->connection_name);
}

static GDBusInterfaceInfo **subtree_introspect(GDBusConnection   *connection,
                                               const gchar       *sender,
                                               const gchar       *object_path,
                                               const gchar       *node,
                                               gpointer           user_data)
{
    HandlersData *data = (HandlersData *)user_data;
    return subtrees_info_table_get_node_interfaces(data->connection_name, node);
}

static const GDBusInterfaceVTable *subtree_dispatch(GDBusConnection *connection,
                                                    const gchar *sender,
                                                    const gchar *object_path,
                                                    const gchar *interface_name,
                                                    const gchar *node,
                                                    gpointer *out_user_data,
                                                    gpointer user_data)
{
    *out_user_data = user_data;
    return &remote_interface_vtable;
}

static const GDBusSubtreeVTable subtree_vtable =
{
    subtree_enumerate,
    subtree_introspect,
    subtree_dispatch,
    { 0 }
};

/* -------------------------------------------------------------------------- */
static guint register_subtree(HandlersData *handlers_data) {
    guint id;
    GError *error = NULL;

    if (handlers_data == NULL) {
            g_warning("Remote: error registering subtree, handlers_data is"
                      " null.");
    }

    g_mutex_lock(&mutex);
    id = g_dbus_connection_register_subtree(local_manager_data->connection,
                                            handlers_data->object_path,
                                            &subtree_vtable,
                                            G_DBUS_SUBTREE_FLAGS_NONE,
                                            handlers_data,  /* user_data */
                                            handlers_data_free,
                                            &error); /* GError** */
    g_mutex_unlock(&mutex);

    if (error) {
        g_warning("Remote: error registering subtree, %s (%s)", error->message,
                  handlers_data->object_path);

        g_dbus_method_invocation_return_value(handlers_data->invocation,
                                              g_variant_new("(bs)", FALSE, ""));

        g_error_free(error);
        handlers_data_free(handlers_data);
    } else {
        g_dbus_method_invocation_return_value(handlers_data->invocation,
                                     g_variant_new("(bs)",
                                                   TRUE,
                                                   handlers_data->object_path));
    }

    return id;
}

/* Free struct SubtreeInfo. */
static void subtree_info_free(void *subtree_info) {
    SubtreeInfo *info = (SubtreeInfo *)subtree_info;

    if (info == NULL) {
        return;
    }

    if (info->nodes_info) {
        g_hash_table_destroy(info->nodes_info);
    }

    g_free(info);
}

static void node_info_free(void *node_info) {
    GDBusNodeInfo *info = (GDBusNodeInfo *)node_info;
    if (info == NULL) {
        return;
    }

    g_dbus_node_info_unref(info);
}

/* Don't forget to free the return value. */
/* node_name -> GDBusNodeInfo* */
static GHashTable *create_nodes_info_for_xmls(GHashTable *xmls) {
    GHashTable *nodes_info = NULL;
    GDBusNodeInfo *node = NULL;
    gchar *key;
    gchar *value;
    GHashTableIter iter;
    GError *error = NULL;

    if (xmls == NULL) {
        return NULL;
    }

    nodes_info = g_hash_table_new_full(g_str_hash,
                                       g_str_equal,
                                       g_free,
                                       node_info_free);

    g_hash_table_iter_init(&iter, xmls);
    while (g_hash_table_iter_next (&iter,
                                   (gpointer *) &key,
                                   (gpointer *) &value))
    {
        node = g_dbus_node_info_new_for_xml(value, &error);
        if (error) {
            g_warning("Remote: error creating node info for xml (%s).",
                      error->message);
            g_error_free(error);
            error = NULL;
            continue;
        }

        g_hash_table_replace(nodes_info, g_strdup(key), node);
    }

    return nodes_info;
}

/* Remove spaces from the beginning and end of the string. Also remove the
   beginning and end double quotes from the string. If one of the quotes is
   missing, return NULL. It returns newly allocated string. */
static gchar *remove_spaces_quotes(const gchar *str) {
    gchar *s = NULL;
    gchar *ret = NULL;

    if (str == NULL) {
        return NULL;
    }

    s = g_strdup(str);
    s = g_strstrip(s);

    if (*s != '"' || *(s + strlen(s) - 1) != '"') {
        g_debug("Remote: remove_spaces_quotes, the string is missing a double"
                " quote.");
        g_free(s);
        return NULL;
    } else {
        ret = g_strndup(s + 1, strlen(s + 1) - 1);
        g_free(s);
    }

    return ret;
}

/* It returns list newly allocated strings from json array. If key does not
   exist or value isn't an array of strings it returns NULL. */
static GList *get_json_value_arr_str(const gchar *json, const gchar *key) {
    GList *result = NULL;
    gchar *s, *end;
    gchar *values;
    gchar **array = NULL;

    s = g_strstr_len(json, -1, key);
    /* There is no such key. */
    if (!s) {
        goto out;
    }

    /* Move to the symbol following the last symbol of the key. */
    s += strlen(key);
    /* Reach the colon symbol. */
    while (*s != ':') {
        if (isspace(*s)) {
            s++;
        } else {
            goto out;
        }
    }

    /* Go to the opening square bracket. */
    s++; //now *s is ':', increment s
    while (*s != '[') {
        if (isspace(*s)) {
            s++;
        } else {
            goto out;
        }
    }

    /* Making a list from an json array. */
    /* 1) Find the position of the closing square bracket (if we don’t find
          it, we exit).
       2) Copy the resulting segment into a new array (without the square
          bracket).
       3) Split the array by commas.
       4) Trim the spaces at the edges of each resulting substring.
       5) For each substring, we remove double quotes at the edges; if there is
          no quote, we exit. (Because this is an array of strings).
       6) Place the received strings into a GList, in their original order. */

    s++; //now *s is '[', increment s
    end = g_strstr_len(s, -1, "]");
    if (!end || s == end) {
        goto out;
    }
    /* Now "end" points to ']'. Copy the contents of the json array to a
       new string. */
    values = g_strndup(s, end - s);
    /* Split this into strings by commas. */
    array = g_strsplit(values, ",", -1);
    g_free(values);

    /* Remove spaces from the beginning and end of each string. Also remove the
       beginning and end double quotes from each string. If one of the quotes is
       missing, exit. */
    for (gchar **v = array; *v != NULL; v++) {
        gchar *user_name = remove_spaces_quotes(*v);
        if (!user_name) {
            if (result) {
                g_list_free_full(g_steal_pointer(&result), g_free);
            }
            goto out;
        } else {
            result = g_list_prepend(result, user_name);
        }
    }

    if (result) {
        result = g_list_reverse(result);
    }

    out:

    g_strfreev(array);

    return result;
}

/* It returns a newly allocated string. If key does not exist or value isn't
   string it returns NULL. */
static gchar *get_json_value_str(const gchar *json, const gchar *key) {
    /* Find "key" (in quotes), then through spaces, tabs and line feeds get to
       the colon (if you encounter something else, then exit). Then, again
       through spaces get to the opening double quote and save everything up to
       the closing double quote, including escaped quotes. If you encounter \0,
       then exit. */
    gchar *result = NULL;
    GString *value;
    gchar *s;

    s = g_strstr_len(json, -1, key);
    /* There is no such key. */
    if (!s) {
        goto out;
    }

    /* Move to the symbol following the last symbol of the key. */
    s += strlen(key);

    /* Reach the colon symbol. */
    while (*s != ':') {
        if (isspace(*s)) {
            s++;
        } else {
            goto out;
        }
    }

    /* Move on to the opening double quote. */
    s++; //now *s is ':', increment s
    while (*s != '"') {
        if (isspace(*s)) {
            s++;
        } else {
            goto out;
        }
    }

    /* Copy the characters up to the closing double quote. */
    s++; //now *s is '"', increment s
    gchar previous = 0;
    if (*s != '\0') {
        value = g_string_new(NULL);
        while (TRUE) {
            if (*s == '"' && previous != '\\') {
                result = g_string_free(value, FALSE);
                break;
            } else if (isprint(*s)) {
                g_string_append_c(value, *s);
                previous = *s;
                s++;
                continue;
            } else {
                g_string_free(value, TRUE);
                goto out;
            }
        }
    }

    out:

    return result;
}

/* It returns a newly allocated string or NULL, and it returns a newly allocated
   string in handlers_data->password or NULL. */
static gchar *select_user_ask_pass(const gchar *action_id,
                                   const gchar *message,
                                   const GList *users,
                                   HandlersData *handlers_data)
{
    gchar *bus_address;
    gchar *pass = NULL;
    gchar *selected_user = NULL;
    GDBusConnection *connection;
    GVariantBuilder *builder;
    GVariant *answer;
    GError *error = NULL;

    /* If the user list is empty, we return NULL instead of the password. We
       send an empty response ("\n") to Polkit. */
    if (!users) {
        g_warning("Remote: List of users from a remote machine is NULL.");
        return NULL;
    }

    bus_address = g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SESSION,
                                                  NULL,
                                                  &error);
    if (error) {
        g_warning("Remote: error getting address for bus (%s).",
                  error->message);
        g_error_free(error);

        /* We can't get the password. So we send an empty password to complete
           the authentication process. */
        return NULL;
    }

    connection =
    g_dbus_connection_new_for_address_sync
                                (bus_address,
                                 G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT |
                                 G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION,
                                 NULL,
                                 NULL,
                                 &error);
    if (error) {
        g_warning("Remote: error creating connection for password-agent, "
                  "address:%s (%s).", bus_address, error->message);

        g_error_free(error);
        g_free(bus_address);
        /* We can't get the password. So we send an empty password to complete
           the authentication process. */
        return NULL;
    }

    /* We call the SelectUser method on the password agent.
       in:  array of users
            action_id
            message
            pty
       out: selected user
            password */
    builder = g_variant_builder_new(G_VARIANT_TYPE ("(assss)"));

    g_variant_builder_open(builder, G_VARIANT_TYPE ("as"));

    for (const GList *a = users; a; a = a->next) {
        g_variant_builder_add(builder, "s", (gchar*) a->data);
    }
    g_variant_builder_close(builder);

    if (action_id) {
        g_variant_builder_add(builder, "s", action_id);
    } else {
        g_variant_builder_add(builder, "s", "");
    }

    if (message) {
        g_variant_builder_add(builder, "s", message);
    } else {
        g_variant_builder_add(builder, "s", "");
    }

    if (handlers_data->pty) {
        g_variant_builder_add(builder, "s", handlers_data->pty);
    } else {
        g_variant_builder_add(builder, "s", "");
    }

    answer = g_dbus_connection_call_sync(connection,
                                         handlers_data->agent_bus_name,
                                         OBJECT_PASSWORD_AGENT,
                                         INTERFACE_PASSWORD_AGENT,
                                         METHOD_SELECTUSER,
                                         g_variant_builder_end(builder),
                                         G_VARIANT_TYPE("(ss)"),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);
    if (error) {
        g_warning("Remote: error asking password on %s (%s).", bus_address,
                  error->message);

        g_error_free(error);
        g_free(bus_address);
        g_object_unref(connection);
        g_variant_builder_unref(builder);
        /* We can't get the password. So we send an empty password to complete
           the authentication process. */
        return NULL;
    }

    g_variant_get(answer, "(ss)", &selected_user, &pass);

    if (handlers_data->password) {
        memset(handlers_data->password, 0, strlen(handlers_data->password));
        g_free(handlers_data->password);
    }
    handlers_data->password = pass;

    g_free(bus_address);
    g_object_unref(connection);
    g_variant_unref(answer);
    g_variant_builder_unref(builder);

    return selected_user;
}

/* Sends authentication result to password agent (Calling a ShowResult method on
   a password agent interface.). */
static void send_authentication_result(const gchar *result,
                                       HandlersData *handlers_data)
{
    gchar *bus_address;
    GDBusConnection *connection;
    GError *error = NULL;

    if (!result) {
        g_warning("Remote: send_authentication_result(), result string is "
                  "NULL");
        return;
    }

    bus_address = g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SESSION,
                                                  NULL,
                                                  &error);
    if (error) {
        g_warning("Remote: send_authentication_result(), error getting address "
                  "for bus (%s).", error->message);
        g_error_free(error);

        return;
    }

    connection =
    g_dbus_connection_new_for_address_sync
                                (bus_address,
                                 G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT |
                                 G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION,
                                 NULL,
                                 NULL,
                                 &error);
    if (error) {
        g_warning("Remote: send_authentication_result(), error creating "
                  "connection for address:%s (%s).", bus_address,
                  error->message);

        g_error_free(error);
        g_free(bus_address);

        return;
    }

    /* We call the ShowResult method on the password agent.
       in:  result string */
    g_dbus_connection_call_sync(connection,
                                handlers_data->agent_bus_name,
                                OBJECT_PASSWORD_AGENT,
                                INTERFACE_PASSWORD_AGENT,
                                METHOD_SHOWRESULT,
                                g_variant_new("(s)", result),
                                NULL,
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);
    if (error) {
        g_warning("Remote: error sending result to %s (%s).",
                  handlers_data->agent_bus_name,
                  error->message);

        g_error_free(error);
        g_free(bus_address);
        g_object_unref(connection);

        return;
    }

    g_free(bus_address);
    g_object_unref(connection);
}

/* Be careful with the template. */
static gint write_string_to_fd(gint fd,
                               const gchar *template,
                               const gchar *str)
{
    gint res;
    gchar *s;

    if (str == NULL) {
        return -1;
    }

    if (!template) {
        /* Add \n if necessary. */
        if (*(str + g_utf8_strlen(str, -1) - 1) != '\n') {
            s = g_strconcat(str, "\n", NULL);
            res = write(fd, s, g_utf8_strlen(s, -1));
            g_free(s);
        } else {
            res = write(fd, str, g_utf8_strlen(str, -1));
        }
    } else {
        s = g_strdup_printf(template, str);
        res = write(fd, s, g_utf8_strlen(s, -1));
        g_free(s);
    }

    if (res <= 0) {
        g_warning("Remote: error writing string to remote (write returned %d).",
                  res);
    }

    return res;
}

/* Selects a user from the list and asks for a password. The password is saved
   in HandlersData *handlers_data. Expected string:
   "{ \"action_id\" : \"...\" , \"message\" : \"...\", \"users\" : [...] }\n" */
static void processing_json_list_users(const gchar *str,
                                       HandlersData *handlers_data)
{
    gchar *action_id;
    gchar *message;
    gchar *selected_user;
    GList *users;

    action_id = get_json_value_str(str, JSON_KEY_ACTION_ID);
    message = get_json_value_str(str, JSON_KEY_MESSAGE);
    users = get_json_value_arr_str(str, JSON_KEY_USERS);

    selected_user = select_user_ask_pass(action_id,
                                         message,
                                         users,
                                         handlers_data);

    if (selected_user) {
        write_string_to_fd(handlers_data->pkstdin,
                           JSON_SELECTED_USER,
                           selected_user);
    } else {
        write_string_to_fd(handlers_data->pkstdin,
                           JSON_SELECTED_USER,
                           "");
        if (handlers_data->password) {
            memset(handlers_data->password,
                   0,
                   strlen(handlers_data->password));
            g_free(handlers_data->password);
            handlers_data->password = NULL;
        }
    }

    g_free(action_id);
    g_free(message);
    g_free(selected_user);
    if (users) {
        g_list_free_full(g_steal_pointer(&users), g_free);
    }
}

/* Sends a password to polkit on a remote machine.
   Expected string: "{ \"request\" : \"password\" }\n" or
   "{ \"request\" : \"%s\" }\n" */
static void processing_json_request_pass_or_other(const gchar *str,
                                                  HandlersData *handlers_data)
{
    gchar *request;

    request = get_json_value_str(str, JSON_KEY_REQUEST);
    if (request) {
        if (g_strcmp0(request, JSON_VALUE_PASSWORD) == 0) {
            if (handlers_data->password) {
                write_string_to_fd(handlers_data->pkstdin,
                                   JSON_RESPONSE,
                                   handlers_data->password);

                if (handlers_data->password) {
                    memset(handlers_data->password,
                           0,
                           strlen(handlers_data->password));

                    g_free(handlers_data->password);
                    handlers_data->password = NULL;
                }
            } else {
                write_string_to_fd(handlers_data->pkstdin,
                                   JSON_RESPONSE,
                                   "");
            }
        } else {
            g_debug("Remote: json request 'other': %s.", request);
        }
    } else {
        g_debug("Remote: wrong json request: %s.", str);
    }

    g_free(request);
}

/* Sends the result of authentication to password-agent.
   Expected string: "{ \"result\" : \"AUTHENTICATION CANCELED\" }\n"
                    "{ \"result\" : \"AUTHENTICATION COMPLETE\" }\n"
                    "{ \"result\" : \"AUTHENTICATION FAILED\" }\n"
                    "{ \"error\" : \"%s\" }\n"
                    "{ \"info\" : \"%s\" }\n" */
static void processing_json_result(const gchar *str,
                                   const gchar *key,
                                   HandlersData *handlers_data)
{
    gchar *result;

    result = get_json_value_str(str, key);
    if (result) {
        send_authentication_result(result, handlers_data);
    } else {
        g_debug("Remote: wrong json: %s.", str);
    }

    if (handlers_data->password) {
        memset(handlers_data->password,
               0,
               strlen(handlers_data->password));

        g_free(handlers_data->password);
        handlers_data->password = NULL;
    }

    g_free(result);
}

static gboolean read_pkstdout(GIOChannel   *channel,
                              GIOCondition  condition,
                              gpointer      datap)
{
    gchar *str;
    gsize length;
    gsize terminator_pos;
    gboolean res = G_SOURCE_CONTINUE;
    HandlersData *handlers_data = (HandlersData *)datap;
    GError *error = NULL;

    if (condition & G_IO_IN || condition & G_IO_HUP) {
        GIOStatus status;

        status = g_io_channel_read_line(channel, &str, &length, &terminator_pos,
                                        &error);

        if (error) {
            g_warning("Remote: read_pkstdout, %s", error->message);
            g_error_free(error);
            handlers_data->pkagent_stdout_done = TRUE;
            res = G_SOURCE_REMOVE;
        }

        if (status == G_IO_STATUS_EOF) {
            handlers_data->pkagent_stdout_done = TRUE;
            res = G_SOURCE_REMOVE;
        }

        /* Clean line break symbol in end of line. */
        if (length > 0 && str[length-1] == '\n') {
            str[length-1] = '\0';
        }


        if (str != NULL) {
            /* { "action_id" : "..." , "message" : "...", "users" : [...] }\n */
            if (g_strstr_len(str, -1, JSON_KEY_ACTION_ID) &&
                g_strstr_len(str, -1, JSON_KEY_MESSAGE) &&
                g_strstr_len(str, -1, JSON_KEY_USERS))
            {
                processing_json_list_users(str, handlers_data);
            }
            /* "{ \"request\" : \"password\" }\n" or
               "{ \"request\" : \"%s\" }\n" */
            else if (g_strstr_len(str, -1, JSON_KEY_REQUEST))
            {
                processing_json_request_pass_or_other(str, handlers_data);
            }
            /* "{ \"result\" : \"AUTHENTICATION CANCELED\" }\n"
               "{ \"result\" : \"AUTHENTICATION COMPLETE\" }\n
               "{ \"result\" : \"AUTHENTICATION FAILED\" }\n */
            else if (g_strstr_len(str, -1, JSON_KEY_RESULT)) {
                processing_json_result(str, JSON_KEY_RESULT, handlers_data);
            }
            /* "{ \"error\" : \"%s\" }\n" */
            else if (g_strstr_len(str, -1, JSON_KEY_ERROR)) {
                processing_json_result(str, JSON_KEY_ERROR, handlers_data);
            }
            /* "{ \"info\" : \"%s\" }\n" */
            else if (g_strstr_len(str, -1, JSON_KEY_INFO)) {
                processing_json_result(str, JSON_KEY_INFO, handlers_data);
            }
            /* "{ \"invalid response\" : \"%s\"}\n" */
            else if (g_strstr_len(str, -1, JSON_KEY_INVALID_RESPONSE)) {
                processing_json_result(str,
                                       JSON_KEY_INVALID_RESPONSE,
                                       handlers_data);
            }

            g_free(str);
        }
    }

    /* We will read until error or eof.
    if (condition & G_IO_HUP) {
        if (child_data->child_exited) {
            res = G_SOURCE_REMOVE;
            child_data->stderr_done = TRUE;
        }
    }*/

    if (condition & G_IO_ERR) {
        g_warning ("Remote: Error reading from pkagent's stdout.");
        handlers_data->pkagent_stdout_done = TRUE;
        res = G_SOURCE_REMOVE;
    }

    if (handlers_data->pkagent_stdout_done)
    {
        subtrees_info_table_stop_loop(handlers_data->connection_name);
    }

    return res;
}

static gpointer handle_connect(gpointer data) {
    GIOStream *stream = NULL;
    gchar *remote_address = NULL;
    gchar *connection_name = NULL;
    gchar *agent_bus_name = NULL;
    gchar *object_path = NULL; //object path for remote subtree
    gchar *pty = NULL;
    GDBusConnection *connection = NULL;
    const gchar *unique_name;
    GVariant *answer = NULL;
    guint pid_bridge = 0;
    guint subtree_id;
    gint pkstdin = -1;
    gint pkstdout = -1;
    GPid pid_pkagent = 0;
    GHashTable *xmls = NULL;
    GHashTable *nodes_info = NULL;
    SubtreeInfo *subtree_info = NULL;
    GMainContext *context = NULL;
    GMainLoop *loop = NULL;
    GSource *source = NULL;
    GIOChannel *channel = NULL;
    HandlersData *handlers_data = NULL;
    GError *error = NULL;

    ThreadFuncParam *thread_func_param = (ThreadFuncParam *)data;

    GVariant *parameters = thread_func_param->parameters;
    GDBusMethodInvocation *invocation = thread_func_param->invocation;
    g_free(thread_func_param);

    g_variant_get(parameters, "(&s&s&s&s)", &remote_address,
                                            &connection_name,
                                            &agent_bus_name,
                                            &pty);

    object_path = g_strconcat(OBJECT_ALTERATOR,
                              PART_OF_OBJECT_FOR_CONNECTION,
                              connection_name,
                              NULL);

    if (!g_variant_is_object_path(object_path)) {
        g_warning("Remote: '%s' is not an object path.", object_path);
        g_dbus_method_invocation_return_dbus_error(invocation,
                                                   INTERFACE_REMOTE,
                                                   "The connection name "
                                                   "is not correct.");
        g_free(object_path);
        return NULL;
    }

    /* Check connection_name in subtrees_info_table. */
    if (connection_name_is_exist_in_table(connection_name)) {
        g_warning("Remote: this connection name already exists (%s).",
                  connection_name);
        g_dbus_method_invocation_return_dbus_error(invocation,
                                                   INTERFACE_REMOTE,
                                                   "This connection name "
                                                   "already exists in the"
                                                   " table.");
        g_free(object_path);
        return NULL;
    }

    /* Creating a stream to a remote machine. */
    stream = spawn_stream_to_remote(remote_address);
    if (stream == NULL) {
        g_warning("Remote: spawn_stream_to_remote returned NULL (%s).",
                   remote_address);
        g_dbus_method_invocation_return_dbus_error(invocation,
                                                   INTERFACE_REMOTE,
                                                   "spawn_stream_to_remote "
                                                   "returned NULL.");
        g_free(object_path);

        return NULL;
    }

    /* Creating a new D-Bus connection to remote machine. */
    connection =
      g_dbus_connection_new_sync(stream,
                                 NULL,
                                 G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT |
                                 G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION,
                                 NULL,
                                 NULL,
                                 &error);

    if (error) {
        g_warning("Remote: error creating a new D-Bus connection: %s.",
                  error->message);

        g_dbus_method_invocation_return_dbus_error(invocation,
                                                   INTERFACE_REMOTE,
                                                   "Error creating a new D-Bus"
                                                   " connection.");
        g_free(object_path);
        g_io_stream_close(stream, NULL, NULL);
        g_error_free(error);

        return NULL;
    }
    g_object_unref(stream);

    /* Getting unique name for D-Bas connection.
       Do not free this string, it is owned by connection. */
    unique_name = g_dbus_connection_get_unique_name(connection);
    if (unique_name == NULL) {
        g_warning("Remote: error getting unique name for D-Bas connection"
                  " (%s).", remote_address);

        g_dbus_method_invocation_return_dbus_error(invocation,
                                                   INTERFACE_REMOTE,
                                                   "Error getting unique name "
                                                   "for D-Bas connection.");

        g_free(object_path);
        g_object_unref(connection);

        return NULL;
    }

    /* Getting pid for stdio-dbus-bridge. */
    answer = g_dbus_connection_call_sync(connection,
                                         "org.freedesktop.DBus",
                                         "/",
                                         "org.freedesktop.DBus",
                                         "GetConnectionUnixProcessID",
                                         g_variant_new("(s)", unique_name),
                                         NULL,
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (error) {
        g_warning("Remote: error getting pid for stdio-dbus-bridge (%s).",
                  error->message);

        g_dbus_method_invocation_return_dbus_error(invocation,
                                                   INTERFACE_REMOTE,
                                                   "Error getting pid for "
                                                   "stdio-dbus-bridge.");
        g_free(object_path);
        g_object_unref(connection);
        g_error_free(error);
        return NULL;
    }

    g_variant_get(answer, "(u)", &pid_bridge);
    g_variant_unref(answer);

    /* Spawn polkit agent on remote machine. */
    pid_pkagent = spawn_polkit_agent_on_remote(pid_bridge,
                                               remote_address,
                                               &pkstdin,
                                               &pkstdout);
    if (pid_pkagent == 0) {
        g_warning("Remote: error spawning a polkit agent on a remote machine"
                  " (%s).", remote_address);

        g_dbus_method_invocation_return_dbus_error(invocation,
                                                   INTERFACE_REMOTE,
                                                   "Error spawning a polkit "
                                                   "agent on a remote "
                                                   "machine.");
        g_free(object_path);
        g_object_unref(connection);

        return NULL;
    }

    /* Receiving introspections of objects from a remote machine. */
    xmls = receive_introspections(connection, remote_address);
    if (xmls == NULL) {
        g_warning("Remote: error receiving introspections of objects from a "
                  "remote machine (%s).", remote_address);

        g_dbus_method_invocation_return_dbus_error(invocation,
                                                   INTERFACE_REMOTE,
                                                   "Error Receiving "
                                                   "introspections of objects "
                                                   "from a remote machine.");
        g_free(object_path);
        g_object_unref(connection);

        return NULL;
    }

    /* Create hash table of GDBusNodeInfo* for xmls. */
    /* The nodes_info must be freed when SubtreeInfo is removed from
       subtrees_info_table. */
    nodes_info = create_nodes_info_for_xmls(xmls);
    g_hash_table_destroy(xmls);

    context = g_main_context_new();
    loop = g_main_loop_new(context, FALSE);

    /* Create struct SubtreeInfo and save it to subtrees_info_table. */
    /* It will be freed via a callback (subtree_info_free) when deleting form
       subtrees_info_table. */
    subtree_info = g_new0(SubtreeInfo, 1);
    subtree_info->nodes_info = nodes_info;
    subtree_info->loop = loop;
    subtree_info->pid_pkagent = pid_pkagent;

    subtrees_info_table_add_info(g_strdup(connection_name), subtree_info);

    /* User data to pass to handlers. */
    handlers_data = g_new0(HandlersData, 1);
    handlers_data->invocation = invocation;//?????????!!!!!!for connection()
    handlers_data->object_path = g_strdup(object_path);
    handlers_data->agent_bus_name = g_strdup(agent_bus_name);
    handlers_data->connection_name = g_strdup(connection_name);
    handlers_data->connection = connection;
    handlers_data->pkstdin = pkstdin;
    handlers_data->pty = g_strdup(pty);
    handlers_data->pkagent_stdout_done = FALSE;
    handlers_data->password = NULL;

    /* Save connection_name, don't forget to free it. */
    connection_name = g_strdup(handlers_data->connection_name);

    /* Read from pkagent stdout. */
    channel = g_io_channel_unix_new(pkstdout);
    source = g_io_create_watch(channel, G_IO_IN | G_IO_HUP | G_IO_ERR);

    g_source_set_callback(source, G_SOURCE_FUNC(read_pkstdout),
                          handlers_data, NULL);
    g_source_attach(source, context);
    g_source_unref(source);
    g_io_channel_unref(channel);

    /* handlers_data will free in register_subtree()
       or when unregister subtree by 'user_data_free_func'. */
    subtree_id = register_subtree(handlers_data);
    if (subtree_id) {
        g_main_loop_run(loop);

        if (!g_dbus_connection_unregister_subtree(
                                                 local_manager_data->connection,
                                                 subtree_id))
        {
            g_warning("Remote: subtree unregistration error (%s).",
                      object_path);
            handlers_data_free(handlers_data);
        }
    } else {
        handlers_data_free(handlers_data);
    }

    g_main_context_unref(context);
    g_main_loop_unref(loop);

    /* Remove the subtree entry from the subtrees_info_table. */
    subtrees_info_table_remove_info(connection_name);

    g_close(pkstdout, NULL);
    g_close(pkstdin, NULL);
    g_free(object_path);
    g_free(connection_name);
    return NULL;
}

static gpointer handle_disconnect(gpointer data) {
    gchar *connection_name;
    gchar *object_path;

    ThreadFuncParam *thread_func_param = (ThreadFuncParam *)data;

    GVariant *parameters = thread_func_param->parameters;
    GDBusMethodInvocation *invocation = thread_func_param->invocation;
    g_free(thread_func_param);

    g_variant_get(parameters, "(&s)", &connection_name);

    object_path = g_strconcat(OBJECT_ALTERATOR,
                              PART_OF_OBJECT_FOR_CONNECTION,
                              connection_name,
                              NULL);

    if (!g_variant_is_object_path(object_path)) {
        g_warning("Remote: '%s' is not an object path.", object_path);
        g_dbus_method_invocation_return_dbus_error(invocation,
                                                   INTERFACE_REMOTE,
                                                   "The connection name "
                                                   "is not correct.");
        g_free(object_path);
        return NULL;
    }

    /* Check connection_name in subtrees_info_table. */
    if (!connection_name_is_exist_in_table(connection_name)) {
        g_warning("Remote: there is no such connection name in the table (%s).",
                  connection_name);
        g_dbus_method_invocation_return_dbus_error(invocation,
                                                   INTERFACE_REMOTE,
                                                   "There is no such connection"
                                                   " name in the table.");
        g_free(object_path);
        return NULL;
    }

    /* Deleting a record from the table will be done in the handle_connect. */
    if (subtrees_info_table_kill_pkagent(connection_name)) {
        g_dbus_method_invocation_return_value(invocation,
                                              g_variant_new("(b)", TRUE));
    } else {
        g_dbus_method_invocation_return_value(invocation,
                                              g_variant_new("(b)", FALSE));
    }

    g_free(object_path);
    return NULL;
}

static gpointer handle_get_connections(gpointer data) {
    GVariantBuilder *builder;
    GList *list;

    builder = g_variant_builder_new(G_VARIANT_TYPE ("(as)"));

    ThreadFuncParam *thread_func_param = (ThreadFuncParam *)data;

    GDBusMethodInvocation *invocation = thread_func_param->invocation;
    g_free(thread_func_param);

    g_mutex_lock(&mutex);
    list = g_hash_table_get_keys(subtrees_info_table);
    g_variant_builder_open(builder, G_VARIANT_TYPE ("as"));

    for (GList *a = list; a; a = a->next) {
        g_variant_builder_add(builder, "s", (gchar*) a->data);
    }

    g_variant_builder_close(builder);

    g_dbus_method_invocation_return_value(invocation,
                                          g_variant_builder_end(builder));
    g_mutex_unlock(&mutex);

    g_variant_builder_unref(builder);
    g_list_free(list);

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
    GThread *thread = NULL;
    ThreadFuncParam *thread_func_param = NULL;
    /* Don't forget to free up memory. (It's freed at the beginning of the
       handle_*().) */
    thread_func_param = g_new0(ThreadFuncParam, 1);

    if (g_strcmp0(method_name, "Connect") == 0) {
        thread_func_param->parameters = parameters;
        thread_func_param->invocation = invocation;

        thread = g_thread_new("Method Connect",
                              handle_connect,
                              thread_func_param);
    } else if (g_strcmp0(method_name, "Disconnect") == 0) {
        thread_func_param->parameters = parameters;
        thread_func_param->invocation = invocation;
        thread = g_thread_new("Method Disconnect",
                              handle_disconnect,
                              thread_func_param);
    } else if (g_strcmp0(method_name, "GetConnections") == 0) {
        thread_func_param->invocation = invocation;
        thread = g_thread_new("Method GetConnections",
                              handle_get_connections,
                              thread_func_param);
    } else {
        g_debug("Remote: there is no handler for the '%s' method.",
                method_name);
        return;
    }

    g_thread_unref(thread);

}

static const GDBusInterfaceVTable interface_vtable =
{
    handle_method_call,
    NULL,
    NULL,
    { 0 }
};

static gboolean module_init(ManagerData *manager_data) {
    GHashTable *interfaces = NULL;
    InterfaceObjectInfo *info = NULL;

    g_debug("Initialize alterator module %s.", PLUGIN_NAME);

    if (!manager_data->user_mode) {
        g_debug("Remote: the remote only works in user mode.\n"
                "FALSE will be returned.");
        return FALSE;
    }

    subtrees_info_table = g_hash_table_new_full(g_str_hash,
                                                g_str_equal,
                                                g_free,
                                                subtree_info_free);
    if (subtrees_info_table == NULL) {
        g_warning("Remote: g_hash_table_new_full returned NULL for "
                  "subtrees_info_table.");
        return FALSE;
    }

    /* If there is the connection node, then we exit. */
    interfaces = (GHashTable *)g_hash_table_lookup(manager_data->backends_data,
                                                   NODE_CONNECTION);
    if (interfaces) {
        g_warning("Remote: node \'%s\' already exists.", NODE_CONNECTION);

        return FALSE;

    }

    /* If there is the remote node and it has the remote interface, then
       we exit. Otherwise, we create both (if necessary). */
    interfaces = (GHashTable *)g_hash_table_lookup(manager_data->backends_data,
                                                   NODE_REMOTE);
    if (interfaces) {
        info = (InterfaceObjectInfo *)g_hash_table_lookup (interfaces,
                                                           INTERFACE_REMOTE);
        if (info) {
            g_warning("Remote: interface \'%s\' already exists in node \'%s\'.",
                      INTERFACE_REMOTE, NODE_REMOTE);
            return FALSE;
        }
    }

    GDBusNodeInfo *introspection_data = NULL;
    introspection_data = g_dbus_node_info_new_for_xml (xml_string, NULL);
    if (introspection_data == NULL) {
        g_warning("Remote: GDBusNodeInfo * == NULL (node = %s, "
                  "interface = %s).", NODE_REMOTE, INTERFACE_REMOTE);

        return FALSE;
    }

    GDBusInterfaceInfo *interface_info = NULL;
    interface_info = g_dbus_node_info_lookup_interface (introspection_data,
                                                        INTERFACE_REMOTE);
    if (interface_info == NULL) {
        g_warning("Remote: GDBusInterfaceInfo * == NULL (node = %s, "
                  "interface = %s).", NODE_REMOTE, INTERFACE_REMOTE);

        g_dbus_node_info_unref(introspection_data);
        return FALSE;
    }

    info = g_new0(InterfaceObjectInfo, 1);
    info->interface_introspection = interface_info;
    info->interface_vtable = &interface_vtable;
    info->module_name = g_strdup(PLUGIN_NAME);
    /* We do not use the following fields in this module. */
    info->action_id = NULL;
    info->thread_limit = 0;
    info->methods = NULL;

    if (!interfaces) {
        interfaces = g_hash_table_new_full(g_str_hash,
                                           g_str_equal,
                                           g_free,
                                           interface_info_free);
        if (interfaces == NULL) {
            g_warning("Remote: g_hash_table_new_full returned NULL for %s.",
                      NODE_REMOTE);
            g_dbus_node_info_unref(introspection_data);
            interface_info_free(info);
            return FALSE;
        }

        g_hash_table_replace(manager_data->backends_data, g_strdup(NODE_REMOTE),
                             interfaces);
    }

    g_hash_table_replace(interfaces, g_strdup(INTERFACE_REMOTE), info);

    local_manager_data = manager_data;

    return TRUE;
};

static gint module_interface_version() {
    return alterator_module_interface_version();
}

static void module_destroy() {
    g_warning("deinitialize alterator module %s.", PLUGIN_NAME);
}

static gboolean is_module_busy()
{
    guint number;

    g_mutex_lock(&mutex);
    number = g_hash_table_size(subtrees_info_table);
    g_mutex_unlock(&mutex);

    if (number) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static AlteratorModuleInterface module_interface =
{
    .interface_version = module_interface_version,
    .init              = module_init,
    .destroy           = module_destroy,
    .is_module_busy    = is_module_busy
};

gboolean alterator_module_init(AlteratorManagerInterface *interface) {
    if (interface == NULL || interface->register_module == NULL) {
        g_warning("Module '%s' initialization failed - invalid "
                  "AlteratorModuleInterface.", PLUGIN_NAME);
        return FALSE;
    }

    if (interface->interface_validation == NULL) {
        g_warning("Module '%s' initialization failed - invalid "
                  "AlteratorModuleInterface. interface_validation == NULL.",
                  PLUGIN_NAME);
        return FALSE;
    }

    manager_interface = interface;

    if (!interface->register_module(&module_interface)) {
        g_warning("Register module '%s' failed.", PLUGIN_NAME);
        return FALSE;
    }

    return TRUE;
}
