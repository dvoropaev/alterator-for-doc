/*Module for alterator-manager (backend3).*/

#include "backend3.h"

static const gchar xml_top[] =
  "<node>"
  "  <interface name='";

static const gchar xml_top_end[] =
  "'>";

static const gchar xml_bottom[] =
  "  </interface>"
  "</node>";

static const gchar xml_method_name[] =
  "    <method name='";

static const gchar xml_method_name_end[] =
  "'>";

static const gchar xml_args[] =
  "      <arg type='a{ss}' name='input' direction='in'/>"
  "      <arg type='aa{ss}' name='output' direction='out'/>"
  "    </method>";

/* (gchar *)node -> (GHashTable *)interfaces */
/* (gchar *)interface -> (GHashTable *)methods */
static GHashTable *methods_table = NULL;
static PolkitAuthority *polkit_authority = NULL;
static AlteratorManagerInterface *manager_interface = NULL;
/* backend3 name -> ChildData */
static GHashTable *backend3_table = NULL;
static GMutex backend3_table_mutex;

/* The str is assumed to point to the first character of the key. */
static gchar *read_pair(GVariantBuilder *builder,
                      gchar *str,
                      GString *data,
                      MethodData *method_data)
{
    gchar *tmp = str;
    gchar *key = NULL;
    gchar *value = NULL;
    gchar *_key;
    gchar *_value;

    /* Read a key. */
    while (*str != '\0' && !isspace(*str)) {
        str++;
    }

    key = g_strndup(tmp, str - tmp);

    if (*str == '\0') {
        g_warning("Backend3: output parsing error: unexpected end of line.\n"
                  "\nObject path: %s\nInterface name: %s\nMethod name: %s",
                  method_data->object_path, method_data->interface_name,
                  method_data->method_name);

        goto out;
    }

    /* We reach the quotation marks. */
    while (isspace(*str)) {
        str++;
    }

    /*if (*str == '\0') {
        g_warning("Backend3: output parsing error (backend3 name: %s). "
                  "Unexpected end of line(2).", backend3_name);

        goto out;
    }*/

    if (*str != '"') {
        g_warning("Backend3: output parsing error: unexpected character in "
                  "key/value pair (character code: %d).\nObject path: %s\n"
                  "Interface name: %s\nMethod name: %s", *str,
                  method_data->object_path, method_data->interface_name,
                  method_data->method_name);
        goto out;
    } else {
        str++;
        tmp = str;
    }
    /* Now str should point to the first character of the value. */

    /* Read a value. */
    while (*str != '\0') {
        if (*str == '"' && *(str - 1) != '\\') {
            value = g_strndup(tmp, str - tmp);
            str++;
            break;
        }
        str++;
    }

    out:

    _key = key ? key : "";
    _value = value ? value : "";

    g_variant_builder_add(builder, "{ss}", _key, _value);

    g_free(key);
    g_free(value);

    return str;
}

static void add_data_to_builder(GVariantBuilder *builder,
                                MethodData *method_data)
{
    GString *data = method_data->stdout_bytes;
    gchar *str = data->str;

    /* Skip the first bracket. */
    while ((str - data->str) < data->len) {
        if (*str != '(') {
            str++;
        } else {
            str++;
            break;
        }
    }

    /* Each key/value pair outside the parentheses is placed in a separate
       array. Key/value pairs inside the parentheses are placed together in a
       single array. */
    gboolean single = TRUE;
    while ((str - data->str) < data->len) {
        if (*str == '(' && single) {
            g_variant_builder_open(builder, G_VARIANT_TYPE("a{ss}"));
            single = FALSE;
            str++;
        } else if (*str == ')' && !single) {
            g_variant_builder_close(builder);
            single = TRUE;
            str++;
        } else if (isalnum(*str) || *str == '_') {
            if (single) {
                g_variant_builder_open(builder, G_VARIANT_TYPE("a{ss}"));
                str = read_pair(builder, str, data, method_data);
                g_variant_builder_close(builder);
            } else {
                str = read_pair(builder, str, data, method_data);
            }
        } else {
            if (!isspace(*str) && *str != ')') {
                g_warning("Backend3: output parsing error, an unexpected "
                          "symbol: %c\nObject path: %s\nInterface name: %s\n"
                          "Method name: %s", *str, method_data->object_path,
                          method_data->interface_name,
                          method_data->method_name);
            }
            str++;
        }
    }

    if (!single) {
        g_warning("Backend3: output parsing error\nObject path: %s\nInterface "
                  "name: %s\nMethod name: %s", method_data->object_path,
                  method_data->interface_name, method_data->method_name);
        g_variant_builder_close(builder);
    }
}

static void invocation_return_value(MethodData *method_data) {
    GVariantBuilder *builder;

    builder = g_variant_builder_new(G_VARIANT_TYPE(OUTPUT_TYPE));

    g_variant_builder_open(builder, G_VARIANT_TYPE("aa{ss}"));

    add_data_to_builder(builder, method_data);

    g_variant_builder_close(builder);

    g_dbus_method_invocation_return_value(method_data->invocation,
                                          g_variant_builder_end(builder));
    g_variant_builder_unref(builder);
}

static void method_data_free(gpointer data) {
    MethodData *method_data = (MethodData *)data;

    if (!method_data) {
        return;
    }

    g_free(method_data->to_stdin);
    g_free(method_data->backend3);
    g_free(method_data->object_path);
    g_free(method_data->interface_name);
    g_free(method_data->method_name);


    if (method_data->stdout_bytes) {
        g_string_free(method_data->stdout_bytes, TRUE);
    }

    if (method_data->timeout_source) {
        g_source_destroy(method_data->timeout_source);
        g_source_unref(method_data->timeout_source);
    }

    g_main_loop_unref(method_data->loop);
    g_main_context_unref(method_data->context);

    g_free(method_data);
}

static void child_data_free(gpointer data) {
    ChildData *child_data = (ChildData *)data;

    if (!child_data) {
        return;
    }

    /* Memory allocated for method_data (child_data->method_data) is freed
       separately when the method completes. */

    g_free(child_data->backend3);

    g_mutex_clear(&child_data->mutex);
    g_cond_clear(&child_data->cond_data);

    g_main_loop_unref(child_data->loop);
    g_main_context_unref(child_data->context);

    g_free(child_data);
}

static gboolean kill_by_pid(gpointer data) {
    ChildData *child_data = (ChildData *)data;

    if (!child_data) {
        return G_SOURCE_REMOVE;
    }

    if (kill(child_data->pid, SIGKILL) == -1) {
        gint err = errno;
        switch (err) {
            case EINVAL:
                g_warning("Backend3: kill_by_pid(), an invalid signal was "
                          "specified. (kill(), errno == EINVAL)");
                break;
            case EPERM:
                g_warning("Backend3: kill_by_pid(), the calling process "
                          "does not have permission to send the signal. "
                          "(kill(), errno == EPERM)");
                break;
            case ESRCH:
                g_warning("Backend3: kill_by_pid(), the target process "
                          "does not exist, pid == %i. (kill(), errno == ESRCH)",
                          child_data->pid);
                break;
            default:
                g_warning("Backend3: kill_by_pid(), unspecified error. "
                          "(kill(), errno == %i)", err);
        }
    }

    return G_SOURCE_REMOVE;
}

static gboolean kill_if_not_used(gpointer data) {
    ChildData *child_data = (ChildData *)data;

    if (!child_data->method_data) {
        return kill_by_pid(child_data);
    }

    return G_SOURCE_CONTINUE;
}

static void check_stdout_chars(MethodData *method_data,
                               char *buf,
                               gsize length)
{
    char *p = buf;
    gsize len = length;

    for (;length > 0; length--, p++) {
        if (method_data->double_quotes && *p != '"') {
            continue;
        }

        if (method_data->quotes && *p != '\'') {
            continue;
        }

        if (*p == '"' && *(p - 1) != '\\') {
            if (method_data->double_quotes) {
                method_data->double_quotes = FALSE;
            } else {
                method_data->double_quotes = TRUE;
            }
        }

        if (*p == '\'' && *(p - 1) != '\\') {
            if (method_data->quotes) {
                method_data->quotes = FALSE;
            } else {
                method_data->quotes = TRUE;
            }
        }

        if (*p == '(') {
            if (!method_data->outer_brackets) {
                method_data->outer_brackets = TRUE;
            } else if (!method_data->inner_brackets) {
                method_data->inner_brackets = TRUE;
            } else {
                buf[len] = '\0';
                g_warning("Backend3: check_stdout_chars(), parsing error.\n"
                          "buf[]:%s\nObject path: %s\nInterface name: %s\n"
                          "Method name: %s", buf, method_data->object_path,
                          method_data->interface_name,
                          method_data->method_name);
                method_data->stdout_done = TRUE;
                return;
            }
        }

        if (*p == ')') {
            if (method_data->inner_brackets) {
                method_data->inner_brackets = FALSE;
            } else if (method_data->outer_brackets) {
                method_data->stdout_done = TRUE;
            } else {
                buf[len] = '\0';
                g_warning("Backend3: check_stdout_chars(), parsing error.\n"
                          "buf[]:%s\nObject path: %s\nInterface name: %s\n"
                          "Method name: %s", buf, method_data->object_path,
                          method_data->interface_name,
                          method_data->method_name);
                method_data->stdout_done = TRUE;
                return;
            }
        }
    }
}

static gboolean on_child_stdout(GIOChannel   *channel,
                                GIOCondition  condition,
                                gpointer      datap)
{
    GError *error = NULL;
    gchar *buf = NULL;
    gsize length;
    GIOStatus status;
    ChildData *child_data = (ChildData *)datap;
    MethodData *method_data = child_data->method_data;
    gboolean res = G_SOURCE_CONTINUE;
    const gchar *object_path;
    const gchar *interface_name;
    const gchar *method_name;
    gboolean stdout_done = FALSE;

    if (method_data) {
        object_path = method_data->object_path;
        interface_name = method_data->interface_name;
        method_name = method_data->method_name;
    } else {
        object_path = "-";
        interface_name = "-";
        method_name = "-";
    }

    if (condition & G_IO_IN || condition & G_IO_HUP) {
        status = g_io_channel_read_line(channel, &buf, &length, NULL, &error);

        if (error) {
            g_warning("Backend3: on_child_stdout, error read chars: %s\nObject "
                      "path: %s\nInterface name: %s\nMethod name: %s",
                      error->message, object_path,
                      interface_name, method_name);
            g_error_free(error);
            error = NULL;
            stdout_done = TRUE;
            res = G_SOURCE_REMOVE;
            goto out;
        }

        if (status == G_IO_STATUS_EOF) {
            if (method_data) {
                g_warning("Backend3: Error reading from child stdin (status =="
                          " EOF).\nObject path: %s\nInterface name: %s\n"
                          "Method name: %s",
                          object_path, interface_name, method_name);
            }

            stdout_done = TRUE;
            res = G_SOURCE_REMOVE;
        }

        if (method_data && length && !method_data->stdout_done) {
            if (method_data->stdout_bytes == NULL) {
                method_data->stdout_bytes = g_string_new(NULL);
            }

            g_string_append_len(method_data->stdout_bytes, buf,
                                (gssize) length);

            check_stdout_chars(method_data, buf, length);
            if (method_data->stdout_done) {
                invocation_return_value(method_data);
                g_main_loop_quit(method_data->loop);
            }
        }

    }

    if (condition & G_IO_ERR) {
        g_warning("Backend3: Error reading from child stdin.\nObject path: %s"
                  "\nInterface name: %s\nMethod name: %s",
                  object_path, interface_name, method_name);
        stdout_done = TRUE;
        res = G_SOURCE_REMOVE;
    }

    out:
    if (stdout_done) {
        kill_by_pid(child_data);
    }

    g_free(buf);
    return res;
}

static gboolean for_cond_signal(gpointer data) {
    ChildData *child_data = (ChildData *)data;

    g_mutex_lock(&child_data->mutex);

    /* We send a signal to the waiting thread that the child thread
       initialization is complete. */
    g_cond_signal(&child_data->cond_data);
    g_mutex_unlock(&child_data->mutex);

    return G_SOURCE_REMOVE;
}

static gboolean on_child_exited(GPid pid, gint wait_status, gpointer data) {
    ChildData *child_data = (ChildData *)data;

    MethodData *method_data = child_data->method_data;
    if (method_data && g_main_loop_is_running(method_data->loop)) {
        g_dbus_method_invocation_return_dbus_error(method_data->invocation,
                                                   DBUS_NAME,
                                                   "Error: backend3 is down.");
        g_main_loop_quit(method_data->loop);
    }

    g_main_loop_quit(child_data->loop);

    return G_SOURCE_REMOVE;
}

static gboolean authorization_check(MethodData *method_data) {
    gboolean result = FALSE;
    PolkitSubject *subject = NULL;
    PolkitCheckAuthorizationFlags flags =
                        POLKIT_CHECK_AUTHORIZATION_FLAGS_ALLOW_USER_INTERACTION;
    PolkitAuthorizationResult *auth_result = NULL;
    GError *error = NULL;

    subject = polkit_system_bus_name_new(
                 g_dbus_method_invocation_get_sender(method_data->invocation));

    gchar *action_id = method_data->method_info->action_id;

    auth_result = polkit_authority_check_authorization_sync(polkit_authority,
                                                            subject,
                                                            action_id,
                                                            NULL,//details,
                                                            flags,
                                                            NULL, /* GCancellable* */
                                                            &error);

    if (auth_result == NULL) {
        g_warning("Backend3: check authorization result == NULL (%s).",
                  error->message);
        g_dbus_method_invocation_return_dbus_error(method_data->invocation,
                                                   DBUS_NAME,
                                         "Check authorization result == NULL.");
    } else if (!polkit_authorization_result_get_is_authorized(auth_result)) {
        g_dbus_method_invocation_return_dbus_error(method_data->invocation,
                                                   DBUS_NAME,
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

static gboolean start_backend3(ChildData *child_data,
                               const gchar *object_path,
                               const gchar *interface_name,
                               const gchar *method_name)
{
    gchar *command_line;
    GSpawnFlags flags;
    gchar **argv;
    GPid child_pid;
    gint child_stdin;
    gint child_stdout;
    GError *error = NULL;

    command_line = g_strconcat(BACKEND3_PATH, child_data->backend3, NULL);
    g_shell_parse_argv(command_line, NULL, &argv, &error);
    g_free(command_line);
    if (error) {
        g_warning("Backend3: start_backend3(), error: %s.\nObject path:"
                  " %s\nInterface name: %s\nMethod name: %s",
                  error->message, object_path, interface_name, method_name);
        g_error_free(error);

        return FALSE;
    }

    flags = G_SPAWN_SEARCH_PATH |
            G_SPAWN_DO_NOT_REAP_CHILD |
            G_SPAWN_STDERR_TO_DEV_NULL;

    g_spawn_async_with_pipes(NULL, argv, NULL, flags, NULL, NULL,
                             &child_pid, &child_stdin, &child_stdout, NULL,
                             &error);


    g_strfreev(argv);

    if (error) {
        g_warning("Backend3: start_backend3(), g_spawn_async_with_pipes(), "
                  "error: %s.\nObject path: %s\nInterface name: %s\n"
                  "Method name: %s", error->message, object_path,
                  interface_name, method_name);
        g_error_free(error);

        return FALSE;
    }

    child_data->stdout_pipe_out = child_stdout;
    child_data->stdin_pipe_out = child_stdin;
    child_data->pid = child_pid;

    return TRUE;
}

static gpointer backend3_thread(gpointer data) {
    ChildData *child_data = (ChildData *)data;
    GIOChannel *channel;
    GSource *source;
    GError *error = NULL;

    /* The callback must be called after the loop starts to send a signal to
       the method handler. */
    source = g_idle_source_new();
    g_source_set_priority(source, G_PRIORITY_DEFAULT);
    g_source_set_callback(source, G_SOURCE_FUNC(for_cond_signal), child_data,
                          NULL);
    g_source_attach(source, child_data->context);
    g_source_unref(source);

    source = g_child_watch_source_new(child_data->pid);
    g_source_set_callback(source, G_SOURCE_FUNC(on_child_exited), child_data,
                          NULL);
    g_source_attach(source, child_data->context);
    g_source_unref(source);

    /* time-out */
    source = g_timeout_source_new_seconds(CHILD_TIMEOUT);
    g_source_set_callback(source,
                          G_SOURCE_FUNC(kill_if_not_used),
                          child_data,
                          NULL);
    g_source_attach(source, child_data->context);
    g_source_unref(source);

    /* stdout */
    channel = g_io_channel_unix_new(child_data->stdout_pipe_out);

    g_io_channel_set_line_term(channel, ")\n\0", -1);
    g_io_channel_set_encoding(channel, NULL, &error);
    if (error) {
        g_warning("Backend3: g_io_channel_set_encoding() error: %s\n"
                  "Backend3 name: %s",
                  error->message, child_data->backend3);
        g_error_free(error);
        error = NULL;
    }

    source = g_io_create_watch(channel, G_IO_IN | G_IO_HUP | G_IO_ERR);
    g_source_set_callback(source, G_SOURCE_FUNC(on_child_stdout),
                          child_data, NULL);
    g_source_attach(source, child_data->context);
    g_source_unref(source);
    g_io_channel_unref(channel);

    g_main_loop_run(child_data->loop);

    /* If the method thread is not completed, then we wait. */
    g_mutex_lock(&child_data->mutex);
    g_mutex_unlock(&child_data->mutex);

    g_mutex_lock(&backend3_table_mutex);
    if (!g_hash_table_remove(backend3_table, child_data->backend3)) {
        g_warning("Backend3: error deleting %s from table.",
                  child_data->backend3);

        child_data_free(child_data);
    }
    g_mutex_unlock(&backend3_table_mutex);

    return NULL;
}

static void set_method_timeout(ChildData *child_data) {
    GSource *source;

    source = g_timeout_source_new_seconds(
                                 child_data->method_data->method_info->timeout);
    g_source_set_callback(source, G_SOURCE_FUNC(kill_by_pid), child_data, NULL);
    g_source_attach(source, child_data->context);

    child_data->method_data->timeout_source = source;
}

static void write_to_stdin(ChildData *child_data) {
    MethodData *method_data = child_data->method_data;

    gssize len = strlen(method_data->to_stdin);
    const gchar *p = method_data->to_stdin;

    while (len > 0) {
        gssize done = write(child_data->stdin_pipe_out, p, len);
        gint err = errno;

        if (done < 0) {
            g_warning("Backend3: error writing to stdin (errno = %d)\nObject "
                      "path: %s\nInterface name: %s\nMethod name: %s", err,
                      method_data->object_path, method_data->interface_name,
                      method_data->method_name);
            break;
        } else if (done == 0) {
            g_warning("Backend3: error writing to stdin (done == 0)\nObject "
                      "path: %s\nInterface name: %s\nMethod name: %s",
                      method_data->object_path, method_data->interface_name,
                      method_data->method_name);
            break;
        }

        if (len < (size_t) done) {
            g_warning("Backend3: wite to many bytes!\nObject "
                      "path: %s\nInterface name: %s\nMethod name: %s",
                      method_data->object_path, method_data->interface_name,
                      method_data->method_name);
            break;
        }
        len -= done;
        p += done;
    }

    memset(method_data->to_stdin, '\0', strlen(method_data->to_stdin));
    g_free(method_data->to_stdin);
    method_data->to_stdin = NULL;
}

static gpointer run_method(gpointer data) {
    MethodData *method_data = (MethodData *)data;
    ChildData *child_data = NULL;

    if (!authorization_check(method_data)) {
        method_data_free(method_data);
        return NULL;
    }

    g_mutex_lock(&backend3_table_mutex);
    child_data = (ChildData *) g_hash_table_lookup(backend3_table,
                                                   method_data->backend3);
    g_mutex_unlock(&backend3_table_mutex);

    if (!child_data) {
        g_dbus_method_invocation_return_dbus_error(method_data->invocation,
                                                   DBUS_NAME,
                                               "Backend3 is not in the table.");

        g_debug("Backend3: %s is not in the table. Object path:%s\n"
                "Interface name:%s\nMethod name:%s", method_data->backend3,
                method_data->object_path, method_data->interface_name,
                method_data->method_name);

        method_data_free(method_data);

        return NULL;
    }

    /* Here the child process's mutex will be locked. */
    if (!g_mutex_trylock(&child_data->mutex)) {
        g_dbus_method_invocation_return_dbus_error(method_data->invocation,
                                                   DBUS_NAME,
                                                   "Backend3 is busy.");

        g_debug("Backend3: backend3 \'%s\' is busy. Object path:%s\n"
                "Interface name:%s\nMethod name:%s", method_data->backend3,
                method_data->object_path, method_data->interface_name,
                method_data->method_name);

        method_data_free(method_data);

        return NULL;
    }

    /* Store method data in backend3 process data.
       After the method completes, the memory allocated for this data must be
       freed and the field must be assigned null. */
    child_data->method_data = method_data;

    /* We set a timer for the method.
       After the method completes, the timer should be removed. */
    set_method_timeout(child_data);

    /* Write ot stdin_pipe_out. */
    write_to_stdin(child_data);

    g_main_loop_run(method_data->loop);

    method_data_free(method_data);
    child_data->method_data = NULL;

    /* Here the child process mutex is unlocked. */
    g_mutex_unlock(&child_data->mutex);

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
    GHashTable *interfaces;
    ChildData *child_data = NULL;
    InterfaceInfo *interface_info;
    MethodInfo *method_info;
    const gchar *node_name;
    GThread *thread;
    MethodData *method_data;
    GVariantIter iter;
    GVariant *param_array;
    gchar *key;
    gchar *value;

    /* Search through the table of known methods. A method is identified by
       a node name, an interface name, and a method name. */
    node_name = object_path + g_utf8_strlen(OBJECT_PATH_PREFIX, -1);
    interfaces = g_hash_table_lookup(methods_table, node_name);
    if (!interfaces) {
        g_warning("Backend3: wrong object_path (%s).", object_path);
        g_dbus_method_invocation_return_dbus_error(invocation,
                                                   DBUS_NAME,
                                                   "Wrong object path.");
        return;
    }

    interface_info = g_hash_table_lookup(interfaces, interface_name);
    if (!interface_info) {
        g_warning("Backend3: wrong interface name (%s).", interface_name);
        g_dbus_method_invocation_return_dbus_error(invocation,
                                                   DBUS_NAME,
                                                   "Wrong interface name.");
        return;
    }

    method_info = g_hash_table_lookup(interface_info->methods, method_name);
    if (!method_info) {
        g_warning("Backend3: wrong method name (%s).", method_name);
        g_dbus_method_invocation_return_dbus_error(invocation,
                                                   DBUS_NAME,
                                                   "Wrong method name.");
        return;
    }

    if (!method_info->backend3 || g_utf8_strlen(method_info->backend3, 2) < 2) {
        g_warning("Backend3: wrong backend3 field.\nObject path: %s\n"
                  "Interface name: %s\nMethod name: %s", object_path,
                  interface_name, method_name);
        g_dbus_method_invocation_return_dbus_error(invocation,
                                                   DBUS_NAME,
                                                   "Wrong backend3 field.");
        return;
    }

    GString *to_stdin = g_string_new("");

    /* _message:begin */
    g_string_append(to_stdin, MESSAGE_BEGIN);

    /* _objects: */
    if (method_info->_objects) {
        g_string_append_printf(to_stdin,
                               MESSAGE_OBJECTS,
                               method_info->_objects);
    }

    /* action */
    if (method_info->action) {
        g_string_append_printf(to_stdin, MESSAGE_ACTION, method_info->action);
    }

    /* language */
    if (method_info->language) {
        g_string_append_printf(to_stdin,
                               MESSAGE_LANGUAGE,
                               method_info->language);
    }

    /* parameters */
    g_variant_get(parameters, "(@a{ss})", &param_array);
    g_variant_iter_init(&iter, param_array);
    while (g_variant_iter_loop(&iter, "{ss}", &key, &value)) {
        g_string_append_printf(to_stdin,
                               MESSAGE_TEMPLATE,
                               key, value);
    }
    g_variant_unref(param_array);

    /* _message:end */
    g_string_append(to_stdin, MESSAGE_END);

    method_data = g_new0(MethodData, 1);
    method_data->invocation = invocation;
    method_data->context = g_main_context_new();
    method_data->loop = g_main_loop_new(method_data->context, TRUE);
    method_data->object_path = g_strdup(object_path);
    method_data->interface_name = g_strdup(interface_name);
    method_data->method_name = g_strdup(method_name);
    method_data->to_stdin = g_string_free(to_stdin, FALSE);
    method_data->backend3 = g_strdup(method_info->backend3);
    method_data->stdout_byte_limit = method_info->stdout_byte_limit;
    method_data->method_info = method_info; //It doesn't need to be freed
    method_data->timeout_source = NULL;
    method_data->stdout_done = FALSE;
    method_data->outer_brackets = FALSE;
    method_data->inner_brackets = FALSE;
    method_data->double_quotes = FALSE;
    method_data->quotes = FALSE;

    /* If the backend is not running, we start it and create a data structure
       for it. */
    g_mutex_lock(&backend3_table_mutex);
    child_data = (ChildData *) g_hash_table_lookup(backend3_table,
                                                   method_info->backend3);
    g_mutex_unlock(&backend3_table_mutex);

    if (!child_data) {
        child_data = g_new0(ChildData, 1);
        child_data->context = g_main_context_new();
        child_data->loop = g_main_loop_new(child_data->context, FALSE);
        child_data->backend3 = g_strdup(method_info->backend3);
        g_mutex_init(&child_data->mutex);
        g_cond_init(&child_data->cond_data);
        child_data->method_data = NULL;

        if (start_backend3(child_data,
                           object_path,
                           interface_name,
                           method_name))
        {
            g_mutex_lock(&backend3_table_mutex);
            g_hash_table_replace(backend3_table,
                                 g_strdup(method_info->backend3),
                                 child_data);
            g_mutex_unlock(&backend3_table_mutex);

            thread = g_thread_new("backend3_child",
                                  backend3_thread,
                                  child_data);
            g_thread_unref(thread);

            /* We wait until the child's main loop starts. */
            g_mutex_lock(&child_data->mutex);
            while (!g_main_loop_is_running(child_data->loop)) {
                g_cond_wait(&child_data->cond_data, &child_data->mutex);
            }
            g_mutex_unlock(&child_data->mutex);
        } else {
            child_data_free(child_data);
            method_data_free(method_data);

            g_dbus_method_invocation_return_dbus_error(invocation,
                                                       DBUS_NAME,
                                                    "Error starting backend3.");
            g_warning("Backend3: error starting backend3. Object path:%s\n"
                      "Interface name:%s\nMethod name:%s", object_path,
                      interface_name, method_name);
            return;
        }
    }

    thread = g_thread_new("backend3", run_method, method_data);
    g_thread_unref(thread);
}

static const GDBusInterfaceVTable interface_vtable =
{
    handle_method_call,
    NULL,
    NULL,
    { 0 }
};

static void hashtable_free(void *hashtable) {
    GHashTable *table = (GHashTable *) hashtable;

    g_hash_table_destroy(table);
}

static void interface_info_free(void *interface_info) {
    InterfaceInfo *info = (InterfaceInfo *)interface_info;

    if (!info) {
        return;
    }

    if (info->methods) {
        g_hash_table_destroy(info->methods);
    }

    g_free(info);
}

static void method_info_free(void *method_info) {
    MethodInfo *info = (MethodInfo *)method_info;

    if (!info) {
        return;
    }

    g_free(info->method_name);
    g_free(info->backend3);
    g_free(info->action_id);
    g_free(info->_objects);
    g_free(info->action);
    g_free(info->language);

    if (info->environment) {
        g_hash_table_destroy(info->environment);
    }

    g_free(info);
}

static void environment_info_free(void *environment_info) {
    EnvironmentObjectInfo *info = (EnvironmentObjectInfo *)environment_info;

    if (info == NULL) {
        return;
    }

    g_free(info->default_value);

    g_free(info);
}

static void add_method_to_table(const gchar *node_name,
                                const gchar *interface_name,
                                const gchar *method_name,
                                MethodInfo *method_info,
                                gint interface_thread_limit)
{
    GHashTable *interfaces = NULL;
    InterfaceInfo *interface_info = NULL;

    if (methods_table == NULL) {
        methods_table = g_hash_table_new_full(g_str_hash,
                                              g_str_equal,
                                              g_free,
                                              hashtable_free);
    }

    if (g_hash_table_contains(methods_table, node_name)) {
        interfaces =
                   (GHashTable *) g_hash_table_lookup(methods_table, node_name);
    } else {
        interfaces = g_hash_table_new_full(g_str_hash,
                                           g_str_equal,
                                           g_free,
                                           interface_info_free);

        g_hash_table_replace(methods_table, g_strdup(node_name), interfaces);
    }

    if (g_hash_table_contains(interfaces, interface_name)) {
        interface_info =
              (InterfaceInfo *) g_hash_table_lookup(interfaces, interface_name);
    } else {
        interface_info = g_new0(InterfaceInfo, 1);
        interface_info->thread_limit = interface_thread_limit;
        interface_info->thread_counter = 0;
        interface_info->methods = g_hash_table_new_full(g_str_hash,
                                                        g_str_equal,
                                                        g_free,
                                                        method_info_free);

        g_hash_table_replace(interfaces,
                             g_strdup(interface_name),
                             interface_info);
    }

    g_hash_table_replace(interface_info->methods,
                         g_strdup(method_name),
                         method_info);
}

/* Validates a valid name for an action_id name. The name can only contain
   letters, numbers, and dashes. It is assumed that the length of the name
   cannot be greater than 100 and less than 1. */
static gboolean is_correct_action_id(gchar* name) {
    if (name == NULL) {
        return FALSE;
    }

    if (g_utf8_strlen(name, 2) < 1) {
        return FALSE;
    }

    for (int i = 0; name[i] != '\0'; i++) {
        if (!g_ascii_isalnum(name[i]) && name[i] != '-' && name[i] != '.') {
            return FALSE;
        }

        if (i >= MAX_NAME_LENGTH - 1) {
            return FALSE;
        }
    }

    return TRUE;
}

static void copy_environment(MethodInfo *method_info, GHashTable *envr) {
    EnvironmentObjectInfo *dst_info;
    EnvironmentObjectInfo *src_info;
    gchar *var_name;
    GHashTableIter iter;

    if (!method_info || !envr) {
        return;
    }

    if (!method_info->environment) {
        method_info->environment = g_hash_table_new_full(g_str_hash,
                                                         g_str_equal,
                                                         g_free,
                                                         environment_info_free);
    }

    g_hash_table_iter_init (&iter, envr);
    while (g_hash_table_iter_next (&iter,
                                   (gpointer *) &var_name,
                                   (gpointer *) &src_info))
    {
        dst_info = g_new0(EnvironmentObjectInfo, 1);
        dst_info->default_value = g_strdup(src_info->default_value);
        dst_info->required = src_info->required;

        g_hash_table_replace(method_info->environment,
                             g_strdup(var_name),
                             dst_info);
    }
}

/* It returns newly allocated string or NULL. */
static gchar *toml_raw_to_string(toml_raw_t raw) {
    gchar *value;
    if (toml_rtos(raw, &value)) {
            return NULL;
    }

    return value;
}

static gint64 toml_raw_to_int(toml_raw_t raw) {
    gint64 value;
    if (toml_rtoi(raw, &value)) {
        return -1;
    }

    return value;
}

static gboolean make_and_save_introspection(gchar *node_name,
                                            gchar *interface_name,
                                            InterfaceObjectInfo *info)
{
    GHashTableIter iter;
    GHashTable *method_data;
    MethodObjectInfo *method_object_info;
    gchar *method_name;
    gchar *additional_val;
    gchar *backend3;
    toml_raw_t raw;
    gint64 limit;
    GString *xml;

    if (info == NULL || info->methods == NULL) {
        g_warning("Backend3: make_and_save_introspection(), node = %s, "
                  "interface name = %s: info or info->methods == NULL.",
                  node_name, interface_name);
        return FALSE;
    }

    /* Make XML. */
    xml = g_string_new(xml_top);
    g_string_append(xml, interface_name);
    g_string_append(xml, xml_top_end);
    /**/

    gint method_counter = 0;

    g_hash_table_iter_init (&iter, info->methods);
    while (g_hash_table_iter_next (&iter,
                                   (gpointer *) &method_name,
                                   (gpointer *) &method_object_info))
    {
        if (!method_object_info || !method_object_info->method_data) {
            continue;
        }

        method_data = method_object_info->method_data;
        raw = g_hash_table_lookup(method_data, INFO_BACKEND3);
        /* The memory for 'backend3' will be freed in method_info_free(). */
        backend3 = toml_raw_to_string(raw);

        if (!backend3) {
            g_warning("Backend3: make_and_save_introspection(), backend3 field"
                      " is NULL, Node name = %s\nInterface name = %s\nMethod "
                      "name = %s", node_name, interface_name, method_name);
            continue;
        }

        MethodInfo *method_info = g_new0(MethodInfo, 1);
        method_info->method_name = g_strdup(method_name);
        method_info->backend3 = g_strdup(backend3);
        g_free(backend3);
        method_info->thread_limit = THREAD_LIMIT;
        method_info->timeout = DEFAULT_TIMEOUT;
        method_info->stdout_byte_limit = DEFAULT_BYTE_LIMIT;

        /* action id */
        raw = g_hash_table_lookup(method_data, INFO_ACTION_ID);
        additional_val = toml_raw_to_string(raw);
        if (is_correct_action_id(additional_val)) {
            method_info->action_id = g_strconcat(info->action_id,
                                                 ".",
                                                 additional_val,
                                                 NULL);
        } else {
            method_info->action_id = g_strdup(info->action_id);
        }
        g_free(additional_val);

        /* thread limit */
        raw = g_hash_table_lookup(method_data, INFO_THREAD_LIMIT);
        if (raw) {
            limit = toml_raw_to_int(raw);

            if (limit > 0 && limit <= G_MAXINT) {
                method_info->thread_limit = (gint) limit;
            } else {
                g_debug("Backend3: thread_limit out of range 0 and %d (node = "
                        "%s, interface = %s, method = %s).", G_MAXINT,
                        node_name, interface_name, method_name);
            }
        }

        /* timeout */
        raw = g_hash_table_lookup(method_data, INFO_TIMEOUT);
        if (raw) {
            limit = toml_raw_to_int(raw);

            if (limit >= G_MININT && limit <= G_MAXINT) {
                method_info->timeout = (gint) limit;
            } else {
                g_debug("Backend3: timeout out of range %d and %d (node = %s,"
                        " interface = %s, method = %s).", G_MININT, G_MAXINT,
                        node_name, interface_name, method_name);
            }
        }

        raw = g_hash_table_lookup(method_data, INFO_STDOUT_BYTE_LIMIT);
        if (raw) {
            limit = toml_raw_to_int(raw);

            if (limit >= 0 && limit <= G_MAXINT) {
                method_info->stdout_byte_limit = (gint) limit;
            } else {
                g_debug("Backend3: stdout_byte_limit out of range 0 and %d "
                        "(node = %s, interface = %s, method = %s).", G_MAXINT,
                        node_name, interface_name, method_name);
            }
        }

        /* _objects */
        raw = g_hash_table_lookup(method_data, INFO__OBJECTS);
        additional_val = toml_raw_to_string(raw);
        if (additional_val) {
            method_info->_objects = g_strdup(additional_val);
        }
        g_free(additional_val);

        /* action */
        raw = g_hash_table_lookup(method_data, INFO_ACTION);
        additional_val = toml_raw_to_string(raw);
        if (additional_val) {
            method_info->action = g_strdup(additional_val);
        }
        g_free(additional_val);

        /* language */
        raw = g_hash_table_lookup(method_data, INFO_LANGUAGE);
        additional_val = toml_raw_to_string(raw);
        if (additional_val) {
            method_info->language = g_strdup(additional_val);
        }
        g_free(additional_val);

        copy_environment(method_info, method_object_info->environment);

        add_method_to_table(node_name, interface_name, method_name,
                            method_info, info->thread_limit);

        /* Make XML. */
        g_string_append(xml, xml_method_name);
        g_string_append(xml, method_name);
        g_string_append(xml, xml_method_name_end);

        g_string_append(xml, xml_args);
        /**/
        method_counter++;
    }

    /* Make XML. */
    g_string_append(xml, xml_bottom);
    /**/

    gchar *xml_string = g_string_free(xml, FALSE);

    if (!xml_string) {
        g_warning("Backend3: xml_string == NULL (node = %s, interface = %s).",
                  node_name, interface_name);
        return FALSE;
    }

    if (method_counter == 0) {
        g_warning("Backend3: method_counter == 0 (node = %s, interface = %s).",
                  node_name, interface_name);
        g_free(xml_string);
        return FALSE;
    }

    GDBusNodeInfo *introspection_data = NULL;
    introspection_data = g_dbus_node_info_new_for_xml(xml_string, NULL);
    if (!introspection_data) {
        g_warning("Backend3: introspection_data == NULL (node = %s, interface "
                  "= %s).", node_name, interface_name);
        g_free(xml_string);
        return FALSE;
    }

    GDBusInterfaceInfo *interface_info = NULL;
    interface_info = g_dbus_node_info_lookup_interface(introspection_data,
                                                       interface_name);
    if (!interface_info) {
        g_warning("Backend3: GDBusInterfaceInfo == NULL (node = %s, interface "
                  "= %s).", node_name, interface_name);
        g_free(xml_string);
        g_dbus_node_info_unref(introspection_data);
        return FALSE;
    }

    /* Checking if introspection matches a pattern in a directory
       /usr/share/dbus-1/interfaces. This returns FALSE if the interface name
       matched, but the method names, the names and types of input and output
       parameters of the methods did not match, or if there are methods in the
       template that are not in the introspection being checked. */
    if (!manager_interface->interface_validation(interface_info,
                                                 interface_name))
    {
        g_warning("Backend3: interface validation failed (node = %s, interface"
                  " = %s).", node_name, interface_name);
        g_free(xml_string);
        g_dbus_node_info_unref(introspection_data);
        return FALSE;
    }

    info->interface_introspection = interface_info;

    g_free(xml_string);
    //g_dbus_node_info_unref(introspection_data); // it contains interface_info
    return TRUE;
}

static void backend3_data_preparation(GHashTable *backends_data) {
    GHashTableIter nodes_iter;
    GHashTableIter interfaces_iter;
    GHashTable *interfaces;
    gchar *interface_name;
    gchar *node_name;
    InterfaceObjectInfo *info;

    g_hash_table_iter_init(&nodes_iter, backends_data);
    while (g_hash_table_iter_next(&nodes_iter,
                                  (gpointer *) &node_name,
                                  (gpointer *) &interfaces))
    {

        if (node_name == NULL) {
            g_debug("Backend3: backend3_data_preparation(), node_name == NULL");
            continue;
        } else if (interfaces == NULL) {
            g_debug("Backend3: backend3_data_preparation(), node_name == %s, "
                    "interfaces == NULL", node_name);
            continue;
        }

        g_hash_table_iter_init(&interfaces_iter, interfaces);
        while (g_hash_table_iter_next(&interfaces_iter,
                                      (gpointer *) &interface_name,
                                      (gpointer *) &info))
        {

            if (interface_name == NULL) {
                g_debug("Backend3: backend3_data_preparation(), node_name == "
                        "%s, interface_name == NULL", node_name);
                continue;
            } else if (info == NULL) {
                g_debug("Backend3: backend3_data_preparation(), node_name == "
                        "%s, interface_name == %s, info == NULL",
                        node_name, interface_name);
                continue;
            }

            if (!g_strcmp0(info->module_name, PLUGIN_NAME)) {
                if(make_and_save_introspection(node_name, interface_name, info))
                {
                    info->interface_vtable = &interface_vtable;
                }
            }
        }
    }
}

static gboolean module_init(ManagerData *manager_data) {
    g_debug("Initialize alterator module %s.", PLUGIN_NAME);

    if (manager_data->user_mode) {
        g_debug("Backend3: the backend3 doesn't work in user mode.\n"
                "FALSE will be returned.");
        return FALSE;
    }

    if (manager_data->authority == NULL) {
        g_warning("Backend3: module_init(), manager_data->authority == "
                  "NULL. NULL will be returned.");
        return FALSE;
    } else {
        polkit_authority = manager_data->authority;
    }

//    sender_environment_data = manager_data->sender_environment_data;
    backend3_table = g_hash_table_new_full(g_str_hash,
                                           g_str_equal,
                                           g_free,
                                           child_data_free);

    backend3_data_preparation(manager_data->backends_data);

    return TRUE;
};

static gint module_interface_version() {
    return alterator_module_interface_version();
}

static void module_destroy() {
    g_warning("deinitialize alterator module %s.", PLUGIN_NAME);
}

static gboolean is_module_busy() {
    GHashTableIter iter;
    ChildData *child_data;

    if (!backend3_table) {
        g_warning("Backend3: is_module_busy(), backend3_table == NULL.");
        return FALSE;
    }

    g_mutex_lock(&backend3_table_mutex);
    g_hash_table_iter_init(&iter, backend3_table);
    while (g_hash_table_iter_next(&iter,
                                  NULL,
                                  (gpointer *) &child_data))
    {
        if (child_data->method_data) {
            g_debug("Backend3 is busy, %s is running.", child_data->backend3);
            return TRUE;
        }
    }
    g_mutex_unlock(&backend3_table_mutex);

    return FALSE;
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
