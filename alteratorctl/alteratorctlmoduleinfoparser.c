#include "alteratorctlmoduleinfoparser.h"
#include "alteratorctlcommon.h"
#include "alteratorctlgdbussource.h"

static GObjectClass *alterator_module_info_parser_parent_class               = NULL;
static gboolean alterator_ctl_module_info_parser_traverse_func_is_error_flag = FALSE;
static void alterator_module_info_parser_class_init(AlteratorCtlModuleInfoParserClass *klass);
static void alterator_module_info_parser_class_finalize(GObject *klass);

typedef int (*AlteratorCtlModuleInfoParserGetObjectsDataCallback)(gpointer self,
                                                                  GNode **object_data,
                                                                  GHashTable *objects_paths,
                                                                  const gchar *iface);

static int alterator_ctl_module_info_parser_get_specified_object_data(
    gpointer self, gpointer gdbus_source, const gchar *path, const gchar *iface, GNode **result);

static int alterator_ctl_module_info_parser_get_data_from_text(gpointer self,
                                                               gpointer gdbus_source,
                                                               const gchar *text,
                                                               GNode **result);

static GNode **alterator_ctl_module_info_parser_get_objects_data(gpointer self,
                                                                 gpointer gdbus_source,
                                                                 const gchar *iface,
                                                                 gsize *amount_of_objects);

static int alterator_ctl_module_info_parser_create_names_by_dbus_paths_table(gpointer self,
                                                                             gpointer gdbus_source,
                                                                             const gchar *iface);

static GNode *alterator_ctl_module_info_parser_get_node_by_name(gpointer self, GNode *root, const gchar *name);

static gboolean alterator_ctl_module_info_parser_find_table(
    gpointer self, GNode *root, GHashTable **result, const gchar *first_table_name, ...);

static gboolean alterator_ctl_module_info_parser_find_value(
    gpointer self, GNode *root, toml_value **result, const gchar *value_key, const gchar *first_table_name, ...);

static gboolean alterator_ctl_module_info_parser_find_node_callback(GNode *node, gpointer data);

static gboolean alterator_ctl_module_info_parser_find_table_callback(GNode *node, gpointer data);

static gboolean alterator_ctl_module_info_parser_find_value_callback(GNode *node, gpointer data);

static void alterator_ctl_module_info_parser_result_tree_free(gpointer self, GNode *root);

static void alterator_ctl_module_info_parser_result_trees_free(gpointer self, GNode **roots, gsize amount_of_trees);

static int alterator_ctl_module_info_parser_recursive_nodes_parse(toml_table_t *table,
                                                                  gchar *table_name,
                                                                  GNode **result);

static GNode **alterator_ctl_module_info_parser_get_objects_data_priv(
    gpointer self,
    gpointer gdbus_source,
    const gchar *iface,
    gsize *amount_of_objects,
    AlteratorCtlModuleInfoParserGetObjectsDataCallback callback);

static int alterator_ctl_module_info_parser_get_objects_names_callback(gpointer self,
                                                                       GNode **parsed_data,
                                                                       GHashTable *objects_paths,
                                                                       const gchar *iface);

static gboolean alterator_ctl_module_info_parser_node_free(GNode *node, gpointer data);

typedef struct
{
    gchar *key;
    toml_value *value;
} toml_value_traverse_ctx;

static void alterator_entry_info_toml_value_free(toml_value *value)
{
    if (value->type == TOML_DATA_STRING)
        g_free(value->str_value);
    else if (value->type == TOML_DATA_ARRAY_OF_STRING)
    {
        for (guint i = 0; i < value->array_length; i++)
            g_free(((gchar **) value->array)[i]);
        g_free(value->array);
    }
    else if (value->type == TOML_DATA_ARRAY_OF_STRING || value->type == TOML_DATA_ARRAY_OF_DOUBLE
             || value->type == TOML_DATA_ARRAY_OF_INT || value->type == TOML_DATA_ARRAY_OF_BOOL)
        g_free(value->array);

    g_free(value);
}

static void alterator_ctl_module_info_parser_traverse_set_error(const gchar *err_message)
{
    if (err_message)
        g_printerr("%s\n", err_message);

    alterator_ctl_module_info_parser_traverse_func_is_error_flag = TRUE;
}

static gboolean alterator_ctl_module_info_parser_traverse_func_is_error()
{
    if (alterator_ctl_module_info_parser_traverse_func_is_error_flag)
    {
        alterator_ctl_module_info_parser_traverse_func_is_error_flag = FALSE;
        return TRUE;
    }

    return FALSE;
}

GType alterator_module_info_parser_get_type()
{
    static GType alterator_module_info_parser_type = 0;

    if (!alterator_module_info_parser_type)
    {
        static const GTypeInfo alterator_module_info_parser_info
            = {sizeof(AlteratorCtlModuleInfoParserClass),                /* class structure size */
               NULL,                                                     /* base class initializer */
               NULL,                                                     /* base class finalizer */
               (GClassInitFunc) alterator_module_info_parser_class_init, /* class initializer */
               NULL,                                                     /* class finalizer */
               NULL,                                                     /* class data */
               sizeof(AlteratorCtlModuleInfoParser),                     /* instance structure size */
               1,                                                        /* preallocated instances */
               NULL,                                                     /* instance initializers */
               NULL};

        alterator_module_info_parser_type = g_type_register_static(G_TYPE_OBJECT, /* parent class */
                                                                   "ModuleInfoParser",
                                                                   &alterator_module_info_parser_info,
                                                                   0);
    }

    return alterator_module_info_parser_type;
}

static void alterator_module_info_parser_class_finalize(GObject *klass)
{
    G_OBJECT_CLASS(alterator_module_info_parser_parent_class)->finalize(klass);
}

static void alterator_module_info_parser_class_init(AlteratorCtlModuleInfoParserClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->finalize = alterator_module_info_parser_class_finalize;

    alterator_module_info_parser_parent_class = g_type_class_peek_parent(klass);

    return;
}

AlteratorCtlModuleInfoParser *alterator_module_info_parser_new()
{
    AlteratorCtlModuleInfoParser *object = g_object_new(TYPE_ALTERATOR_MODULE_INFO_PARSER, NULL);

    //Initialize functions pointers
    object->alterator_ctl_module_info_parser_get_specified_object_data
        = &alterator_ctl_module_info_parser_get_specified_object_data;

    object->alterator_ctl_module_info_parser_get_data_from_text = &alterator_ctl_module_info_parser_get_data_from_text;

    object->alterator_ctl_module_info_parser_get_objects_data = &alterator_ctl_module_info_parser_get_objects_data;

    object->alterator_ctl_module_info_parser_create_names_by_dbus_paths_table
        = &alterator_ctl_module_info_parser_create_names_by_dbus_paths_table;

    object->alterator_ctl_module_info_parser_get_node_by_name = &alterator_ctl_module_info_parser_get_node_by_name;

    object->alterator_ctl_module_info_parser_find_table = &alterator_ctl_module_info_parser_find_table;

    object->alterator_ctl_module_info_parser_find_value = &alterator_ctl_module_info_parser_find_value;

    object->alterator_ctl_module_info_parser_result_tree_free = &alterator_ctl_module_info_parser_result_tree_free;

    object->alterator_ctl_module_info_parser_result_trees_free = &alterator_ctl_module_info_parser_result_trees_free;

    return object;
}

void alterator_module_info_parser_free(AlteratorCtlModuleInfoParser *info_parser)
{
    if (info_parser->names_by_dbus_paths)
        g_hash_table_destroy(info_parser->names_by_dbus_paths);

    g_object_unref(info_parser);
}

static int alterator_ctl_module_info_parser_create_names_by_dbus_paths_table(gpointer self,
                                                                             gpointer gdbus_source,
                                                                             const gchar *iface)
{
    int ret                 = 0;
    GNode **result          = NULL;
    gsize amount_of_objects = 0;

    AlteratorGDBusSource *source = (AlteratorGDBusSource *) gdbus_source;
    if (!self)
    {
        g_printerr(_("Module info parser object wasn't created.\n"));
        ERR_EXIT();
    }

    if (!source)
    {
        g_printerr(_("Module info parser error. Can't get AlteratorCtlGDBusSource.\n"));
        ERR_EXIT();
    }

    if (!iface)
    {
        g_printerr(_("Module info parser error. Module iface isn't initialized.\n"));
        ERR_EXIT();
    }

    result = alterator_ctl_module_info_parser_get_objects_data_priv(
        self, gdbus_source, iface, &amount_of_objects, alterator_ctl_module_info_parser_get_objects_names_callback);

end:
    if (result)
        alterator_ctl_module_info_parser_result_trees_free(self, result, amount_of_objects);

    return ret;
}

static GNode *alterator_ctl_module_info_parser_get_node_by_name(gpointer self, GNode *root, const gchar *name)
{
    GNode *result = NULL;
    if (!self)
    {
        g_printerr(_("Module info parser object wasn't created.\n"));
        return NULL;
    }

    gpointer params[] = {(gpointer) &result, (gpointer) &name};
    g_node_traverse(root, G_IN_ORDER, G_TRAVERSE_ALL, -1, alterator_ctl_module_info_parser_find_node_callback, params);

    return result;
}

static GNode **alterator_ctl_module_info_parser_get_objects_data(gpointer self,
                                                                 gpointer gdbus_source,
                                                                 const gchar *iface,
                                                                 gsize *amount_of_objects)
{
    GNode **result               = NULL;
    AlteratorGDBusSource *source = (AlteratorGDBusSource *) gdbus_source;

    if (!self)
    {
        g_printerr(_("Module info parser object wasn't created.\n"));
        return NULL;
    }

    if (!source)
    {
        g_printerr(_("Module info parser error. Can't get AlteratorGDBusSource.\n"));
        return NULL;
    }

    if (!iface)
    {
        g_printerr(_("Module info parser error. Module iface isn't initialized.\n"));
        return NULL;
    }

    if (!amount_of_objects)
    {
        g_printerr(_("Invalid addres of amount_of_objects in module info parser.\n"));
        return NULL;
    }

    result = alterator_ctl_module_info_parser_get_objects_data_priv(self, gdbus_source, iface, amount_of_objects, NULL);

    return result;
}

static int alterator_ctl_module_info_parser_get_specified_object_data(
    gpointer self, gpointer gdbus_source, const gchar *path, const gchar *iface, GNode **result)
{
    int ret                             = 0;
    toml_table_t *alterator_entry_table = NULL;
    gchar *alterator_entry_text         = NULL;

    AlteratorGDBusSource *source = (AlteratorGDBusSource *) gdbus_source;

    if (!self)
    {
        g_printerr(_("Module info parser object wasn't created.\n"));
        ERR_EXIT();
    }

    if (!source)
    {
        g_printerr(_("Module info parser error. Can't get AlteratorGDBusSource.\n"));
        ERR_EXIT();
    }

    if (!path)
    {
        g_printerr(_("Module info parser error. Module path isn't initialized.\n"));
        ERR_EXIT();
    }

    if (!iface)
    {
        g_printerr(_("Module info parser error. Module iface isn't initialized.\n"));
        ERR_EXIT();
    }

    if (source->alterator_gdbus_source_get_text_of_alterator_entry_by_path(source, path, iface, &alterator_entry_text)
        < 0)
    {
        g_printerr(_("Can't parse alterator entry info. Can't get alterator entry text.\n"));
        ERR_EXIT();
    }

    char errbuf[TOML_ERROR_BUFFER_SIZE];
    alterator_entry_table = toml_parse((gchar *) alterator_entry_text, errbuf, sizeof(errbuf));
    if (!alterator_entry_table)
    {
        g_printerr(_("Can't parse alterator entry of %s, %s: %s\n"), path, iface, errbuf);
        ERR_EXIT();
    }

    alterator_ctl_module_info_parser_recursive_nodes_parse(alterator_entry_table, NULL, result);

end:
    if (alterator_entry_table)
        toml_free(alterator_entry_table);

    g_free(alterator_entry_text);

    return ret;
}

static int alterator_ctl_module_info_parser_get_data_from_text(gpointer self,
                                                               gpointer gdbus_source,
                                                               const gchar *text,
                                                               GNode **result)
{
    int ret                             = 0;
    toml_table_t *alterator_entry_table = NULL;

    AlteratorGDBusSource *source = (AlteratorGDBusSource *) gdbus_source;

    if (!self)
    {
        g_printerr(_("Module info parser object wasn't created.\n"));
        ERR_EXIT();
    }

    if (!source)
    {
        g_printerr(_("Module info parser error. Can't get AlteratorGDBusSource.\n"));
        ERR_EXIT();
    }

    if (!text)
    {
        g_printerr(_("Module info parser error. alterator entry text is empty.\n"));
        ERR_EXIT();
    }

    char errbuf[TOML_ERROR_BUFFER_SIZE];
    alterator_entry_table = toml_parse((gchar *) text, errbuf, sizeof(errbuf));
    if (!alterator_entry_table)
    {
        g_printerr(_("Can't parse alterator entry.\n"));
        ERR_EXIT();
    }

    alterator_ctl_module_info_parser_recursive_nodes_parse(alterator_entry_table, NULL, result);

end:
    if (alterator_entry_table)
        toml_free(alterator_entry_table);

    return ret;
}

static gboolean alterator_ctl_module_info_parser_find_table(
    gpointer self, GNode *root, GHashTable **result, const gchar *first_table_name, ...)
{
    GHashTable *toml_pairs = NULL;
    GList *tables_list     = NULL;
    va_list args;
    va_start(args, first_table_name);

    const char *current = first_table_name;
    while (current)
    {
        tables_list = g_list_append(tables_list, g_strdup(current));
        current     = va_arg(args, const char *);
    }
    va_end(args);

    GList *first_elem  = tables_list;
    gpointer params[2] = {(gpointer) &toml_pairs, (gpointer) &tables_list};
    g_node_traverse(root, G_IN_ORDER, G_TRAVERSE_ALL, -1, alterator_ctl_module_info_parser_find_table_callback, params);

    for (GList *current_elem = first_elem; current_elem != NULL; current_elem = current_elem->next)
        g_free(current_elem->data);

    if (first_elem)
        g_list_free(first_elem);

    if (result)
        (*result) = toml_pairs;

    return toml_pairs != NULL;
}

static gboolean alterator_ctl_module_info_parser_find_value(
    gpointer self, GNode *root, toml_value **finded_data, const gchar *value_key, const gchar *first_table_name, ...)
{
    GList *tables_list = NULL;
    va_list args;
    va_start(args, first_table_name);
    toml_value_traverse_ctx ctx;
    ctx.key   = (gchar *) value_key;
    ctx.value = NULL;

    const char *current = first_table_name;
    while (current)
    {
        if (!g_list_append(tables_list, (gpointer) current))
        {
            if (tables_list)
                g_list_free(tables_list);
            va_end(args);
            return FALSE;
        }
        current = va_arg(args, const char *);
    }
    va_end(args);

    gpointer params[2] = {(gpointer) &ctx, (gpointer) tables_list};
    g_node_traverse(root, G_IN_ORDER, G_TRAVERSE_ALL, -1, alterator_ctl_module_info_parser_find_value_callback, params);

    if (tables_list)
        g_list_free(tables_list);

    if (finded_data)
        (*finded_data) = ctx.value;

    return ctx.value != NULL;
}

static GNode **alterator_ctl_module_info_parser_get_objects_data_priv(
    gpointer self,
    gpointer gdbus_source,
    const gchar *iface,
    gsize *amount_of_objects,
    AlteratorCtlModuleInfoParserGetObjectsDataCallback callback)
{
    GNode **result;
    GHashTable *objects = NULL;

    AlteratorGDBusSource *source = (AlteratorGDBusSource *) gdbus_source;

    if (source->alterator_gdbus_source_get_iface_objects(source, iface, &objects) < 0)
    {
        g_printerr(_("Can't get list of objects paths by iface %s in module info parser.\n"), iface);
        return NULL;
    }

    if (!objects)
        return NULL;

    *amount_of_objects = g_hash_table_size(objects);
    if (!(*amount_of_objects))
    {
        g_hash_table_destroy(objects);
        return NULL;
    }

    result = (GNode **) g_malloc0((*amount_of_objects) * sizeof(GNode *));

    GHashTableIter iter;
    gpointer object_path = NULL, value = NULL;
    g_hash_table_iter_init(&iter, objects);

    int node_index = 0;
    while (g_hash_table_iter_next(&iter, &object_path, &value))
    {
        if (alterator_ctl_module_info_parser_get_specified_object_data(self,
                                                                       source,
                                                                       (const gchar *) object_path,
                                                                       iface,
                                                                       &result[node_index])
            < 0)
        {
            g_free(result);
            return NULL;
        }
        node_index++;
    }

    if (callback)
        callback(self, result, objects, iface);

    g_hash_table_destroy(objects);

    return result;
}

static int alterator_ctl_module_info_parser_recursive_nodes_parse(toml_table_t *table, gchar *table_name, GNode **result)
{
    int ret                = 0;
    GHashTable *toml_pairs = g_hash_table_new_full(g_str_hash,
                                                   g_str_equal,
                                                   (GDestroyNotify) g_free,
                                                   (GDestroyNotify) alterator_entry_info_toml_value_free);

    if (!table)
    {
        g_printerr(_("Can't parse toml file: Uninitialized toml_table_t.\n"));
        ERR_EXIT();
    }

    if (!result)
    {
        g_printerr(_("Can't parse toml file: GNode **result is NULL.\n"));
        ERR_EXIT();
    }

    alterator_entry_node *node_data = g_malloc0(sizeof(alterator_entry_node));
    node_data->node_name            = (table_name) ? g_strdup(table_name) : g_strdup("");
    *result                         = g_node_new(NULL);

    for (int i = 0;; i++)
    {
        const gchar *key = toml_key_in(table, i);
        if (!key)
            break;

        toml_table_t *subtable = NULL;
        toml_datum_t parsed_value;
        toml_array_t *parsed_array;
        if ((parsed_value = toml_string_in(table, key)).ok)
        {
            toml_value *value = g_malloc0(sizeof(toml_value));
            value->type       = TOML_DATA_STRING;
            value->str_value  = parsed_value.u.s;
            g_hash_table_insert(toml_pairs, g_strdup(key), value);
        }
        else if ((parsed_value = toml_double_in(table, key)).ok)
        {
            toml_value *value   = g_malloc0(sizeof(toml_value));
            value->type         = TOML_DATA_DOUBLE;
            value->double_value = parsed_value.u.d;
            g_hash_table_insert(toml_pairs, g_strdup(key), value);
        }
        else if ((parsed_value = toml_bool_in(table, key)).ok)
        {
            toml_value *value = g_malloc0(sizeof(toml_value));
            value->type       = TOML_DATA_BOOL;
            value->bool_value = parsed_value.u.b;
            g_hash_table_insert(toml_pairs, g_strdup(key), value);
        }
        else if ((subtable = toml_table_in(table, key)))
        {
            GNode *child = NULL;
            if (alterator_ctl_module_info_parser_recursive_nodes_parse(subtable, (gchar *) key, &child) < 0)
            {
                g_free(node_data);
                ERR_EXIT();
            }
            else
                g_node_append(*result, child);
        }
        else if ((parsed_array = toml_array_in(table, key)))
        {
            toml_value *value   = g_malloc0(sizeof(toml_value));
            value->array_length = toml_array_nelem(parsed_array);

            char array_type = toml_array_type(parsed_array);

            switch (array_type)
            {
            case 'i': {
                value->type  = TOML_DATA_ARRAY_OF_INT;
                value->array = (gpointer *) g_malloc0(sizeof(int64_t) * value->array_length);
                for (gsize i = 0; i < value->array_length; i++)
                {
                    toml_raw_t raw = toml_raw_at(parsed_array, i);
                    if (raw)
                        toml_rtoi(raw, &((int64_t *) value->array)[i]);
                }
                break;
            }
            case 'd': {
                value->type  = TOML_DATA_ARRAY_OF_DOUBLE;
                value->array = (gpointer *) g_malloc0(sizeof(double) * value->array_length);
                for (gsize i = 0; i < value->array_length; i++)
                {
                    toml_raw_t raw = toml_raw_at(parsed_array, i);
                    if (raw)
                        toml_rtod(raw, &((double *) value->array)[i]);
                }
                break;
            }
            case 'b': {
                value->type  = TOML_DATA_ARRAY_OF_BOOL;
                value->array = (gpointer *) g_malloc0(sizeof(int) * value->array_length);
                for (gsize i = 0; i < value->array_length; i++)
                {
                    toml_raw_t raw = toml_raw_at(parsed_array, i);
                    if (raw)
                        toml_rtob(raw, &((int *) value->array)[i]);
                }
                break;
            }
            case 's': {
                value->type  = TOML_DATA_ARRAY_OF_STRING;
                value->array = (gpointer *) g_malloc0(sizeof(gchar *) * value->array_length);
                for (gsize i = 0; i < value->array_length; i++)
                {
                    toml_raw_t raw = toml_raw_at(parsed_array, i);
                    if (raw)
                        ((gchar **) value->array)[i] = g_utf8_substring(raw, 1, strlen(raw) - 1);
                }
                break;
            }
            default: {
                // STUB
                value->type = TOML_DATA_ARRAY_OF_MIXED_TYPES;
            }
            }

            g_hash_table_insert(toml_pairs, g_strdup(key), value);
        }
        else
        {
            g_printerr(_("It is impossible to determine the data type of the value by key %s.\n"), key);
            g_free(node_data);
            ERR_EXIT();
        }
    }

    node_data->toml_pairs = g_hash_table_ref(toml_pairs);
    (*result)->data       = node_data;

end:

    if (toml_pairs)
        g_hash_table_unref(toml_pairs);

    return ret;
}

static void alterator_ctl_module_info_parser_result_tree_free(gpointer self, GNode *root)
{
    if (!root)
        return;

    g_node_traverse(root,
                    G_IN_ORDER,
                    G_TRAVERSE_ALL,
                    -1,
                    (GNodeTraverseFunc) alterator_ctl_module_info_parser_node_free,
                    NULL);
    g_node_destroy(root);
}

static void alterator_ctl_module_info_parser_result_trees_free(gpointer self, GNode **roots, gsize amount_of_trees)
{
    if (!roots)
        return;

    for (gsize i = 0; i < amount_of_trees; i++)
        alterator_ctl_module_info_parser_result_tree_free(self, roots[i]);

    g_free(roots);
}

static gboolean alterator_ctl_module_info_parser_find_node_callback(GNode *node, gpointer data)
{
    gpointer *params                = (gpointer *) data;
    GNode **result                  = (GNode **) params[0];
    gchar **target_node_name        = (gchar **) params[1];
    alterator_entry_node *node_data = (alterator_entry_node *) node->data;

    if (g_strcmp0(node_data->node_name, *target_node_name) == 0)
    {
        (*result) = node;
        return TRUE;
    }

    return FALSE;
}

static gboolean alterator_ctl_module_info_parser_find_table_callback(GNode *node, gpointer data)
{
    gpointer *params         = (gpointer *) data;
    GHashTable **result      = (GHashTable **) params[0];
    GList **table_names_list = (GList **) params[1];

    alterator_entry_node *node_data = (alterator_entry_node *) node->data;

    if (g_strcmp0(node_data->node_name, (gchar *) (*table_names_list)->data) == 0)
    {
        (*table_names_list) = (*table_names_list)->next;

        if (!(*table_names_list))
        {
            if (result)
                (*result) = node_data->toml_pairs;
            return TRUE;
        }
    }

    return FALSE;
}

static gboolean alterator_ctl_module_info_parser_find_value_callback(GNode *node, gpointer data)
{
    gpointer *params                = (gpointer *) data;
    toml_value_traverse_ctx *ctx    = (toml_value_traverse_ctx *) params[0];
    GList *table_names_list         = (GList *) params[1];
    alterator_entry_node *node_data = (alterator_entry_node *) node->data;

    // Finding in all document
    if (!table_names_list)
    {
        if (!g_hash_table_contains(node_data->toml_pairs, ctx->key))
            return FALSE;

        toml_value *value = NULL;
        if ((value = (toml_value *) g_hash_table_lookup(node_data->toml_pairs, ctx->key)))
            ctx->value = value;

        return value && ctx->value != NULL;
    }

    GList *current_list_elem = table_names_list;
    if (g_strcmp0(node_data->node_name, (gchar *) current_list_elem->data) == 0)
    {
        current_list_elem = current_list_elem->next;

        if (!current_list_elem)
        {
            ctx->value = g_hash_table_lookup(node_data->toml_pairs, ctx->key);
            return TRUE;
        }
    }

    return FALSE;
}

static int alterator_ctl_module_info_parser_get_objects_names_callback(gpointer self,
                                                                       GNode **parsed_data,
                                                                       GHashTable *objects_paths,
                                                                       const gchar *iface)
{
    int ret = 0;

    if (!self)
    {
        g_printerr(_("Module info parser error: Module info parser object wasn't created.\n"));
        ERR_EXIT();
    }

    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) self;
    if (info_parser->names_by_dbus_paths && g_hash_table_size(info_parser->names_by_dbus_paths))
        g_hash_table_remove_all(info_parser->names_by_dbus_paths);

    if (!parsed_data)
    {
        g_printerr(_("Module info parser error: No alterator entries data.\n"));
        ERR_EXIT();
    }

    if (!objects_paths)
        ERR_EXIT();

    if (!info_parser->names_by_dbus_paths)
        info_parser->names_by_dbus_paths = g_hash_table_new_full(g_str_hash,
                                                                 g_str_equal,
                                                                 (GDestroyNotify) g_free,
                                                                 (GDestroyNotify) g_free);

    GHashTableIter iter;
    gpointer path = NULL, value = NULL;
    g_hash_table_iter_init(&iter, objects_paths);

    int node_index = 0;
    while (g_hash_table_iter_next(&iter, &path, &value))
    {
        toml_value_traverse_ctx ctx;
        ctx.key                  = "name";
        ctx.value                = NULL;
        gpointer travers_data[2] = {&ctx, NULL};
        if (!alterator_ctl_module_info_parser_find_value_callback(parsed_data[node_index], travers_data))
        {
            g_printerr(_("Module info parser error: Object with path %s and iface %s has no name found.\n"),
                       (const gchar *) path,
                       iface);

            alterator_ctl_module_info_parser_result_trees_free(self, parsed_data, node_index + 1);
            g_hash_table_destroy(info_parser->names_by_dbus_paths);
            ERR_EXIT();
        }

        g_hash_table_insert(info_parser->names_by_dbus_paths, g_strdup(ctx.value->str_value), g_strdup((gchar *) path));
        node_index++;
    }

end:
    return ret;
}

static gboolean alterator_ctl_module_info_parser_node_free(GNode *node, gpointer data)
{
    if (node->data)
    {
        alterator_entry_node *node_data = (alterator_entry_node *) node->data;
        g_hash_table_destroy((GHashTable *) node_data->toml_pairs);
        g_free(node_data->node_name);
        g_free(node->data);
    }
    return FALSE;
}
