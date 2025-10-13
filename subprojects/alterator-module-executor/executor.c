/*Module for alterator-manager (executor).*/

#include "executor.h"
#include <alterator/alterator_manager_module_info.h>

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

static const gchar xml_arg_string_name[] =
  "      <arg type='s' name='";

static const gchar xml_arg_string_array_name[] =
  "      <arg type='as' name='";

static const gchar xml_arg_input_name_end[] =
  "' direction='in'/>";

static const gchar xml_arg_output_name_end[] =
  "' direction='out'/>";

static const gchar xml_arg_output_strings[] =
  "      <arg type='as' name='stdout_strings' direction='out'/>";

static const gchar xml_arg_output_bytes[] =
  "      <arg type='ay' name='stdout_bytes' direction='out'/>";

static const gchar xml_arg_output_byte_arrays[] =
  "      <arg type='aay' name='stdout_byte_arrays' direction='out'/>";

static const gchar xml_arg_output_string_array[] =
  "      <arg type='as' name='stdout_string_array' direction='out'/>";

static const gchar xml_arg_error_strings[] =
  "      <arg type='as' name='stderr_strings' direction='out'/>";

static const gchar xml_method_end[] =
  "      <arg type='i' name='response' direction='out'/>"
  "    </method>";

static const gchar xml_signal_start[] =
  "    <signal name='";

static const gchar xml_signal_end[] =
  "'>"
  "      <arg type='s' name='line'/>"
  "    </signal>";


/* node -> interfaces
   interface -> methods
   method -> method data*/
static GHashTable *methods_table = NULL;
static PolkitAuthority* polkit_authority = NULL;
static gboolean user_mode = FALSE;
static GMutex global_mutex;
static AlteratorManagerInterface *manager_interface = NULL;
/* sender -> environment_variables_table
   environment_variables_table: key -> value */
static GHashTable *sender_environment_data = NULL;
static gint global_thread_counter = 0;

/* This function checks that the string is valid, namely that it is at least two
   characters long and that the curly braces are in the correct position
   (close/open).*/
static gboolean check_execute_string(gchar *execute_string) {
    if (execute_string == NULL || (g_utf8_strlen(execute_string, 3) < 2)) {
        return FALSE;
    }

    gboolean braces = FALSE;
    for (int i = 0; execute_string[i] != '\0'; i++) {
        if (execute_string[i] == '{' && braces) {
            return FALSE;
        } else if (execute_string[i] == '{' && !braces) {
            braces = TRUE;
        } else if (execute_string[i] == '}' && !braces) {
            return FALSE;
        } else if (execute_string[i] == '}' && braces) {
            braces = FALSE;
        }
    }

    if (braces) {
        return FALSE;
    } else {
        return TRUE;
    }
}

/* Make a type string for stdout_json from param_names.
   It returns newly allocated string. */
static gchar *make_type_string_for_stdout_json(gchar **param_names,
                                               gboolean stderr_array)
{
    GString *str;

    str = g_string_new(NULL);

    g_string_append_c(str, '(');

    /* Names from json. */
    while (param_names && *param_names) {
        if (g_str_has_suffix(*param_names, "[]")) {
            g_string_append(str, "as");
        } else {
            g_string_append_c(str, 's');
        }
        param_names++;
    }

    /* Array for stderr. */
    if (stderr_array) {
        g_string_append(str, "as");
    }

    /* Exit status. */
    g_string_append_c(str, 'i');
    g_string_append_c(str, ')');

    return g_string_free(str, FALSE);
}

/*  */
static void put_string_from_json_to_builder(struct json_object *obj,
                                            GVariantBuilder *builder)
{
    const gchar *str;

    if (!builder) {
        return;
    }

    if (!obj || !json_object_is_type(obj, json_type_string)) {
        str = "";
    } else {
        str = json_object_get_string(obj);
    }

    g_variant_builder_add(builder, "s", str);
}

/* Parsing a JSON object and filling the builder with lines from json. */
static gboolean parse_json_fill_builder(ThreadFuncParam *param,
                                        GVariantBuilder *builder)
{
    struct json_object *jobj;
    struct json_object *value;
    struct json_object *arr_lmnt;
    gchar *array_name;

    /* Parsing a JSON object. */
    jobj = json_tokener_parse(param->stdout_bytes->str);
    if (!jobj) {
        g_debug("Executor: error parsing of json (Object path: %s, "
                "interface name: %s, method name: %s).", param->object_path,
                param->interface_name, param->method_info->method_name);

        return FALSE;
    }

    /* Names to be extracted from the JSON object. */
    gchar **strv = param->stdout_param_names;
    while (strv && *strv) {
        size_t array_length;
        /* Array. */
        if (g_str_has_suffix(*strv, "[]")) {
            array_name = g_strndup(*strv, g_utf8_strlen(*strv, -1) - 2);
            g_variant_builder_open(builder, G_VARIANT_TYPE ("as"));

            if (json_object_object_get_ex(jobj, array_name, &value) &&
                json_object_is_type(value, json_type_array))
            {
                array_length = json_object_array_length(value);

                for (size_t i = 0; i < array_length; i++) {
                    arr_lmnt = json_object_array_get_idx(value, i);
                    put_string_from_json_to_builder(arr_lmnt, builder);
                }

            } else {
                g_variant_builder_add(builder, "s", "");
            }

            g_variant_builder_close(builder);

            g_free(array_name);
        /* String. */
        } else {
            if (json_object_object_get_ex(jobj, *strv, &value)) {
                put_string_from_json_to_builder(value, builder);
            } else {
                g_variant_builder_add(builder, "s", "");
            }
        }
        strv++;
    }
    /* Free it. */
    json_object_put(jobj);

    return TRUE;
}

static void invocation_return_value(ThreadFuncParam *param, gint exit_status) {
    /* Selection of the required format string and formation of the g_variant.*/
    GVariantBuilder *builder;
    gboolean is_ay_open = FALSE;
    gchar *type_string;

    /* Strings from json and error strings. */
    if (param->stdout_json_enabled && param->stderr_strings_enabled) {
        type_string =
              make_type_string_for_stdout_json(param->stdout_param_names, TRUE);
        builder = g_variant_builder_new(G_VARIANT_TYPE (type_string));
        g_free(type_string);

        /* Parsing a JSON object and filling the builder with lines from json.
           If the parsing fails, we send a response with an error and exit. */
        if (!parse_json_fill_builder(param, builder)) {
            g_dbus_method_invocation_return_dbus_error(param->invocation,
                                                       DBUS_NAME,
                                                  "Error parsing JSON object.");
            g_variant_builder_unref(builder);

            return;
        }

        /* Error strings. */
        param->stderr_strings = g_list_reverse(param->stderr_strings);

        g_variant_builder_open(builder, G_VARIANT_TYPE ("as"));
        for (GList *a = param->stderr_strings; a; a = a->next) {
            g_variant_builder_add(builder, "s", (gchar*) a->data);
        }
        g_variant_builder_close(builder);

        /* Exit status. */
        g_variant_builder_add(builder, "i", exit_status);
    /* Strings from json only. */
    } else if (param->stdout_json_enabled) {
        type_string =
             make_type_string_for_stdout_json(param->stdout_param_names, FALSE);
        builder = g_variant_builder_new(G_VARIANT_TYPE (type_string));
        g_free(type_string);

        /* Parsing a JSON object and filling the builder with lines from json.
           If the parsing fails, we send a response with an error and exit. */
        if (!parse_json_fill_builder(param, builder)) {
            g_dbus_method_invocation_return_dbus_error(param->invocation,
                                                       DBUS_NAME,
                                                  "Error parsing JSON object.");
            g_variant_builder_unref(builder);

            return;
        }

        /* Exit status. */
        g_variant_builder_add(builder, "i", exit_status);
    /* String array and error strings. */
    } else if (param->stdout_string_array_enabled &&
               param->stderr_strings_enabled)
    {
        builder = g_variant_builder_new(G_VARIANT_TYPE (TWO_ARRAYS));

        g_variant_builder_open(builder, G_VARIANT_TYPE ("as"));

        for (gchar *str = param->stdout_bytes->str;
             str < param->stdout_bytes->str + param->stdout_bytes->len;
             str++)
        {
            if (strlen(str)) {
                g_variant_builder_add(builder, "s", str);
                str += strlen(str);
            }
        }

        g_variant_builder_close(builder);

        /* error strings */
        param->stderr_strings = g_list_reverse(param->stderr_strings);

        g_variant_builder_open(builder, G_VARIANT_TYPE ("as"));
        for (GList *a = param->stderr_strings; a; a = a->next) {
            g_variant_builder_add(builder, "s", (gchar*) a->data);
        }
        g_variant_builder_close(builder);

        g_variant_builder_add(builder, "i", exit_status);
    /* String array only. */
    } else if (param->stdout_string_array_enabled) {
        builder = g_variant_builder_new(G_VARIANT_TYPE (ARRAY_ONLY));

        g_variant_builder_open(builder, G_VARIANT_TYPE ("as"));

        for (gchar *str = param->stdout_bytes->str;
             str < param->stdout_bytes->str + param->stdout_bytes->len;
             str++)
        {
            if (strlen(str)) {
                g_variant_builder_add(builder, "s", str);
                str += strlen(str);
            }
        }

        g_variant_builder_close(builder);

        g_variant_builder_add(builder, "i", exit_status);
    /* Blob arrays and error strings. */
    } else if (param->stdout_byte_arrays_enabled &&
               param->stderr_strings_enabled)
    {
        param->stderr_strings = g_list_reverse(param->stderr_strings);

        builder = g_variant_builder_new(G_VARIANT_TYPE (BYTE_ARRAYS_AND_ARRAY));

        g_variant_builder_open(builder, G_VARIANT_TYPE ("aay"));
        is_ay_open = FALSE;
        for (gsize i = 0; i < param->stdout_bytes->len; i++) {
            if (!is_ay_open) {
                g_variant_builder_open(builder, G_VARIANT_TYPE ("ay"));
                is_ay_open = TRUE;
            }

            if (param->stdout_bytes->str[i] == '\0') {
                g_variant_builder_close(builder);// "ay"
                is_ay_open = FALSE;

                continue;
            }

            g_variant_builder_add(builder, "y", param->stdout_bytes->str[i]);
        }

        if (is_ay_open) {
            g_variant_builder_close(builder);// "ay"
            is_ay_open = FALSE;
        }
        g_variant_builder_close(builder);// "aay"

        g_variant_builder_open (builder, G_VARIANT_TYPE ("as"));
        for (GList *a = param->stderr_strings; a; a = a->next) {
            g_variant_builder_add(builder, "s", (gchar*) a->data);
        }
        g_variant_builder_close (builder);

        g_variant_builder_add(builder, "i", exit_status);
    /* Blob arrays only. */
    } else if (param->stdout_byte_arrays_enabled) {
        builder = g_variant_builder_new(G_VARIANT_TYPE (BYTE_ARRAYS_ONLY));

        g_variant_builder_open(builder, G_VARIANT_TYPE ("aay"));

        is_ay_open = FALSE;
        for (gsize i = 0; i < param->stdout_bytes->len; i++) {
            if (!is_ay_open) {
                g_variant_builder_open(builder, G_VARIANT_TYPE ("ay"));
                is_ay_open = TRUE;
            }

            if (param->stdout_bytes->str[i] == '\0') {
                g_variant_builder_close(builder);// "ay"
                is_ay_open = FALSE;

                continue;
            }

            g_variant_builder_add(builder, "y", param->stdout_bytes->str[i]);
        }

        if (is_ay_open) {
            g_variant_builder_close(builder);// "ay"
            is_ay_open = FALSE;
        }
        g_variant_builder_close(builder);// "aay"

        g_variant_builder_add(builder, "i", exit_status);
    /* Blob and error strings. */
    } else if (param->stdout_bytes_enabled && param->stderr_strings_enabled) {
        param->stderr_strings = g_list_reverse(param->stderr_strings);

        builder = g_variant_builder_new (G_VARIANT_TYPE (BYTES_AND_ARRAY));

        g_variant_builder_open (builder, G_VARIANT_TYPE ("ay"));
        for (gsize i = 0; i < param->stdout_bytes->len; i++) {
            g_variant_builder_add(builder, "y", param->stdout_bytes->str[i]);
        }
        g_variant_builder_close (builder);

        g_variant_builder_open (builder, G_VARIANT_TYPE ("as"));
        for (GList *a = param->stderr_strings; a; a = a->next) {
            g_variant_builder_add(builder, "s", (gchar*) a->data);
        }
        g_variant_builder_close (builder);

        g_variant_builder_add(builder, "i", exit_status);
    /* Blob only. */
    } else if (param->stdout_bytes_enabled) {
        builder = g_variant_builder_new (G_VARIANT_TYPE (BYTES_ONLY));

        g_variant_builder_open (builder, G_VARIANT_TYPE ("ay"));
        for (gsize i = 0; i < param->stdout_bytes->len; i++) {
            g_variant_builder_add(builder, "y", param->stdout_bytes->str[i]);
        }
        g_variant_builder_close (builder);

        g_variant_builder_add(builder, "i", exit_status);
    /* Out strings and error strings. */
    } else if (param->stderr_strings_enabled && param->stdout_strings_enabled) {
        param->stderr_strings = g_list_reverse(param->stderr_strings);
        param->stdout_strings = g_list_reverse(param->stdout_strings);

        builder = g_variant_builder_new (G_VARIANT_TYPE (TWO_ARRAYS));

        g_variant_builder_open (builder, G_VARIANT_TYPE ("as"));
        for (GList *a = param->stdout_strings; a; a = a->next) {
            g_variant_builder_add(builder, "s", (gchar*) a->data);
        }
        g_variant_builder_close (builder);

        g_variant_builder_open (builder, G_VARIANT_TYPE ("as"));
        for (GList *a = param->stderr_strings; a; a = a->next) {
            g_variant_builder_add(builder, "s", (gchar*) a->data);
        }
        g_variant_builder_close (builder);

        g_variant_builder_add(builder, "i", exit_status);
    /* Out strings. */
    } else if (param->stdout_strings_enabled) {
        param->stdout_strings = g_list_reverse(param->stdout_strings);

        builder = g_variant_builder_new (G_VARIANT_TYPE (ARRAY_ONLY));

        g_variant_builder_open (builder, G_VARIANT_TYPE ("as"));
        for (GList *a = param->stdout_strings; a; a = a->next) {
            g_variant_builder_add(builder, "s", (gchar*) a->data);
        }
        g_variant_builder_close (builder);

        g_variant_builder_add(builder, "i", exit_status);
    /* Error strings. */
    } else if (param->stderr_strings_enabled) {
        param->stderr_strings = g_list_reverse(param->stderr_strings);

        builder = g_variant_builder_new (G_VARIANT_TYPE (ARRAY_ONLY));

        g_variant_builder_open (builder, G_VARIANT_TYPE ("as"));
        for (GList *a = param->stderr_strings; a; a = a->next) {
            g_variant_builder_add(builder, "s", (gchar*) a->data);
        }
        g_variant_builder_close (builder);

        g_variant_builder_add(builder, "i", exit_status);
    /* Exit status only.*/
    } else {
        builder = g_variant_builder_new (G_VARIANT_TYPE (EXIT_STATUS_ONLY));

        g_variant_builder_add(builder, "i", exit_status);
    }

    g_dbus_method_invocation_return_value(param->invocation,
                                          g_variant_builder_end(builder));
    g_variant_builder_unref(builder);
}

static void thread_func_param_free(ThreadFuncParam *thread_param) {
    if (thread_param->command_line) {
        g_free(thread_param->command_line);
    }
    if (thread_param->stdin_string) {
        g_free(thread_param->stdin_string);
    }
    if (thread_param->stdout_strings) {
        g_list_free_full(thread_param->stdout_strings, g_free);
    }
    if (thread_param->stderr_strings) {
        g_list_free_full(thread_param->stderr_strings, g_free);
    }
    if (thread_param->stdout_bytes) {
        g_string_free(thread_param->stdout_bytes, TRUE);
    }
    if (thread_param->stdout_signal_name) {
        g_free(thread_param->stdout_signal_name);
    }
    if (thread_param->stderr_signal_name) {
        g_free(thread_param->stderr_signal_name);
    }
    if (thread_param->sender) {
        g_free(thread_param->sender);
    }
    if (thread_param->envp) {
        g_strfreev(thread_param->envp);
    }
    g_strfreev(thread_param->stdout_param_names);

    g_free(thread_param);
}

static void return_value_and_free_child_data(ChildData *child_data) {
    ThreadFuncParam *thread_param = child_data->thread_func_param;

    if (thread_param->stdout_exit_status) {
        invocation_return_value(thread_param, thread_param->stdout_exit_status);
    } else if (thread_param->stderr_exit_status) {
        invocation_return_value(thread_param, thread_param->stderr_exit_status);
    } else {
        invocation_return_value(thread_param, thread_param->exit_status);
    }

    g_main_loop_quit(child_data->loop);

    g_mutex_lock (&global_mutex);
    thread_param->interface_info->thread_counter--;
    thread_param->method_info->thread_counter--;
    global_thread_counter--;
    g_mutex_unlock (&global_mutex);
    //free thread_param
    thread_func_param_free(thread_param);

    if (child_data->stdout_pipe_out) {
        g_close(child_data->stdout_pipe_out, NULL);
    }
    if (child_data->stderr_pipe_out) {
        g_close(child_data->stderr_pipe_out, NULL);
    }
}

static gboolean exit_by_timeout(gpointer data) {
    GPid pid = *(GPid *)data;

    if (kill(pid, SIGKILL) == -1) {
        switch (errno) {
            case EINVAL:
                g_warning("Executor: exit_by_timeout(), an invalid signal was "
                          "specified. (kill(), errno == EINVAL)");
                break;
            case EPERM:
                g_warning("Executor: exit_by_timeout(), the calling process "
                          "does not have permission to send the signal. "
                          "(kill(), errno == EPERM)");
                break;
            case ESRCH:
                g_warning("Executor: exit_by_timeout(), the target process "
                          "does not exist, pid == %i. (kill(), errno == ESRCH)",
                          pid);
                break;
            default:
                g_warning("Executor: exit_by_timeout(), unspecified error. "
                          "(kill(), errno == %i)", errno);
        }
    }

    return G_SOURCE_REMOVE;
}

static gboolean on_child_exited(GPid pid, gint wait_status, gpointer data) {
    ChildData *child_data = (ChildData *)data;
    ThreadFuncParam *thread_param = child_data->thread_func_param;
    GError *error = NULL;

#if GLIB_CHECK_VERSION(2,70,0)
    g_spawn_check_wait_status(wait_status, &error);
#else
    g_spawn_check_exit_status(wait_status, &error);
#endif

    if (error) {
        g_warning("Executor: spawn_check_wait_status(), %s\n"
                  "Object path: %s\nInterface name: %s\nMethod name: %s",
                  error->message, thread_param->object_path,
                  thread_param->interface_name,
                  thread_param->method_info->method_name);

        thread_param->exit_status = error->code;
        g_error_free(error);
        error = NULL;
    } else {
        thread_param->exit_status = 0;
    }

    //it doesn't do anything under unix.
    //g_spawn_close_pid(pid);

    child_data->child_exited = TRUE;

    if (child_data->child_exited &&
        child_data->stdout_done &&
        child_data->stderr_done)
    {
        return_value_and_free_child_data(child_data);
    }

    return G_SOURCE_REMOVE;
}

static gboolean on_child_stdout(GIOChannel   *channel,
                                GIOCondition  condition,
                                gpointer      datap)
{
    gchar *str;
    GError *error = NULL;
    char buf[1024];
    gsize length;
    gsize terminator_pos;
    GIOStatus status;
    ChildData *child_data = datap;
    ThreadFuncParam *thread_param = child_data->thread_func_param;
    gboolean res = G_SOURCE_CONTINUE;

    if ((condition & G_IO_IN || condition & G_IO_HUP) &&
         (thread_param->stdout_bytes_enabled ||
          thread_param->stdout_byte_arrays_enabled ||
          thread_param->stdout_string_array_enabled ||
          thread_param->stdout_json_enabled))
    {
        status = g_io_channel_read_chars(channel, buf, sizeof(buf), &length,
                                         &error);

        if (error) {
            g_warning("Executor: on_child_stdout, read chars, %s",
                      error->message);
            g_error_free(error);
            error = NULL;
            child_data->stdout_done = TRUE;
            res = G_SOURCE_REMOVE;
        }

        if (status == G_IO_STATUS_EOF) {
            child_data->stdout_done = TRUE;
            res = G_SOURCE_REMOVE;
        }

        thread_param->stdout_byte_counter += length;

        if (thread_param->stdout_byte_counter >
            thread_param->stdout_byte_limit)
        {
            child_data->stdout_done = TRUE;
            res = G_SOURCE_REMOVE;
            thread_param->stdout_exit_status = 2;
            g_warning("Executor: on_child_stdout, byte limit reached "
                      "(%d), status 2 will be returned.",
                      thread_param->stdout_byte_limit);
        }

        if (length && (thread_param->stdout_exit_status == 0)) {
            if (thread_param->stdout_bytes == NULL) {
                thread_param->stdout_bytes = g_string_new(NULL);
            }

            g_string_append_len(thread_param->stdout_bytes, buf,
                                (gssize) length);
        }

    } else if ((condition & G_IO_IN || condition & G_IO_HUP) &&
               (thread_param->stdout_strings_enabled ||
                thread_param->stdout_signals_enabled))
    {
        status = g_io_channel_read_line(channel, &str, &length, &terminator_pos,
                                        &error);

        if (error) {
            g_warning("Executor: on_child_stdout, read line, %s",
                      error->message);
            g_error_free(error);
            g_free(str);
            error = NULL;
            child_data->stdout_done = TRUE;
            res = G_SOURCE_REMOVE;
            goto out;
        }

        if (status == G_IO_STATUS_EOF) {
            child_data->stdout_done = TRUE;
            res = G_SOURCE_REMOVE;
        }

        thread_param->stdout_strings_counter += length;

        /* Clean line break symbol in end of line. */
        if (length > 0 && str[length-1] == '\n')
            str[length-1] = '\0';

        if (thread_param->stdout_strings_counter >
            thread_param->stdout_strings_limit)
        {
            res = G_SOURCE_REMOVE;
            thread_param->stdout_exit_status = 2;
            child_data->stdout_done = TRUE;
            g_warning("Executor: on_child_stdout, strings limit reached "
                      "(%d), status 2 will be returned.",
                      thread_param->stdout_strings_limit);
        }

        if (str != NULL && thread_param->stdout_signals_enabled &&
            (thread_param->stdout_exit_status == 0))
        {
            g_dbus_connection_emit_signal(thread_param->connection,
                                          NULL, //destination_bus_name
                                          thread_param->object_path, //object_path
                                          thread_param->interface_name, //interface_name
                                          thread_param->stdout_signal_name, //signal_name
                                          g_variant_new ("(s)", str),
                                          &error);

            if (error) {
                g_warning("Executor: on_child_stdout, %s", error->message);
                g_error_free(error);
                error = NULL;
            }
        }

        if (str != NULL && thread_param->stdout_strings_enabled &&
            (thread_param->stdout_exit_status == 0))
        {
            /* Reverse must be done when building GVariant. */
            thread_param->stdout_strings =
                    g_list_prepend(thread_param->stdout_strings, g_strdup(str));
        }

        if (str != NULL) {
            g_free(str);
        }
    }

    /* We will read until error or eof.
    if (condition & G_IO_HUP) {
        if (child_data->child_exited) {
            child_data->stdout_done = TRUE;
            res = G_SOURCE_REMOVE;
        }
    }*/

    if (condition & G_IO_ERR) {
        g_warning ("Executor: Error reading from child stdin.");
        child_data->stdout_done = TRUE;
        res = G_SOURCE_REMOVE;
    }

out:
    if (child_data->child_exited &&
        child_data->stdout_done &&
        child_data->stderr_done)
    {
        return_value_and_free_child_data(child_data);
    }

    return res;
}

static gboolean on_child_stderr(GIOChannel   *channel,
                                GIOCondition  condition,
                                gpointer      datap)
{
    gchar *str;
    GError *error = NULL;
    gsize length;
    gsize terminator_pos;
    ChildData *child_data = datap;
    ThreadFuncParam *thread_param = child_data->thread_func_param;
    gboolean res = G_SOURCE_CONTINUE;

    if (condition & G_IO_IN || condition & G_IO_HUP) {
        GIOStatus status;

        status = g_io_channel_read_line(channel, &str, &length, &terminator_pos,
                                        &error);

        if (error) {
            g_warning("Executor: on_child_stderr, %s", error->message);
            g_error_free(error);
            child_data->stderr_done = TRUE;
            res = G_SOURCE_REMOVE;
        }

        if (status == G_IO_STATUS_EOF) {
            child_data->stderr_done = TRUE;
            res = G_SOURCE_REMOVE;
        }

        thread_param->stderr_strings_counter += length;

        /* Clean line break symbol in end of line. */
        if (length > 0 && str[length-1] == '\n')
            str[length-1] = '\0';

        if (thread_param->stderr_strings_counter >
            thread_param->stderr_strings_limit)
        {
            res = G_SOURCE_REMOVE;
            thread_param->stderr_exit_status = 3;
            child_data->stderr_done = TRUE;
            g_warning("Executor: on_child_stderr, strings limit reached "
                      "(%d), status 3 will be returned.",
                      thread_param->stderr_strings_limit);
        }

        if (str != NULL && thread_param->stderr_signals_enabled &&
            (thread_param->stderr_exit_status == 0))
        {
            g_dbus_connection_emit_signal(thread_param->connection,
                                          NULL, //destination_bus_name
                                          thread_param->object_path, //object_path
                                          thread_param->interface_name, //interface_name
                                          thread_param->stderr_signal_name, //signal_name
                                          g_variant_new ("(s)", str),
                                          &error);

            if (error) {
                g_warning("Executor: on_child_stdout, %s", error->message);
                g_error_free(error);
                error = NULL;
            }
        }

        if (str != NULL && thread_param->stderr_strings_enabled &&
            (thread_param->stderr_exit_status == 0))
        {
            /* Reverse must be done when building GVariant. */
            thread_param->stderr_strings =
                    g_list_prepend(thread_param->stderr_strings, g_strdup(str));
        }

        if (str != NULL) {
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
        g_warning ("Executor: Error reading from child stderr.");
        res = G_SOURCE_REMOVE;
        child_data->stderr_done = TRUE;
    }

    if (child_data->child_exited &&
        child_data->stdout_done &&
        child_data->stderr_done)
    {
        return_value_and_free_child_data(child_data);
    }

    return res;
}

static gboolean authorization_check(ThreadFuncParam *param) {
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

    subject = polkit_system_bus_name_new (
                       g_dbus_method_invocation_get_sender (param->invocation));

    gchar *action_id = param->method_info->action_id;

    auth_result = polkit_authority_check_authorization_sync (polkit_authority,
                                                             subject,
                                                             action_id,
                                                             NULL,//details,
                                                             flags,
                                                             NULL, /* GCancellable* */
                                                             &error);

    if (auth_result == NULL) {
        g_warning("Executor: check authorization result == NULL (%s).",
                  error->message);
        g_dbus_method_invocation_return_dbus_error(param->invocation,
                                                   DBUS_NAME,
                                         "Check authorization result == NULL.");
    } else if (!polkit_authorization_result_get_is_authorized (auth_result)) {
        g_dbus_method_invocation_return_dbus_error(param->invocation,
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

static gpointer execute_command_line(gpointer data) {
    ThreadFuncParam *param = (ThreadFuncParam *)data;
    GPid child_pid;
    GMainContext *context;
    GMainLoop *loop;
    GSource *source;
    GIOChannel *channel;
    gint child_stdin;
    gint child_stdout;
    gint child_stderr;
    gint *child_stdout_;
    gint *child_stderr_;
    GSpawnFlags flags;
    ChildData child_data;
    gchar **argv;
    gboolean result = FALSE;
    GError *error = NULL;

    if (!authorization_check(param)) {
        g_mutex_lock (&global_mutex);
        param->interface_info->thread_counter--;
        param->method_info->thread_counter--;
        global_thread_counter--;
        g_mutex_unlock (&global_mutex);
        thread_func_param_free(param);
        return NULL;
    }

    const gchar *command_line = param->command_line;

    result = g_shell_parse_argv(command_line, NULL, &argv, &error);

    if (!result) {
        g_warning("Executor: execute_command_line(), %s", error->message);
        g_error_free(error);
        invocation_return_value(param, 1);

        g_mutex_lock (&global_mutex);
        param->interface_info->thread_counter--;
        param->method_info->thread_counter--;
        global_thread_counter--;
        g_mutex_unlock (&global_mutex);

        thread_func_param_free(param);
        return NULL;
    }

    context = g_main_context_new ();
    loop = g_main_loop_new (context, TRUE);

    child_stdout = 0;
    child_stderr = 0;
    child_stdout_ = &child_stdout;
    child_stderr_ = &child_stderr;
    flags = G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD;

    if (!param->stdout_signals_enabled &&
        !param->stdout_strings_enabled &&
        !param->stdout_bytes_enabled &&
        !param->stdout_byte_arrays_enabled &&
        !param->stdout_string_array_enabled &&
        !param->stdout_json_enabled)
    {
        child_stdout_ = NULL;
        flags |= G_SPAWN_STDOUT_TO_DEV_NULL;
    }

    if (!param->stderr_signals_enabled && !param->stderr_strings_enabled)
    {
        child_stderr_ = NULL;
        flags |= G_SPAWN_STDERR_TO_DEV_NULL;
    }

    g_spawn_async_with_pipes(NULL, argv, param->envp, flags, NULL, NULL,
                             &child_pid, &child_stdin,
                             child_stdout_, child_stderr_, &error);


    g_strfreev(argv);

    if (error) {
        g_warning("Executor: g_spawn_async_with_pipes(), %s", error->message);
        g_error_free(error);
        g_main_loop_unref(loop);
        g_main_context_unref(context);
        invocation_return_value(param, 1);

        g_mutex_lock (&global_mutex);
        param->interface_info->thread_counter--;
        param->method_info->thread_counter--;
        global_thread_counter--;
        g_mutex_unlock (&global_mutex);

        thread_func_param_free(param);
        return NULL;
    }

    child_data.loop = loop;
    child_data.child_exited = FALSE;
    child_data.stdout_done = TRUE;
    child_data.stderr_done = TRUE;
    child_data.thread_func_param = param;
    child_data.stdout_pipe_out = child_stdout;
    child_data.stderr_pipe_out = child_stderr;

    source = g_child_watch_source_new (child_pid);
    g_source_set_callback (source, G_SOURCE_FUNC(on_child_exited), &child_data,
                           NULL);
    g_source_attach (source, context);
    g_source_unref (source);

    if (param->method_info->timeout > 0) {
        source = g_timeout_source_new_seconds(param->method_info->timeout);
        g_source_set_callback (source, G_SOURCE_FUNC(exit_by_timeout),
                               &child_pid, NULL);
        g_source_attach (source, context);
        g_source_unref (source);
    }

    if (param->stdout_signals_enabled ||
        param->stdout_strings_enabled ||
        param->stdout_bytes_enabled ||
        param->stdout_byte_arrays_enabled ||
        param->stdout_string_array_enabled ||
        param->stdout_json_enabled)
    {
        child_data.stdout_done = FALSE;

        channel = g_io_channel_unix_new (child_stdout);

        if (param->stdout_bytes_enabled ||
            param->stdout_byte_arrays_enabled ||
            param->stdout_string_array_enabled ||
            param->stdout_json_enabled)
        {
            g_io_channel_set_encoding(channel, NULL, &error);
            if (error) {
                g_warning("Executor: g_io_channel_set_encoding(), %s",
                error->message);
                g_error_free(error);
                error = NULL;
            }
        }

        source = g_io_create_watch (channel, G_IO_IN | G_IO_HUP | G_IO_ERR);
        g_source_set_callback (source, G_SOURCE_FUNC(on_child_stdout),
                               &child_data, NULL);
        g_source_attach (source, context);
        g_source_unref (source);
        g_io_channel_unref (channel);
    }

    if (param->stderr_signals_enabled || param->stderr_strings_enabled) {
        child_data.stderr_done = FALSE;

        channel = g_io_channel_unix_new (child_stderr);
        source = g_io_create_watch (channel, G_IO_IN | G_IO_HUP | G_IO_ERR);
        g_source_set_callback (source, G_SOURCE_FUNC(on_child_stderr),
                               &child_data, NULL);
        g_source_attach (source, context);
        g_source_unref (source);
        g_io_channel_unref (channel);
    }

    /* Writing a special parameter to stdin. */
    if (param->stdin_string_enabled && param->stdin_string) {
        gssize len = strlen(param->stdin_string);
        const gchar *p = param->stdin_string;

        while (len > 0) {
            gssize done = write(child_stdin, p, len);
            gint err = errno;

            if (done < 0) {
                g_warning("Error writing to stdin (errno = %d)", err);
                break;
            } else if (done == 0) {
                g_warning("Error writing to stdin (done == 0)");
                break;
            }

            if (len < (size_t) done) {
                g_warning("Write to many bytes!");
                break;
            }
            len -= done;
            p += done;
        }

        memset(param->stdin_string, '\0', strlen(param->stdin_string));
        g_free(param->stdin_string);
        param->stdin_string = NULL;
    }
    g_close(child_stdin, NULL);

    g_main_loop_run (loop);

    g_main_context_unref (context);
    g_main_loop_unref (loop);

    return NULL;
}

static gboolean is_thread_limit_reached(InterfaceInfo *interface_info,
                                        MethodInfo *method_info)
{
    gboolean result = FALSE;

    g_mutex_lock (&global_mutex);

    if (interface_info->thread_counter >= interface_info->thread_limit ||
        method_info->thread_counter >= method_info->thread_limit)
    {
        result = TRUE;
    }

    g_mutex_unlock (&global_mutex);

    return result;
}

/* The return value must be freed. */
static gchar** make_environment(const gchar *sender, GHashTable *allowed_vars) {
    GHashTableIter iter;
    GHashTable *sender_envp;
    EnvironmentObjectInfo *envr_info;
    gchar *var_name;
    gchar *value;
    gchar **envp = g_get_environ();

    if (sender_environment_data) {
        sender_envp = (GHashTable *)
                      g_hash_table_lookup(sender_environment_data, sender);
    } else {
        sender_envp = NULL;
    }

    if (allowed_vars) {
        g_hash_table_iter_init(&iter, allowed_vars);
        while (g_hash_table_iter_next(&iter,
                                      (gpointer *) &var_name,
                                      (gpointer *) &envr_info))
        {
            if (sender_envp) {
                value = g_hash_table_lookup(sender_envp, var_name);
            } else {
                value = NULL;
            }

            if (!value && envr_info && envr_info->default_value) {
                value = envr_info->default_value;
            }

            if (value) {
                envp = g_environ_setenv(envp, var_name, value, TRUE);
            }
        }
    }

    return envp;
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
    GHashTableIter nodes_iter;
    GHashTableIter interfaces_iter;
    GHashTableIter methods_iter;
    GHashTable *interfaces;
    GHashTable *envr_variables = NULL;
    gchar *node_name_table = NULL;
    gchar *interface_name_table = NULL;
    gchar *method_name_table = NULL;
    InterfaceInfo *interface_info = NULL;
    MethodInfo *method_info = NULL;
    gchar **execute = NULL;
    gchar *stdout_signal_name = NULL;
    gchar *stderr_signal_name = NULL;
    gchar *node_name_full = NULL;
    gchar **stdout_param_names = NULL;
    gboolean stdin_string_enabled        = FALSE;
    gboolean stdout_signals_enabled      = FALSE;
    gboolean stdout_strings_enabled      = FALSE;
    gboolean stdout_bytes_enabled        = FALSE;
    gboolean stdout_byte_arrays_enabled  = FALSE;
    gboolean stdout_string_array_enabled = FALSE;
    gboolean stdout_json_enabled         = FALSE;
    gboolean stderr_signals_enabled      = FALSE;
    gboolean stderr_strings_enabled      = FALSE;
    gint stdout_strings_limit = DEFAULT_BYTE_LIMIT;
    gint stdout_byte_limit = DEFAULT_BYTE_LIMIT;
    gint stderr_strings_limit = DEFAULT_BYTE_LIMIT;

    /* Search through the table of known methods. A method is identified by
       a node name, an interface name, and a method name. */
    g_hash_table_iter_init (&nodes_iter, methods_table);
    while (g_hash_table_iter_next (&nodes_iter,
                                   (gpointer *) &node_name_table,
                                   (gpointer *) &interfaces))
    {
        node_name_full = g_strconcat(OBJECT_PATH_PREFIX, node_name_table, NULL);
        int cmp = g_strcmp0(object_path, node_name_full);
        g_free(node_name_full);

        if (!cmp) {
            g_hash_table_iter_init (&interfaces_iter, interfaces);
            while (g_hash_table_iter_next (&interfaces_iter,
                                           (gpointer *) &interface_name_table,
                                           (gpointer *) &interface_info))
            {
                if (!g_strcmp0(interface_name, interface_name_table)) {
                    g_hash_table_iter_init (&methods_iter,
                                            interface_info->methods);
                    while (g_hash_table_iter_next (&methods_iter,
                                                (gpointer *) &method_name_table,
                                                (gpointer *) &method_info))
                    {
                        if (!g_strcmp0(method_name, method_name_table)) {
                            execute = method_info->data;

                            stdin_string_enabled =
                                            method_info->stdin_string_enabled;

                            stdout_signals_enabled =
                                            method_info->stdout_signals_enabled;

                            stdout_strings_enabled =
                                            method_info->stdout_strings_enabled;

                            stdout_bytes_enabled =
                                            method_info->stdout_bytes_enabled;

                            stdout_byte_arrays_enabled =
                                        method_info->stdout_byte_arrays_enabled;

                            stdout_string_array_enabled =
                                       method_info->stdout_string_array_enabled;

                            stdout_json_enabled =
                                               method_info->stdout_json_enabled;

                            stderr_signals_enabled =
                                            method_info->stderr_signals_enabled;

                            stderr_strings_enabled =
                                            method_info->stderr_strings_enabled;

                            stdout_strings_limit =
                                            method_info->stdout_strings_limit;

                            stdout_byte_limit =
                                            method_info->stdout_byte_limit;

                            stderr_strings_limit =
                                            method_info->stderr_strings_limit;

                            stdout_param_names =
                                            method_info->stdout_param_names;


                            if (method_info->stdout_signal_name != NULL) {
                                /* Replacement . and : on _ */
                                gchar *newsender =
                                    g_strdelimit (g_strdup (sender), ":.", '_');

                                stdout_signal_name =
                                    g_strconcat(method_info->stdout_signal_name,
                                                newsender, NULL);

                                g_free(newsender);
                            }

                            if (method_info->stderr_signal_name != NULL) {
                                gchar *newsender =
                                    g_strdelimit (g_strdup (sender), ":.", '_');

                                stderr_signal_name =
                                    g_strconcat(method_info->stderr_signal_name,
                                                newsender, NULL);

                                g_free(newsender);
                            }

                            envr_variables = method_info->environment;

                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }
    }

    /*The command line is formed from the **execute and parameters. Every second
    value in the **execute is a parameter, and therefore is replaced by the
    value from parameters.*/
    GString *exe_line = g_string_new("");
    GVariantIter iter;
    GVariant *child;
    const gchar *parameter_str;
    const gchar **parameter_strv;
    const gchar **strv;
    int counter = 1;

    g_variant_iter_init(&iter, parameters);
    while (*execute != NULL) {
        if (counter % 2 == 0) {
            if ((child = g_variant_iter_next_value(&iter)) != NULL) {
                /* If the parameter is an array, each of its elements is
                   enclosed in quotation marks. Spaces are placed between
                   elements. */
                if (g_str_has_suffix(*execute, "[]")) {
                    parameter_strv = g_variant_get_strv(child, NULL);
                    strv = parameter_strv;

                    while (*strv != NULL) {
                        g_string_append(exe_line, "'");
                        g_string_append(exe_line, *strv);
                        g_string_append(exe_line, "'");

                        strv++;

                        if (*strv != NULL) {
                            g_string_append(exe_line, " ");
                        }
                    }

                    g_free(parameter_strv);
                } else {
                    parameter_str = g_variant_get_string(child, NULL);
                    g_string_append(exe_line, parameter_str);
                }
                g_variant_unref(child);
            }
        } else {
            g_string_append(exe_line, *execute);
        }

        counter++;
        execute++;
    }

    /* If true, then there is one more parameter. */
    if (stdin_string_enabled &&
        (child = g_variant_iter_next_value(&iter)) != NULL) {
        parameter_str = g_variant_get_string(child, NULL);
        g_variant_unref(child);
    } else {
        parameter_str = NULL;
    }

    ThreadFuncParam *thread_func_param = g_new0(ThreadFuncParam, 1);

    thread_func_param->command_line = g_string_free(exe_line, FALSE);
    thread_func_param->invocation = invocation;
    thread_func_param->connection = connection;
    thread_func_param->sender = g_strdup(sender);
    thread_func_param->envp = make_environment(sender, envr_variables);
    thread_func_param->method_info = method_info; //It doesn't need to be freed,
    thread_func_param->interface_info = interface_info; //it belongs to methods_table.
    thread_func_param->object_path = object_path;
    thread_func_param->interface_name = interface_name;
    thread_func_param->stdout_signal_name = stdout_signal_name;
    thread_func_param->stderr_signal_name = stderr_signal_name;
    thread_func_param->stdin_string_enabled = stdin_string_enabled;
    thread_func_param->stdout_strings_enabled = stdout_strings_enabled;
    thread_func_param->stdout_bytes_enabled = stdout_bytes_enabled;
    thread_func_param->stdout_byte_arrays_enabled = stdout_byte_arrays_enabled;
    thread_func_param->stdout_string_array_enabled =
                                                    stdout_string_array_enabled;
    thread_func_param->stdout_json_enabled = stdout_json_enabled;
    thread_func_param->stdout_signals_enabled = stdout_signals_enabled;
    thread_func_param->stderr_strings_enabled = stderr_strings_enabled;
    thread_func_param->stderr_signals_enabled = stderr_signals_enabled;
    thread_func_param->stdout_param_names = g_strdupv(stdout_param_names);
    thread_func_param->stdout_strings = NULL;
    thread_func_param->stdout_bytes = g_string_new(NULL);
    thread_func_param->stderr_strings = NULL;
    thread_func_param->stdout_strings_limit = stdout_strings_limit;
    thread_func_param->stdout_byte_limit = stdout_byte_limit;
    thread_func_param->stderr_strings_limit = stderr_strings_limit;
    thread_func_param->stdout_strings_counter = 0;
    thread_func_param->stdout_byte_counter = 0;
    thread_func_param->stderr_strings_counter = 0;
    thread_func_param->stdout_exit_status = 0;
    thread_func_param->stderr_exit_status = 0;
    thread_func_param->exit_status = 0;
    if (stdin_string_enabled && parameter_str) {
        thread_func_param->stdin_string = g_strdup(parameter_str);
    } else {
        thread_func_param->stdin_string = NULL;
    }

    /* This is where the thread limit is checked. If the thread limit is
       reached, then we free thread_func_param and return d-bus error. */

    if (is_thread_limit_reached(interface_info, method_info)) {
        thread_func_param_free(thread_func_param);

        g_warning("Executor: thread limit reached. node: %s, interface: %s, "
                  "method: %s, interface->thread_limit: %d, "
                  "interface->thread_counter: %d, method->thread_limit: %d, "
                  "method->thread_counter: %d.", node_name_table,
                  interface_name_table, method_name_table,
                  interface_info->thread_limit, interface_info->thread_counter,
                  method_info->thread_limit, method_info->thread_counter);

        g_dbus_method_invocation_return_dbus_error(invocation,
                                                   DBUS_NAME,
                                                   "Thread limit reached.");
        return;
    }

    GThread *thread = g_thread_new("executor", execute_command_line,
                                   thread_func_param);
    g_mutex_lock(&global_mutex);

    interface_info->thread_counter++;
    method_info->thread_counter++;
    global_thread_counter++;

    g_mutex_unlock(&global_mutex);

    g_thread_unref(thread);

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

/* Validates a valid name for a d-bus method or parameter. The name can only
   contain letters, numbers, and underscores. It is assumed that the length of
   the name cannot be less than 2. */
static gboolean is_correct_name(const gchar *name, gssize len) {
    if (name == NULL) {
        return FALSE;
    }

    if (g_utf8_strlen(name, 3) < 2) {
        return FALSE;
    }

    if (len == 0) {
        return FALSE;
    }

    if (len < 0) {
        len = g_utf8_strlen(name, -1);
    }

    for (int i = 0; i < len; i++) {
        if (!g_ascii_isalnum(name[i]) && name[i] != '_') {
            return FALSE;
        }
    }

    return TRUE;
}

/* The first part is the name of the program. It cannot be shorter than two
   characters. Every second part is a parameter name. */
static gboolean is_correct_execute(gchar **execute) {
    if (execute == NULL) {
        return FALSE;
    }

    int counter = 1;
    while (*execute != NULL) {
        //program name
        if (counter == 1 && g_utf8_strlen(*execute, 3) < 2) {
            return FALSE;
        }

        //d-bus method parameters
        if (counter % 2 == 0) {
            // If the parameter is an array.
            if (g_str_has_suffix(*execute, "[]")) {
                if (!is_correct_name(*execute,
                                     g_utf8_strlen(*execute, -1) - 2))
                {
                    return FALSE;
                }
            } else if (!is_correct_name(*execute, -1)) {
                return FALSE;
            }
        }

        counter++;
        execute++;
    }

    return TRUE;
}

static const GDBusInterfaceVTable interface_vtable =
{
    handle_method_call,
};

static gint module_interface_version()
{
    return alterator_module_interface_version();
}

static void hashtable_free(void *hashtable) {
    GHashTable *table = (GHashTable *) hashtable;

    g_hash_table_destroy(table);
}

static void interface_info_free(void *interface_info) {
    InterfaceInfo *info = (InterfaceInfo *)interface_info;

    if (info == NULL) {
        return;
    }

    if (info->methods != NULL) {
        g_hash_table_destroy(info->methods);
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

static void method_info_free(void *method_info) {
    MethodInfo *info = (MethodInfo *)method_info;

    if (info == NULL) {
        return;
    }

    if (info->method_name) {
        g_free(info->method_name);
    }

    if (info->execute) {
        g_free(info->execute);
    }

    if (info->stdout_signal_name) {
        g_free(info->stdout_signal_name);
    }

    if (info->stderr_signal_name) {
        g_free(info->stderr_signal_name);
    }

    if (info->action_id) {
        g_free(info->action_id);
    }

    if (info->environment) {
        g_hash_table_destroy(info->environment);
    }

    if (info->data) {
        g_strfreev(info->data);
    }

    g_strfreev(info->stdout_param_names);

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

static int toml_raw_to_boolean(toml_raw_t raw) {
    int boolean = 0;
    if (toml_rtob(raw, &boolean)) {
            boolean = 0;
    }

    return boolean;
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

/* Get a list of output parameter names. */
static gchar **get_stdout_param_names(const gchar *const *strv,
                                      const gchar *node_name,
                                      const gchar *interface_name,
                                      const gchar *method_name)
{
    GStrvBuilder* builder;
    GStrv res;
    gchar *arg_name;
    gboolean correct;

    builder = g_strv_builder_new();
    while (strv && *strv) {
        arg_name = toml_raw_to_string(*strv);
        correct = FALSE;

        /* array name */
        if (g_str_has_suffix(arg_name, "[]")) {
            if (is_correct_name(arg_name, g_utf8_strlen(arg_name, -1) - 2))
            {
                g_strv_builder_add(builder, arg_name);
                correct = TRUE;
            }
        /* string name */
        } else if (is_correct_name(arg_name, -1)) {
            g_strv_builder_add(builder, arg_name);
            correct = TRUE;
        }

        if (!correct) {
            g_debug("Executor: invalid return arg name (node name: %s, "
                    "interface name: %s, method name: %s, arg name: \'%s\').",
                    node_name, interface_name, method_name, arg_name);
        }

        g_free(arg_name);
        strv++;
    }

    res = g_strv_builder_end(builder);
    g_strv_builder_unref(builder);

    return res;
}

static void introspect_add_output_args(GString *xml, GStrv arg_names) {
    while (arg_names && *arg_names) {
        if (g_str_has_suffix(*arg_names, "[]")) {
            g_string_append(xml, xml_arg_string_array_name);
            g_string_append_len(xml,
                                *arg_names,
                                g_utf8_strlen(*arg_names, -1) - 2);
            g_string_append(xml, xml_arg_output_name_end);
        } else {
            g_string_append(xml, xml_arg_string_name);
            g_string_append(xml, *arg_names);
            g_string_append(xml, xml_arg_output_name_end);
        }
        arg_names++;
    }
}

static gboolean make_and_save_introspection(gchar *node_name,
                                            gchar *interface_name,
                                            InterfaceObjectInfo *info)
{
    GHashTableIter iter;
    GHashTable *method_data;
    GHashTable *method_data_arrays;
    MethodObjectInfo *method_object_info;
    gchar *method_name;
    gchar *execute;
    gchar *additional_val;
    const gchar **strv;
    gint64 limit;
    toml_raw_t raw;
    GString *xml;
    GHashTable *signal_names = NULL;

    if (info == NULL || info->methods == NULL) {
        g_warning("node = %s, interface name = (%s): info or info->methods == "
                  "NULL", node_name, interface_name);
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
        method_data_arrays = method_object_info->method_data_arrays;
        raw = g_hash_table_lookup(method_data, INFO_EXECUTE);
        /* Do not forget to free 'execute'. */
        execute = toml_raw_to_string(raw);

        if (execute == NULL || !check_execute_string(execute)) {
            if (execute == NULL) {
                g_debug("execute == NULL");
            } else {
                g_debug("execute:%s", execute);
            }
            g_debug("execute string isn't valid (node = %s, interface = %s, "
                    "method = %s).", node_name, interface_name, method_name);

            g_free(execute);
            continue;
        }

        gchar **data = g_strsplit_set(execute, "{}", -1);

        if (!is_correct_execute(data)) {
            g_debug("**data string isn't valid (node = %s, interface = %s, "
                    "method = %s).", node_name, interface_name, method_name);
            g_strfreev(data);
            g_free(execute);
            continue;
        }

        MethodInfo *method_info = g_new0(MethodInfo, 1);
        method_info->method_name = g_strdup(method_name);
        /* Now you don't need to free up memory for execute. */
        method_info->execute = execute;
        method_info->data = data;
        method_info->stdout_byte_limit = DEFAULT_BYTE_LIMIT;
        method_info->stdout_strings_limit = DEFAULT_BYTE_LIMIT;
        method_info->stderr_strings_limit = DEFAULT_BYTE_LIMIT;
        method_info->thread_limit = THREAD_LIMIT;
        method_info->timeout = DEFAULT_TIMEOUT;

        raw = g_hash_table_lookup(method_data, INFO_STDIN_STRING);
        if (toml_raw_to_boolean(raw)) {
            method_info->stdin_string_enabled = TRUE;
        } else {
            method_info->stdin_string_enabled = FALSE;
        }

        raw = g_hash_table_lookup(method_data, INFO_STDOUT_STRINGS);
        if (toml_raw_to_boolean(raw)) {
            method_info->stdout_strings_enabled = TRUE;
        } else {
            method_info->stdout_strings_enabled = FALSE;
        }

        raw = g_hash_table_lookup(method_data, INFO_STDOUT_BYTES);
        if (toml_raw_to_boolean(raw)) {
            method_info->stdout_bytes_enabled = TRUE;
        } else {
            method_info->stdout_bytes_enabled = FALSE;
        }

        raw = g_hash_table_lookup(method_data, INFO_STDOUT_BYTE_ARRAYS);
        if (toml_raw_to_boolean(raw)) {
            method_info->stdout_byte_arrays_enabled = TRUE;
        } else {
            method_info->stdout_byte_arrays_enabled = FALSE;
        }

        raw = g_hash_table_lookup(method_data, INFO_STDOUT_STRING_ARRAY);
        if (toml_raw_to_boolean(raw)) {
            method_info->stdout_string_array_enabled = TRUE;
        } else {
            method_info->stdout_string_array_enabled = FALSE;
        }

        if (method_data_arrays &&
            g_hash_table_contains(method_data_arrays, INFO_STDOUT_JSON))
        {
            strv = g_hash_table_lookup(method_data_arrays, INFO_STDOUT_JSON);
            /* stdout_json will freed in method_info_free(). */
            method_info->stdout_param_names =
                                          get_stdout_param_names(strv,
                                                                 node_name,
                                                                 interface_name,
                                                                 method_name);
            method_info->stdout_json_enabled = TRUE;
        } else {
            method_info->stdout_param_names = NULL;
            method_info->stdout_json_enabled = FALSE;
        }

        raw = g_hash_table_lookup(method_data, INFO_STDERR_STRINGS);
        if (toml_raw_to_boolean(raw)) {
            method_info->stderr_strings_enabled = TRUE;
        } else {
            method_info->stderr_strings_enabled = FALSE;
        }

        raw = g_hash_table_lookup(method_data, INFO_STDOUT_SIGNAL_NAME);
        additional_val = toml_raw_to_string(raw);
        if (g_dbus_is_member_name(additional_val)) {
            method_info->stdout_signals_enabled = TRUE;
            method_info->stdout_signal_name = additional_val;
            if (!signal_names) {
                signal_names = g_hash_table_new(g_str_hash, g_str_equal);
            }
            g_hash_table_add(signal_names, additional_val);
        } else {
            if (additional_val) {
                g_debug("stdout signal name isn't valid (node = %s, interface ="
                        " %s, signal = %s).", node_name, interface_name,
                        additional_val);
                g_free(additional_val);
            }
            method_info->stdout_signals_enabled = FALSE;
            method_info->stdout_signal_name = NULL;
        }

        raw = g_hash_table_lookup(method_data, INFO_STDERR_SIGNAL_NAME);
        additional_val = toml_raw_to_string(raw);
        if (g_dbus_is_member_name(additional_val)) {
            method_info->stderr_signals_enabled = TRUE;
            method_info->stderr_signal_name = additional_val;
            if (!signal_names) {
                signal_names = g_hash_table_new(g_str_hash, g_str_equal);
            }
            g_hash_table_add(signal_names, additional_val);
        } else {
            if (additional_val) {
                g_debug("stderr signal name isn't valid (node = %s, interface ="
                        " %s, signal = %s).", node_name, interface_name,
                        additional_val);
                g_free(additional_val);
            }
            method_info->stderr_signals_enabled = FALSE;
            method_info->stderr_signal_name = NULL;
        }

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

        raw = g_hash_table_lookup(method_data, INFO_STDOUT_BYTE_LIMIT);
        if (raw) {
            limit = toml_raw_to_int(raw);

            if (limit >= 0 && limit <= G_MAXINT) {
                method_info->stdout_byte_limit = (gint) limit;
            } else {
                g_debug("stdout_byte_limit out of range 0 and %d (node = %s,"
                        " interface = %s, method = %s).", G_MAXINT, node_name,
                        interface_name, method_name);
            }
        }

        raw = g_hash_table_lookup(method_data, INFO_STDOUT_STRINGS_LIMIT);
        if (raw) {
            limit = toml_raw_to_int(raw);

            if (limit >= 0 && limit <= G_MAXINT) {
                method_info->stdout_strings_limit = (gint) limit;
            } else {
                g_debug("stdout_strings_limit out of range 0 and %d (node = %s,"
                        " interface = %s, method = %s).", G_MAXINT, node_name,
                        interface_name, method_name);
            }
        }

        raw = g_hash_table_lookup(method_data, INFO_STDERR_STRINGS_LIMIT);
        if (raw) {
            limit = toml_raw_to_int(raw);

            if (limit >= 0 && limit <= G_MAXINT) {
                method_info->stderr_strings_limit = (gint) limit;
            } else {
                g_debug("stderr_strings_limit out of range 0 and %d (node = %s,"
                        " interface = %s, method = %s).", G_MAXINT, node_name,
                        interface_name, method_name);
            }
        }

        raw = g_hash_table_lookup(method_data, INFO_THREAD_LIMIT);
        if (raw) {
            limit = toml_raw_to_int(raw);

            if (limit > 0 && limit <= G_MAXINT) {
                method_info->thread_limit = (gint) limit;
            } else {
                g_debug("thread_limit out of range 0 and %d (node = %s,"
                        " interface = %s, method = %s).", G_MAXINT, node_name,
                        interface_name, method_name);
            }
        }

        raw = g_hash_table_lookup(method_data, INFO_TIMEOUT);
        if (raw) {
            limit = toml_raw_to_int(raw);

            if (limit >= G_MININT && limit <= G_MAXINT) {
                method_info->timeout = (gint) limit;
            } else {
                g_debug("timeout out of range %d and %d (node = %s,"
                        " interface = %s, method = %s).", G_MININT, G_MAXINT,
                        node_name, interface_name, method_name);
            }
        }

        copy_environment(method_info, method_object_info->environment);

        add_method_to_table(node_name, interface_name, method_name,
                            method_info, info->thread_limit);

        /* Make XML. */
        g_string_append(xml, xml_method_name);
        g_string_append(xml, method_name);
        g_string_append(xml, xml_method_name_end);

        /* Every second element is the name of the method parameter. */
        gint counter = 1;
        gchar **execute_data = method_info->data;
        while (*execute_data != NULL) {
            if (counter % 2 == 0) {
                /* If the parameter is an array. */
                if (g_str_has_suffix(*execute_data, "[]")) {
                    g_string_append(xml, xml_arg_string_array_name);
                    g_string_append_len(xml,
                                        *execute_data,
                                        g_utf8_strlen(*execute_data, -1) - 2);
                } else {
                    g_string_append(xml, xml_arg_string_name);
                    g_string_append(xml, *execute_data);
                }
                g_string_append(xml, xml_arg_input_name_end);
            }
            counter++;
            execute_data++;
        }

        /* The last parameter is a string for stdin. */
        if (method_info->stdin_string_enabled) {
            g_string_append(xml, xml_arg_string_name);
            g_string_append(xml, PRM_STDIN);
            g_string_append(xml, xml_arg_input_name_end);
        }

        if (method_info->stdout_json_enabled) {
            introspect_add_output_args(xml, method_info->stdout_param_names);
        } else if (method_info->stdout_string_array_enabled) {
            g_string_append(xml, xml_arg_output_string_array);
        } else if (method_info->stdout_byte_arrays_enabled) {
            g_string_append(xml, xml_arg_output_byte_arrays);
        } else if (method_info->stdout_bytes_enabled) {
            g_string_append(xml, xml_arg_output_bytes);
        } else if (method_info->stdout_strings_enabled) {
            g_string_append(xml, xml_arg_output_strings);
        }

        if (method_info->stderr_strings_enabled) {
            g_string_append(xml, xml_arg_error_strings);
        }

        g_string_append(xml, xml_method_end);
        /**/
        method_counter++;
    }

    /* Make XML. */
    if (signal_names) {
        gpointer signal_name;
        g_hash_table_iter_init (&iter, signal_names);
        while (g_hash_table_iter_next (&iter, &signal_name, NULL)) {
            g_string_append(xml, xml_signal_start);
            g_string_append(xml, (gchar*) signal_name);
            g_string_append(xml, xml_signal_end);
        }
        g_hash_table_destroy(signal_names);
    }

    g_string_append(xml, xml_bottom);
    /**/

    gchar *xml_string = g_string_free(xml, FALSE);

    if (xml_string == NULL) {
        g_warning("xml_string == NULL (node = %s, interface = %s).",
                  node_name, interface_name);
        return FALSE;
    }

    if (method_counter == 0) {
        g_warning("method_counter == 0 (node = %s, interface = %s).",
                  node_name, interface_name);
        g_free(xml_string);
        return FALSE;
    }

    GDBusNodeInfo *introspection_data = NULL;
    introspection_data = g_dbus_node_info_new_for_xml (xml_string, NULL);
    if (introspection_data == NULL) {
        g_warning("introspection_data == NULL (node = %s, interface = %s).",
                  node_name, interface_name);
        g_free(xml_string);
        return FALSE;
    }

    GDBusInterfaceInfo *interface_info = NULL;
    interface_info = g_dbus_node_info_lookup_interface (introspection_data,
                                                        interface_name);
    if (interface_info == NULL) {
        g_warning("GDBusInterfaceInfo == NULL (node = %s, interface = %s).",
                  node_name, interface_name);
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
        g_warning("Interface validation failed (node = %s, interface = %s).",
                  node_name, interface_name);
        g_free(xml_string);
        g_dbus_node_info_unref(introspection_data);
        return FALSE;
    }

    info->interface_introspection = g_dbus_interface_info_ref (interface_info);

    g_free(xml_string);
    //g_dbus_node_info_unref(introspection_data); // it contains interface_info
    return TRUE;
}

static gboolean module_init(ManagerData *manager_data)
{
    GHashTableIter nodes_iter;
    GHashTableIter interfaces_iter;
    GHashTable *interfaces;
    gchar *interface_name;
    gchar *node_name;
    InterfaceObjectInfo *info;

    g_debug("Initialize alterator module %s.", PLUGIN_NAME);

    user_mode = manager_data->user_mode;

    /* authority is only needed in system mode. */
    if (manager_data->authority == NULL && !user_mode) {
        g_warning("Executor: module_init(), manager_data->authority == "
                  "NULL. NULL will be returned.");
        return FALSE;
    } else {
        polkit_authority = manager_data->authority;
    }

    sender_environment_data = manager_data->sender_environment_data;

    g_hash_table_iter_init (&nodes_iter, manager_data->backends_data);
    while (g_hash_table_iter_next (&nodes_iter,
                                   (gpointer *) &node_name,
                                   (gpointer *) &interfaces))
    {

        if (node_name == NULL) {
            g_debug("Initialize: node_name == NULL");
            continue;
        } else if (interfaces == NULL) {
            g_debug("Initialize: node_name == %s, interfaces == NULL",
                    node_name);
            continue;
        }

        g_hash_table_iter_init (&interfaces_iter, interfaces);
        while (g_hash_table_iter_next (&interfaces_iter,
                                       (gpointer *) &interface_name,
                                       (gpointer *) &info))
        {

            if (interface_name == NULL) {
                g_debug("Initialize: node_name == %s, interface_name == NULL",
                        node_name);
                continue;
            } else if (info == NULL) {
                g_debug("Initialize: node_name == %s, interface_name == %s, "
                        "info == NULL", node_name, interface_name);
                continue;
            }

            if (info != NULL && info->module_name != NULL) {
                if (!g_strcmp0(info->module_name, PLUGIN_NAME)) {
                    if(make_and_save_introspection(node_name, interface_name,
                                                   info))
                    {
                        info->interface_vtable = &interface_vtable;
                    }
                }
            }
        }
    }

    return TRUE;
};

static void module_destroy()
{
    g_warning("deinitialize alterator module %s.", PLUGIN_NAME);
}

static gboolean is_module_busy()
{
    gboolean result = FALSE;

    g_mutex_lock(&global_mutex);

    if (global_thread_counter > 0) {
        result = TRUE;
    }

    g_mutex_unlock(&global_mutex);

    return result;
}

static AlteratorModuleInterface module_interface =
{
    .interface_version = module_interface_version,
    .init              = module_init,
    .destroy           = module_destroy,
    .is_module_busy    = is_module_busy
};

gboolean alterator_module_init(AlteratorManagerInterface *interface)
{
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
    global_thread_counter = 0;

    if (!interface->register_module(&module_interface)) {
        g_warning("Register module '%s' failed.", PLUGIN_NAME);
        return FALSE;
    }

    return TRUE;
}
