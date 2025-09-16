/**/

#include "alterator_manager_backends.h"

static GHashTable *backends_data = NULL;

/* Validates a valid name for a d-bus method or parameter. The name can only
contain letters, numbers, and underscores.
It is assumed that the length of the name cannot be greater than 100 and less
than 2. */
static gboolean is_correct_name(const gchar* name) {
    if (name == NULL) {
        return FALSE;
    }

    if (g_utf8_strlen(name, 3) < 2) {
        return FALSE;
    }

    for (int i = 0; name[i] != '\0'; i++) {
        if (!g_ascii_isalnum(name[i]) && name[i] != '_') {
            return FALSE;
        }

        if (i >= MAX_NAME_LENGTH - 1) {
            return FALSE;
        }
    }

    return TRUE;
}

static GPtrArray *read_backend_file_names(const gchar *key_files_path) {
    GDir *dir = NULL;
    GError *error = NULL;
    GPtrArray *file_names = NULL;
    gboolean is_correct_suffix = FALSE;

    file_names = g_ptr_array_new_full(5, g_free);
    if (file_names == NULL) {
        g_warning("read_backend_file_names(), g_ptr_array_new() "
                  "returned NULL.");
        return NULL;
    }

    dir = g_dir_open(key_files_path, 0, &error);

    if (error) {
        g_warning("Open directory %s, %s.", key_files_path, error->message);
        g_error_free(error);
        g_ptr_array_free(file_names, TRUE);
        return NULL;
    }

    const gchar *file_name = NULL;
    while ((file_name = g_dir_read_name(dir)) != NULL) {

        is_correct_suffix = g_str_has_suffix(file_name, KEY_FILE_NAME_SUFFIX);

        if (is_correct_suffix) {
            g_debug("Found backend file: %s%s.", key_files_path, file_name);
            g_ptr_array_add(file_names, g_strdup(file_name));
        }
    }

    g_dir_close(dir);
    return file_names;
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
    MethodObjectInfo *info = (MethodObjectInfo *)method_info;

    if (info == NULL) {
        return;
    }

    if (info->method_data != NULL) {
        g_hash_table_unref(info->method_data);
    }

    if (info->environment != NULL) {
        g_hash_table_unref(info->environment);
    }

    g_free(info);
}

static void interface_info_free(void *interface_info) {
    InterfaceObjectInfo *info = (InterfaceObjectInfo *)interface_info;

    if (info == NULL) {
        return;
    }

    g_free(info->module_name);
    //GHashTable *interface_introspection; //belongs to the module
    if (info->methods != NULL) {
        g_hash_table_unref(info->methods);
    }
    //GDBusInterfaceVTable *interface_vtable; //belongs to the module
    g_free(info);
}

static void hashtable_free(void *hashtable) {
    GHashTable *table = (GHashTable *) hashtable;

    g_hash_table_destroy(table);
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

static GHashTable *save_methods_arrays(toml_table_t *table,
                                       const gchar *node_name,
                                       const gchar *interface_name,
                                       const gchar *method_name)
{
    GHashTable *res;
    GStrvBuilder* builder;
    toml_raw_t raw;
    toml_array_t *array;
    gint narr;
    gint nkval;
    gint arr_size;

    res = g_hash_table_new_full(g_str_hash,
                                g_str_equal,
                                g_free,
                                (GDestroyNotify) g_strfreev);

    /* To get to the arrays. */
    narr = toml_table_narr(table);
    nkval = toml_table_nkval(table);

    for (gint i = 0; i < narr; i++) {
        const gchar *arr_key = toml_key_in(table, nkval + i);
        array = toml_array_in(table, arr_key);
        arr_size = toml_array_nelem(array);
        builder = g_strv_builder_new();
        GStrv strv;

        for (gint ii = 0; ii < arr_size; ii++) {
            raw = toml_raw_at(array, ii);

            if (raw) {
                g_strv_builder_add(builder, raw);
            } else {
                g_debug("Invalid array element (node name: %s, interface name:"
                        " %s, method name: %s, method field: %s, index: %d).",
                        node_name, interface_name, method_name, arr_key, ii);
            }
        }
        strv = g_strv_builder_end(builder);
        g_strv_builder_unref(builder);

        g_hash_table_replace(res, g_strdup(arr_key), strv);
    }

    return res;
}

static GHashTable *save_methods_environment(toml_table_t *table) {
    GHashTable *res = NULL;
    EnvironmentObjectInfo *envr_info = NULL;
    toml_datum_t datum;

    if (!table) {
        return NULL;
    }

    toml_table_t *envr = toml_table_in(table, TABLE_ENVIRONMENT);
    if (!envr) {
        return NULL;
    }

    /* To get to the tables. */
    gint k = toml_table_nkval(envr) + toml_table_narr(envr);
    gint ntab = toml_table_ntab(envr);

    for (gint i = 0; i < ntab; i++) {
        const gchar *env_var = toml_key_in(envr, k + i);

        if (!res) {
            res = g_hash_table_new_full(g_str_hash,
                                        g_str_equal,
                                        g_free,
                                        environment_info_free);
        }

        envr_info = g_new0(EnvironmentObjectInfo, 1);
        if (!envr_info) {
            g_warning("g_new0(EnvironmentObjectInfo, 1) returned NULL.");
            continue;
        }

        toml_table_t *envr_var_table = toml_table_in(envr, env_var);
        if (envr_var_table) {
            datum = toml_string_in(envr_var_table, ENVR_DEFAULT_VALUE);
            if (datum.ok) {
                envr_info->default_value = g_strdup(datum.u.s);
            }
            g_free(datum.u.s);

            datum = toml_bool_in(envr_var_table, ENVR_REQUIRED);
            if (datum.ok && datum.u.b) {
                envr_info->required = TRUE;
            } else {
                envr_info->required = FALSE;
            }
        }

        g_hash_table_replace(res, g_strdup(env_var), envr_info);
    }

    return res;
}

static void save_info_from_backend_file(toml_table_t *backend_toml,
                                        gchar *file_name)
{
    GHashTable *interfaces = NULL;
    InterfaceObjectInfo *interface_info = NULL;
    MethodObjectInfo *method_info = NULL;
    gchar *type_name;
    gchar *module_name;
    gchar *node_name;
    gchar *interface_name;
    gchar *action_id;
    gint thread_limit;
    toml_datum_t datum;

    if (backend_toml == NULL) {
        return;
    }

    datum = toml_string_in(backend_toml, TYPE_NAME);
    if (!datum.ok) {
        g_warning("Error reading \'%s\' from %s.", TYPE_NAME, file_name);
        return;
    }
    // type_name must be freed
    type_name = datum.u.s;


    if (type_name == NULL) {
        g_warning("backend_file (%s) has %s == NULL.", file_name, TYPE_NAME);
        return;
    }

    if (g_strcmp0(type_name, TYPE_BACKEND)) {
        g_free(type_name);
        return;
    }
    g_free(type_name);

    toml_table_t *methods = toml_table_in(backend_toml, TABLE_METHODS);
    if (!methods) {
        g_warning("There are no methods in the backend file (%s).", file_name);
        return;
    }

    datum = toml_string_in(backend_toml, MODULE_NAME);
    if (!datum.ok) {
        g_warning("Error reading \'%s\' from %s.", MODULE_NAME, file_name);
        return;
    }
    // module_name must be freed
    module_name = datum.u.s;

    if (module_name == NULL) {
        g_warning("backend_file (%s) has %s == NULL.", file_name, MODULE_NAME);
        return;
    }

    datum = toml_int_in(backend_toml, THREAD_LIMIT_NAME);
    if (datum.ok) {
        thread_limit = (gint)datum.u.i;
    } else {
        thread_limit = -1;
    }

    if (thread_limit <= 0) {
        g_debug("backend_file (%s) has thread_limit <= 0. "
                "This field may not have been set or may have been set "
                "incorrectly. It will be given a default value (%d).",
                file_name, THREAD_LIMIT);
        thread_limit = THREAD_LIMIT;
    }

    datum = toml_string_in(backend_toml, NODE_NAME);
    if (!datum.ok) {
        g_warning("Error reading \'%s\' from %s.", NODE_NAME, file_name);

        g_free(module_name);
        return;
    }
    // node_name must be freed
    node_name = datum.u.s;


    if (node_name == NULL) {
        g_warning("backend_file (%s) has %s == NULL.", file_name, NODE_NAME);

        g_free(module_name);
        return;
    }

    if (!is_correct_name(node_name)) {
        g_warning("Incorrect node name (%s) in backend file (%s).", node_name,
                  file_name);

        g_free(module_name);
        g_free(node_name);
        return;
    }

    datum = toml_string_in(backend_toml, INTERFACE_NAME);
    if (!datum.ok) {
        g_warning("Error reading \'%s\' from %s.", INTERFACE_NAME, file_name);

        g_free(module_name);
        g_free(node_name);
        return;
    }
    // interface_name must be freed
    interface_name = datum.u.s;

    if (interface_name == NULL) {
        g_warning("backend_file (%s) has %s == NULL.",
                  file_name, INTERFACE_NAME);

        g_free(module_name);
        g_free(node_name);
        return;
    }

    if (strchr(interface_name, '.') == NULL) {
        gchar *interface_name_short = interface_name;
        interface_name = g_strconcat(INTERFACE_PREFIX, interface_name, NULL);
        g_free(interface_name_short);
    }

    if (!g_dbus_is_interface_name(interface_name)) {
        g_warning("Incorrect interface name (%s) in backend file (%s).",
                interface_name, file_name);

        g_free(interface_name);
        g_free(node_name);
        g_free(module_name);
        return;
    }

    datum = toml_string_in(backend_toml, ACTION_ID);
    if (datum.ok) {
        // action_id must be freed
        action_id = datum.u.s;
    } else {
        action_id = NULL;
    }

    /* If action_id == NULL or it is not correct we use interface name. */
    if (action_id == NULL) {
        action_id = g_strdelimit (g_strdup (interface_name), "_", '-');
    } else if (!is_correct_action_id(action_id)) {
        g_warning("Incorrect actiont_id (%s) in backend file (%s).",
                   interface_name, file_name);
        g_free(action_id);
        action_id = g_strdelimit (g_strdup (interface_name), "_", '-');
    }

    /* If the node already contains the interface name we exit, because a node
       can't contains two interfaces with one name. */
    if (g_hash_table_contains(backends_data, node_name)) {
        interfaces =
                   (GHashTable *) g_hash_table_lookup(backends_data, node_name);

        if (g_hash_table_contains(interfaces, interface_name)) {
            g_warning("backends_data: %s already contains interface named %s.",
                      node_name, interface_name);

            g_free(interface_name);
            g_free(node_name);
            g_free(module_name);
            g_free(action_id);
            return;
        }
    } else {
        interfaces = g_hash_table_new_full(g_str_hash,
                                           g_str_equal,
                                           g_free,
                                           interface_info_free);

        g_hash_table_replace(backends_data, g_strdup(node_name), interfaces);
    }

    /* To get to the tables. */
    gint k = toml_table_nkval(methods) + toml_table_narr(methods);
    gint ntab = toml_table_ntab(methods);

    for (gint i = 0; i < ntab; i++) {
        const gchar *method_name = toml_key_in(methods, k + i);
        g_debug("methods[%d] = %s", i, method_name);

        if (!g_dbus_is_member_name(method_name)) {
            g_warning("Invalid method name: %s.", method_name);
            continue;
        }

        toml_table_t *method_table = toml_table_in(methods, method_name);
        if (!method_table) {
            g_warning("There are not \'%s\' in \'%s\'.", method_name,
                      TABLE_METHODS);
            continue;
        }

        /* An interface is created if it has at least one method. */
        if (!g_hash_table_contains(interfaces, interface_name)) {
            interface_info = g_new0(InterfaceObjectInfo, 1);
            if (interface_info == NULL) {
                g_warning("save_info_from_backend_file(), g_new0() for"
                           "interface_info returned NULL.");
                break;
            }

            /* interface_introspection and vtable are created and populated
               inside a specific module */
            interface_info->interface_introspection = NULL;
            interface_info->interface_vtable = NULL;
            interface_info->module_name = g_strdup(module_name);
            interface_info->action_id = g_strdup(action_id);
            interface_info->thread_limit = thread_limit;
            interface_info->methods = g_hash_table_new_full(g_str_hash,
                                                            g_str_equal,
                                                            g_free,
                                                            method_info_free);

            g_hash_table_replace(interfaces,
                                 g_strdup(interface_name),
                                 interface_info);
        }

        method_info = g_new0(MethodObjectInfo, 1);
        if (method_info == NULL) {
            g_warning("save_info_from_backend_file(), g_new0() for"
                      "method_info returned NULL.");
            break;
        }

        GHashTable *method_data = g_hash_table_new_full(g_str_hash,
                                                        g_str_equal,
                                                        g_free,
                                                        g_free);

        gint method_keys_number = toml_table_nkval(method_table);

        for (int ii = 0; ii < method_keys_number; ii++) {
            const gchar *key = toml_key_in(method_table, ii);
            const gchar *value = toml_raw_in(method_table, key);

            g_hash_table_replace(method_data, g_strdup(key), g_strdup(value));
        }

        method_info->method_data = method_data;
        method_info->environment = save_methods_environment(method_table);
        method_info->method_data_arrays = save_methods_arrays(method_table,
                                                              node_name,
                                                              interface_name,
                                                              method_name);

        g_hash_table_replace(interface_info->methods,
                             g_strdup(method_name),
                             method_info);
    }

    g_free(node_name);
    g_free(action_id);
    g_free(interface_name);
    g_free(module_name);
}

static gboolean read_data_from_backend_files(const gchar *backend_files_path,
                                             GPtrArray   *backend_file_names)
{
    gchar* file_path = NULL;
    gchar* file_name = NULL;
    FILE* fp;
    char errbuf[200];

    if (backend_file_names == NULL) {
        return FALSE;
    }

    if (backends_data == NULL) {
        backends_data = g_hash_table_new_full(g_str_hash,
                                              g_str_equal,
                                              g_free,
                                              hashtable_free);
    }

    for (int i = 0; i < backend_file_names->len; i++) {
        file_name = (gchar *)g_ptr_array_index(backend_file_names, i);
        if (file_name == NULL) {
            continue;
        }

        file_path = g_strconcat(backend_files_path, file_name, NULL);

        fp = fopen(file_path, "r");
        if (!fp) {
            g_warning("Cannot open %s", file_path);
            g_free(file_path);
            file_name = NULL;
            continue;
        }

        toml_table_t *backend = toml_parse_file(fp, errbuf, sizeof(errbuf));
        fclose(fp);

        if (!backend) {
            g_warning("Backend file parse error (%s).", file_path);
            g_free(file_path);
            file_name = NULL;
            continue;
        }

        save_info_from_backend_file(backend, file_name);

        toml_free(backend);
        g_free(file_path);
        file_name = NULL;
    }

    if (backends_data != NULL) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static gboolean alterator_manager_backends_load(const gchar *key_files_path) {
    gboolean result = FALSE;
    GPtrArray *backend_file_names = read_backend_file_names(key_files_path);
    if (backend_file_names == NULL || key_files_path == NULL) {
        return FALSE;
    }

    if (backend_file_names->len == 0) {
        g_debug(".backend files not found (%s).", key_files_path);
    }

    result = read_data_from_backend_files(key_files_path, backend_file_names);
    if (!result) {
        g_warning("read_data_from_backend_files() returned FALSE.");
    }

    g_ptr_array_free(backend_file_names, TRUE);

    return result;
}

gboolean alterator_manager_backends_init(gboolean is_session) {
    gboolean result = FALSE;
    gboolean res = FALSE;

    if (is_session) {
        res = alterator_manager_backends_load(KEY_FILES_PATH_USER);
        if (res) {
            result = res;
        } else {
            g_warning("alterator_manager_backends_load() returned FALSE "
                      "for %s.", KEY_FILES_PATH_USER);
        }

        res = alterator_manager_backends_load(KEY_FILES_PATH_ETC_USER);
        if (res) {
            result = res;
        } else {
            g_warning("alterator_manager_backends_load() returned FALSE "
                      "for %s.", KEY_FILES_PATH_ETC_USER);
        }
    } else {
        res = alterator_manager_backends_load(KEY_FILES_PATH_SYS);
        if (res) {
            result = res;
        } else {
            g_warning("alterator_manager_backends_load() returned FALSE "
                      "for %s.", KEY_FILES_PATH_SYS);
        }

        res = alterator_manager_backends_load(KEY_FILES_PATH_SYS_NEW);
        if (res) {
            result = res;
        } else {
            g_warning("alterator_manager_backends_load() returned FALSE "
                      "for %s.", KEY_FILES_PATH_SYS_NEW);
        }

        res = alterator_manager_backends_load(KEY_FILES_PATH_ETC_SYS);
        if (res) {
            result = res;
        } else {
            g_warning("alterator_manager_backends_load() returned FALSE "
                      "for %s.", KEY_FILES_PATH_ETC_SYS);
        }

        res = alterator_manager_backends_load(KEY_FILES_PATH_ETC_SYS_NEW);
        if (res) {
            result = res;
        } else {
            g_warning("alterator_manager_backends_load() returned FALSE "
                      "for %s.", KEY_FILES_PATH_ETC_SYS_NEW);
        }
    }

    return result;
}

GHashTable* alterator_manager_backends_get_data(void) {
    return backends_data;
}
