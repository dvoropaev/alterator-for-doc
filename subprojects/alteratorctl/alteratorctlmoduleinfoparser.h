#ifndef MODULEINFOPARSER_H
#define MODULEINFOPARSER_H

#include <gio/gio.h>
#include <glib-object.h>
#include <stdarg.h>
#include <toml.h>

#define TYPE_ALTERATOR_MODULE_INFO_PARSER (alterator_module_info_parser_get_type())
#define ALTERATOR_INFO_PARSER(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_ALTERATOR_MODULE_INFO_PARSER, ModuleInfoParser))
#define IS_ALTERATOR_INFO_PARSER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_ALTERATOR_MODULE_INFO_PARSER))
#define ALTERATOR_INFO_PARSER_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_ALTERATOR_MODULE_INFO_PARSER, ModuleInfoParserClass))
#define IS_ALTERATOR_INFO_PARSER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_ALTERATOR_MODULE_INFO_PARSER))
#define ALTERATOR_INFO_PARSER_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_ALTERATOR_MODULE_INFO_PARSER, ModuleInfoParserClass))

typedef enum
{
    TOML_DATA_STRING,
    TOML_DATA_DOUBLE,
    TOML_DATA_BOOL,
    TOML_DATA_ARRAY_OF_INT,
    TOML_DATA_ARRAY_OF_DOUBLE,
    TOML_DATA_ARRAY_OF_BOOL,
    TOML_DATA_ARRAY_OF_STRING,
    TOML_DATA_ARRAY_OF_MIXED_TYPES
} toml_value_type;

typedef struct
{
    toml_value_type type;
    union
    {
        gchar *str_value;
        double double_value;
        gboolean bool_value;
    };
    gpointer *array;
    gsize array_length;
} toml_value;

gboolean alterator_ctl_compare_toml_values(toml_value *first, toml_value *second, GError **error);

typedef struct
{
    gchar *node_name;
    GHashTable *toml_pairs;
} alterator_entry_node;

typedef struct
{
    GObjectClass parent_instance;

} AlteratorCtlModuleInfoParserClass;

typedef struct
{
    GObject parent_instance;

    GHashTable *names_by_dbus_paths;

    int (*alterator_ctl_module_info_parser_get_specified_object_data)(
        gpointer self, gpointer gdbus_source, const gchar *path, const gchar *iface, GNode **result);

    int (*alterator_ctl_module_info_parser_get_data_from_text)(gpointer self,
                                                               gpointer gdbus_source,
                                                               const gchar *text,
                                                               GNode **result);

    GNode **(*alterator_ctl_module_info_parser_get_objects_data)(gpointer self,
                                                                 gpointer gdbus_source,
                                                                 const gchar *iface,
                                                                 gsize *amount_of_objects);

    int (*alterator_ctl_module_info_parser_create_names_by_dbus_paths_table)(gpointer self,
                                                                             gpointer gdbus_source,
                                                                             const gchar *iface);

    GNode *(*alterator_ctl_module_info_parser_get_node_by_name)(gpointer self,
                                                                GNode *root,
                                                                const gchar *name,
                                                                guint depth);

    // Depth = -1 -> unlimited depth
    gboolean (*alterator_ctl_module_info_parser_find_table)(
        gpointer self, GNode *root, GHashTable **result, guint depth, const gchar *first_table_name, ...);

    gboolean (*alterator_ctl_module_info_parser_find_value)(
        gpointer self, GNode *root, toml_value **result, const gchar *value_key, const gchar *first_table_name, ...);

} AlteratorCtlModuleInfoParser;

AlteratorCtlModuleInfoParser *alterator_module_info_parser_new();

void alterator_module_info_parser_free(AlteratorCtlModuleInfoParser *info_parser);

void alterator_ctl_module_info_parser_result_tree_free(GNode *root);

void alterator_ctl_module_info_parser_result_trees_free(GNode **roots, gsize amount_of_trees);

GNode *alterator_ctl_module_info_parser_tree_deep_copy(GNode *tree);

#endif //MODULEINFOPARSER_H
