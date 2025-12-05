#include "alteratorctlcomponentsmodule.h"
#include "alteratorctleditionsmodule.h"
#include "alteratorctlpackagesmodule.h"
#include "alteratorctlsysteminfomodule.h"

#include <gio/gioenums.h>
#include <glib/gregex.h>
#include <locale.h>
#include <toml.h>

#define COMPONENT_INTERFACE_NAME "org.altlinux.alterator.component1"
#define COMPONENT_DESCRIPTION_METHOD_NAME "Description"
#define COMPONENT_INFO_METHOD_NAME "Info"
#define COMPONENT_STATUS_METHOD_NAME "Status"
#define COMPONENT_INFO_PACKAGES_DELIMITER " "

#define COMPONENT_CATEGORIES_INTERFACE_NAME "org.altlinux.alterator.component_categories1"
#define COMPONENTS_CATEGORIES_INFO_METHOD_NAME "Info"
#define COMPONENTS_CATEGORIES_LIST_METHOD_NAME "List"

#define BATCH_COMPONENTS_INTERFACE_NAME "org.altlinux.alterator.batch_components1"
#define BATCH_COMPONENTS_INFO_METHOD_NAME "Info"
#define BATCH_COMPONENTS_STATUS_METHOD_NAME "Status"

#define BATCH_COMPONENT_CATEGORIES_INTERFACE_NAME "org.altlinux.alterator.batch_component_categories1"
#define BATCH_COMPONENT_CATEGORIES_INFO_METHOD_NAME "Info"

#define COMPONENT_ALTERATOR_ENTRY_COMPONENT_DRAFT_KEY_NAME "draft"
#define COMPONENT_ALTERATOR_ENTRY_COMPONENT_NAME_KEY_NAME "name"
#define COMPONENT_ALTERATOR_ENTRY_COMPONENT_CATEGORY_KEY_NAME "category"
#define COMPONENT_ALTERATOR_ENTRY_COMPONENT_PACKAGES_TABLE_NAME "packages"
#define COMPONENT_ALTERATOR_ENTRY_COMPONENT_DISPLAY_NAME_TABLE_NAME "display_name"

#define COMPONENT_CATEGORY_ALTERATOR_ENTRY_COMPONENT_DRAFT_KEY_NAME "draft"
#define COMPONENT_CATEGORY_ALTERATOR_ENTRY_COMPONENT_NAME_KEY_NAME "name"
#define COMPONENT_CATEGORY_ALTERATOR_ENTRY_COMPONENT_CATEGORY_KEY_NAME "category"
#define COMPONENT_CATEGORY_ALTERATOR_ENTRY_COMPONENT_PACKAGE_EXCLUDE_ARCH_KEY_NAME "exclude_arch"
#define COMPONENT_CATEGORY_ALTERATOR_ENTRY_COMPONENT_PACKAGE_ARCH_KEY_NAME "arch"
#define COMPONENT_CATEGORY_ALTERATOR_ENTRY_COMPONENT_PACKAGE_KERNEL_KEY_NAME "kernel_module"
#define COMPONENT_CATEGORY_ALTERATOR_ENTRY_COMPONENT_PACKAGE_LANGUAGE_KEY_NAME "language"
#define COMPONENT_CATEGORY_ALTERATOR_ENTRY_COMPONENT_PACKAGE_DESKTOP_KEY_NAME "desktop"

#define COMPONENTS_EDITION_SECTION_TABLE_NAME "sections"
#define COMPONENTS_EDITION_SECTION_DISPLAY_NAME_TABLE_NAME "display_name"
#define COMPONENTS_EDITION_SECTION_COMPONENTS_ARRAY_KEY_NAME "components"
#define COMPONENTS_EDITION_SECTION_NAME_KEY_NAME "name"

// Output list delimiters: runtime-selected ASCII when non-UTF, Unicode otherwise
static const char *components_module_tree_top_glyph    = "┌";
static const char *components_module_tree_vline_glyph  = "│";
static const char *components_module_tree_hline_glyph  = "─";
static const char *components_module_tree_branch_glyph = "├";
static const char *components_module_tree_end_glyph    = "└";

static void components_module_setup_tree_glyphs(void)
{
    if (!alterator_ctl_is_utf8_locale())
    {
        components_module_tree_top_glyph    = "+";
        components_module_tree_vline_glyph  = "|";
        components_module_tree_hline_glyph  = "-";
        components_module_tree_branch_glyph = "+";
        components_module_tree_end_glyph    = "`";
    }
}

#define COMPONENTS_MODULE_TREE_TOP components_module_tree_top_glyph
#define COMPONENTS_MODULE_TREE_VLINE components_module_tree_vline_glyph
#define COMPONENTS_MODULE_TREE_HLINE components_module_tree_hline_glyph
#define COMPONENTS_MODULE_TREE_BRANCH components_module_tree_branch_glyph
#define COMPONENTS_MODULE_TREE_END components_module_tree_end_glyph

#define COMPONENTS_MODULE_COMPONENT_INSTALLED_MARKER "[*]"
#define COMPONENTS_MODULE_COMPONENT_NOT_INSTALLED_MARKER "[ ]"
#define COMPONENTS_MODULE_COMPONENT_PARTIALLY_INSTALLED_MARKER "[~]"
#define COMPONENTS_MODULE_COMPONENT_UNKNOWN "[?]"

// TODO: for temporary optimisation with working with names
#define COMPONENTS_MODULE_PREFIX_PATH "/org/altlinux/alterator/component_"

typedef struct components_module_subcommands_t
{
    char *subcommand;
    enum components_sub_commands id;
} components_module_subcommands_t;

typedef enum
{
    CATEGORY,
    COMPONENT
} components_module_tree_elem_type;

typedef enum
{
    INSTALLED,
    NOT_INSTALLED,
    PARTIALLY_INSTALLED,
    NONE
} components_module_item_installed_status;

typedef enum
{
    INSTALL,
    REMOVE
} components_module_transaction_mode;

typedef struct components_module_tree_elem_t
{
    components_module_tree_elem_type type;
    gchar *name;
    gchar *category;
    gchar *display_name;
    components_module_item_installed_status installed_status;
} components_module_tree_elem_t;

static components_module_subcommands_t components_module_subcommands_list[] = {{"info", COMPONENTS_INFO},
                                                                               {"list", COMPONENTS_LIST},
                                                                               {"install", COMPONENTS_INSTALL},
                                                                               {"remove", COMPONENTS_REMOVE},
                                                                               {"description", COMPONENTS_DESCRIPTION},
                                                                               {"search", COMPONENTS_SEARCH},
                                                                               {"status", COMPONENTS_STATUS}};

static GObjectClass *components_module_parent_class = NULL;
static alterator_ctl_module_t components_module     = {0};
static gboolean draft                               = FALSE;
static gboolean name_only                           = FALSE;
static gboolean path_only                           = FALSE;
static gboolean display_name_only                   = FALSE;
static gboolean no_display_name                     = FALSE;
static gboolean enable_display_name                 = FALSE;
static gboolean no_name                             = FALSE;
static gboolean show_installed                      = FALSE;
static gboolean show_uninstalled                    = FALSE;
static gboolean print_pseudo_graphic_tree           = FALSE;
static gboolean print_simple_tree                   = FALSE;
static gboolean print_list                          = FALSE;
static gboolean ignore_sections                     = FALSE;
static gboolean ignore_legend                       = FALSE;
static gboolean no_apt_update                       = FALSE;
static gboolean hide_installed_markers              = FALSE;
static gboolean allow_remove_manually               = FALSE;
static gboolean allow_remove_base_components        = FALSE;
static gboolean force_yes                           = FALSE;
static gchar *printing_category_name                = NULL;
static gchar **chosen_sections                      = NULL;

static void components_module_class_init(AlteratorCtlComponentsModuleClass *klass);
static void components_ctl_class_finalize(GObject *klass);

static void components_module_alterator_interface_init(gpointer iface, gpointer iface_data);
static void components_module_alterator_interface_finalize(gpointer iface, gpointer iface_data);

static AlteratorCtlComponentsModule *components_module_new(gpointer app);
static void components_module_free(AlteratorCtlComponentsModule *module);

static void fill_command_hash_table(GHashTable *command);

static int components_module_parse_options(AlteratorCtlComponentsModule *module, int *argc, char **argv);
static int components_module_parse_arguments(AlteratorCtlComponentsModule *module,
                                             int argc,
                                             char **argv,
                                             alteratorctl_ctx_t **ctx);

static int components_module_list_subcommand(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx);
static int components_module_info_subcommand(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx);
static int components_module_install_subcommand(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx);
static int components_module_remove_subcommand(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx);
static int components_module_description_subcommand(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx);
static int components_module_status_subcommand(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx);
static int components_module_search_subcommand(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx);

static int components_module_handle_results(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t *ctx);
static int components_module_handle_info_results(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx);
static int components_module_handle_list_results(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx);
static int components_module_handle_install_results(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx);
static int components_module_handle_remove_results(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx);
static int components_module_handle_description_results(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx);
static int components_module_handle_status_results(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx);

static int components_module_print_list_with_filters(AlteratorCtlComponentsModule *module,
                                                     GNode *root,
                                                     const gchar *section_display_name,
                                                     const gchar *section_name,
                                                     gboolean is_first_section,
                                                     gboolean is_last_section);

static GNode **components_module_get_section_nodes(AlteratorCtlComponentsModule *module, gsize *amount_of_sections);

static int components_module_get_kflavour(gchar **result);
static int components_module_get_arch(AlteratorCtlComponentsModule *module, gchar **result);
static gchar **components_module_get_locales(AlteratorCtlComponentsModule *module, gsize *len);
static gchar **components_module_get_desktop_environments(AlteratorCtlComponentsModule *module, gsize *len);

static int components_module_is_ignore_package_by_arch(AlteratorCtlComponentsModule *module,
                                                       GNode *package_data,
                                                       gboolean *result,
                                                       gchar *optional_current_arch);
static int components_module_is_ignore_package_by_language(AlteratorCtlComponentsModule *module,
                                                           GNode *package_data,
                                                           gboolean *result,
                                                           gchar **optional_current_locales,
                                                           gsize len);
static int components_module_is_ignore_package_by_desktop_env(AlteratorCtlComponentsModule *module,
                                                              GNode *package_data,
                                                              gboolean *result,
                                                              gchar **optional_current_desktop_envs,
                                                              gboolean use_optional_current_desktop_envs,
                                                              gsize len);
static int components_module_get_package_name(AlteratorCtlComponentsModule *module, GNode *package_data, gchar **result);

static void components_module_print_legend();

static int components_module_print_pseudo_graphic_tree(AlteratorCtlComponentsModule *module, GNode *node);
static int components_module_print_pseudo_graphic_tree_to_buffer(AlteratorCtlComponentsModule *module,
                                                                 GNode *node,
                                                                 GString *master_buffer);
static int components_module_print_pseudo_graphic_subtree(
    AlteratorCtlComponentsModule *module, GNode *node, gchar *prefix, gboolean is_last, gboolean is_first);

static int components_module_print_tree(AlteratorCtlComponentsModule *module, GNode *node);
static int components_module_print_tree_to_buffer(AlteratorCtlComponentsModule *module,
                                                  GNode *node,
                                                  GString *master_buffer);
static int components_module_print_subtree(AlteratorCtlComponentsModule *module, GNode *node, gchar *prefix);

static int components_module_print_list(AlteratorCtlComponentsModule *module, GNode *node);
static int components_module_print_list_to_buffer(AlteratorCtlComponentsModule *module,
                                                  GNode *node,
                                                  GString *master_buffer);
static int components_module_print_sublist(AlteratorCtlComponentsModule *module, GNode *node, gchar *prefix);

static int components_module_update_all_trees_elems_is_installed(GNode *root);
static int components_module_unlink_all_trees_uninstalled_elems(GNode *root);
static int components_module_unlink_all_trees_installed_elems(GNode *root);
static gboolean components_module_update_tree_elem_is_installed(GNode *root, gpointer data);
static gboolean components_module_unlink_uninstalled_tree_elem(GNode *root, gpointer data);
static gboolean components_module_unlink_installed_tree_elem(GNode *root, gpointer data);
static GNode *components_module_get_tree_elem_by_name(GNode *root, const gchar *name);
static int components_module_sort_tree_elems_by_name(AlteratorCtlComponentsModule *module, GNode **root);

static gboolean components_module_get_tree_elem_by_name_traverse_func(GNode *node, gpointer data);
static gboolean components_module_sort_childrens_by_name_traverse_func(GNode *node, gpointer data);
static gint components_module_sort_strings(gconstpointer a, gconstpointer b);
static gint components_module_sort_tree_compare_func(gconstpointer a, gconstpointer b, gpointer user_data);

static int components_module_validate_alterator_entry(AlteratorGDBusSource *source, GNode *alterator_entry_data);

static int components_module_get_components(AlteratorCtlComponentsModule *module, GHashTable **result);
static int components_module_install_component(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx);
static int components_module_remove_component(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx);

static int components_module_get_components_info(AlteratorCtlComponentsModule *module, GHashTable **result);

static int components_module_get_components_uninstalled_packages(AlteratorCtlComponentsModule *module,
                                                                 GHashTable **result);

static int components_module_get_component_status_by_uninstalled_packages_list(
    AlteratorCtlComponentsModule *module,
    const GNode *parsed_component,
    const GHashTable *uninstalled_packages,
    gchar *current_arch,
    gchar **current_locales,
    gsize locales_len,
    gchar **current_desktop_environments,
    gsize env_len,
    components_module_item_installed_status *status);

static int components_module_get_component_packages(AlteratorCtlComponentsModule *module,
                                                    const char *component_info,
                                                    GHashTable **packages);
static int components_module_get_installed_packages(AlteratorCtlComponentsModule *module, GHashTable **packages);
static int components_module_remove_packages_from_hash_table(AlteratorCtlComponentsModule *module,
                                                             GHashTable **from_hash_table,
                                                             GHashTable **lookup_hash_table);

static int components_module_apt_update_packages_list(AlteratorCtlComponentsModule *module);

static int components_module_install_component_packages(AlteratorCtlComponentsModule *module,
                                                        gchar *component_str_id,
                                                        GHashTable *components_packages);

static int components_module_remove_component_packages(AlteratorCtlComponentsModule *module,
                                                       gchar *component_str_id,
                                                       GHashTable *components_packages);

static int components_module_calculate_affected_components(AlteratorCtlComponentsModule *module,
                                                           const gchar *target_component_str_id,
                                                           gchar **packages,
                                                           components_module_transaction_mode action,
                                                           GHashTable **result);

static int components_module_print_affected_components(AlteratorCtlComponentsModule *module,
                                                       gchar *target_component_str_id,
                                                       GVariant *packages_check_apply_result,
                                                       components_module_transaction_mode action,
                                                       gboolean target_print_separately);

static int components_module_get_base_section_packages(AlteratorCtlComponentsModule *module, GHashTable **result);

static int components_module_get_component_status(AlteratorCtlComponentsModule *module,
                                                  char *component_str_id,
                                                  int *status,
                                                  GHashTable **installed_packages);

static int components_module_get_packages_of_component(AlteratorCtlComponentsModule *module,
                                                       GHashTable **components_packages,
                                                       const gchar *component_str_id,
                                                       gboolean is_installed_component);

static int components_module_validate_object_and_iface(AlteratorCtlComponentsModule *module,
                                                       const gchar *object,
                                                       const gchar *iface);

static int components_module_categories_validate_alterator_entry(AlteratorGDBusSource *source,
                                                                 GNode *alterator_entry_data);

static int components_module_get_category_info(AlteratorCtlComponentsModule *module,
                                               const gchar *category_str_id,
                                               gchar **result);

static int components_module_get_categories_info(AlteratorCtlComponentsModule *module, GHashTable **result);

static int components_module_categories_list(AlteratorCtlComponentsModule *module, GHashTable **result);

static void components_module_tree_elem_init(components_module_tree_elem_t *elem,
                                             components_module_tree_elem_type type,
                                             gchar *name,
                                             gchar *display_name,
                                             gchar *category);

static void components_module_tree_elem_free(gpointer node_elem);

static void components_module_create_categories_links(gpointer key, gpointer value, gpointer user_data);

static void components_module_create_sections_categories_links(gpointer key, gpointer value, gpointer user_data);

static void components_module_delete_sections_categories_links(gpointer key, gpointer value, gpointer user_data);

static void components_module_reset_all_trees_category_installed_status(gpointer key,
                                                                        gpointer value,
                                                                        gpointer user_data);

static int components_module_get_display_name(AlteratorCtlComponentsModule *module,
                                              const gchar *elem_str_id,
                                              components_module_tree_elem_type type,
                                              GNode *data,
                                              gchar **result);

static int components_module_get_section_display_name(AlteratorCtlComponentsModule *module, GNode *data, gchar **result);

// TODO: temporary functions for optimization. pending changes in alterator-manager
static gchar *components_module_name_from_path(const gchar *path);

static gchar *components_module_path_from_name(const gchar *name);

GType alterator_ctl_components_module_get_type(void)
{
    static GType components_module_type = 0;

    if (!components_module_type)
    {
        static const GTypeInfo components_module_info
            = {sizeof(AlteratorCtlComponentsModuleClass),     /* class structure size */
               NULL,                                          /* base class initializer */
               NULL,                                          /* base class finalizer */
               (GClassInitFunc) components_module_class_init, /* class initializer */
               NULL,                                          /* class finalizer */
               NULL,                                          /* class data */
               sizeof(AlteratorCtlComponentsModule),          /* instance structure size */
               1,                                             /* preallocated instances */
               NULL,                                          /* instance initializers */
               NULL};

        const GInterfaceInfo alterator_module_interface_info = {
            (GInterfaceInitFunc) components_module_alterator_interface_init,         /* interface_init */
            (GInterfaceFinalizeFunc) components_module_alterator_interface_finalize, /* interface_finalize */
            NULL                                                                     /* interface_data */
        };

        components_module_type = g_type_register_static(G_TYPE_OBJECT, /* parent class */
                                                        "AlteratorCtlComponentsModule",
                                                        &components_module_info,
                                                        0);

        g_type_add_interface_static(components_module_type, TYPE_ALTERATOR_CTL_MODULE, &alterator_module_interface_info);
    }

    return components_module_type;
}

static void components_module_class_init(AlteratorCtlComponentsModuleClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->finalize = components_ctl_class_finalize;

    components_module_parent_class = g_type_class_peek_parent(klass);
}

static void components_ctl_class_finalize(GObject *klass)
{
    AlteratorCtlComponentsModuleClass *obj = (AlteratorCtlComponentsModuleClass *) klass;

    G_OBJECT_CLASS(components_module_parent_class)->finalize(klass);
}

static void components_module_alterator_interface_init(gpointer iface, gpointer iface_data)
{
    AlteratorCtlModuleInterface *interface = iface;

    interface->run_with_args = components_module_run_with_args;

    interface->run = components_module_run;

    interface->print_help = components_module_print_help;
}
static void components_module_alterator_interface_finalize(gpointer iface, gpointer iface_data) {}

alterator_ctl_module_t *get_components_module(void)
{
    static gsize components_ctl_module_init = 0;
    if (g_once_init_enter(&components_ctl_module_init))
    {
        gsize module_id_size = g_strlcpy(components_module.id,
                                         ALTERATOR_CTL_COMPONENTS_MODULE_NAME,
                                         strlen(ALTERATOR_CTL_COMPONENTS_MODULE_NAME) + 1);

        if (module_id_size != strlen(ALTERATOR_CTL_COMPONENTS_MODULE_NAME))
        {
            g_printerr(_("Internal error in get_components_module: unvaliable id of components module.\n"));
            return NULL;
        }

        components_module.new_object_func  = (gpointer) components_module_new;
        components_module.free_object_func = (gpointer) components_module_free;

        gsize tmp = 42;

        g_once_init_leave(&components_ctl_module_init, tmp);
    }

    return &components_module;

end:
    return NULL;
}

AlteratorCtlComponentsModule *components_module_new(gpointer app)
{
    AlteratorCtlComponentsModule *object = g_object_new(TYPE_ALTERATOR_CTL_COMPONENTS_MODULE, NULL);

    object->commands = g_hash_table_new(g_str_hash, g_str_equal);

    fill_command_hash_table(object->commands);

    object->alterator_ctl_app = (AlteratorCtlApp *) app;

    object->gdbus_source = alterator_gdbus_source_new(object->alterator_ctl_app->arguments->verbose, G_BUS_TYPE_SYSTEM);

    return object;
}

void components_module_free(AlteratorCtlComponentsModule *module)
{
    g_free(printing_category_name);

    g_hash_table_destroy(module->commands);

    if (module->gdbus_source)
        alterator_gdbus_source_free(module->gdbus_source);

    g_strfreev(chosen_sections);

    g_object_unref(module);
}

static void fill_command_hash_table(GHashTable *command)
{
    for (int i = 0; i < sizeof(components_module_subcommands_list) / sizeof(components_module_subcommands_t); i++)
        g_hash_table_insert(command,
                            components_module_subcommands_list[i].subcommand,
                            &components_module_subcommands_list[i].id);
}

int components_module_run_with_args(gpointer self, int argc, char **argv)
{
    int ret = 0;

    alteratorctl_ctx_t *ctx = NULL;

    AlteratorCtlComponentsModule *module = ALTERATOR_CTL_COMPONENTS_MODULE(self);

    if (!module)
    {
        g_printerr(
            _("Internal data error in editions module with args: AlteratorCtlEditionsModule *module is NULL.\n"));
        ERR_EXIT();
    }

    if (components_module_parse_arguments(module, argc, argv, &ctx) < 0)
        ERR_EXIT();

    if (components_module_run(module, ctx) < 0)
        ERR_EXIT();

    if (components_module_handle_results(module, ctx) < 0)
        ERR_EXIT();

end:
    if (ctx)
        alteratorctl_ctx_free(ctx);

    return ret;
}

int components_module_run(gpointer self, gpointer data)
{
    int ret                              = 0;
    const gchar *parameter1              = NULL;
    gchar *locale                        = NULL;
    AlteratorCtlModuleInterface *iface   = GET_ALTERATOR_CTL_MODULE_INTERFACE(self);
    AlteratorCtlComponentsModule *module = ALTERATOR_CTL_COMPONENTS_MODULE(self);

    if (!self)
    {
        g_printerr(_("Internal error in components module run: *module is NULL.\n"));
        ERR_EXIT();
    }

    if (module->alterator_ctl_app->arguments->module_help)
        goto end;

    if (!data)
    {
        g_printerr(_("Internal error in components module run: *context is NULL.\n"));
        ERR_EXIT();
    }

    alteratorctl_ctx_t *ctx = (alteratorctl_ctx_t *) data;

    int subcommand_id = g_variant_get_int32(ctx->subcommands_ids);
    switch (subcommand_id)
    {
    case COMPONENTS_DESCRIPTION:
    case COMPONENTS_REMOVE:
    case COMPONENTS_INSTALL:
        if (!(locale = alterator_ctl_get_effective_locale()))
            ERR_EXIT();

        if (module->gdbus_source->alterator_gdbus_source_set_env_value(module->gdbus_source, "LC_ALL", locale) < 0)
            ERR_EXIT();
    default:
        break;
    }

    switch (subcommand_id)
    {
    case COMPONENTS_LIST:
        if (components_module_list_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;

    case COMPONENTS_INFO:
        if (components_module_info_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;

    case COMPONENTS_INSTALL:
        if (components_module_install_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;

    case COMPONENTS_REMOVE:
        if (components_module_remove_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;

    case COMPONENTS_DESCRIPTION:
        if (components_module_description_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;

    case COMPONENTS_SEARCH:
        if (components_module_search_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;

    case COMPONENTS_STATUS:
        if (components_module_status_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;

    default:
        g_printerr(_("Try to run unknown components module command.\n"));
        iface->print_help(module);
        ERR_EXIT();
    }

end:
    if (!parameter1)
        g_free((gpointer) parameter1);

    if (!locale)
        g_free(locale);

    return ret;
}

static int components_module_list_subcommand(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                                   = 0;
    GHashTable *categories_info_list_table    = NULL;
    GHashTable *components_info_list_table    = NULL;
    GHashTable *categories_nodes_table        = g_hash_table_new_full(g_str_hash,
                                                               g_str_equal,
                                                               (GDestroyNotify) g_free,
                                                               (GDestroyNotify) components_module_tree_elem_free);
    GHashTable *components_nodes_table        = g_hash_table_new_full(g_str_hash,
                                                               g_str_equal,
                                                               (GDestroyNotify) g_free,
                                                               (GDestroyNotify) components_module_tree_elem_free);
    GHashTable *uninstalled_packages          = NULL;
    GNode *result                             = g_node_new(NULL);
    GNode **sections                          = NULL;
    gsize amount_of_sections                  = 0;
    gchar *current_arch                       = NULL;
    gchar **current_locales                   = NULL;
    gsize locales_len                         = 0;
    gchar **current_desktop_environments      = NULL;
    gsize de_env_len                          = 0;
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    if (!module)
    {
        g_printerr(_("The call to the components list method failed. The module doesn't exist.\n"));
        ERR_EXIT();
    }

    if (!ctx || !(*ctx))
    {
        g_printerr(_("The call to the components list method failed. The context doesn't exist.\n"));
        ERR_EXIT();
    }

    if (components_module_get_categories_info(module, &categories_info_list_table))
        ERR_EXIT();

    if (!(sections = components_module_get_section_nodes(module, &amount_of_sections)))
        ignore_sections = TRUE;

    GHashTableIter categories_info_list_iter;
    g_hash_table_iter_init(&categories_info_list_iter, categories_info_list_table);
    gpointer category_info = NULL, categories_value = NULL;
    while (g_hash_table_iter_next(&categories_info_list_iter, &category_info, &categories_value))
    {
        GNode *category_parsed_data = NULL;
        if (info_parser->alterator_ctl_module_info_parser_get_data_from_text(info_parser,
                                                                             module->gdbus_source,
                                                                             category_info,
                                                                             &category_parsed_data)
            < 0)
        {
            g_printerr(_("Printing list of components with categories failed. Can't parse categories data.\n"));
            if (category_parsed_data)
                alterator_ctl_module_info_parser_result_tree_free(category_parsed_data);
            ERR_EXIT();
        }

        alterator_entry_node *node_data = (alterator_entry_node *) category_parsed_data->data;
        toml_value *draft_status        = !draft
                                              ? g_hash_table_lookup(node_data->toml_pairs,
                                                             COMPONENT_CATEGORY_ALTERATOR_ENTRY_COMPONENT_DRAFT_KEY_NAME)
                                              : NULL;
        if (draft_status && draft_status->bool_value)
        {
            alterator_ctl_module_info_parser_result_tree_free(category_parsed_data);
            continue;
        }

        toml_value *category_name_toml_value
            = g_hash_table_lookup(node_data->toml_pairs, COMPONENT_CATEGORY_ALTERATOR_ENTRY_COMPONENT_NAME_KEY_NAME);

        gchar *category_name = category_name_toml_value ? category_name_toml_value->str_value : NULL;
        gchar *display_name  = NULL;
        if (!name_only && !path_only && !no_display_name
            && components_module_get_display_name(module, category_name, CATEGORY, category_parsed_data, &display_name)
                   < 0)
        {
            alterator_ctl_module_info_parser_result_tree_free(category_parsed_data);
            ERR_EXIT();
        }

        toml_value *parent_category_toml_value
            = g_hash_table_lookup(node_data->toml_pairs, COMPONENT_CATEGORY_ALTERATOR_ENTRY_COMPONENT_CATEGORY_KEY_NAME);

        gchar *parent_category = parent_category_toml_value ? parent_category_toml_value->str_value : NULL;

        components_module_tree_elem_t *elem = g_malloc0(sizeof(components_module_tree_elem_t));
        components_module_tree_elem_init(elem, CATEGORY, category_name, display_name, parent_category);

        // Default value before traversing the tree to update the installation status
        elem->installed_status = NONE;

        GNode *category_tree_node = g_node_new(elem);
        if ((!parent_category || !strlen(parent_category)) && ignore_sections)
            g_node_append(result, category_tree_node);

        g_hash_table_insert(categories_nodes_table, g_strdup(category_name), category_tree_node);
        alterator_ctl_module_info_parser_result_tree_free(category_parsed_data);
        g_free(display_name);
    }

    if (components_module_get_components_info(module, &components_info_list_table) < 0)
        ERR_EXIT();

    if (components_module_get_components_uninstalled_packages(module, &uninstalled_packages) < 0)
        ERR_EXIT();

    GHashTableIter components_info_list_iter;
    g_hash_table_iter_init(&components_info_list_iter, components_info_list_table);
    gchar *component_info    = NULL;
    gpointer component_value = NULL;

    if (components_module_get_arch(module, &current_arch) < 0)
        ERR_EXIT();

    if (!(current_locales = components_module_get_locales(module, &locales_len)))
        ERR_EXIT();

    current_desktop_environments = components_module_get_desktop_environments(module, &de_env_len);

    while (g_hash_table_iter_next(&components_info_list_iter, (gpointer) &component_info, &component_value))
    {
        GNode *component_parsed_data = NULL;
        if (info_parser->alterator_ctl_module_info_parser_get_data_from_text(info_parser,
                                                                             module->gdbus_source,
                                                                             component_info,
                                                                             &component_parsed_data)
            < 0)
        {
            g_printerr(_("Printing list of components with categories failed. Can't parse components data.\n"));
            if (component_parsed_data)
                alterator_ctl_module_info_parser_result_tree_free(component_parsed_data);
            ERR_EXIT();
        }

        components_module_item_installed_status status = NONE;
        if (components_module_get_component_status_by_uninstalled_packages_list(module,
                                                                                component_parsed_data,
                                                                                uninstalled_packages,
                                                                                current_arch,
                                                                                current_locales,
                                                                                locales_len,
                                                                                current_desktop_environments,
                                                                                de_env_len,
                                                                                &status)
            < 0)
        {
            alterator_ctl_module_info_parser_result_tree_free(component_parsed_data);
            ERR_EXIT();
        }

        alterator_entry_node *node_data = (alterator_entry_node *) component_parsed_data->data;
        toml_value *draft_status        = !draft ? g_hash_table_lookup(node_data->toml_pairs,
                                                                COMPONENT_ALTERATOR_ENTRY_COMPONENT_DRAFT_KEY_NAME)
                                                 : NULL;
        if (draft_status && draft_status->bool_value)
        {
            alterator_ctl_module_info_parser_result_tree_free(component_parsed_data);
            continue;
        }

        toml_value *parent_category_toml_value
            = g_hash_table_lookup(node_data->toml_pairs, COMPONENT_ALTERATOR_ENTRY_COMPONENT_CATEGORY_KEY_NAME);

        toml_value *component_name_toml_value = g_hash_table_lookup(node_data->toml_pairs,
                                                                    COMPONENT_ALTERATOR_ENTRY_COMPONENT_NAME_KEY_NAME);
        if (!component_name_toml_value)
        {
            g_printerr(_("Can,t print components list. Failed to get some component name.\n"));
            ERR_EXIT();
        }

        gchar *component_path = components_module_path_from_name(component_name_toml_value->str_value);

        gchar *display_name = NULL;
        if (!name_only && !path_only && !no_display_name
            && components_module_get_display_name(module,
                                                  (gchar *) component_path,
                                                  COMPONENT,
                                                  component_parsed_data,
                                                  &display_name)
                   < 0)
        {
            g_free(component_path);
            alterator_ctl_module_info_parser_result_tree_free(component_parsed_data);
            ERR_EXIT();
        }

        gchar *parent_category              = parent_category_toml_value ? parent_category_toml_value->str_value : NULL;
        components_module_tree_elem_t *elem = g_malloc0(sizeof(components_module_tree_elem_t));
        components_module_tree_elem_init(elem,
                                         COMPONENT,
                                         component_name_toml_value->str_value,
                                         display_name,
                                         parent_category);
        elem->installed_status = status;

        GNode *component_tree_node = g_node_new(elem);
        g_hash_table_insert(components_nodes_table, g_strdup(component_name_toml_value->str_value), component_tree_node);

        alterator_ctl_module_info_parser_result_tree_free(component_parsed_data);

        g_free(component_path);
        g_free(display_name);
    }

    if (!ignore_sections)
    {
        if (chosen_sections)
        {
            gchar **section_names               = NULL;
            GStrvBuilder *section_names_builder = g_strv_builder_new();
            for (gsize section_index = 0; section_index < amount_of_sections; section_index++)
                g_strv_builder_add(section_names_builder,
                                   ((alterator_entry_node *) sections[section_index]->data)->node_name);
            section_names = g_strv_builder_unref_to_strv(section_names_builder);

            for (gsize i = 0; i < g_strv_length(chosen_sections); i++)
                if (!g_strv_contains((const gchar *const *) section_names, chosen_sections[i]))
                {
                    g_strfreev(section_names);
                    g_printerr(_("Section %s does not exist.\n"), chosen_sections[i]);
                    ERR_EXIT();
                }

            g_strfreev(section_names);
        }

        for (int section_index = 0; section_index < amount_of_sections; section_index++)
        {
            gchar *section_name = ((alterator_entry_node *) sections[section_index]->data)->node_name;
            if (chosen_sections && !g_strv_contains((const gchar *const *) chosen_sections, section_name))
                continue;

            // Clear links before parsing next section
            g_hash_table_foreach(components_nodes_table, components_module_delete_sections_categories_links, NULL);
            g_hash_table_foreach(categories_nodes_table, components_module_delete_sections_categories_links, NULL);

            // Restore default categories links
            g_hash_table_foreach(categories_nodes_table,
                                 components_module_create_categories_links,
                                 categories_nodes_table);

            // Clear categories installed status before parsing next section
            g_hash_table_foreach(categories_nodes_table,
                                 components_module_reset_all_trees_category_installed_status,
                                 NULL);

            toml_value *components_array
                = g_hash_table_lookup(((alterator_entry_node *) sections[section_index]->data)->toml_pairs,
                                      COMPONENTS_EDITION_SECTION_COMPONENTS_ARRAY_KEY_NAME);

            GHashTable *section_components = g_hash_table_new(g_str_hash, g_str_equal);
            for (int i = 0; i < components_array->array_length; i++)
                g_hash_table_add(section_components, ((gchar **) components_array->array)[i]);

            gpointer components_links_params[2] = {(gpointer) categories_nodes_table, (gpointer) section_components};
            g_hash_table_foreach(components_nodes_table,
                                 components_module_create_sections_categories_links,
                                 components_links_params);

            gpointer categories_links_params[2] = {(gpointer) categories_nodes_table, (gpointer) section_components};
            g_hash_table_foreach(categories_nodes_table,
                                 components_module_create_sections_categories_links,
                                 categories_links_params);

            g_hash_table_unref(section_components);

            GNode *section_result = g_node_new(NULL);
            GHashTableIter iter;
            g_hash_table_iter_init(&iter, categories_nodes_table);

            gchar *node_name = NULL;
            GNode *category  = NULL;

            while (g_hash_table_iter_next(&iter, (gpointer) &node_name, (gpointer) &category))
            {
                components_module_tree_elem_t *category_data = (components_module_tree_elem_t *) category->data;
                if (!category_data->category && category->children /* && !category->parent*/)
                    g_node_append(section_result, category);
            }

            gchar *locale_display_name = NULL;
            if (components_module_get_section_display_name(module, sections[section_index], &locale_display_name) < 0)
            {
                g_node_destroy(section_result);
                ERR_EXIT();
            }

            components_module_sort_tree_elems_by_name(module, &section_result);
            components_module_update_all_trees_elems_is_installed(section_result);

            gboolean is_first_section = FALSE;
            gboolean is_last_section  = FALSE;
            if (chosen_sections && g_strv_length(chosen_sections))
            {
                is_first_section = g_strcmp0(chosen_sections[0], section_name) == 0 ? TRUE : is_first_section;
                is_last_section  = g_strcmp0(chosen_sections[g_strv_length(chosen_sections) - 1], section_name) == 0
                                       ? TRUE
                                       : is_last_section;
            }
            else
            {
                is_first_section = section_index == 0 ? TRUE : is_first_section;
                is_last_section  = section_index == amount_of_sections - 1 ? TRUE : is_last_section;
            }

            components_module_print_list_with_filters(module,
                                                      section_result,
                                                      locale_display_name,
                                                      section_name,
                                                      is_first_section,
                                                      is_last_section);

            g_free(locale_display_name);

            if (section_result->data)
                alterator_ctl_module_info_parser_result_tree_free(section_result);
            else
                g_free(section_result);
        }
    }
    else
    {
        g_hash_table_foreach(components_nodes_table, components_module_create_categories_links, categories_nodes_table);
        g_hash_table_foreach(categories_nodes_table, components_module_create_categories_links, categories_nodes_table);

        components_module_sort_tree_elems_by_name(module, &result);
        components_module_update_all_trees_elems_is_installed(result);
        components_module_print_list_with_filters(module, result, NULL, NULL, TRUE, TRUE);
    }

end:
    if (components_nodes_table)
        g_hash_table_destroy(components_nodes_table);

    if (categories_nodes_table)
        g_hash_table_destroy(categories_nodes_table);

    if (categories_info_list_table)
        g_hash_table_destroy(categories_info_list_table);

    if (components_info_list_table)
        g_hash_table_destroy(components_info_list_table);

    if (uninstalled_packages)
        g_hash_table_destroy(uninstalled_packages);

    if (sections)
        alterator_ctl_module_info_parser_result_trees_free(sections, amount_of_sections);

    g_free(current_arch);

    g_strfreev(current_locales);

    g_strfreev(current_desktop_environments);

    if (result)
        g_node_destroy(result);

    return ret;
}

static int components_module_info_subcommand(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                       = 0;
    const gchar *component_str_id = NULL;
    gchar *component_path         = NULL;
    gchar *result                 = NULL;

    g_variant_get((*ctx)->parameters, "(ms)", &component_str_id);
    if (!component_str_id)
    {
        g_printerr(_("Internal error in components module: parameter1 is NULL.\n"));
        ERR_EXIT();
    }

    component_path = component_str_id[0] != '/' ? components_module_path_from_name(component_str_id)
                                                : g_strdup((gchar *) component_str_id);

    if (components_module_validate_object_and_iface(module, component_path, COMPONENT_INTERFACE_NAME) < 0)
        ERR_EXIT();

    if (module->gdbus_source->alterator_gdbus_source_get_text_of_alterator_entry_by_path(module->gdbus_source,
                                                                                         (gchar *) component_path,
                                                                                         COMPONENT_INTERFACE_NAME,
                                                                                         &result)
        < 0)
    {
        g_printerr(_("Error while getting info of %s component on alterator entry format.\n"), component_str_id);
        ERR_EXIT();
    }

    (*ctx)->free_results = g_free;
    (*ctx)->results      = result;

end:
    if (component_str_id)
        g_free((gpointer) component_str_id);

    g_free(component_path);

    return ret;
}

static int components_module_install_subcommand(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret = 0;

    GHashTable *components_table = NULL;
    gchar *component_str_id      = NULL;
    gchar *component_path        = NULL;

    g_variant_get((*ctx)->parameters, "(ms)", &component_str_id);
    if (!component_str_id)
    {
        g_printerr(_("Internal error in components module: component name or path is empty.\n"));
        ERR_EXIT();
    }

    component_path = component_str_id[0] != '/' ? components_module_path_from_name(component_str_id)
                                                : g_strdup((gchar *) component_str_id);

    if (components_module_validate_object_and_iface(module, component_path, COMPONENT_INTERFACE_NAME) < 0)
        ERR_EXIT();

    //Get list of components
    if (components_module_get_components(module, &components_table) < 0)
        ERR_EXIT();

    //find component to install
    if (!g_hash_table_contains(components_table, component_path))
    {
        g_printerr(_("Component : %s not installed.\n"), component_str_id);
        ERR_EXIT();
    }

    int status = -1;

    //Get current status of component
    if (components_module_get_component_status(module, component_path, &status, NULL) < 0)
        ERR_EXIT();

    if (!status)
    {
        g_printerr(_("Component: %s is already installed.\n"), component_str_id);
        ERR_EXIT();
    }

    if (!no_apt_update && components_module_apt_update_packages_list(module))
        ERR_EXIT();

    //Install component
    if (components_module_install_component(module, ctx) < 0)
        ERR_EXIT();

end:
    if (components_table)
        g_hash_table_destroy(components_table);

    g_free(component_str_id);

    g_free(component_path);

    return ret;
}

static int components_module_remove_subcommand(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                      = 0;
    gchar *component_str_id      = NULL;
    gchar *component_path        = NULL;
    GHashTable *components_table = NULL;

    g_variant_get((*ctx)->parameters, "(ms)", &component_str_id);
    if (!component_str_id)
    {
        g_printerr(_("Internal error in components module: component path or name is empty.\n"));
        ERR_EXIT();
    }

    component_path = component_str_id[0] != '/' ? components_module_path_from_name(component_str_id)
                                                : g_strdup((gchar *) component_str_id);

    //Check object
    if (components_module_validate_object_and_iface(module, component_path, COMPONENT_INTERFACE_NAME) < 0)
        ERR_EXIT();

    //Get list of components
    if (components_module_get_components(module, &components_table) < 0)
        ERR_EXIT();

    if (!g_hash_table_contains(components_table, component_path))
    {
        g_printerr(_("Component : %s not installed.\n"), component_str_id);
        ERR_EXIT();
    }

    int status = -1;

    //Get components status
    if (components_module_get_component_status(module, component_path, &status, NULL) < 0)
        ERR_EXIT();

    if (status)
    {
        g_printerr(_("Component: %s isn't installed.\n"), component_str_id);
        ERR_EXIT();
    }

    if (!no_apt_update && components_module_apt_update_packages_list(module))
        ERR_EXIT();

    //Remove component
    if (components_module_remove_component(module, ctx) < 0)
        ERR_EXIT();

end:
    if (components_table)
        g_hash_table_destroy(components_table);

    g_free(component_str_id);

    g_free(component_path);

    return ret;
}

static int components_module_description_subcommand(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                 = 0;
    dbus_ctx_t *d_ctx       = NULL;
    GError *dbus_call_error = NULL;
    gchar *component_str_id = NULL;
    gchar *component_path   = NULL;

    g_variant_get((*ctx)->parameters, "(ms)", &component_str_id);
    if (!component_str_id)
    {
        g_printerr(_("Can't get %s component description: path or name is empty.\n"), component_str_id);
        ERR_EXIT();
    }

    component_path = component_str_id[0] != '/' ? components_module_path_from_name(component_str_id)
                                                : g_strdup((gchar *) component_str_id);

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                          component_path,
                          COMPONENT_INTERFACE_NAME,
                          COMPONENT_DESCRIPTION_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);
    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in components_module_description_subcommand\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(ayi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    d_ctx->verbose = module->alterator_ctl_app->arguments->verbose;

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Can't get description of non-existent %s component: %s.\n"),
                   component_str_id,
                   dbus_call_error->message);
        ERR_EXIT();
    }

    if (dbus_call_error)
    {
        g_printerr(_("Getting of component %s description failed: %s.\n"), component_str_id, dbus_call_error->message);
        ERR_EXIT();
    }

    (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);

end:
    dbus_ctx_free(d_ctx);

    g_clear_error(&dbus_call_error);

    g_free(component_str_id);

    g_free(component_path);

    return ret;
}

static int components_module_status_subcommand(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                       = 0;
    int status                    = -1;
    gchar *component_str_id       = NULL;
    gchar *component_path         = NULL;
    gchar *category_info          = NULL;
    GNode *component_parse_info   = NULL;
    gchar *component_display_name = NULL;
    gchar *category_display_name  = NULL;

    gchar *current_arch          = NULL;
    gchar **locales              = NULL;
    gchar **desktop_environments = NULL;

    GHashTable *component_packages_names = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);
    GHashTable *component_installed_packages_names = NULL;

    GPtrArray *packages = g_ptr_array_new();

    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    g_variant_get((*ctx)->parameters, "(ms)", &component_str_id);
    if (!component_str_id)
    {
        g_printerr(_("Internal error in components module: path or name is empty.\n"));
        ERR_EXIT();
    }

    component_path = component_str_id[0] != '/' ? components_module_path_from_name(component_str_id)
                                                : g_strdup((gchar *) component_str_id);

    //Check object
    if (components_module_validate_object_and_iface(module, component_path, COMPONENT_INTERFACE_NAME) < 0)
        ERR_EXIT();

    if (info_parser->alterator_ctl_module_info_parser_get_specified_object_data(info_parser,
                                                                                module->gdbus_source,
                                                                                component_path,
                                                                                COMPONENT_INTERFACE_NAME,
                                                                                &component_parse_info)
        < 0)
    {
        g_printerr(_("Can't get component %s data. Component info parsing failed.\n"), component_str_id);
        ERR_EXIT();
    }

    GNode *component_packages_table = NULL;
    if (!(component_packages_table = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
              info_parser, component_parse_info, COMPONENT_ALTERATOR_ENTRY_COMPONENT_PACKAGES_TABLE_NAME, -1)))
    {
        g_printerr(_("Can't get component %s data. Getting packages list failed.\n"), component_str_id);
        ERR_EXIT();
    }

    if (components_module_get_arch(module, &current_arch) < 0)
        ERR_EXIT();

    gsize locales_len = 0;
    if (!(locales = components_module_get_locales(module, &locales_len)))
        ERR_EXIT();

    gsize env_len        = 0;
    desktop_environments = components_module_get_desktop_environments(module, &env_len);

    gboolean is_has_ingnored_packages = FALSE;
    for (GNode *component_package = component_packages_table->children; component_package != NULL;
         component_package        = g_node_next_sibling(component_package))
    {
        gboolean ignore_by_arch = FALSE;
        if (components_module_is_ignore_package_by_arch(module, component_package, &ignore_by_arch, current_arch) < 0)
            ERR_EXIT();

        gboolean ignore_by_lang = FALSE;
        if (components_module_is_ignore_package_by_language(module,
                                                            component_package,
                                                            &ignore_by_lang,
                                                            locales,
                                                            locales_len)
            < 0)
            ERR_EXIT();

        gboolean ignore_by_desktop_env = FALSE;
        if (components_module_is_ignore_package_by_desktop_env(module,
                                                               component_package,
                                                               &ignore_by_desktop_env,
                                                               desktop_environments,
                                                               TRUE,
                                                               env_len)
            < 0)
            ERR_EXIT();

        if (ignore_by_arch || ignore_by_lang || ignore_by_desktop_env)
            is_has_ingnored_packages = TRUE;

        gchar *package_name = NULL;
        if (components_module_get_package_name(module, component_package, &package_name) < 0)
            ERR_EXIT();

        if (!ignore_by_arch && !ignore_by_lang && !ignore_by_desktop_env)
            g_hash_table_add(component_packages_names, g_strdup(package_name));
        g_free(package_name);
    }

    if (components_module_get_component_status(module,
                                               (gchar *) component_path,
                                               &status,
                                               &component_installed_packages_names)
        < 0)
    {
        g_printerr(_("Can't get component %s status.\n"), component_str_id);
        ERR_EXIT();
    }

    if (components_module_get_display_name(module,
                                           component_path,
                                           COMPONENT,
                                           component_parse_info,
                                           &component_display_name)
        < 0)
        ERR_EXIT();

    toml_value *category_value = g_hash_table_lookup(((alterator_entry_node *) component_parse_info->data)->toml_pairs,
                                                     COMPONENT_ALTERATOR_ENTRY_COMPONENT_CATEGORY_KEY_NAME);

    if (category_value->type != TOML_DATA_STRING || !category_value->str_value)
    {
        g_printerr(_("Can't get component %s parent category. Component category is empty.\n"), component_str_id);
        ERR_EXIT();
    }

    if (components_module_get_display_name(module, category_value->str_value, CATEGORY, NULL, &category_display_name)
        < 0)
        ERR_EXIT();

    g_print(_("Component: %s\n"), component_display_name);
    g_print(_("Category: %s\n"), category_display_name);
    g_print(_("Status: %s\n"), !status ? _("installed") : _("uninstalled"));
    if (!g_hash_table_size(component_packages_names) && is_has_ingnored_packages)
        g_printerr(_("Component %s has no applicable packages: all were\n"
                     "filtered out due to mismatch with the required\n"
                     "architecture, localization, or desktop environments\n"
                     "specified in the component's alterator entry.\n"),
                   component_str_id);
    else
        g_print(_("List of component packages:\n"));

    GHashTableIter iter;
    gchar *package_name = NULL;
    gpointer value      = NULL;
    g_hash_table_iter_init(&iter, component_packages_names);
    while (g_hash_table_iter_next(&iter, (gpointer *) &package_name, &value))
        g_ptr_array_add(packages, package_name);

    g_ptr_array_sort(packages, components_module_sort_strings);

    for (gsize i = 0; i < packages->len; i++)
    {
        if (g_hash_table_contains(component_installed_packages_names, packages->pdata[i]))
            g_print("%s%s%s\n",
                    !hide_installed_markers ? COMPONENTS_MODULE_COMPONENT_INSTALLED_MARKER : "",
                    !hide_installed_markers ? " " : "",
                    (gchar *) packages->pdata[i]);
        else
            g_print("%s%s%s\n",
                    !hide_installed_markers ? COMPONENTS_MODULE_COMPONENT_NOT_INSTALLED_MARKER : "",
                    !hide_installed_markers ? " " : "",
                    (gchar *) packages->pdata[i]);
    }

end:
    if (component_packages_names)
        g_hash_table_destroy(component_packages_names);

    if (packages)
        g_ptr_array_unref(packages);

    if (component_installed_packages_names)
        g_hash_table_destroy(component_installed_packages_names);

    if (component_parse_info)
        alterator_ctl_module_info_parser_result_tree_free(component_parse_info);

    g_free(component_str_id);

    g_free(category_info);

    g_free(component_path);

    g_free(component_display_name);

    g_free(category_display_name);

    g_free(current_arch);

    g_strfreev(locales);

    g_strfreev(desktop_environments);

    return ret;
}

static int components_module_search_subcommand(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                            = 0;
    gchar *component_str_id            = NULL;
    GHashTable *components_paths_table = NULL;
    gchar *components_paths            = NULL;
    GRegex *search_regex               = NULL;
    GError *regex_error                = NULL;
    g_variant_get((*ctx)->parameters, "(ms)", &component_str_id);
    if (!component_str_id || (component_str_id && !strlen(component_str_id)))
    {
        g_printerr(_("Can't search unspecified component.\n"));
        ERR_EXIT();
    }

    if (module->gdbus_source->alterator_gdbus_source_get_iface_objects(module->gdbus_source,
                                                                       COMPONENT_INTERFACE_NAME,
                                                                       &components_paths_table)
        < 0)
        ERR_EXIT();

    if (!components_paths_table || (components_paths_table && !g_hash_table_size(components_paths_table)))
    {
        g_printerr(_("Can't search unspecified component.\n"));
        ERR_EXIT();
    }

    search_regex = g_regex_new(component_str_id, 0, 0, &regex_error);
    if (regex_error)
        g_printerr("%s\n", regex_error->message);

    GHashTableIter iter;
    g_hash_table_iter_init(&iter, components_paths_table);
    gchar *component_path = NULL;
    gboolean print_title  = TRUE;
    while (g_hash_table_iter_next(&iter, (gpointer *) &component_path, NULL))
    {
        gchar *current_component = component_str_id[0] != '/'
                                       ? g_utf8_substring(component_path, strlen(COMPONENTS_MODULE_PREFIX_PATH), -1)
                                       : g_strdup(component_path);

        if (g_regex_match(search_regex, current_component, 0, NULL))
        {
            if (print_title)
            {
                g_print(_("Search results:\n"));
                print_title = FALSE;
            }
            g_print("%s\n", current_component);
        }

        g_free(current_component);
    }

end:
    g_free(component_str_id);

    if (components_paths_table)
        g_hash_table_destroy(components_paths_table);

    if (search_regex)
        g_regex_unref(search_regex);

    g_clear_error(&regex_error);

    return ret;
}

static int components_module_handle_results(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t *ctx)
{
    int ret = 0;
    if (module->alterator_ctl_app->arguments->module_help)
        goto end;

    if (!module || !ctx)
    {
        g_printerr(_("Internal error in components module when handle results: *module or *context is NULL.\n"));
        ERR_EXIT();
    }

    switch (g_variant_get_int32(ctx->subcommands_ids))
    {
    case COMPONENTS_LIST:
        break;

    case COMPONENTS_INFO:
        if (components_module_handle_info_results(module, &ctx) < 0)
            ERR_EXIT();
        break;

    case COMPONENTS_INSTALL:
        break;

    case COMPONENTS_REMOVE:
        break;

    case COMPONENTS_DESCRIPTION:
        if (components_module_handle_description_results(module, &ctx) < 0)
            ERR_EXIT();
        break;

    case COMPONENTS_SEARCH:
        break;

    case COMPONENTS_STATUS:
        if (components_module_handle_status_results(module, &ctx) < 0)
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

static int components_module_parse_options(AlteratorCtlComponentsModule *module, int *argc, char **argv)
{
    int ret                                   = 0;
    static gchar **parsed_components_sections = NULL;
    GOptionContext *option_context            = NULL;
    AlteratorCtlModuleInterface *iface        = GET_ALTERATOR_CTL_MODULE_INTERFACE((void *) module);

    // clang-format off
    static GOptionEntry components_module_options[]
        = {{"category", 'c', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, &printing_category_name,
                                                    "Printing tree or list of specified category",
                                                    "PRINT_CATEGORY"},
           {"allow-remove-manually", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &allow_remove_manually,
                                                    "Allow of removing manually installed packages",
                                                    "ALLOW_REMOVE_MANUALLY"},
           {"allow-remove-base", 'b', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &allow_remove_base_components,
                                                    "Allow of removing packages which contains in base components",
                                                    "ALLOW_REMOVE_BASE"},
           {"force-yes", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &force_yes,
                                                    "Allow deleting or installing a component without confirming the transaction",
                                                    "FORCE_YES"},
           {"sections", 's', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING_ARRAY, &parsed_components_sections,
                                                    "Printing tree or list of specified sections",
                                                    "PRINT_SECTIONS"},
           {"ignore-sections", 'I', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &ignore_sections,
                                                    "Disable sections filter",
                                                    "LIST"},
           {"graphic-tree", 'g', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &print_pseudo_graphic_tree,
                                                    "Printing components and categories in preudo-graphic tree format",
                                                    "PSEUDO_GRAPHIC_TREE"},
           {"simple-tree", 't', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &print_simple_tree,
                                                    "Printing components and categories in simple tree format",
                                                    "SIMLE_TREE"},
           {"list", 'l', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &print_list,
                                                    "Printing components and categories in list format",
                                                    "LIST"},
           {"name-only", 'n', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &name_only,
                                                    "Printing only components names",
                                                    "NAME_ONLY"},
           {"no-update", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &no_apt_update,
                                                    "Don't update the packages lists",
                                                    "NO_UPDATE"},
           {"path-only", 'p', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &path_only,
                                                    "Printing only components D-Bus paths",
                                                    "PATH_ONLY"},
           {"display-name-only", 'd', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &display_name_only,
                                                    "Printing only components and categories display names",
                                                    "DISPLAY_NAME_ONLY"},
           {"no-display-name", 'D', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &no_display_name,
                                                    "Printing components and categories without display names",
                                                    "NO_DISPLAY_NAME"},
           {"enable-display-name", 'e', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &enable_display_name,
                                                    "Printing components and categories with display names in list view",
                                                    "ENABLE_DISPLAY_NAME"},
           {"no-name", 'N', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &no_name,
                                                    "Printing components and categories without names", "NO_NAME"},
           {"installed", 'i', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &show_installed,
                                                    "Printing only installed components", "ONLY_INSTALLED"},
           {"not-installed", 'u', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &show_uninstalled,
                                                    "Printing only not installed components", "ONLY_UNINSTALLED"},
           {"ignore-legend", 'L', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &ignore_legend,
                                                    "Printing without legend", "IGNORE_LEGEND"},
           {"draft", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &draft,
                                                    "Add draft to output", "DRAFT"},
           {"hide-installation-markers", 'H', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &hide_installed_markers,
                                                    "Hide installation status markers", "HIDE_INSTALLATION_MARKERS"},
           {NULL}};
    // clang-format on

    GError *error  = NULL;
    option_context = g_option_context_new("Components module options");
    g_option_context_add_main_entries(option_context, components_module_options, NULL);
    if (!g_option_context_parse(option_context, argc, &argv, &error))
    {
        g_printerr(_("Module components options parsing failed: %s\n"), error->message);
        g_error_free(error);
        ERR_EXIT();
    }

    if (parsed_components_sections)
    {
        // Combine --sections values (repeatable, CSV) and positional args into chosen_sections
        GPtrArray *sections_arr = g_ptr_array_new_with_free_func(g_free);
        for (gsize i = 0; i < g_strv_length(parsed_components_sections); i++)
            if (parsed_components_sections[i])
                g_ptr_array_add(sections_arr, g_strdup(parsed_components_sections[i]));

        gboolean has_command_in_argv = iface->get_command_id(module->commands, argv[2]) >= 0;
        for (int i = has_command_in_argv ? 3 : 2; i < *argc; i++)
            g_ptr_array_add(sections_arr, g_strdup(argv[i]));
        g_ptr_array_sort(sections_arr, components_module_sort_strings);

        GStrvBuilder *sections_builder = g_strv_builder_new();
        for (gsize i = 0; i < sections_arr->len; i++)
            g_strv_builder_add(sections_builder, sections_arr->pdata[i]);
        chosen_sections = g_strv_builder_end(sections_builder);
        g_strv_builder_unref(sections_builder);
        g_ptr_array_free(sections_arr, TRUE);
    }

    if (show_installed && show_uninstalled)
    {
        g_printerr(_("It is not possible to use options --installed and --not-installed together.\n"));
        ERR_EXIT();
    }

    if (ignore_sections && chosen_sections)
    {
        g_printerr(_("It is not possible to use option --ignore-sections and --sections arguments together.\n"));
        ERR_EXIT();
    }

    // Default option
    if (!print_pseudo_graphic_tree && !print_simple_tree && !print_list)
        print_pseudo_graphic_tree = TRUE;

    // Prohibit crossing of opposite options
    if (name_only & path_only)
    {
        g_printerr(_("It is not possible to use options --name-only and --path-only together.\n"));
        ERR_EXIT();
    }
    else if (name_only & display_name_only)
    {
        g_printerr(_("It is not possible to use options --name-only and --display-name-only together.\n"));
        ERR_EXIT();
    }
    else if (display_name_only & path_only)
    {
        g_printerr(_("It is not possible to use options --display-name-only and --path-only together.\n"));
        ERR_EXIT();
    }
    else if (name_only & path_only & display_name_only)
    {
        g_printerr(
            _("It is not possible to use options --name-only and --path-only and --display-name-only together.\n"));
        ERR_EXIT();
    }
    else if (display_name_only & no_display_name)
    {
        g_printerr(_("It is not possible to use options --display-name-only and --no-display-name together.\n"));
        ERR_EXIT();
    }
    else if (module->alterator_ctl_app->arguments->verbose && display_name_only)
    {
        g_printerr(_("It is not possible to use options --verbose and --display-name-only together.\n"));
        ERR_EXIT();
    }
    else if (module->alterator_ctl_app->arguments->verbose && name_only)
    {
        g_printerr(_("It is not possible to use options --verbose and --name-only together.\n"));
        ERR_EXIT();
    }
    else if (module->alterator_ctl_app->arguments->verbose && path_only)
    {
        g_printerr(_("It is not possible to use options --verbose and --path-only together.\n"));
        ERR_EXIT();
    }
    else if (no_name && name_only)
    {
        g_printerr(_("It is not possible to use options --no-name and --name-only together.\n"));
        ERR_EXIT();
    }
    else if (no_name && no_display_name)
    {
        g_printerr(_("It is not possible to use options --no-name and --no-display-name together.\n"));
        ERR_EXIT();
    }
    else if (no_name && module->alterator_ctl_app->arguments->verbose)
    {
        g_printerr(_("It is not possible to use options --verbose and --no-name together.\n"));
        ERR_EXIT();
    }
    else if (print_pseudo_graphic_tree && print_simple_tree)
    {
        g_printerr(_("It is not possible to use options --preudo-graphic-tree and --simple-tree together.\n"));
        ERR_EXIT();
    }
    else if (print_pseudo_graphic_tree && print_list)
    {
        g_printerr(_("It is not possible to use options --preudo-graphic-tree and --list together.\n"));
        ERR_EXIT();
    }
    else if (print_simple_tree && print_list)
    {
        g_printerr(_("It is not possible to use options --simple-tree and --list together.\n"));
        ERR_EXIT();
    }
    else if (enable_display_name && print_pseudo_graphic_tree)
    {
        g_printerr(_("The --enable-display-name option can only be used with the --list option.\n"));
        ERR_EXIT();
    }
    else if (enable_display_name && print_pseudo_graphic_tree)
    {
        g_printerr(_("The --enable-display-name option can only be used with the --list option.\n"));
        ERR_EXIT();
    }
    else if (enable_display_name && print_simple_tree)
    {
        g_printerr(_("The --enable-display-name option can only be used with the --list option.\n"));
        ERR_EXIT();
    }
    else if (enable_display_name && no_display_name)
    {
        g_printerr(_("It is not possible to use options --enable-display-name and --no-display-name together.\n"));
        ERR_EXIT();
    }
    else if (enable_display_name && (name_only || path_only))
    {
        g_printerr(_("It is not possible to use options --enable-display-name and %s together.\n"),
                   name_only ? "--name-only" : "--path-only");
        ERR_EXIT();
    }

end:
    if (option_context)
        g_option_context_free(option_context);

    g_strfreev(parsed_components_sections);

    return ret;
}

int components_module_parse_arguments(AlteratorCtlComponentsModule *module,
                                      int argc,
                                      char **argv,
                                      alteratorctl_ctx_t **ctx)
{
    int ret                            = 0;
    AlteratorCtlModuleInterface *iface = GET_ALTERATOR_CTL_MODULE_INTERFACE((void *) module);

    if (!iface)
    {
        g_printerr("%s", _("Internal error in components module while parsing arguments: *iface is NULL.\n"));
        ERR_EXIT();
    }

    if (module->alterator_ctl_app->arguments->module_help)
    {
        iface->print_help(module);
        goto end;
    }

    if (components_module_parse_options(module, &argc, argv) < 0)
        ERR_EXIT();

    if (argc < 2)
    {
        g_printerr(_("Wrong arguments in components module.\n"));
        iface->print_help(module);
        ERR_EXIT();
    }

    int selected_subcommand = iface->get_command_id(module->commands, argv[2]);
    switch (selected_subcommand)
    {
    case COMPONENTS_LIST:
        if (argc >= 3)
        {
            (*ctx) = alteratorctl_ctx_init_components(COMPONENTS_LIST, NULL, NULL, NULL);
            if (!(*ctx))
            {
                g_printerr(_("Can't initialize context.\n"));
                ERR_EXIT();
            }
        }
        else
        {
            g_printerr(_("Wrong arguments to list module command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }

        break;

    case COMPONENTS_INFO:
        if (argc == 4)
        {
            (*ctx) = alteratorctl_ctx_init_components(COMPONENTS_INFO, argv[3], NULL, NULL);
            if (!(*ctx))
            {
                g_printerr(_("Can't initialize context.\n"));
                ERR_EXIT();
            }
        }
        else
        {
            g_printerr(_("Wrong arguments to info module command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    case COMPONENTS_INSTALL:
        if (argc == 4)
        {
            (*ctx) = alteratorctl_ctx_init_components(COMPONENTS_INSTALL, argv[3], NULL, NULL);
            if (!(*ctx))
            {
                g_printerr(_("Can't initialize context.\n"));
                ERR_EXIT();
            }
        }
        else
        {
            g_printerr(_("Wrong arguments to install module command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    case COMPONENTS_REMOVE:
        if (argc == 4)
        {
            (*ctx) = alteratorctl_ctx_init_components(COMPONENTS_REMOVE, argv[3], NULL, NULL);
            if (!(*ctx))
            {
                g_printerr(_("Can't initialize context.\n"));
                ERR_EXIT();
            }
        }
        else
        {
            g_printerr(_("Wrong arguments to remove module command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    case COMPONENTS_DESCRIPTION:
        if (argc == 4)
        {
            (*ctx) = alteratorctl_ctx_init_components(COMPONENTS_DESCRIPTION, argv[3], NULL, NULL);
            if (!(*ctx))
            {
                g_printerr(_("Can't initialize context.\n"));
                ERR_EXIT();
            }
        }
        else
        {
            g_printerr(_("Wrong arguments to description module command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    case COMPONENTS_SEARCH:
        if (argc == 4)
        {
            (*ctx) = alteratorctl_ctx_init_components(COMPONENTS_SEARCH, argv[3], NULL, NULL);
            if (!(*ctx))
            {
                g_printerr(_("Can't initialize context.\n"));
                ERR_EXIT();
            }
        }
        else
        {
            g_printerr(_("Wrong arguments to search module command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    case COMPONENTS_STATUS:
        if (argc == 4)
        {
            (*ctx) = alteratorctl_ctx_init_components(COMPONENTS_STATUS, argv[3], NULL, NULL);
            if (!(*ctx))
            {
                g_printerr(_("Can't initialize context.\n"));
                ERR_EXIT();
            }
        }
        else
        {
            g_printerr(_("Wrong arguments to status module command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
        break;

    default:
        if (argc >= 2)
            (*ctx) = alteratorctl_ctx_init_components(COMPONENTS_LIST, NULL, NULL, NULL);
        else
        {
            g_printerr(_("Wrong components module command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
    }

end:
    return ret;
}

int components_module_print_help(gpointer self)
{
    g_print(_("Usage:\n"));
    g_print(_("  alteratorctl components [COMMAND [arguments]] [OPTIONS] [PARAMETERS <args...>]\n"));
    g_print(_("  alteratorctl components     List all components in a preudo-graphic tree view\n\n"));
    g_print(_("Commands:\n"));
    g_print(_("  description <component>     Prints component description\n"));
    g_print(_("  list [OPTIONS] [PARAMETERS <args...>]\n"
              "                              List all components (default command)\n"));
    g_print(_("  info <component>            Print component info\n"));
    g_print(_("  install <component>         Install component\n"));
    g_print(_("  remove <component>          Remove component\n"));
    g_print(_("  status <component>          View the installation status of component\n"));
    g_print(_("  search <component>          Search component by name or path\n\n"));
    g_print(_("Parameters:\n"));
    g_print(_("  -c, --category <category>   Display the list of components of the selected\n"
              "                              category\n"));
    g_print(_("  -s, --sections <section1> [section2 ...]\n"
              "                              Display components and categories list from chosen\n"
              "                              sections\n\n"));
    g_print(_("Options for formatting output:\n"));
    g_print(_("  -v, --verbose               Add components paths to components list output\n"));
    g_print(_("  -d, --display-name-only     Show only components and categories display names\n"));
    g_print(_("  -D, --no-display-name       Show components and categories without display\n"
              "                              names\n"));
    g_print(_("  -e, --enable-display-name   Show components and categories\n"
              "                              display names in list view\n"));
    g_print(_("  -p, --path-only             Show only components objects paths on D-Bus\n"));
    g_print(_("  -n, --name-only             Show only components objects names\n"));
    g_print(_("  -N, --no-name               Hide component and category names\n\n"));
    g_print(_("Options for formatting the component list output:\n"));
    g_print(_("  --draft                     Add draft components and categories to the\n"
              "                              output\n"));
    g_print(_("  -g, --graphic-tree          Print components with categories in a\n"
              "                              preudo-graphic tree view (default option)\n"));
    g_print(_("  -t, --simple-tree           Print components with categories in a simple tree\n"
              "                              view without display names\n"));
    g_print(_("  -l, --list                  Print components with categories in a list view\n"
              "                              To printing display name, use the\n"
              "                              --show-display-name option (valid for --list\n"
              "                              option only)\n"));
    g_print(_("  -i, --installed             Show only installed components\n"));
    g_print(_("  -u, --not-installed         Show only not installed components\n"));
    g_print(_("  -I, --ignore-sections       Print all components regardless of edition\n"
              "                              sections\n"));
    g_print(_("  -L, --ignore-legend         Hide the description of the component output\n"
              "                              conventions\n"));
    g_print(_("  -H, --hide-installation-markers\n"
              "                              Hide installation status markers of components and\n"
              "                              categories\n\n"));
    g_print(_("Options for installing and removing components:\n"));
    g_print(_("  --force-yes                 Disable confirmation before removing and\n"
              "                              installing a component. Caution, this action may\n"
              "                              result in immediate removal of system-critical\n"
              "                              packages. Use this option only if you fully\n"
              "                              understand the consequences!\n\n"));
    g_print(_("Options for removing components:\n"));
    g_print(_("  --allow-remove-manually     Allow removal of manually installed packages.\n"
              "                              Enabling this option may lead to the removal\n"
              "                              of packages required for proper system operation\n"));
    g_print(_("  -b, --allow-remove-base     Allow removing of packages which contains in\n"
              "                              base components\n\n"));
    g_print(_("Options for package base synchronization:\n"));
    g_print(_("  --no-update                 Don't update the package lists before installing\n"
              "                              or uninstalling a components\n\n"));
    g_print(_("Usage help:\n"));
    g_print(_("  -h, --help                  Show module components usage\n\n"));
    g_print(_("The term \"base components\" is applicable only if a product edition is\n"
              "installed. Base components are those assigned to the section of the same name\n"
              "in the current edition. By default, removing base components is prohibited, as\n"
              "is removing any component that includes packages from any base component. Use\n"
              "the --allow-remove-base option to allow such removals.\n"
              "\n"
              "\"Manually installed\" packages are those marked as manual by the apt system.\n"
              "This means the package was installed explicitly by the user. You can get a\n"
              "list of such packages by running the command apt-mark showmanual. This list\n"
              "contains many packages essential to system operation. Therefore, their\n"
              "removal is prohibited by default. To remove components including these\n"
              "packages, use the --allow-remove-manually option. Keep in mind that removing\n"
              "such components may disrupt your system. Use this option only if you fully\n"
              "understand the consequences!\n\n"));

    return 0;
}

static int components_module_handle_list_results(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret = 0;

end:
    return ret;
}

static int components_module_handle_info_results(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                                 = 0;
    const gchar *component_str_id           = NULL;
    gchar *component_path                   = NULL;
    GNode *parsed_component_alterator_entry = NULL;
    AlteratorGDBusSource *source            = (AlteratorGDBusSource *) module->gdbus_source;

    g_variant_get((*ctx)->parameters, "(ms)", &component_str_id);
    if (!component_str_id)
    {
        g_printerr(_("Internal error in components module: component_str_id is NULL.\n"));
        ERR_EXIT();
    }

    component_path = component_str_id[0] != '/' ? components_module_path_from_name(component_str_id)
                                                : g_strdup(component_str_id);

    if (source->info_parser->alterator_ctl_module_info_parser_get_specified_object_data(
            source->info_parser, source, component_path, COMPONENT_INTERFACE_NAME, &parsed_component_alterator_entry)
        < 0)
    {
        g_printerr(_("Error in component module: can't get alterator entry data of component %s.\n"), component_str_id);
        ERR_EXIT();
    }

    if (components_module_validate_alterator_entry(source, parsed_component_alterator_entry) < 0)
        ERR_EXIT();

    g_print("%s\n", (gchar *) (*ctx)->results);

end:
    g_free((gpointer) component_str_id);

    g_free(component_path);

    alterator_ctl_module_info_parser_result_tree_free(parsed_component_alterator_entry);

    return ret;
}
static int components_module_handle_install_results(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret = 0;

end:
    return ret;
}
static int components_module_handle_remove_results(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret = 0;

end:
    return ret;
}
static int components_module_handle_description_results(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                 = 0;
    gchar *component_str_id = NULL;
    gchar *dbus_result_text = NULL;
    gchar *result           = NULL;

    g_variant_get((*ctx)->parameters, "(ms)", &component_str_id);
    if (!component_str_id)
    {
        g_printerr(_("Can't handling the result od description method: component path or name is empty.\n"));
        ERR_EXIT();
    }

    if (!(*ctx)->results)
    {
        g_printerr(_("Component %s description is empty.\n"), component_str_id);
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(ayi)")))
    {
        g_printerr(_("Wrong answer type in components description.\n"));
        ERR_EXIT();
    }

    GVariant *answer_array = g_variant_get_child_value((*ctx)->results, 0);
    GVariant *exit_code    = g_variant_get_child_value((*ctx)->results, 1);

    if (!exit_code)
    {
        g_printerr(_("Handling of component %s description failed. Exit code uninitialized.\n"), component_str_id);
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);

    gsize array_size;
    gconstpointer gvar_info = g_variant_get_fixed_array(answer_array, &array_size, sizeof(guint8));

    dbus_result_text = g_malloc0(array_size + 1);
    if (!dbus_result_text)
        ERR_EXIT();

    memcpy(dbus_result_text, gvar_info, array_size);

    if (alterator_ctl_print_html(dbus_result_text) < 0)
        ERR_EXIT();

end:
    g_free(component_str_id);

    g_free(dbus_result_text);

    g_free(result);

    return ret;
}

static int components_module_handle_status_results(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret = 0;

end:
    return ret;
}

static int components_module_print_list_with_filters(AlteratorCtlComponentsModule *module,
                                                     GNode *root,
                                                     const gchar *section_display_name,
                                                     const gchar *section_name,
                                                     gboolean is_first_section,
                                                     gboolean is_last_section)
{
    int ret                             = 0;
    GNode *output                       = NULL;
    GNode *elem                         = NULL;
    static GHashTable *categories_names = NULL;
    static GString *master_buffer       = NULL;

    if (!categories_names)
        categories_names = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);

    if (!master_buffer && is_first_section)
        master_buffer = g_string_new(NULL);

    if (printing_category_name && strlen(printing_category_name))
    {
        if (!(elem = components_module_get_tree_elem_by_name(root, printing_category_name)) && is_last_section
            && !g_hash_table_contains(categories_names, printing_category_name))
        {
            g_printerr(_("Category %s not found.\n"), printing_category_name);
            ERR_EXIT();
        }
        else if (!g_hash_table_contains(categories_names, printing_category_name) && elem
                 && ((components_module_tree_elem_t *) elem->data)->type == COMPONENT)
        {
            g_printerr(_("Category %s not found: %s is a component, not a category.\n"),
                       printing_category_name,
                       printing_category_name);
            ERR_EXIT();
        }
        else if (!elem || (g_hash_table_contains(categories_names, printing_category_name) && !is_last_section))
            goto end;

        output = g_node_new(NULL);

        g_node_unlink(elem);
        elem->next = NULL;
        elem->prev = NULL;

        g_node_append(output, elem);

        if (!is_last_section)
            g_hash_table_add(categories_names, g_strdup(printing_category_name));
    }
    else
        output = root;

    if (!hide_installed_markers && is_first_section && !ignore_legend)
    {
        g_string_append(master_buffer, _("Notations:\n"));
        g_string_append_printf(master_buffer, _("  %s Installed\n"), COMPONENTS_MODULE_COMPONENT_INSTALLED_MARKER);
        g_string_append_printf(master_buffer, _("  %s Uninstalled\n"), COMPONENTS_MODULE_COMPONENT_NOT_INSTALLED_MARKER);
        g_string_append_printf(master_buffer,
                               _("  %s Partially installed\n"),
                               COMPONENTS_MODULE_COMPONENT_PARTIALLY_INSTALLED_MARKER);
        g_string_append_printf(master_buffer,
                               _("  %s Draft item (--draft) or unknown installation status\n\n"),
                               COMPONENTS_MODULE_COMPONENT_UNKNOWN);
    }

    if (section_display_name && section_name)
    {
        if (print_list)
        {
            if (enable_display_name && !no_name)
                g_string_append_printf(master_buffer, "%s (%s):\n", section_display_name, section_name);
            else if (!enable_display_name)
                g_string_append_printf(master_buffer, "%s:\n", section_name);
        }
        else if (display_name_only || no_name)
            g_string_append_printf(master_buffer, "%s:\n", section_display_name);
        else if (no_display_name || name_only)
            g_string_append_printf(master_buffer, "%s:\n", section_name);
        else if (!display_name_only && !name_only)
            g_string_append_printf(master_buffer, "%s (%s):\n", section_display_name, section_name);
    }

    if (show_installed)
        components_module_unlink_all_trees_uninstalled_elems(output);
    else if (show_uninstalled)
        components_module_unlink_all_trees_installed_elems(output);

    if (print_pseudo_graphic_tree
        && components_module_print_pseudo_graphic_tree_to_buffer(module, output, master_buffer) < 0)
        ERR_EXIT();
    else if (print_simple_tree && components_module_print_tree_to_buffer(module, output, master_buffer) < 0)
        ERR_EXIT();
    else if (print_list && components_module_print_list_to_buffer(module, output, master_buffer) < 0)
        ERR_EXIT();

    if ((section_display_name || section_name) && !is_last_section)
        g_string_append(master_buffer, "\n");

end:
    if (is_last_section && master_buffer)
    {
        if (categories_names && output)
            g_string_append(master_buffer, "\n");

        print_with_pager(master_buffer->str);
        g_string_free(master_buffer, TRUE);
        master_buffer = NULL;
    }

    if (categories_names && (is_last_section || ret == -1))
    {
        g_hash_table_destroy(categories_names);
        categories_names = NULL;
    }

    if (output != root)
    {
        if (elem)
            g_node_unlink(elem);

        if (output)
            g_node_destroy(output);
    }
    return ret;
}

static GNode **components_module_get_section_nodes(AlteratorCtlComponentsModule *module, gsize *amount_of_sections)
{
    GNode **result                            = NULL;
    GNode *parsed_edition_info                = NULL;
    AlteratorCtlModule *editions_module       = NULL;
    alteratorctl_ctx_t *e_ctx                 = alteratorctl_ctx_init_editions(EDITIONS_INFO, NULL, NULL, NULL);
    GVariant *editions_info_exit_code         = NULL;
    GVariant *editions_info_info_array        = NULL;
    gchar *alterator_entry                    = NULL;
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    if (alterator_ctl_get_registered_module(module->alterator_ctl_app,
                                            ALTERATOR_CTL_EDITIONS_MODULE_NAME,
                                            &editions_module)
        < 0)
    {
        g_printerr(_("Can't get sections with components: editions module isn't registered.\n"));
        goto end;
    }

    if (editions_module->module_iface->run(editions_module->module_instance, e_ctx) < 0)
        goto end;

    if (!e_ctx->results)
        goto end;

    editions_info_info_array = g_variant_get_child_value(e_ctx->results, 0);
    editions_info_exit_code  = g_variant_get_child_value(e_ctx->results, 1);

    if (!editions_info_info_array)
        goto end;

    gsize array_size;
    gconstpointer gvar_info = g_variant_get_fixed_array(editions_info_info_array, &array_size, sizeof(guint8));

    if (!array_size)
        return NULL;

    alterator_entry = g_malloc0(array_size + 1);
    if (!alterator_entry)
        goto end;
    memcpy(alterator_entry, gvar_info, array_size);

    if (info_parser->alterator_ctl_module_info_parser_get_data_from_text(info_parser,
                                                                         module->gdbus_source,
                                                                         alterator_entry,
                                                                         &parsed_edition_info)
        < 0)
        goto end;

    GNode *sections = info_parser
                          ->alterator_ctl_module_info_parser_get_node_by_name(info_parser,
                                                                              parsed_edition_info,
                                                                              COMPONENTS_EDITION_SECTION_TABLE_NAME,
                                                                              -1);
    if (!sections)
    {
        alterator_ctl_module_info_parser_result_tree_free(parsed_edition_info);
        goto end;
    }

    (*amount_of_sections) = g_node_n_children(sections);
    if (!(*amount_of_sections))
    {
        alterator_ctl_module_info_parser_result_tree_free(parsed_edition_info);
        goto end;
    }

    result         = g_malloc0(sizeof(GNode *) * (*amount_of_sections));
    GNode *section = sections->children;
    for (guint i = 0; i < (*amount_of_sections); i++)
    {
        result[i] = section;
        section   = g_node_next_sibling(section);
        g_node_unlink(result[i]);
    }

end:

    if (parsed_edition_info)
        alterator_ctl_module_info_parser_result_tree_free(parsed_edition_info);

    g_free(alterator_entry);

    if (e_ctx)
        alteratorctl_ctx_free(e_ctx);

    if (editions_module)
    {
        editions_module->free_module_func(editions_module->module_instance);
        g_free(editions_module);
    }

    return result;
}

static int components_module_get_kflavour(gchar **result)
{
    int ret = 0;
    GRegex *kflavour_regex;
    GMatchInfo *kflavour_match;
    gchar *kernel_name = NULL;

    kflavour_regex = g_regex_new("[^-]*-(.*)-[^-]*", 0, 0, NULL);
    if (!kflavour_regex)
        ERR_EXIT();

    kernel_name = call_bash_command("uname -r", NULL);

    if (g_regex_match(kflavour_regex, kernel_name, 0, &kflavour_match))
        (*result) = g_match_info_fetch(kflavour_match, 1);

end:
    if (kflavour_regex)
        g_regex_unref(kflavour_regex);

    if (kflavour_match)
        g_match_info_free(kflavour_match);

    g_free(kernel_name);

    return ret;
}

static int components_module_get_arch(AlteratorCtlComponentsModule *module, gchar **result)
{
    int ret                               = 0;
    AlteratorCtlModule *systeminfo_module = NULL;
    alteratorctl_ctx_t *sys_ctx           = alteratorctl_ctx_init_systeminfo(SYSTEMINFO_GET_ARCH, NULL, NULL);
    GVariant *answer                      = NULL;
    GVariant *exit_code                   = NULL;

    if (alterator_ctl_get_registered_module(module->alterator_ctl_app,
                                            ALTERATOR_CTL_SYSTEMINFO_MODULE_NAME,
                                            &systeminfo_module)
        < 0)
    {
        g_printerr(_("Can't get current arch: systeminfo module isn't registered.\n"));
        ERR_EXIT();
    }

    if (systeminfo_module->module_iface->run(systeminfo_module->module_instance, sys_ctx) < 0)
        ERR_EXIT();

    answer    = g_variant_get_child_value(sys_ctx->results, 0);
    exit_code = g_variant_get_child_value(sys_ctx->results, 1);

    ret = g_variant_get_int32(exit_code);

    GVariantIter *iter = NULL;
    gchar *str         = NULL;
    g_variant_get(answer, "as", &iter);

    int i = 0;
    while (g_variant_iter_loop(iter, "s", &str))
    {
        i++;
        (*result) = g_strdup(str);
    }

    g_variant_iter_free(iter);

    if (!i)
        ERR_EXIT();

end:
    if (sys_ctx)
        alteratorctl_ctx_free(sys_ctx);

    if (exit_code)
        g_variant_unref(exit_code);

    if (answer)
        g_variant_unref(answer);

    if (systeminfo_module)
    {
        systeminfo_module->free_module_func(systeminfo_module->module_instance);
        g_free(systeminfo_module);
    }

    return ret;
}

static gchar **components_module_get_locales(AlteratorCtlComponentsModule *module, gsize *len)
{
    int ret                               = 0;
    AlteratorCtlModule *systeminfo_module = NULL;
    alteratorctl_ctx_t *sys_ctx           = alteratorctl_ctx_init_systeminfo(SYSTEMINFO_GET_LOCALES, NULL, NULL);
    GVariant *answer                      = NULL;
    GVariant *exit_code                   = NULL;
    gchar **result                        = NULL;

    if (alterator_ctl_get_registered_module(module->alterator_ctl_app,
                                            ALTERATOR_CTL_SYSTEMINFO_MODULE_NAME,
                                            &systeminfo_module)
        < 0)
    {
        g_printerr(_("Can't get current locales list: systeminfo module isn't registered.\n"));
        goto end;
    }

    if (systeminfo_module->module_iface->run(systeminfo_module->module_instance, sys_ctx) < 0)
        goto end;

    answer    = g_variant_get_child_value(sys_ctx->results, 0);
    exit_code = g_variant_get_child_value(sys_ctx->results, 1);

    ret = g_variant_get_int32(exit_code);

    GVariantIter *iter = NULL;
    gchar *str         = NULL;
    g_variant_get(answer, "as", &iter);
    (*len) = g_variant_iter_n_children(iter);

    if (!(*len))
    {
        g_printerr(_("Getting system languages failed: empty languages list.\n"));
        g_variant_iter_free(iter);
        goto end;
    }
    result = g_malloc0(sizeof(gchar *) * (*len + 1));
    for (gsize i = 0; i < g_variant_iter_loop(iter, "s", &str); i++)
    {
        gchar *lang = g_utf8_substring(str, 0, 2);
        result[i]   = lang;
    }

    g_variant_iter_free(iter);

    if (!result)
    {
        g_printerr(_("Getting system languages failed: empty languages list.\n"));
        goto end;
    }

end:
    if (sys_ctx)
        alteratorctl_ctx_free(sys_ctx);

    if (exit_code)
        g_variant_unref(exit_code);

    if (answer)
        g_variant_unref(answer);

    if (systeminfo_module)
    {
        systeminfo_module->free_module_func(systeminfo_module->module_instance);
        g_free(systeminfo_module);
    }

    return result;
}

static gchar **components_module_get_desktop_environments(AlteratorCtlComponentsModule *module, gsize *len)
{
    int ret                               = 0;
    AlteratorCtlModule *systeminfo_module = NULL;
    alteratorctl_ctx_t *sys_ctx = alteratorctl_ctx_init_systeminfo(SYSTEMINFO_GET_DESKTOP_ENVIRONMENT, NULL, NULL);
    GVariant *answer            = NULL;
    GVariant *exit_code         = NULL;
    gchar **result              = NULL;

    if (alterator_ctl_get_registered_module(module->alterator_ctl_app,
                                            ALTERATOR_CTL_SYSTEMINFO_MODULE_NAME,
                                            &systeminfo_module)
        < 0)
    {
        g_printerr(_("Can't get desktop environments: systeminfo module isn't registered.\n"));
        goto end;
    }

    if (systeminfo_module->module_iface->run(systeminfo_module->module_instance, sys_ctx) < 0)
        goto end;

    answer    = g_variant_get_child_value(sys_ctx->results, 0);
    exit_code = g_variant_get_child_value(sys_ctx->results, 1);

    ret = g_variant_get_int32(exit_code);

    GVariantIter *iter = NULL;
    gchar *str         = NULL;
    g_variant_get(answer, "as", &iter);
    (*len) = g_variant_iter_n_children(iter);
    if (!(*len))
    {
        g_variant_iter_free(iter);
        goto end;
    }

    result = g_malloc0(sizeof(gchar *) * (*len + 1));
    for (gsize i = 0; i < g_variant_iter_loop(iter, "s", &str); i++)
        result[i] = g_strdup(str);

    g_variant_iter_free(iter);

    if (!result)
        goto end;

end:
    if (sys_ctx)
        alteratorctl_ctx_free(sys_ctx);

    if (exit_code)
        g_variant_unref(exit_code);

    if (answer)
        g_variant_unref(answer);

    if (systeminfo_module)
    {
        systeminfo_module->free_module_func(systeminfo_module->module_instance);
        g_free(systeminfo_module);
    }

    return result;
}

static int components_module_is_ignore_package_by_arch(AlteratorCtlComponentsModule *module,
                                                       GNode *package_data,
                                                       gboolean *result,
                                                       gchar *optional_current_arch)
{
    int ret             = 0;
    gchar *current_arch = optional_current_arch;
    if (!current_arch && components_module_get_arch(module, &current_arch) < 0)
    {
        if (!current_arch)
            g_printerr(_("Failed to get current arch.\n"));
        ERR_EXIT();
    }

    (*result)               = TRUE;
    toml_value *archs_array = g_hash_table_lookup(((alterator_entry_node *) package_data->data)->toml_pairs,
                                                  COMPONENT_CATEGORY_ALTERATOR_ENTRY_COMPONENT_PACKAGE_ARCH_KEY_NAME);

    toml_value *exclude_archs_array
        = g_hash_table_lookup(((alterator_entry_node *) package_data->data)->toml_pairs,
                              COMPONENT_CATEGORY_ALTERATOR_ENTRY_COMPONENT_PACKAGE_EXCLUDE_ARCH_KEY_NAME);
    if (!archs_array && !exclude_archs_array)
        (*result) = FALSE;
    else
    {
        if ((archs_array))
        {
            for (gsize i = 0; i < archs_array->array_length; i++)
                if (g_strcmp0((gchar *) archs_array->array[i], current_arch) == 0)
                    (*result) = FALSE;
        }
        else
            (*result) = FALSE;

        gboolean is_exclude_current_arch = FALSE;
        if ((exclude_archs_array))
            for (gsize i = 0; i < exclude_archs_array->array_length; i++)
                if (g_strcmp0((gchar *) exclude_archs_array->array[i], current_arch) == 0)
                {
                    // If the package architecture is contained in the arch array,
                    // but at the same time we try to exclude it
                    if (*result)
                    {
                        g_printerr("Package %s is both in the list of allowed and prohibited "
                                   "architectures for value \"%s\"\n",
                                   (gchar *) ((alterator_entry_node *) package_data->data)->node_name,
                                   current_arch);
                        ERR_EXIT();
                    }

                    (*result) = TRUE;
                }
    }

end:
    if (!optional_current_arch)
        g_free(current_arch);

    return ret;
}

static int components_module_is_ignore_package_by_language(AlteratorCtlComponentsModule *module,
                                                           GNode *package_data,
                                                           gboolean *result,
                                                           gchar **optional_current_locales,
                                                           gsize len)
{
    int ret           = 0;
    gchar **locales   = optional_current_locales;
    gsize locales_len = 0;
    if (!optional_current_locales && !(locales = components_module_get_locales(module, &locales_len)))
        ERR_EXIT();

    if (len)
        locales_len = len;

    (*result) = TRUE;
    toml_value *package_language
        = g_hash_table_lookup(((alterator_entry_node *) package_data->data)->toml_pairs,
                              COMPONENT_CATEGORY_ALTERATOR_ENTRY_COMPONENT_PACKAGE_LANGUAGE_KEY_NAME);
    if (!package_language || !package_language->str_value)
        (*result) = FALSE;
    else
        for (gsize i = 0; i < locales_len; i++)
            if (g_strcmp0((gchar *) locales[i], package_language->str_value) == 0)
                (*result) = FALSE;

end:
    if (!optional_current_locales)
        g_strfreev(locales);

    return ret;
}

static int components_module_is_ignore_package_by_desktop_env(AlteratorCtlComponentsModule *module,
                                                              GNode *package_data,
                                                              gboolean *result,
                                                              gchar **optional_current_desktop_envs,
                                                              gboolean use_optional_current_desktop_envs,
                                                              gsize len)
{
    int ret                      = 0;
    gchar **desktop_environments = optional_current_desktop_envs;
    gsize env_len                = 0;
    gchar *current_desktop       = NULL;

    if (!use_optional_current_desktop_envs && !optional_current_desktop_envs
        && !(desktop_environments = components_module_get_desktop_environments(module, &env_len)))
        goto end;

    if (len)
        env_len = len;

    (*result) = TRUE;
    toml_value *desktops_array
        = g_hash_table_lookup(((alterator_entry_node *) package_data->data)->toml_pairs,
                              COMPONENT_CATEGORY_ALTERATOR_ENTRY_COMPONENT_PACKAGE_DESKTOP_KEY_NAME);

    if (!desktops_array)
        (*result) = FALSE;
    else
    {
        if (use_optional_current_desktop_envs && !optional_current_desktop_envs)
        {
            (*result) = TRUE;
            goto end;
        }

        for (gsize i = 0; desktops_array && i < desktops_array->array_length; i++)
            for (gsize j = 0; j < env_len; j++)
                if (g_strcmp0((gchar *) desktops_array->array[i], desktop_environments[j]) == 0)
                    (*result) = FALSE;
    }

end:
    if (!optional_current_desktop_envs)
        g_strfreev(desktop_environments);

    g_free(current_desktop);

    return ret;
}

static int components_module_get_package_name(AlteratorCtlComponentsModule *module, GNode *package_data, gchar **result)
{
    int ret         = 0;
    gchar *kflavour = NULL;
    toml_value *is_kernel_module
        = g_hash_table_lookup(((alterator_entry_node *) package_data->data)->toml_pairs,
                              COMPONENT_CATEGORY_ALTERATOR_ENTRY_COMPONENT_PACKAGE_KERNEL_KEY_NAME);
    if (is_kernel_module && is_kernel_module->bool_value)
        if (components_module_get_kflavour(&kflavour))
        {
            g_printerr(_("Can't get kernel kflavour.\n"));
            ERR_EXIT();
        }

    (*result) = g_strconcat((((alterator_entry_node *) package_data->data)->node_name),
                            kflavour ? "-" : "",
                            kflavour,
                            NULL);

end:
    g_free(kflavour);

    return ret;
}

static void components_module_print_legend()
{
    g_print(_("Notations:\n"));
    g_print(_("  %s Installed\n"), COMPONENTS_MODULE_COMPONENT_INSTALLED_MARKER);
    g_print(_("  %s Uninstalled\n"), COMPONENTS_MODULE_COMPONENT_NOT_INSTALLED_MARKER);
    g_print(_("  %s Partially installed\n"), COMPONENTS_MODULE_COMPONENT_PARTIALLY_INSTALLED_MARKER);
    g_print(_("  %s Draft item (--draft) or unknown installation status\n\n"), COMPONENTS_MODULE_COMPONENT_UNKNOWN);
}

static int components_module_print_pseudo_graphic_subtree_buffered(AlteratorCtlComponentsModule *module,
                                                                   GNode *node,
                                                                   gchar *prefix,
                                                                   gboolean is_last,
                                                                   gboolean is_first,
                                                                   GString *buffer)
{
    int ret                      = 0;
    gchar *printing_elem_id      = NULL;
    gchar *printing_display_name = NULL;
    gchar *installed_marker      = NULL;

    if (!node || !node->data)
        ERR_EXIT();

    gchar *connector        = NULL;
    gboolean is_single_node = is_first && !node->next;
    if (is_first)
        connector = (gchar *) COMPONENTS_MODULE_TREE_TOP;
    else if (is_last && !node->parent->parent && !is_first)
        connector = (gchar *) COMPONENTS_MODULE_TREE_BRANCH;
    else if (is_last)
        connector = (gchar *) COMPONENTS_MODULE_TREE_END;
    else
        connector = (gchar *) COMPONENTS_MODULE_TREE_BRANCH;

    components_module_tree_elem_t *data = (components_module_tree_elem_t *) node->data;

    switch (data->installed_status)
    {
    case INSTALLED:
        installed_marker = g_strdup(COMPONENTS_MODULE_COMPONENT_INSTALLED_MARKER);
        break;

    case NOT_INSTALLED:
        installed_marker = g_strdup(COMPONENTS_MODULE_COMPONENT_NOT_INSTALLED_MARKER);
        break;

    case PARTIALLY_INSTALLED:
        installed_marker = g_strdup(COMPONENTS_MODULE_COMPONENT_PARTIALLY_INSTALLED_MARKER);
        break;

    default:
        installed_marker = g_strdup(COMPONENTS_MODULE_COMPONENT_UNKNOWN);
        break;
    };

    if (module->alterator_ctl_app->arguments->verbose)
    {
        if (data->type == COMPONENT)
        {
            gchar *path = components_module_path_from_name(data->name);
            if (!no_display_name)
                printing_elem_id = g_strconcat("(", data->name, ": ", path, ")", NULL);
            else
                printing_elem_id = g_strconcat(data->name, ": ", path, NULL);
            g_free(path);
        }
        else if (data->type == CATEGORY)
            if (!no_display_name)
                printing_elem_id = g_strconcat("(", data->name, ")", NULL);
            else
                printing_elem_id = g_strdup(data->name);
        else if (!no_display_name)
            printing_elem_id = g_strdup(data->name);
    }
    else if (display_name_only || no_name)
        printing_elem_id = g_strdup("");
    else if (no_display_name || name_only)
        printing_elem_id = g_strdup(data->name);
    else if (!display_name_only && !path_only && !name_only)
        printing_elem_id = g_strconcat("(", data->name, ")", NULL);
    else if (path_only)
    {
        gchar *path = data->type == COMPONENT ? components_module_path_from_name(data->name) : g_strdup(data->name);
        printing_elem_id = g_strdup(path);
        g_free(path);
    }

    if (display_name_only || no_name)
        printing_display_name = g_strconcat(!hide_installed_markers ? " " : "", data->display_name, NULL);
    else if (!name_only && !path_only && !no_display_name)
        printing_display_name = g_strconcat(!hide_installed_markers ? " " : "", data->display_name, " ", NULL);
    else
        printing_display_name = g_strdup(!hide_installed_markers ? " " : "");

    g_string_append_printf(buffer,
                           "%s%s%s%s%s%s\n",
                           prefix,
                           connector,
                           COMPONENTS_MODULE_TREE_HLINE,
                           !hide_installed_markers ? installed_marker : "",
                           printing_display_name,
                           printing_elem_id);

    GNode *child = node->children;
    while (child)
    {
        gchar *tree_line = NULL;

        if (!node->parent->parent && (is_single_node || !is_first))
            tree_line = (gchar *) COMPONENTS_MODULE_TREE_VLINE;
        else if (is_last)
            tree_line = " ";
        else
            tree_line = (gchar *) COMPONENTS_MODULE_TREE_VLINE;

        gchar *new_prefix = g_strconcat(prefix, tree_line, !hide_installed_markers ? "  " : " ", NULL);
        components_module_print_pseudo_graphic_subtree_buffered(module,
                                                                child,
                                                                new_prefix,
                                                                child->next == NULL,
                                                                FALSE,
                                                                buffer);
        g_free(new_prefix);
        child = child->next;
    }

end:
    g_free(printing_elem_id);

    g_free(printing_display_name);

    g_free(installed_marker);

    return ret;
}

static int components_module_print_pseudo_graphic_tree_to_buffer(AlteratorCtlComponentsModule *module,
                                                                 GNode *node,
                                                                 GString *master_buffer)
{
    int ret = 0;
    components_module_setup_tree_glyphs();
    GNode *child      = node->children;
    gboolean is_first = TRUE;

    while (child)
    {
        if (components_module_print_pseudo_graphic_subtree_buffered(module,
                                                                    child,
                                                                    "",
                                                                    child->next == NULL,
                                                                    is_first,
                                                                    master_buffer))
        {
            g_printerr(_("Printing list of components with categories failed.\n"));
            ERR_EXIT();
        }

        is_first = FALSE;
        child    = child->next;
    }

end:
    return ret;
}

static int components_module_print_pseudo_graphic_tree(AlteratorCtlComponentsModule *module, GNode *node)
{
    int ret = 0;
    components_module_setup_tree_glyphs();
    GNode *child      = node->children;
    gboolean is_first = TRUE;
    GString *buffer   = g_string_new(NULL);

    while (child)
    {
        if (components_module_print_pseudo_graphic_subtree_buffered(module,
                                                                    child,
                                                                    "",
                                                                    child->next == NULL,
                                                                    is_first,
                                                                    buffer))
        {
            g_printerr(_("Printing list of components with categories failed.\n"));
            ERR_EXIT();
        }

        is_first = FALSE;
        child    = child->next;
    }

    print_with_pager(buffer->str);

end:
    if (buffer)
        g_string_free(buffer, TRUE);

    return ret;
}

static int components_module_print_subtree_buffered(AlteratorCtlComponentsModule *module,
                                                    GNode *node,
                                                    gchar *prefix,
                                                    GString *buffer)
{
    int ret                      = 0;
    gchar *printing_elem_id      = NULL;
    gchar *printing_display_name = NULL;
    gchar *installed_marker      = NULL;

    if (!node || !node->data)
        ERR_EXIT();

    components_module_tree_elem_t *data = (components_module_tree_elem_t *) node->data;
    gchar *connector                    = NULL;

    connector = data->type == CATEGORY ? "/" : "";

    switch (data->installed_status)
    {
    case INSTALLED:
        installed_marker = g_strdup(COMPONENTS_MODULE_COMPONENT_INSTALLED_MARKER);
        break;

    case NOT_INSTALLED:
        installed_marker = g_strdup(COMPONENTS_MODULE_COMPONENT_NOT_INSTALLED_MARKER);
        break;

    case PARTIALLY_INSTALLED:
        installed_marker = g_strdup(COMPONENTS_MODULE_COMPONENT_PARTIALLY_INSTALLED_MARKER);
        break;

    default:
        installed_marker = g_strdup(COMPONENTS_MODULE_COMPONENT_UNKNOWN);
        break;
    };

    if (module->alterator_ctl_app->arguments->verbose)
    {
        if (data->type == COMPONENT)
        {
            gchar *path = components_module_path_from_name(data->name);
            if (!no_display_name)
                printing_elem_id = g_strconcat("(", data->name, ": ", path, ")", NULL);
            else
                printing_elem_id = g_strconcat(data->name, ": ", path, NULL);
            g_free(path);
        }
        else if (data->type == CATEGORY)
            if (!no_display_name)
                printing_elem_id = g_strconcat("(", data->name, ")", NULL);
            else
                printing_elem_id = g_strdup(data->name);
        else if (!no_display_name)
            printing_elem_id = g_strdup(data->name);
    }
    else if (display_name_only || no_name)
        printing_elem_id = g_strdup("");
    else if (no_display_name || name_only)
        printing_elem_id = g_strdup(data->name);
    else if (!display_name_only && !path_only && !name_only)
        printing_elem_id = g_strconcat("(", data->name, ")", NULL);
    else if (path_only)
    {
        gchar *path = data->type == COMPONENT ? components_module_path_from_name(data->name) : g_strdup(data->name);
        printing_elem_id = g_strdup(path);
        g_free(path);
    }

    if (display_name_only || no_name)
        printing_display_name = g_strconcat(data->display_name, NULL);
    else if (!name_only && !path_only && !no_display_name)
        printing_display_name = g_strconcat(data->display_name, " ", NULL);
    else
        printing_display_name = g_strdup("");

    g_string_append_printf(buffer,
                           "%s%s%s%s%s\n",
                           prefix,
                           !installed_marker ? installed_marker : "",
                           connector,
                           printing_display_name,
                           printing_elem_id);

    GNode *child = node->children;
    while (child)
    {
        gchar *new_prefix = g_strconcat(prefix, "  ", NULL);
        components_module_print_subtree_buffered(module, child, new_prefix, buffer);
        g_free(new_prefix);
        child = child->next;
    }

end:
    g_free(printing_elem_id);

    g_free(printing_display_name);

    g_free(installed_marker);

    return ret;
}

static int components_module_print_tree_to_buffer(AlteratorCtlComponentsModule *module,
                                                  GNode *node,
                                                  GString *master_buffer)
{
    int ret      = 0;
    GNode *child = node->children;

    while (child)
    {
        if (components_module_print_subtree_buffered(module, child, "", master_buffer))
        {
            g_printerr(_("Printing list of components with categories failed.\n"));
            ERR_EXIT();
        }

        child = child->next;
    }

end:
    return ret;
}

static int components_module_print_tree(AlteratorCtlComponentsModule *module, GNode *node)
{
    int ret         = 0;
    GNode *child    = node->children;
    GString *buffer = g_string_new(NULL);
    while (child)
    {
        if (components_module_print_subtree_buffered(module, child, "", buffer))
        {
            g_printerr(_("Printing list of components with categories failed.\n"));
            ERR_EXIT();
        }

        child = child->next;
    }
    print_with_pager(buffer->str);

end:
    if (buffer)
        g_string_free(buffer, TRUE);

    return ret;
}

static int components_module_print_sublist_buffered(AlteratorCtlComponentsModule *module,
                                                    GNode *node,
                                                    gchar *prefix,
                                                    GString *buffer)
{
    int ret                      = 0;
    gchar *printing_elem_id      = NULL;
    gchar *printing_display_name = NULL;
    gchar *installed_marker      = NULL;
    gchar *new_prefix            = NULL;

    if (!node || !node->data)
        ERR_EXIT();

    components_module_tree_elem_t *data = (components_module_tree_elem_t *) node->data;

    if (module->alterator_ctl_app->arguments->verbose)
    {
        if (data->type == COMPONENT)
        {
            gchar *path = components_module_path_from_name(data->name);
            if (enable_display_name)
                printing_elem_id = g_strconcat("(", data->name, ": ", path, ")", NULL);
            else
                printing_elem_id = g_strconcat(data->name, ": ", path, NULL);
            g_free(path);
        }
        else if (data->type == CATEGORY)
            if (enable_display_name)
                printing_elem_id = g_strconcat("(", data->name, ")", NULL);
            else
                printing_elem_id = g_strdup(data->name);
        else if (enable_display_name)
            printing_elem_id = g_strdup(data->name);
    }
    else if (display_name_only || no_name)
        printing_elem_id = g_strdup("");
    else if (path_only)
    {
        gchar *path = data->type == COMPONENT ? components_module_path_from_name(data->name) : g_strdup(data->name);
        printing_elem_id = g_strdup(path);
        g_free(path);
    }
    else if (no_display_name || name_only || !enable_display_name)
        printing_elem_id = g_strdup(data->name);
    else if (!display_name_only && !path_only && !name_only)
        printing_elem_id = g_strconcat("(", data->name, ")", NULL);

    if ((display_name_only || no_name) || (!name_only && !path_only && enable_display_name))
        printing_display_name = g_strconcat("\"", data->display_name, "\"", NULL);
    else
        printing_display_name = g_strdup("");

    if (data->type == COMPONENT)
    {
        switch (data->installed_status)
        {
        case INSTALLED:
            installed_marker = g_strconcat(COMPONENTS_MODULE_COMPONENT_INSTALLED_MARKER,
                                           !hide_installed_markers ? " " : "",
                                           NULL);
            break;

        case NOT_INSTALLED:
            installed_marker = g_strconcat(COMPONENTS_MODULE_COMPONENT_NOT_INSTALLED_MARKER,
                                           !hide_installed_markers ? " " : "",
                                           NULL);
            break;

        case PARTIALLY_INSTALLED:
            installed_marker = g_strconcat(COMPONENTS_MODULE_COMPONENT_PARTIALLY_INSTALLED_MARKER,
                                           !hide_installed_markers ? " " : "",
                                           NULL);
            break;

        default:
            installed_marker = g_strconcat(COMPONENTS_MODULE_COMPONENT_UNKNOWN,
                                           !hide_installed_markers ? " " : "",
                                           NULL);
            break;
        };
    }
    else
        installed_marker = g_strdup("");

    new_prefix = g_strconcat(!hide_installed_markers ? installed_marker : "",
                             prefix,
                             "/",
                             printing_display_name,
                             printing_elem_id,
                             NULL);

    if (data->type == COMPONENT)
        g_string_append_printf(buffer, "%s\n", new_prefix);

    GNode *child = node->children;
    while (child)
    {
        components_module_print_sublist_buffered(module, child, new_prefix, buffer);
        child = child->next;
    }

end:
    g_free(printing_elem_id);

    g_free(printing_display_name);

    g_free(installed_marker);

    g_free(new_prefix);

    return ret;
}

static int components_module_print_list_to_buffer(AlteratorCtlComponentsModule *module,
                                                  GNode *node,
                                                  GString *master_buffer)
{
    int ret      = 0;
    GNode *child = node->children;

    while (child)
    {
        if (components_module_print_sublist_buffered(module, child, "", master_buffer))
        {
            g_printerr(_("Printing list of components with categories failed.\n"));
            ERR_EXIT();
        }
        child = child->next;
    }

end:
    return ret;
}

static int components_module_print_list(AlteratorCtlComponentsModule *module, GNode *node)
{
    int ret         = 0;
    GString *buffer = g_string_new(NULL);
    GNode *child    = node->children;
    while (child)
    {
        if (components_module_print_sublist_buffered(module, child, "", buffer))
        {
            g_printerr(_("Printing list of components with categories failed.\n"));
            ERR_EXIT();
        }
        child = child->next;
    }
    print_with_pager(buffer->str);

end:
    if (buffer)
        g_string_free(buffer, TRUE);

    return ret;
}

static int components_module_update_all_trees_elems_is_installed(GNode *root)
{
    int ret = 0;
    if (!root)
    {
        g_printerr(_("Incorrect data was passed to update the installation status of components and categories.\n"));
        ERR_EXIT();
    }

    g_node_traverse(root, G_POST_ORDER, G_TRAVERSE_ALL, -1, components_module_update_tree_elem_is_installed, root->data);

end:
    return ret;
}

static int components_module_unlink_all_trees_uninstalled_elems(GNode *root)
{
    int ret = 0;
    if (!root)
    {
        g_printerr(_("Incorrect data was transmitted to filter out components that not installed.\n"));
        ERR_EXIT();
    }

    g_node_traverse(root, G_POST_ORDER, G_TRAVERSE_ALL, -1, components_module_unlink_uninstalled_tree_elem, root->data);

end:
    return ret;
}

static int components_module_unlink_all_trees_installed_elems(GNode *root)
{
    int ret = 0;
    if (!root)
    {
        g_printerr(_("Incorrect data was transmitted to filter out components that installed.\n"));
        ERR_EXIT();
    }

    g_node_traverse(root, G_POST_ORDER, G_TRAVERSE_ALL, -1, components_module_unlink_installed_tree_elem, root->data);

end:
    return ret;
}

static gboolean components_module_update_tree_elem_is_installed(GNode *node, gpointer data)
{
    components_module_tree_elem_t *current_elem = (components_module_tree_elem_t *) node->data;

    // Is root
    if (!node->parent || !node->parent->data)
        return FALSE;

    components_module_tree_elem_t *parent_elem = (components_module_tree_elem_t *) node->parent->data;

    switch (current_elem->installed_status)
    {
    case INSTALLED:
    case NOT_INSTALLED:
        if (parent_elem->installed_status == NONE) // First children
            parent_elem->installed_status = current_elem->installed_status;
        else if (parent_elem->installed_status != current_elem->installed_status)
            parent_elem->installed_status = PARTIALLY_INSTALLED;

        break;

    case PARTIALLY_INSTALLED:
        parent_elem->installed_status = PARTIALLY_INSTALLED;
        break;

    default:
        break;
    };

    return FALSE;
}

static gboolean components_module_unlink_uninstalled_tree_elem(GNode *root, gpointer data)
{
    components_module_tree_elem_t *current_elem = (components_module_tree_elem_t *) root->data;

    // Is root
    if (!root->parent)
        return FALSE;

    if (current_elem->installed_status == NOT_INSTALLED || current_elem->installed_status == NONE)
        g_node_unlink(root);

    return FALSE;
}

static gboolean components_module_unlink_installed_tree_elem(GNode *root, gpointer data)
{
    components_module_tree_elem_t *current_elem = (components_module_tree_elem_t *) root->data;

    // Is root
    if (!root->parent)
        return FALSE;

    if (current_elem->installed_status == INSTALLED || current_elem->installed_status == NONE)
        g_node_unlink(root);

    return FALSE;
}

static GNode *components_module_get_tree_elem_by_name(GNode *root, const gchar *name)
{
    GNode *result     = NULL;
    gpointer params[] = {(gpointer) &result, (gpointer) &name};
    if (!root)
    {
        g_printerr(_("Incorrect data was transmitted to filter out components that installed.\n"));
        return NULL;
    }

    g_node_traverse(root,
                    G_POST_ORDER,
                    G_TRAVERSE_ALL,
                    -1,
                    components_module_get_tree_elem_by_name_traverse_func,
                    params);

    return result;
}

static int components_module_sort_tree_elems_by_name(AlteratorCtlComponentsModule *module, GNode **root)
{
    int ret = 0;
    if (!root && !(*root))
    {
        g_printerr(_("Incorrect data was transmitted to filter out components that installed.\n"));
        ERR_EXIT();
    }
    g_node_traverse(*root,
                    G_POST_ORDER,
                    G_TRAVERSE_ALL,
                    -1,
                    components_module_sort_childrens_by_name_traverse_func,
                    &module->alterator_ctl_app->arguments->verbose);

end:
    return ret;
}

static gint components_module_sort_strings(gconstpointer a, gconstpointer b)
{
    return g_utf8_collate((gchar *) ((GPtrArray *) a)->pdata, (gchar *) ((GPtrArray *) b)->pdata);
}

static gint components_module_sort_tree_compare_func(gconstpointer a, gconstpointer b, gpointer user_data)
{
    gint result;
    components_module_tree_elem_t *first_node_data  = (components_module_tree_elem_t *) ((const GNode *) a);
    components_module_tree_elem_t *second_node_data = (components_module_tree_elem_t *) ((const GNode *) b);
    const gchar *first_comparable_data              = NULL;
    const gchar *second_comparable_data             = NULL;
    gboolean is_verbose                             = *((gboolean *) user_data);

    if ((display_name_only || no_name || enable_display_name) && !no_display_name && !path_only && !name_only)
    {
        first_comparable_data  = g_strdup((const gchar *) first_node_data->display_name);
        second_comparable_data = g_strdup((const gchar *) second_node_data->display_name);
    }
    else if ((no_display_name || name_only || print_list) && !path_only && !display_name_only)
    {
        first_comparable_data  = g_strdup((const gchar *) first_node_data->name);
        second_comparable_data = g_strdup((const gchar *) second_node_data->name);
    }
    else if ((no_display_name || (is_verbose && no_name) || path_only) && !name_only && !display_name_only)
    {
        first_comparable_data  = components_module_path_from_name((const gchar *) first_node_data->name);
        second_comparable_data = components_module_path_from_name((const gchar *) second_node_data->name);
    }
    else
    {
        // Default
        first_comparable_data  = g_strdup((const gchar *) first_node_data->display_name);
        second_comparable_data = g_strdup((const gchar *) second_node_data->display_name);
    }

    result = g_utf8_collate(first_comparable_data, second_comparable_data);

end:
    g_free((gpointer) first_comparable_data);
    g_free((gpointer) second_comparable_data);

    return result;
}

static gboolean components_module_get_tree_elem_by_name_traverse_func(GNode *node, gpointer data)
{
    gpointer *params         = (gpointer *) data;
    GNode **result           = (GNode **) params[0];
    gchar **target_node_name = (gchar **) params[1];

    // Is root
    if (!node->parent)
        return FALSE;

    gchar *current_node_name = ((components_module_tree_elem_t *) node->data)->name;
    if (g_strcmp0(*target_node_name, current_node_name) == 0)
    {
        (*result) = node;
        return TRUE;
    }

    return FALSE;
}

static gboolean components_module_sort_childrens_by_name_traverse_func(GNode *node, gpointer data)
{
    node->children = (GNode *) g_list_sort_with_data((GList *) node->children,
                                                     components_module_sort_tree_compare_func,
                                                     data);

    return FALSE;
}

static int components_module_validate_alterator_entry(AlteratorGDBusSource *source, GNode *alterator_entry_data)
{
    int ret = 0;

    if (!source->info_parser->alterator_ctl_module_info_parser_find_value(
            source->info_parser, alterator_entry_data, NULL, COMPONENT_ALTERATOR_ENTRY_COMPONENT_NAME_KEY_NAME, NULL))
    {
        g_printerr(_("Can't get alterator entry data for validation in module components: field \"%s\" is missing.\n"),
                   COMPONENT_ALTERATOR_ENTRY_COMPONENT_NAME_KEY_NAME);
        ERR_EXIT();
    }

    if (!source->info_parser
             ->alterator_ctl_module_info_parser_find_table(source->info_parser,
                                                           alterator_entry_data,
                                                           NULL,
                                                           -1,
                                                           COMPONENT_ALTERATOR_ENTRY_COMPONENT_PACKAGES_TABLE_NAME,
                                                           NULL))
    {
        g_printerr(_("Can't get alterator entry data for validation in module components: table \"%s\" is missing.\n"),
                   COMPONENT_ALTERATOR_ENTRY_COMPONENT_PACKAGES_TABLE_NAME);
        ERR_EXIT();
    }

end:

    return ret;
}

static int components_module_categories_validate_alterator_entry(AlteratorGDBusSource *source,
                                                                 GNode *alterator_entry_data)
{
    int ret = 0;

    if (!source->info_parser
             ->alterator_ctl_module_info_parser_find_value(source->info_parser,
                                                           alterator_entry_data,
                                                           NULL,
                                                           COMPONENT_CATEGORY_ALTERATOR_ENTRY_COMPONENT_NAME_KEY_NAME,
                                                           NULL))
    {
        g_printerr(
            _("Can't get alterator entry data for validation component category info: field \"%s\" is missing.\n"),
            COMPONENT_CATEGORY_ALTERATOR_ENTRY_COMPONENT_NAME_KEY_NAME);
        ERR_EXIT();
    }

end:

    return ret;
}

static int components_module_get_category_info(AlteratorCtlComponentsModule *module,
                                               const gchar *category_str_id,
                                               gchar **result)
{
    int ret                 = 0;
    GVariant *info_array    = NULL;
    dbus_ctx_t *d_ctx       = NULL;
    GError *dbus_call_error = NULL;

    if (!category_str_id)
    {
        g_printerr(_("Can't get category info. Category name is empty.\n"));
        ERR_EXIT();
    }

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                          ALTERATOR_GLOBAL_PATH,
                          COMPONENT_CATEGORIES_INTERFACE_NAME,
                          COMPONENTS_CATEGORIES_INFO_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in components_module_get_category_info.\n"));
        ERR_EXIT();
    }

    d_ctx->parameters = g_variant_new("(s)", category_str_id);
    d_ctx->reply_type = G_VARIANT_TYPE("(ayi)");

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Failed call category info from non-existent object: %s.\n"), ALTERATOR_GLOBAL_PATH);
        ERR_EXIT();
    }

    if (dbus_call_error)
    {
        g_printerr(_("Failed to get component category info: %s.\n"), dbus_call_error->message);
        ERR_EXIT();
    }

    info_array = g_variant_get_child_value(d_ctx->result, 0);

    if (!info_array)
        ERR_EXIT();

    gsize array_size;
    gconstpointer gvar_info = g_variant_get_fixed_array(info_array, &array_size, sizeof(guint8));

    (*result) = g_malloc0(array_size + 1);
    if (!(*result))
        ERR_EXIT();
    memcpy((*result), gvar_info, array_size);

end:
    if (info_array)
        g_variant_unref(info_array);

    if (d_ctx)
        dbus_ctx_free(d_ctx);

    g_clear_error(&dbus_call_error);

    return ret;
}

static int components_module_get_categories_info(AlteratorCtlComponentsModule *module, GHashTable **result)
{
    int ret                       = 0;
    dbus_ctx_t *d_ctx             = NULL;
    GError *dbus_call_error       = NULL;
    GVariant *answer_string_array = NULL;
    GVariant *exit_code           = NULL;
    const gchar **string_array    = NULL;

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                          ALTERATOR_GLOBAL_PATH,
                          BATCH_COMPONENT_CATEGORIES_INTERFACE_NAME,
                          BATCH_COMPONENT_CATEGORIES_INFO_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);
    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in components_module_get_categories_info.\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Failed call get all categories info from non-existent object: %s.\n"), ALTERATOR_GLOBAL_PATH);
        ERR_EXIT();
    }

    if (dbus_call_error)
    {
        g_printerr(_("Failed to get all categories info: %s.\n"), dbus_call_error->message);
        ERR_EXIT();
    }

    answer_string_array = g_variant_get_child_value((GVariant *) d_ctx->result, 0);
    exit_code           = g_variant_get_child_value((GVariant *) d_ctx->result, 1);
    string_array        = g_variant_get_strv(answer_string_array, NULL);
    (*result)           = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);
    for (const gchar **string_ptr = string_array; *string_ptr != NULL; ++string_ptr)
    {
        g_hash_table_add(*result, g_strdup(*string_ptr));
    }

end:
    dbus_ctx_free(d_ctx);

    g_clear_error(&dbus_call_error);

    if (answer_string_array)
        g_variant_unref(answer_string_array);

    if (string_array)
        g_free(string_array);

    if (exit_code)
        g_variant_unref(exit_code);

    return ret;
}

static int components_module_categories_list(AlteratorCtlComponentsModule *module, GHashTable **result)
{
    int ret                 = 0;
    GVariant *array         = NULL;
    dbus_ctx_t *d_ctx       = NULL;
    GError *dbus_call_error = NULL;

    (*result) = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                          ALTERATOR_GLOBAL_PATH,
                          COMPONENT_CATEGORIES_INTERFACE_NAME,
                          COMPONENTS_CATEGORIES_LIST_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in components_module_categories_list\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Failed to get components categories list via non-existing object %s.\n"), ALTERATOR_GLOBAL_PATH);
        ERR_EXIT();
    }

    if (dbus_call_error)
    {
        g_printerr(_("Failed to get components categories list: %s.\n"), dbus_call_error->message);
        ERR_EXIT();
    }

    array = g_variant_get_child_value(d_ctx->result, 0);

    GVariantIter *iter = NULL;
    gchar *str         = NULL;

    g_variant_get(array, "as", &iter);
    while (g_variant_iter_loop(iter, "s", &str))
        g_hash_table_add(*result, g_strdup(str));

    g_variant_iter_free(iter);

end:
    if (array)
        g_variant_unref(array);

    if (d_ctx)
        dbus_ctx_free(d_ctx);

    g_clear_error(&dbus_call_error);

    return ret;
}

static void components_module_tree_elem_init(components_module_tree_elem_t *elem,
                                             components_module_tree_elem_type type,
                                             gchar *name,
                                             gchar *display_name,
                                             gchar *category)
{
    memset(elem, 0, sizeof(components_module_tree_elem_t));
    elem->type         = type;
    elem->name         = g_strdup(name);
    elem->display_name = g_strdup(display_name);
    elem->category     = g_strdup(category);
}

static void components_module_tree_elem_free(gpointer node_elem)
{
    GNode *node                         = (GNode *) node_elem;
    components_module_tree_elem_t *elem = (components_module_tree_elem_t *) node->data;
    if (!elem)
        return;

    g_free(elem->name);
    g_free(elem->category);
    g_free(elem->display_name);
    g_free(elem);

    g_node_unlink(node);
    while (node->children)
        g_node_unlink(node->children);

    g_node_destroy(node);
}

static int components_module_get_components(AlteratorCtlComponentsModule *module, GHashTable **result)
{
    int ret = 0;
    //*result = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    module->gdbus_source->alterator_gdbus_source_get_iface_objects(module->gdbus_source,
                                                                   COMPONENT_INTERFACE_NAME,
                                                                   result);
    if (!(*result))
    {
        g_printerr(_("Components list is empty.\n"));
        ERR_EXIT();
    }

end:
    return ret;
}

static void components_module_create_categories_links(gpointer key, gpointer value, gpointer user_data)
{
    const gchar *element_name           = (const gchar *) key;
    GNode *elem                         = (GNode *) value;
    GHashTable *categories_table        = (GHashTable *) user_data;
    components_module_tree_elem_t *data = (components_module_tree_elem_t *) elem->data;

    if (!data->category || !strlen(data->category))
        return;

    GNode *target_elem = g_hash_table_lookup(categories_table, data->category);
    if (!target_elem && data->type == CATEGORY)
    {
        g_printerr(_("Can't find parent category %s for category %s.\n"), data->category, element_name);
        return;
    }
    else if (!target_elem && data->type == COMPONENT)
    {
        g_printerr(_("Can't find parent category %s for component %s.\n"), data->category, element_name);
        return;
    }

    g_node_append(target_elem, elem);
}

static void components_module_create_sections_categories_links(gpointer key, gpointer value, gpointer user_data)
{
    const gchar *element_name       = (const gchar *) key;
    GNode *elem                     = (GNode *) value;
    gpointer *params                = (gpointer *) user_data;
    GHashTable *categories_table    = (GHashTable *) params[0];
    GHashTable *sections_components = (GHashTable *) params[1];

    components_module_tree_elem_t *data = (components_module_tree_elem_t *) elem->data;

    if (!data->category || !strlen(data->category))
        return;

    if (data->type == COMPONENT && !g_hash_table_contains(sections_components, element_name))
        return;

    if (data->type == CATEGORY && !elem->children)
    {
        GNode *current_elem = elem;
        while (current_elem && !current_elem->children)
        {
            GNode *parent = current_elem->parent;
            if (parent)
                g_node_unlink(current_elem);
            current_elem = parent;
        }
        return;
    }

    GNode *target_elem = g_hash_table_lookup(categories_table, data->category);
    if (!target_elem && data->type == CATEGORY)
    {
        g_printerr(_("Can't find parent category %s for category %s.\n"), data->category, element_name);
        return;
    }
    else if (!target_elem && data->type == COMPONENT)
    {
        g_printerr(_("Can't find parent category %s for component %s.\n"), data->category, element_name);
        return;
    }

    if (!G_NODE_IS_ROOT(elem))
        g_node_unlink(elem);
    g_node_append(target_elem, elem);
}

static void components_module_delete_sections_categories_links(gpointer key, gpointer value, gpointer user_data)
{
    const gchar *element_name = (const gchar *) key;
    GNode *elem               = (GNode *) value;

    if (elem->parent)
        g_node_unlink(elem);

    while (elem->children)
        g_node_unlink(elem->children);
}

static void components_module_reset_all_trees_category_installed_status(gpointer key, gpointer value, gpointer user_data)
{
    const gchar *element_name = (const gchar *) key;
    GNode *elem               = (GNode *) value;

    components_module_tree_elem_t *data = (components_module_tree_elem_t *) elem->data;

    if (data->type == CATEGORY)
        data->installed_status = NONE;
}

static int components_module_get_display_name(AlteratorCtlComponentsModule *module,
                                              const gchar *elem_str_id,
                                              components_module_tree_elem_type type,
                                              GNode *data,
                                              gchar **result)
{
    int ret                                   = 0;
    GNode *elem_data                          = data;
    gchar *locale                             = NULL;
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    if (type == COMPONENT && !elem_data)
    {
        if (info_parser->alterator_ctl_module_info_parser_get_specified_object_data(info_parser,
                                                                                    module->gdbus_source,
                                                                                    elem_str_id,
                                                                                    COMPONENT_INTERFACE_NAME,
                                                                                    &elem_data)
            < 0)
        {
            g_printerr(_("Can't get display name of component %s. Parsing of component data failed.\n"), elem_str_id);
            ERR_EXIT();
        }
    }
    else if (type == CATEGORY && !elem_data)
    {
        gchar *category_info = NULL;
        if (components_module_get_category_info(module, elem_str_id, &category_info) < 0)
        {
            g_printerr(_("Can't get display name of %s category. Getting of category alterator entry failed.\n"),
                       elem_str_id);
            g_free(category_info);
            ERR_EXIT();
        }

        if (info_parser->alterator_ctl_module_info_parser_get_data_from_text(info_parser,
                                                                             module->gdbus_source,
                                                                             category_info,
                                                                             &elem_data)
            < 0)
        {
            g_printerr(_("Can't get display name of %s category. Parsing of category data failed.\n"), elem_str_id);
            g_free(category_info);
            ERR_EXIT();
        }

        g_free(category_info);
    }

    if (!(locale = alterator_ctl_get_effective_language()))
        ERR_EXIT();

    GHashTable *display_name = NULL;
    if (!info_parser->alterator_ctl_module_info_parser_find_table(
            info_parser, elem_data, &display_name, -1, COMPONENT_ALTERATOR_ENTRY_COMPONENT_DISPLAY_NAME_TABLE_NAME, NULL))
    {
        g_printerr(_("Can't get display name of component %s. Display name data by key %s is empty.\n"),
                   elem_str_id,
                   COMPONENT_ALTERATOR_ENTRY_COMPONENT_DISPLAY_NAME_TABLE_NAME);
        ERR_EXIT();
    }

    toml_value *display_name_locale_value = g_hash_table_lookup(display_name, locale);
    if (!display_name_locale_value)
        display_name_locale_value = g_hash_table_lookup(display_name, LOCALE_FALLBACK);

    (*result) = g_strdup(display_name_locale_value->str_value);

end:
    if (elem_data && !data)
        alterator_ctl_module_info_parser_result_tree_free(elem_data);

    g_free(locale);

    return ret;
}

static int components_module_get_section_display_name(AlteratorCtlComponentsModule *module, GNode *data, gchar **result)
{
    int ret                                   = 0;
    gchar *locale                             = NULL;
    GHashTable *display_name                  = NULL;
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    GNode *display_name_node = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
        info_parser, data, COMPONENT_ALTERATOR_ENTRY_COMPONENT_DISPLAY_NAME_TABLE_NAME, -1);
    if (!display_name_node)
    {
        g_printerr(_("Finding section display name failed.\n"));
        ERR_EXIT();
    }

    if (!(locale = alterator_ctl_get_effective_language()))
        ERR_EXIT();
    toml_value *display_name_locale_value
        = g_hash_table_lookup(((alterator_entry_node *) display_name_node->data)->toml_pairs, locale);
    if (!display_name_locale_value)
        display_name_locale_value = g_hash_table_lookup(((alterator_entry_node *) display_name_node->data)->toml_pairs,
                                                        LOCALE_FALLBACK);

    (*result) = g_strdup(display_name_locale_value->str_value);

end:
    if (display_name)
        g_hash_table_destroy(display_name);

    g_free(locale);

    return ret;
}

static gchar *components_module_name_from_path(const gchar *path)
{
    gchar *result         = NULL;
    GRegex *regex_match   = NULL;
    GRegex *regex_replace = NULL;
    GMatchInfo *match     = NULL;

    regex_match = g_regex_new("^(\\/org\\/altlinux\\/alterator\\/component_)(.+)", G_REGEX_DOTALL, 0, NULL);
    if (!regex_match)
        return NULL;

    if (g_regex_match(regex_match, path, 0, &match))
        result = g_match_info_fetch(match, 2);

    g_regex_unref(regex_match);
    g_match_info_unref(match);

    if (!result)
        return NULL;

    regex_replace = g_regex_new("_", 0, 0, NULL);
    g_regex_replace_literal(regex_replace, result, -1, 0, "-", 0, NULL);

    g_regex_unref(regex_replace);

    return result;
}

static gchar *components_module_path_from_name(const gchar *name)
{
    gchar *result         = NULL;
    GRegex *regex_replace = NULL;
    gchar *modified_name  = NULL;

    regex_replace = g_regex_new("-", 0, 0, NULL);
    modified_name = g_regex_replace_literal(regex_replace, name, -1, 0, "_", 0, NULL);

    result = g_strconcat(COMPONENTS_MODULE_PREFIX_PATH, modified_name, NULL);

    g_regex_unref(regex_replace);
    g_free(modified_name);

    return result;
}

static int components_module_install_component(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                        = 0;
    gchar *info                    = NULL;
    GHashTable *component_packages = NULL;
    gchar *parameter1              = NULL;

    g_variant_get((*ctx)->parameters, "(ms)", &parameter1);
    if (!parameter1)
    {
        g_printerr(_("Internal error in components module: parameter1 is NULL.\n"));
        ERR_EXIT();
    }

    if (components_module_get_packages_of_component(module, &component_packages, parameter1, FALSE) < 0)
        ERR_EXIT();

    if (components_module_install_component_packages(module, parameter1, component_packages) < 0)
        ERR_EXIT();

end:
    if (info)
        g_free(info);

    if (component_packages)
        g_hash_table_destroy(component_packages);

    if (parameter1)
        g_free(parameter1);

    return ret;
}

static int components_module_get_components_info(AlteratorCtlComponentsModule *module, GHashTable **result)
{
    int ret                       = 0;
    dbus_ctx_t *d_ctx             = NULL;
    GError *dbus_call_error       = NULL;
    GVariant *answer_string_array = NULL;
    GVariant *exit_code           = NULL;
    const gchar **string_array    = NULL;

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                          ALTERATOR_GLOBAL_PATH,
                          BATCH_COMPONENTS_INTERFACE_NAME,
                          BATCH_COMPONENTS_INFO_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);
    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in components_module_get_components_info.\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Getting of non-existing component info failed.\n"));
        ERR_EXIT();
    }

    if (dbus_call_error)
    {
        g_printerr(_("Failed to get component category info: %s.\n"), dbus_call_error->message);
        ERR_EXIT();
    }

    answer_string_array = g_variant_get_child_value((GVariant *) d_ctx->result, 0);
    exit_code           = g_variant_get_child_value((GVariant *) d_ctx->result, 1);
    string_array        = g_variant_get_strv(answer_string_array, NULL);
    (*result)           = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);
    for (const gchar **string_ptr = string_array; *string_ptr != NULL; ++string_ptr)
    {
        g_hash_table_add(*result, g_strdup(*string_ptr));
    }

end:
    dbus_ctx_free(d_ctx);

    g_clear_error(&dbus_call_error);

    if (answer_string_array)
        g_variant_unref(answer_string_array);

    if (string_array)
        g_free(string_array);

    if (exit_code)
        g_variant_unref(exit_code);

    return ret;
}

static int components_module_get_component_status_by_uninstalled_packages_list(
    AlteratorCtlComponentsModule *module,
    const GNode *parsed_component,
    const GHashTable *uninstalled_packages,
    gchar *current_arch,
    gchar **current_locales,
    gsize locales_len,
    gchar **current_desktop_environments,
    gsize env_len,
    components_module_item_installed_status *status)
{
    int ret                    = 0;
    GNode *packages_node       = NULL;
    alterator_entry_node *data = (alterator_entry_node *) parsed_component->data;

    if (!uninstalled_packages)
    {
        g_printerr(_("Cat'n get component status by empty uninstalled packages list.\n"));
        ERR_EXIT();
    }

    if (!parsed_component)
    {
        g_printerr(_("Can't get installed status of empty component.\n"));
        ERR_EXIT();
    }

    if (!status)
    {
        g_printerr(_("No variable defined to save the installation status of the component.\n"));
        ERR_EXIT();
    }

    toml_value *component_name_toml_value = g_hash_table_lookup(data->toml_pairs,
                                                                COMPONENT_ALTERATOR_ENTRY_COMPONENT_NAME_KEY_NAME);

    if (!(packages_node = module->gdbus_source->info_parser->alterator_ctl_module_info_parser_get_node_by_name(
              module->gdbus_source->info_parser,
              (GNode *) parsed_component,
              COMPONENT_ALTERATOR_ENTRY_COMPONENT_PACKAGES_TABLE_NAME,
              -1)))
    {
        g_printerr(_("Can't get packages list of component %s.\n"), component_name_toml_value->str_value);
        ERR_EXIT();
    }

    for (GNode *component_package = packages_node->children; component_package != NULL;
         component_package        = g_node_next_sibling(component_package))
    {
        gchar *current_package_name = NULL;
        if (components_module_get_package_name(module, component_package, &current_package_name) < 0)
            ERR_EXIT();

        gboolean ignore_by_arch = FALSE;
        if (components_module_is_ignore_package_by_arch(module, component_package, &ignore_by_arch, current_arch) < 0)
        {
            g_free(current_package_name);
            ERR_EXIT();
        }

        gboolean ignore_by_lang = FALSE;
        if (components_module_is_ignore_package_by_language(module,
                                                            component_package,
                                                            &ignore_by_lang,
                                                            current_locales,
                                                            locales_len)
            < 0)
        {
            g_free(current_package_name);
            ERR_EXIT();
        }

        gboolean ignore_by_desktop_env = FALSE;
        if (components_module_is_ignore_package_by_desktop_env(module,
                                                               component_package,
                                                               &ignore_by_desktop_env,
                                                               current_desktop_environments,
                                                               TRUE,
                                                               env_len)
            < 0)
        {
            g_free(current_package_name);
            ERR_EXIT();
        }

        if (ignore_by_arch || ignore_by_lang || ignore_by_desktop_env)
        {
            if (!g_node_next_sibling(component_package) && (*status) != INSTALLED)
                (*status) = NOT_INSTALLED;
            g_free(current_package_name);
            continue;
        }

        if (g_hash_table_contains((GHashTable *) uninstalled_packages, current_package_name))
        {
            (*status) = NOT_INSTALLED;
            g_free(current_package_name);
            goto end;
        }
        else
        {
            if ((*status) != NOT_INSTALLED)
                (*status) = INSTALLED;
        }

        g_free(current_package_name);
    }

end:
    return ret;
}

static int components_module_get_components_uninstalled_packages(AlteratorCtlComponentsModule *module,
                                                                 GHashTable **result)
{
    int ret                 = 0;
    dbus_ctx_t *d_ctx       = NULL;
    GError *dbus_call_error = NULL;
    GVariant *answer_bytes  = NULL;
    GVariant *exit_code     = NULL;
    gchar *answer_array     = NULL;
    gchar **packages        = NULL;

    int object_exist = 0;
    //Check object
    if (module->gdbus_source->alterator_gdbus_source_check_object_by_path(module->gdbus_source,
                                                                          ALTERATOR_GLOBAL_PATH,
                                                                          &object_exist)
        < 0)
        g_printerr(_("The object %s doesn't exist.\n"), ALTERATOR_GLOBAL_PATH);

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                          ALTERATOR_GLOBAL_PATH,
                          BATCH_COMPONENTS_INTERFACE_NAME,
                          BATCH_COMPONENTS_STATUS_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);
    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in components_module_get_components_uninstalled_packages.\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(ayi)");

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (!d_ctx->result)
    {
        g_printerr(_("Getting of components batch status failed: %s.\n"), dbus_call_error->message);
        ERR_EXIT();
    }

    answer_bytes = g_variant_get_child_value((GVariant *) d_ctx->result, 0);
    exit_code    = g_variant_get_child_value((GVariant *) d_ctx->result, 1);

    gsize array_size;
    gconstpointer gvar_info = g_variant_get_fixed_array(answer_bytes, &array_size, sizeof(guint8));

    answer_array = g_malloc0(array_size + 1);
    if (!answer_array)
        ERR_EXIT();
    memcpy(answer_array, gvar_info, array_size);

    packages  = g_strsplit(answer_array, "\n", -1);
    (*result) = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);
    for (guint i = 0; i < g_strv_length(packages); i++)
        if (strlen(packages[i]))
            g_hash_table_add(*result, g_strdup(packages[i]));

end:
    dbus_ctx_free(d_ctx);

    g_clear_error(&dbus_call_error);

    if (answer_bytes)
        g_variant_unref(answer_bytes);

    if (exit_code)
        g_variant_unref(exit_code);

    g_free(answer_array);

    g_strfreev(packages);

    return ret;
}

static int components_module_get_component_packages(AlteratorCtlComponentsModule *module,
                                                    const char *component_info,
                                                    GHashTable **packages)
{
    int ret                            = 0;
    toml_table_t *component_info_table = NULL;

    char errbuf[TOML_ERROR_BUFFER_SIZE];
    component_info_table = toml_parse((gchar *) component_info, errbuf, sizeof(errbuf));
    if (!component_info_table)
    {
        g_printerr(_("Can't parse alterator entry in components module: %s\n"), errbuf);
        ERR_EXIT();
    }

    toml_table_t *packages_toml = toml_table_in(component_info_table, "packages");
    if (!packages_toml)
    {
        g_printerr(_("Can't get value packages names.\n"));
        ERR_EXIT();
    }

    (*packages) = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    for (int i = 0;; i++)
    {
        const char *key = toml_key_in(packages_toml, i);
        if (!key)
            break;

        g_hash_table_add((*packages), (gpointer) g_strdup(key));
    }

end:

    if (component_info_table)
        toml_free(component_info_table);

    return ret;
}

static int components_module_get_installed_packages(AlteratorCtlComponentsModule *module, GHashTable **packages)
{
    int ret                                  = 0;
    GVariant *array                          = NULL;
    GVariant *exit_code                      = NULL;
    AlteratorCtlModule *packages_module_info = NULL;
    alteratorctl_ctx_t *p_ctx                = NULL;
    GRegex *regex                            = NULL;
    GMatchInfo *match_info                   = NULL;

    if (alterator_ctl_get_registered_module(module->alterator_ctl_app,
                                            ALTERATOR_CTL_PACKAGES_MODULE_NAME,
                                            &packages_module_info)
        < 0)
    {
        g_printerr(_("Packages module isn't registered.\n"));
        ERR_EXIT();
    }

    p_ctx = alteratorctl_ctx_init_packages(PACKAGES_RPM, RPM_LIST, NULL, NULL, NULL);

    if (packages_module_info->module_iface->run(packages_module_info->module_instance, p_ctx) < 0)
        ERR_EXIT();

    if (!p_ctx->results)
        ERR_EXIT();

    array     = g_variant_get_child_value(p_ctx->results, 0);
    exit_code = g_variant_get_child_value(p_ctx->results, 2);

    if (g_variant_get_int32(exit_code))
    {
        g_printerr(_("Error exit code from rpm list method.\n"));
        ERR_EXIT();
    }

    GVariantIter *iter = NULL;
    gchar *str         = NULL;

    regex = g_regex_new("^(.+)-((?:\\d+(?:\\.\\w+)*|\\w*))-(alt\\d+.*).(x86_64|ppc64le|i586|armh|aarch64|noarch)$",
                        0,
                        0,
                        NULL);

    g_variant_get(array, "as", &iter);

    (*packages) = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    while (g_variant_iter_loop(iter, "s", &str))
    {
        g_regex_match(regex, str, 0, &match_info);
        if (g_match_info_matches(match_info))
        {
            gchar *package_name = g_match_info_fetch(match_info, 1);
            g_hash_table_add((*packages), package_name);
        }
    }

    g_variant_iter_free(iter);

end:
    if (regex)
        g_regex_unref(regex);

    if (match_info)
        g_match_info_free(match_info);

    if (exit_code)
        g_variant_unref(exit_code);

    if (array)
        g_variant_unref(array);

    if (p_ctx)
        alteratorctl_ctx_free(p_ctx);

    if (packages_module_info)
    {
        packages_module_info->free_module_func(packages_module_info->module_instance);
        g_free(packages_module_info);
    }

    return ret;
}

static int components_module_remove_packages_from_hash_table(AlteratorCtlComponentsModule *module,
                                                             GHashTable **from_hash_table,
                                                             GHashTable **lookup_hash_table)
{
    int ret                        = 0;
    GList *installed_packages_list = NULL;

    if (!module)
    {
        g_printerr(_("Internal error in components module - remove installed packages.\n"));
        ERR_EXIT();
    }

    if (!module)
    {
        g_printerr(
            _("Internal data error in components module with args: AlteratorCtlComponentsModule *module is NULL in "
              "\"components remove\".\n"));
        ERR_EXIT();
    }

    installed_packages_list = g_hash_table_get_keys((*lookup_hash_table));

    if (!installed_packages_list)
        goto end;

    do
    {
        gpointer founded = g_hash_table_lookup((*lookup_hash_table), (gchar *) installed_packages_list->data);
        if (founded)
            g_hash_table_remove((*from_hash_table), (gchar *) installed_packages_list->data);

        installed_packages_list = installed_packages_list->next;
    } while (installed_packages_list);

end:
    if (installed_packages_list)
        g_list_free(installed_packages_list);

    return ret;
}

static int components_module_apt_update_packages_list(AlteratorCtlComponentsModule *module)
{
    int ret                                  = 0;
    AlteratorCtlModule *packages_module_info = NULL;
    alteratorctl_ctx_t *p_ctx                = NULL;
    GVariant *exit_code                      = NULL;

    if (alterator_ctl_get_registered_module(module->alterator_ctl_app,
                                            ALTERATOR_CTL_PACKAGES_MODULE_NAME,
                                            &packages_module_info)
        < 0)
    {
        g_printerr(_("Packages module isn't registered.\n"));
        ERR_EXIT();
    }

    p_ctx = alteratorctl_ctx_init_packages(PACKAGES_APT, APT_UPDATE, NULL, NULL, NULL);
    if (packages_module_info->module_iface->run(packages_module_info->module_instance, p_ctx) < 0)
        ERR_EXIT();

    if (!p_ctx->results)
        ERR_EXIT();

    exit_code = g_variant_get_child_value(p_ctx->results, 0);

    if (g_variant_get_int32(exit_code))
    {
        g_printerr(_("Apt update failed with exit code: %i\n"), g_variant_get_int32(exit_code));
        ERR_EXIT();
    }

end:
    if (exit_code)
        g_variant_unref(exit_code);

    if (p_ctx)
        alteratorctl_ctx_free(p_ctx);

    if (packages_module_info)
    {
        packages_module_info->free_module_func(packages_module_info->module_instance);
        g_free(packages_module_info);
    }

    return ret;
}

static int components_module_install_component_packages(AlteratorCtlComponentsModule *module,
                                                        gchar *component_str_id,
                                                        GHashTable *components_packages)
{
    int ret                                  = 0;
    AlteratorCtlModule *packages_module_info = NULL;
    alteratorctl_ctx_t *p_ctx                = NULL;
    GVariant *exit_code                      = NULL;
    gchar **keys                             = NULL;
    gchar *all_packages                      = NULL;
    guint len                                = 0;

    if (!module)
    {
        g_printerr(_("Internal error in components module - install component packages.\n"));
        ERR_EXIT();
    }

    if (alterator_ctl_get_registered_module(module->alterator_ctl_app,
                                            ALTERATOR_CTL_PACKAGES_MODULE_NAME,
                                            &packages_module_info)
        < 0)
    {
        g_printerr(_("Packages module isn't registered.\n"));
        ERR_EXIT();
    }

    keys = (gchar **) g_hash_table_get_keys_as_array(components_packages, &len);

    if (!keys)
    {
        g_printerr(_("No packages to install.\n"));
        ERR_EXIT();
    }

    all_packages = g_strjoinv(" ", keys);
    p_ctx        = alteratorctl_ctx_init_packages(PACKAGES_APT, APT_CHECK_APPLY_PRIV, all_packages, NULL, NULL);

    if (packages_module_info->module_iface->run(packages_module_info->module_instance, p_ctx) < 0)
        ERR_EXIT();

    if (components_module_print_affected_components(module, component_str_id, (GVariant *) p_ctx->results, INSTALL, TRUE)
        < 0)
        ERR_EXIT();

    GVariant *to_install_var              = g_variant_get_child_value(p_ctx->results, 0);
    GVariant *to_remove_var               = g_variant_get_child_value(p_ctx->results, 1);
    GVariant *extra_remove_var            = g_variant_get_child_value(p_ctx->results, 2);
    GVariant *additional_data_childrens[] = {to_install_var,
                                             to_remove_var,
                                             extra_remove_var,
                                             g_variant_new_boolean(allow_remove_manually),
                                             g_variant_new_boolean(force_yes)};
    GVariant *additional_data             = g_variant_new_tuple(additional_data_childrens, 5);

    alteratorctl_ctx_free(p_ctx);
    p_ctx = alteratorctl_ctx_init_packages(PACKAGES_APT,
                                           APT_APPLY_ASYNC | APT_INSTALL,
                                           all_packages,
                                           NULL,
                                           (gpointer) g_variant_ref(additional_data));

    if (to_install_var)
        g_variant_unref(to_install_var);
    if (to_remove_var)
        g_variant_unref(to_remove_var);
    if (extra_remove_var)
        g_variant_unref(extra_remove_var);

    if (packages_module_info->module_iface->run(packages_module_info->module_instance, p_ctx) < 0)
        ERR_EXIT();

    if (!p_ctx->results)
        goto end;

    exit_code            = g_variant_get_child_value(p_ctx->results, 0);
    int apt_install_code = g_variant_get_int32(exit_code);
    if (apt_install_code)
    {
        g_printerr(_("Error exit code from apt install method: %i.\n"), apt_install_code);
        ERR_EXIT();
    }

end:
    if (all_packages)
        g_free(all_packages);

    if (exit_code)
        g_variant_unref(exit_code);

    if (p_ctx)
    {
        p_ctx->additional_data = NULL;
        alteratorctl_ctx_free(p_ctx);
    }

    if (packages_module_info)
    {
        packages_module_info->free_module_func(packages_module_info->module_instance);
        g_free(packages_module_info);
    }

    if (keys)
        g_free(keys);

    return ret;
}

static int components_module_get_component_status(AlteratorCtlComponentsModule *module,
                                                  char *component_str_id,
                                                  int *status,
                                                  GHashTable **installed_packages)
{
    int ret                              = 0;
    dbus_ctx_t *d_ctx                    = NULL;
    GError *dbus_call_error              = NULL;
    GVariant *status_variant             = NULL;
    GVariant *installed_packages_variant = NULL;

    if (!module)
    {
        g_printerr(
            _("Internal error in components module - AlteratorCtlComponentsModule *module is NULL in \"components "
              "status\".\n"));
        ERR_EXIT();
    }

    if (!component_str_id)
    {
        g_printerr(_("Internal error in components module - path to component is NULL in \"components status\".\n"));
        ERR_EXIT();
    }

    if (!status)
    {
        g_printerr(_("Internal error in components module - status is NULL in \"components status\".\n"));
        ERR_EXIT();
    }

    //Check component object
    int object_exist = 0;
    if (module->gdbus_source->alterator_gdbus_source_check_object_by_path(module->gdbus_source,
                                                                          component_str_id,
                                                                          &object_exist)
        < 0)
    {
        g_printerr(_("Error when checking the presence of an object on D-Bus.\n"));
        ERR_EXIT();
    }

    if (object_exist == 0)
    {
        g_printerr(_("Object %s isn't found on D-Bus.\n"), component_str_id);
        ERR_EXIT();
    }

    //check component interface of the object
    int iface_exists = 0;
    if (module->gdbus_source->alterator_gdbus_source_check_object_by_iface(module->gdbus_source,
                                                                           component_str_id,
                                                                           COMPONENT_INTERFACE_NAME,
                                                                           &iface_exists)
        < 0)
    {
        g_printerr(_("Error when checking if an object: %s has an interface %s.\n"),
                   component_str_id,
                   COMPONENT_INTERFACE_NAME);
        ERR_EXIT();
    }

    if (iface_exists == 0)
    {
        g_printerr(_("Object %s has no interface %s.\n"), component_str_id, COMPONENT_INTERFACE_NAME);
        ERR_EXIT();
    }

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                          component_str_id,
                          COMPONENT_INTERFACE_NAME,
                          COMPONENT_STATUS_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    if (!d_ctx)
    {
        g_printerr(_("Can't allocate dbus_ctx_t in component_module_get_component_status.\n"));
        ERR_EXIT();
    }

    d_ctx->reply_type = G_VARIANT_TYPE("(asi)");

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (!d_ctx->result)
    {
        g_printerr(_("D-Bus error in components module while calling Status(): failed to produce a result in "
                     "component: %s.\n"),
                   component_str_id);
        ERR_EXIT();
    }

    status_variant = g_variant_get_child_value(d_ctx->result, 1);
    *status        = g_variant_get_int32(status_variant);

    if (!installed_packages)
        goto end;

    // Not installed, but has installed packages
    *status = (*status) == NOT_INSTALLED ? PARTIALLY_INSTALLED : INSTALLED;

    installed_packages_variant = g_variant_get_child_value(d_ctx->result, 0);

    GVariantIter *iter = NULL;
    gchar *str         = NULL;

    g_variant_get(installed_packages_variant, "as", &iter);

    (*installed_packages) = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);

    while (g_variant_iter_loop(iter, "s", &str))
        g_hash_table_add(*installed_packages, g_strdup(str));

    g_variant_iter_free(iter);

end:
    if (d_ctx)
        dbus_ctx_free(d_ctx);

    g_clear_error(&dbus_call_error);

    if (status_variant)
        g_variant_unref(status_variant);

    if (installed_packages_variant)
        g_variant_unref(installed_packages_variant);

    return ret;
}

static int components_module_remove_component(AlteratorCtlComponentsModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                                    = 0;
    GHashTable *component_packages             = NULL;
    GHashTable *base_components_packages_table = NULL;
    GPtrArray *base_component_packages_arr     = g_ptr_array_new_full(0, (GDestroyNotify) g_free);
    gchar *base_components_packages            = NULL;
    const gchar *component_str_id              = NULL;

    g_variant_get((*ctx)->parameters, "(ms)", &component_str_id);
    if (!component_str_id)
    {
        g_printerr(_("Internal error in components module: component_str_id is NULL.\n"));
        ERR_EXIT();
    }

    if (components_module_get_packages_of_component(module, &component_packages, component_str_id, TRUE) < 0)
        ERR_EXIT();

    if (components_module_get_base_section_packages(module, &base_components_packages_table) < 0)
        ERR_EXIT();

    if (!base_components_packages_table)
    {
        if (components_module_remove_component_packages(module, (gchar *) component_str_id, component_packages) < 0)
            ERR_EXIT();
        else
            goto end;
    }

    GHashTableIter iter;
    g_hash_table_iter_init(&iter, base_components_packages_table);
    gpointer package = NULL;
    for (gsize i = 0; g_hash_table_iter_next(&iter, &package, NULL); i++)
        if (g_hash_table_contains(component_packages, (gchar *) package))
        {
            gchar *tmp               = base_components_packages;
            base_components_packages = g_strconcat(tmp ? tmp : "",
                                                   g_strdup((gchar *) package),
                                                   i != g_hash_table_size(base_components_packages_table) - 1 ? " " : "",
                                                   NULL);
            g_free(tmp);

            if (!allow_remove_base_components)
                g_ptr_array_add(base_component_packages_arr, g_strdup((gchar *) package));
        }

    if (!allow_remove_base_components && base_component_packages_arr->len)
    {
        g_ptr_array_sort(base_component_packages_arr, components_module_sort_strings);
        if (!no_apt_update)
            g_printerr("\n");
        g_printerr(_("Can't remove %s component. The component contains packages on base "
                     "components packages:\n"),
                   component_str_id);

        for (gsize i = 0; i < base_component_packages_arr->len; i++)
            g_printerr("%s\n", (gchar *) base_component_packages_arr->pdata[i]);
        ERR_EXIT();
    }

    if (components_module_remove_component_packages(module, (gchar *) component_str_id, component_packages) < 0)
        ERR_EXIT();

end:

    if (component_packages)
        g_hash_table_destroy(component_packages);

    if (base_components_packages_table)
        g_hash_table_destroy(base_components_packages_table);

    if (base_component_packages_arr)
        g_ptr_array_unref(base_component_packages_arr);

    if (component_str_id)
        g_free((gpointer) component_str_id);

    g_free(base_components_packages);

    return ret;
}

static int components_module_get_packages_of_component(AlteratorCtlComponentsModule *module,
                                                       GHashTable **components_packages,
                                                       const gchar *component_str_id,
                                                       gboolean is_installed_component)
{
    int ret = 0;
    GHashTable *component_packages_names = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);
    GHashTable *component_installed_packages_names   = NULL;
    GHashTable *component_uninstalled_packages_names = NULL;
    GNode *parsed_component                          = NULL;
    gchar *component_path                            = NULL;
    GHashTable *component_packages                   = NULL;
    (*components_packages) = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    if (!component_str_id)
    {
        g_printerr(_("Internal error in components getting installed packages: path or name is empty.\n"));
        ERR_EXIT();
    }

    component_path = component_str_id[0] != '/' ? components_module_path_from_name(component_str_id)
                                                : g_strdup((gchar *) component_str_id);

    //Check object
    if (components_module_validate_object_and_iface(module, component_path, COMPONENT_INTERFACE_NAME) < 0)
        ERR_EXIT();

    if (info_parser->alterator_ctl_module_info_parser_get_specified_object_data(info_parser,
                                                                                module->gdbus_source,
                                                                                component_path,
                                                                                COMPONENT_INTERFACE_NAME,
                                                                                &parsed_component)
        < 0)
    {
        g_printerr(_("Can't get component %s installed packages. Component info parsing failed.\n"), component_str_id);
        ERR_EXIT();
    }

    GNode *component_packages_table = NULL;
    if (!(component_packages_table = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
              info_parser, parsed_component, COMPONENT_ALTERATOR_ENTRY_COMPONENT_PACKAGES_TABLE_NAME, -1)))
    {
        g_printerr(_("Can't get component %s installed packages. Getting packages list failed.\n"), component_str_id);
        ERR_EXIT();
    }

    gboolean is_has_ingnored_packages = FALSE;
    for (GNode *component_package = component_packages_table->children; component_package != NULL;
         component_package        = g_node_next_sibling(component_package))
    {
        gboolean ignore_by_arch = FALSE;
        if (components_module_is_ignore_package_by_arch(module, component_package, &ignore_by_arch, NULL) < 0)
            ERR_EXIT();

        gboolean ignore_by_lang = FALSE;
        if (components_module_is_ignore_package_by_language(module, component_package, &ignore_by_lang, NULL, 0) < 0)
            ERR_EXIT();

        gboolean ignore_by_desktop_env = FALSE;
        if (components_module_is_ignore_package_by_desktop_env(module,
                                                               component_package,
                                                               &ignore_by_desktop_env,
                                                               NULL,
                                                               FALSE,
                                                               0)
            < 0)
            ERR_EXIT();

        if (ignore_by_arch || ignore_by_lang || ignore_by_desktop_env)
            is_has_ingnored_packages = TRUE;

        gchar *package_name = NULL;
        if (components_module_get_package_name(module, component_package, &package_name) < 0)
            ERR_EXIT();

        if (!ignore_by_arch && !ignore_by_lang && !ignore_by_desktop_env)
            g_hash_table_add(component_packages_names, g_strdup(package_name));
        g_free(package_name);
    }

    if (!g_hash_table_size(component_packages_names) && is_has_ingnored_packages)
    {
        g_printerr(_("Component %s has no applicable packages: all were\n"
                     "filtered out due to mismatch with the required\n"
                     "architecture, localization, or desktop environments\n"
                     "specified in the component's alterator entry.\n"),
                   component_str_id);
        ERR_EXIT();
    }

    int status = -1;
    if (components_module_get_component_status(module,
                                               (gchar *) component_path,
                                               &status,
                                               &component_installed_packages_names)
        < 0)
    {
        g_printerr(_("Can't get component %s installed packages.\n"), component_str_id);
        ERR_EXIT();
    }

    if (status == INSTALLED && !is_installed_component)
        g_printerr(_("Component %s already installed.\n"), component_str_id);
    else if (status == NOT_INSTALLED && is_installed_component)
        g_printerr(_("Component %s not installed.\n"), component_str_id);

    if (!is_installed_component
        && components_module_get_components_uninstalled_packages(module, &component_uninstalled_packages_names) < 0)
        ERR_EXIT();

    GHashTableIter iter;
    gchar *package_name = NULL;
    gpointer value      = NULL;
    g_hash_table_iter_init(&iter, component_packages_names);
    while (g_hash_table_iter_next(&iter, (gpointer *) &package_name, &value))
        if (is_installed_component && g_hash_table_contains(component_installed_packages_names, package_name))
            g_hash_table_add(*components_packages, g_strdup(package_name));
        else if (!is_installed_component && g_hash_table_contains(component_uninstalled_packages_names, package_name))
            g_hash_table_add(*components_packages, g_strdup(package_name));

end:
    if (component_packages)
        g_hash_table_destroy(component_packages);

    if (component_installed_packages_names)
        g_hash_table_destroy(component_installed_packages_names);

    if (component_uninstalled_packages_names)
        g_hash_table_destroy(component_uninstalled_packages_names);

    if (parsed_component)
        alterator_ctl_module_info_parser_result_tree_free(parsed_component);

    g_free(component_path);

    if (component_packages_names)
        g_hash_table_destroy(component_packages_names);

    return ret;
}

static int components_module_remove_component_packages(AlteratorCtlComponentsModule *module,
                                                       gchar *component_str_id,
                                                       GHashTable *components_packages)
{
    int ret                                  = 0;
    AlteratorCtlModule *packages_module_info = NULL;
    alteratorctl_ctx_t *p_ctx                = NULL;
    GVariant *exit_code                      = NULL;
    GList *packages_to_remove                = NULL;
    gchar **keys                             = NULL;
    gchar *all_packages                      = NULL;
    guint len                                = 0;

    if (!module)
    {
        g_printerr(
            _("Internal error in components module - AlteratorCtlComponentsModule *module is NULL in \"components "
              "remove\".\n"));
        ERR_EXIT();
    }

    if (alterator_ctl_get_registered_module(module->alterator_ctl_app,
                                            ALTERATOR_CTL_PACKAGES_MODULE_NAME,
                                            &packages_module_info)
        < 0)
    {
        g_printerr(_("Packages module isn't registered.\n"));
        ERR_EXIT();
    }

    keys = (gchar **) g_hash_table_get_keys_as_array(components_packages, &len);

    all_packages = g_strjoinv(" ", keys);

    if (!keys)
    {
        g_printerr(_("No packages to remove.\n"));
        ERR_EXIT();
    }

    gchar *check_apply_str = g_strjoinv("- ", keys);
    gchar *tmp             = check_apply_str;
    check_apply_str        = g_strconcat(tmp, "-", NULL);
    g_free(tmp);

    p_ctx = alteratorctl_ctx_init_packages(PACKAGES_APT, APT_CHECK_APPLY_PRIV, check_apply_str, NULL, NULL);
    g_free(check_apply_str);

    if (packages_module_info->module_iface->run(packages_module_info->module_instance, p_ctx) < 0)
        ERR_EXIT();

    if (components_module_print_affected_components(module, component_str_id, (GVariant *) p_ctx->results, REMOVE, TRUE)
        < 0)
        ERR_EXIT();

    GVariant *to_install_var              = g_variant_get_child_value(p_ctx->results, 0);
    GVariant *to_remove_var               = g_variant_get_child_value(p_ctx->results, 1);
    GVariant *extra_remove_var            = g_variant_get_child_value(p_ctx->results, 2);
    GVariant *additional_data_childrens[] = {to_install_var,
                                             to_remove_var,
                                             extra_remove_var,
                                             g_variant_new_boolean(allow_remove_manually),
                                             g_variant_new_boolean(force_yes)};
    GVariant *additional_data             = g_variant_new_tuple(additional_data_childrens, 5);

    alteratorctl_ctx_free(p_ctx);
    p_ctx = alteratorctl_ctx_init_packages(PACKAGES_APT,
                                           APT_APPLY_ASYNC | APT_REMOVE,
                                           all_packages,
                                           NULL,
                                           (gpointer) g_variant_ref(additional_data));
    if (to_install_var)
        g_variant_unref(to_install_var);
    if (to_remove_var)
        g_variant_unref(to_remove_var);
    if (extra_remove_var)
        g_variant_unref(extra_remove_var);

    if (packages_module_info->module_iface->run(packages_module_info->module_instance, p_ctx) < 0)
        ERR_EXIT();

    if (!p_ctx->results)
        goto end;

    exit_code           = g_variant_get_child_value(p_ctx->results, 0);
    int apt_remove_code = g_variant_get_int32(exit_code);
    if (apt_remove_code)
    {
        g_printerr(_("Error exit code from apt remove method: %i.\n"), apt_remove_code);
        ERR_EXIT();
    }

end:
    if (all_packages)
        g_free(all_packages);

    if (exit_code)
        g_variant_unref(exit_code);

    if (p_ctx)
    {
        p_ctx->additional_data = NULL;
        alteratorctl_ctx_free(p_ctx);
    }

    if (packages_module_info)
    {
        packages_module_info->free_module_func(packages_module_info->module_instance);
        g_free(packages_module_info);
    }

    if (packages_to_remove)
        g_list_free(packages_to_remove);

    if (keys)
        g_free(keys);

    return ret;
}

static int components_module_calculate_affected_components(AlteratorCtlComponentsModule *module,
                                                           const gchar *target_component_str_id,
                                                           gchar **packages,
                                                           components_module_transaction_mode action,
                                                           GHashTable **result)
{
    int ret                                   = 0;
    GHashTable *packages2components_table     = g_hash_table_new_full(g_str_hash,
                                                                  g_str_equal,
                                                                  (GDestroyNotify) g_free,
                                                                  (GDestroyNotify) g_hash_table_destroy);
    GHashTable *components_batch_info         = NULL;
    GHashTable *uninstalled_packages          = NULL;
    gchar *current_arch                       = NULL;
    gchar **current_locales                   = NULL;
    gsize locales_len                         = 0;
    gchar **current_desktop_environments      = NULL;
    gsize de_env_len                          = 0;
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    if (!target_component_str_id || (target_component_str_id && !strlen(target_component_str_id)))
    {
        if (action != INSTALL && action != REMOVE)
        {
            g_printerr(_("Failed to calculate affected components for unknown action with unspecified component.\n"));
            ERR_EXIT();
        }

        g_printerr(_("Failed to calculate affected components for %s of unspecified component.\n"),
                   action == REMOVE ? _("remove") : _("install"));
        ERR_EXIT();
    }

    if (action != INSTALL && action != REMOVE)
    {
        g_printerr(_("Failed to calculate affected components for unknown action with component %s.\n"),
                   target_component_str_id);
        ERR_EXIT();
    }

    if (!module)
    {
        g_printerr(_("Failed to calculate affected components for %s of component %s.\n"),
                   action == REMOVE ? _("remove") : _("install"),
                   target_component_str_id);
        ERR_EXIT();
    }

    if (!packages || (packages && !g_strv_length(packages)))
    {
        g_printerr(_("Failed to calculate affected components for %s of component %s. Get empty packages list of %s "
                     "component.\n"),
                   action == REMOVE ? _("remove") : _("install"),
                   target_component_str_id,
                   target_component_str_id);
        ERR_EXIT();
    }

    if (!result)
    {
        g_printerr(_("Failed to calculate affected components for %s of component %s. Result variable is NULL.\n"),
                   action == REMOVE ? _("remove") : _("install"),
                   target_component_str_id);
        ERR_EXIT();
    }

    if (components_module_get_components_info(module, &components_batch_info) < 0)
        ERR_EXIT();

    if (!components_batch_info || (components_batch_info && !g_hash_table_size(components_batch_info)))
    {
        g_printerr(_("Failed to calculate affected components for %s of component %s. Components alterator entry info "
                     "is empty.\n"),
                   action == REMOVE ? _("remove") : _("install"),
                   target_component_str_id);
        ERR_EXIT();
    }

    if (components_module_get_components_uninstalled_packages(module, &uninstalled_packages) < 0)
        ERR_EXIT();

    if (!uninstalled_packages)
    {
        g_printerr(_("Failed to calculate affected components for %s of component %s. Can't get uninstalled "
                     "packages list.\n"),
                   action == REMOVE ? _("remove") : _("install"),
                   target_component_str_id);
        ERR_EXIT();
    }

    if (components_module_get_arch(module, &current_arch) < 0)
        ERR_EXIT();

    if (!(current_locales = components_module_get_locales(module, &locales_len)))
        ERR_EXIT();

    current_desktop_environments = components_module_get_desktop_environments(module, &de_env_len);

    GHashTableIter iter;
    g_hash_table_iter_init(&iter, components_batch_info);
    gpointer info = NULL;
    while (g_hash_table_iter_next(&iter, &info, NULL))
    {
        GNode *component_info = NULL;
        if (info_parser->alterator_ctl_module_info_parser_get_data_from_text(info_parser,
                                                                             module->gdbus_source,
                                                                             (gchar *) info,
                                                                             &component_info)
            < 0)
            ERR_EXIT();

        toml_value *component_name_toml = g_hash_table_lookup(((alterator_entry_node *) component_info->data)->toml_pairs,
                                                              COMPONENT_ALTERATOR_ENTRY_COMPONENT_NAME_KEY_NAME);
        gchar *component_name = component_name_toml ? component_name_toml->str_value : NULL;

        toml_value *is_draft = g_hash_table_lookup(((alterator_entry_node *) component_info->data)->toml_pairs,
                                                   COMPONENT_ALTERATOR_ENTRY_COMPONENT_DRAFT_KEY_NAME);
        if (is_draft && is_draft->bool_value)
        {
            alterator_ctl_module_info_parser_result_tree_free(component_info);
            continue;
        }

        components_module_item_installed_status status = NONE;
        components_module_get_component_status_by_uninstalled_packages_list(module,
                                                                            component_info,
                                                                            uninstalled_packages,
                                                                            current_arch,
                                                                            current_locales,
                                                                            locales_len,
                                                                            current_desktop_environments,
                                                                            de_env_len,
                                                                            &status);

        if (action == INSTALL && status == INSTALLED)
            continue;

        if (action == REMOVE && (status == NOT_INSTALLED || status == PARTIALLY_INSTALLED))
            continue;

        GNode *packages = NULL;
        if (!(packages = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
                  info_parser, component_info, COMPONENT_ALTERATOR_ENTRY_COMPONENT_PACKAGES_TABLE_NAME, -1)))
        {
            g_printerr(_("Failed to calculate affected components for %s of component %s. Can't get packages list of "
                         "component %s.\n"),
                       action == REMOVE ? _("remove") : _("install"),
                       target_component_str_id,
                       component_name);
            ERR_EXIT();
        }

        for (GNode *package = packages->children; package != NULL; package = g_node_next_sibling(package))
        {
            gboolean ignore_by_arch = FALSE;
            if (components_module_is_ignore_package_by_arch(module, package, &ignore_by_arch, current_arch) < 0)
                ERR_EXIT();

            gboolean ignore_by_lang = FALSE;
            if (components_module_is_ignore_package_by_language(module,
                                                                package,
                                                                &ignore_by_lang,
                                                                current_locales,
                                                                locales_len)
                < 0)
                ERR_EXIT();

            gboolean ignore_by_desktop_env = FALSE;
            if (components_module_is_ignore_package_by_desktop_env(module,
                                                                   package,
                                                                   &ignore_by_desktop_env,
                                                                   current_desktop_environments,
                                                                   TRUE,
                                                                   de_env_len)
                < 0)
                ERR_EXIT();

            if (ignore_by_arch || ignore_by_lang || ignore_by_desktop_env)
                continue;

            gchar *package_name = NULL;
            if (components_module_get_package_name(module, package, &package_name) < 0)
                ERR_EXIT();

            GHashTable *components_names = g_hash_table_lookup(packages2components_table, package_name);
            if (!components_names)
            {
                components_names = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);
                g_hash_table_add(components_names, g_strdup(component_name));
                g_hash_table_insert(packages2components_table, g_strdup(package_name), components_names);
            }
            else
                g_hash_table_add(components_names, g_strdup(component_name));
        }

        alterator_ctl_module_info_parser_result_tree_free(component_info);
    }

    *result = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);
    for (gsize i = 0; i < g_strv_length(packages); i++)
    {
        GHashTable *components_names_table = g_hash_table_lookup(packages2components_table, packages[i]);
        if (!components_names_table)
            continue;

        GList *components_names = g_hash_table_get_keys(components_names_table);
        for (GList *component_name = components_names; component_name != NULL; component_name = component_name->next)
            g_hash_table_add(*result, g_strdup((gchar *) component_name->data));

        g_list_free(components_names);
    }

end:
    if (packages2components_table)
        g_hash_table_destroy(packages2components_table);

    if (components_batch_info)
        g_hash_table_destroy(components_batch_info);

    if (uninstalled_packages)
        g_hash_table_destroy(uninstalled_packages);

    g_free(current_arch);

    g_strfreev(current_locales);

    g_strfreev(current_desktop_environments);

    return ret;
}

static int components_module_print_affected_components(AlteratorCtlComponentsModule *module,
                                                       gchar *target_component_str_id,
                                                       GVariant *packages_check_apply_result,
                                                       components_module_transaction_mode action,
                                                       gboolean target_print_separately)
{
    int ret                                 = 0;
    gchar **pkgs_to_install                 = NULL;
    gchar **pkgs_to_remove                  = NULL;
    gchar **pkgs_extra_remove               = NULL;
    GHashTable *components_to_install_table = NULL;
    GHashTable *components_to_remove_table  = NULL;
    if (!target_component_str_id || (target_component_str_id && !strlen(target_component_str_id)))
    {
        if (action != INSTALL && action != REMOVE)
        {
            g_printerr(_("Failed to print affected components for unknown action with unspecified component.\n"));
            ERR_EXIT();
        }

        g_printerr(_("Failed to print affected components for %s of unspecified component.\n"),
                   action == REMOVE ? _("remove") : _("install"));
        ERR_EXIT();
    }

    if (action != INSTALL && action != REMOVE)
    {
        g_printerr(_("Failed to print affected components for unknown action with component %s.\n"),
                   target_component_str_id);
        ERR_EXIT();
    }

    if (!module)
    {
        g_printerr(_("Failed to print affected components for %s of component %s.\n"),
                   action == REMOVE ? _("remove") : _("install"),
                   target_component_str_id);
        ERR_EXIT();
    }

    if (!packages_check_apply_result)
    {
        g_printerr(_("Failed to print affected components for %s of component %s. Empty list of packages to install "
                     "and to remove.\n"),
                   action == REMOVE ? _("remove") : _("install"),
                   target_component_str_id);
        ERR_EXIT();
    }

    g_variant_get(packages_check_apply_result,
                  "(m^asm^asm^asi)",
                  &pkgs_to_install,
                  &pkgs_to_remove,
                  &pkgs_extra_remove,
                  &ret);

    if (ret)
    {
        g_printerr(_("Failed to print affected components for %s of component %s. Error from getting of packages to "
                     "install and to remove. Exit code: %i.\n"),
                   action == REMOVE ? _("remove") : _("install"),
                   target_component_str_id,
                   ret);
        ERR_EXIT();
    }

    if ((!pkgs_to_install || (pkgs_to_install && !g_strv_length(pkgs_to_install)))
        && (!pkgs_to_remove || (pkgs_to_remove && !g_strv_length(pkgs_to_remove)))
        && (!pkgs_extra_remove || (pkgs_extra_remove && !g_strv_length(pkgs_extra_remove))))
    {
        g_printerr(_("Failed to print affected components for %s of component %s. Empty list of packages to install "
                     "and to remove.\n"),
                   action == REMOVE ? _("remove") : _("install"),
                   target_component_str_id);
        ERR_EXIT();
    }

    if (pkgs_to_install && g_strv_length(pkgs_to_install)
        && components_module_calculate_affected_components(module,
                                                           target_component_str_id,
                                                           pkgs_to_install,
                                                           INSTALL,
                                                           &components_to_install_table)
               < 0)
        ERR_EXIT();

    if ((pkgs_to_remove && g_strv_length(pkgs_to_remove)) || (pkgs_extra_remove && g_strv_length(pkgs_extra_remove)))
    {
        gchar **to_all_remove_pkgs = NULL;
        if ((pkgs_to_remove && g_strv_length(pkgs_to_remove))
            && !(pkgs_extra_remove && g_strv_length(pkgs_extra_remove)))
            to_all_remove_pkgs = g_strdupv(pkgs_to_remove);

        if (allow_remove_manually && pkgs_extra_remove && g_strv_length(pkgs_extra_remove))
            pkgs_extra_remove = g_strdupv(pkgs_extra_remove);

        if ((to_all_remove_pkgs && g_strv_length(to_all_remove_pkgs))
            || (pkgs_extra_remove && g_strv_length(pkgs_extra_remove)))
        {
            GStrvBuilder *builder = g_strv_builder_new();
            g_strv_builder_addv(builder, (const gchar **) pkgs_to_remove);
            g_strv_builder_addv(builder, (const gchar **) pkgs_extra_remove);
            to_all_remove_pkgs = g_strv_builder_unref_to_strv(builder);

            if (components_module_calculate_affected_components(module,
                                                                target_component_str_id,
                                                                to_all_remove_pkgs,
                                                                REMOVE,
                                                                &components_to_remove_table)
                < 0)
            {
                g_strfreev(to_all_remove_pkgs);
                ERR_EXIT();
            }
        }
        g_strfreev(to_all_remove_pkgs);
    }

    if (target_print_separately)
    {
        if (!no_apt_update)
            g_print("\n");
        if (action == INSTALL)
            g_print(_("Components to install:\n"));
        else
            g_print(_("Components to remove:\n"));
        g_print("  %s%s\n", target_component_str_id, no_apt_update ? "\n" : "");
    }

    if (components_to_install_table && !(target_print_separately && g_hash_table_size(components_to_install_table) == 1)
        && g_hash_table_contains(components_to_install_table, target_component_str_id))
    {
        GPtrArray *components_to_install_arr = g_hash_table_get_keys_as_ptr_array(components_to_install_table);
        g_ptr_array_sort(components_to_install_arr, components_module_sort_strings);
        GStrvBuilder *builder = g_strv_builder_new();
        for (gsize i = 0; i < components_to_install_arr->len; i++)
            g_strv_builder_add(builder, components_to_install_arr->pdata[i]);
        gchar **components_to_install_strv = g_strv_builder_unref_to_strv(builder);
        gchar *components_to_install       = NULL;
        if (!(components_to_install = columnize_text(components_to_install_strv)))
        {
            g_printerr(_("Failed to print affected components for %s of component %s. Failed to make pretty output.\n"),
                       action == REMOVE ? _("remove") : _("install"),
                       target_component_str_id);
            g_ptr_array_unref(components_to_install_arr);
            g_strfreev(components_to_install_strv);
            ERR_EXIT();
        }
        if (!no_apt_update)
            g_print("\n");
        g_print(_("The following components will also be installed:\n"));
        g_print("%s\n", components_to_install);

        g_ptr_array_unref(components_to_install_arr);
        g_strfreev(components_to_install_strv);
    }

    if (components_to_remove_table && !(target_print_separately && g_hash_table_size(components_to_remove_table) == 1)
        && g_hash_table_contains(components_to_remove_table, target_component_str_id))
    {
        GPtrArray *components_to_remove_arr = g_hash_table_get_keys_as_ptr_array(components_to_remove_table);
        g_ptr_array_sort(components_to_remove_arr, components_module_sort_strings);
        GStrvBuilder *builder = g_strv_builder_new();
        for (gsize i = 0; i < components_to_remove_arr->len; i++)
            g_strv_builder_add(builder, components_to_remove_arr->pdata[i]);
        gchar **components_to_remove_strv = g_strv_builder_unref_to_strv(builder);
        gchar *components_to_remove       = NULL;
        if (!(components_to_remove = columnize_text(components_to_remove_strv)))
        {
            g_printerr(_("Failed to print affected components for %s of component %s. Failed to make pretty output.\n"),
                       action == REMOVE ? _("remove") : _("install"),
                       target_component_str_id);
            g_ptr_array_unref(components_to_remove_arr);
            g_strfreev(components_to_remove_strv);
            ERR_EXIT();
        }

        if (!no_apt_update)
            g_print("\n");
        g_print(_("The following components will also be removed:\n"));
        g_print("%s\n", components_to_remove);

        g_ptr_array_unref(components_to_remove_arr);
        g_strfreev(components_to_remove_strv);
    }

end:
    g_strfreev(pkgs_to_install);

    g_strfreev(pkgs_to_remove);

    g_strfreev(pkgs_extra_remove);

    if (components_to_install_table)
        g_hash_table_destroy(components_to_install_table);

    if (components_to_remove_table)
        g_hash_table_destroy(components_to_remove_table);

    return ret;
}

static int components_module_get_base_section_packages(AlteratorCtlComponentsModule *module, GHashTable **result)
{
    int ret                                   = 0;
    gsize amounth_of_sections                 = 0;
    GNode **edition_sections                  = NULL;
    GNode *components_info                    = g_node_new(NULL);
    GHashTable *components_info_table         = NULL;
    GHashTable *base_components               = g_hash_table_new(g_str_hash, g_str_equal);
    gchar *arch                               = NULL;
    gchar **locales                           = NULL;
    gchar **desktop_environments              = NULL;
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;
    if (!module)
    {
        g_printerr(_("Internal error in components module - AlteratorCtlComponentsModule *module is NULL in "
                     "components_module_get_base_section_packages.\n"));
        ERR_EXIT();
    }

    // No current edition
    if (!(edition_sections = components_module_get_section_nodes(module, &amounth_of_sections)))
        return ret;

    GNode *section_base = NULL;
    for (gsize i = 0; i < amounth_of_sections; i++)
        if (g_strcmp0(((alterator_entry_node *) edition_sections[i]->data)->node_name, "base") == 0)
        {
            section_base = edition_sections[i];
            break;
        }

    if (!section_base)
    {
        g_printerr("Can't get components of non-existent section \"base\" of current edition.\n");
        ERR_EXIT();
    }

    toml_value *base_components_arr = g_hash_table_lookup(((alterator_entry_node *) section_base->data)->toml_pairs,
                                                          "components");
    if (!base_components_arr || (base_components_arr && base_components_arr->type != TOML_DATA_ARRAY_OF_STRING)
        || (base_components_arr && !base_components_arr->array_length))
    {
        g_printerr(_("Failed to get base components packages. Failed to get base components list.\n"));
        ERR_EXIT();
    }

    for (gsize i = 0; i < base_components_arr->array_length; i++)
        g_hash_table_add(base_components, ((gchar **) base_components_arr->array)[i]);

    if (components_module_get_components_info(module, &components_info_table) < 0)
        ERR_EXIT();

    GHashTableIter iter;
    g_hash_table_iter_init(&iter, components_info_table);
    gchar *component_alterator_entry = NULL;
    while (g_hash_table_iter_next(&iter, (gpointer *) &component_alterator_entry, NULL))
    {
        GNode *component_info = NULL;
        if (info_parser->alterator_ctl_module_info_parser_get_data_from_text(info_parser,
                                                                             module->gdbus_source,
                                                                             component_alterator_entry,
                                                                             &component_info)
            < 0)
            ERR_EXIT();

        toml_value *component_name = g_hash_table_lookup(((alterator_entry_node *) component_info->data)->toml_pairs,
                                                         COMPONENT_ALTERATOR_ENTRY_COMPONENT_NAME_KEY_NAME);
        if (!component_name || (component_name && component_name->type != TOML_DATA_STRING)
            || (component_name && !strlen(component_name->str_value)))
        {
            g_printerr(_("Failed to get base components packages. Failed to get base component name.\n"));
            ERR_EXIT();
        }

        if (!g_hash_table_contains(base_components, component_name->str_value))
        {
            alterator_ctl_module_info_parser_result_tree_free(component_info);
            continue;
        }

        g_node_append(components_info, component_info);
    }

    if (components_module_get_arch(module, &arch) < 0)
        ERR_EXIT();

    gsize locales_len = 0;
    locales           = components_module_get_locales(module, &locales_len);

    gsize de_len         = 0;
    desktop_environments = components_module_get_desktop_environments(module, &de_len);

    *result = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);
    for (GNode *component_info = components_info->children; component_info != NULL;
         component_info        = g_node_next_sibling(component_info))
    {
        toml_value *component_name = g_hash_table_lookup(((alterator_entry_node *) component_info->data)->toml_pairs,
                                                         COMPONENT_ALTERATOR_ENTRY_COMPONENT_NAME_KEY_NAME);
        if (!component_name || (component_name && component_name->type != TOML_DATA_STRING)
            || (component_name && !strlen(component_name->str_value)))
        {
            g_printerr(_("Failed to get base components packages. Failed to get base component name.\n"));
            ERR_EXIT();
        }

        GNode *component_packages_table = NULL;
        if (!(component_packages_table = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
                  info_parser, component_info, COMPONENT_ALTERATOR_ENTRY_COMPONENT_PACKAGES_TABLE_NAME, -1)))
        {
            g_printerr(_("Can't get component %s packages. Getting base components packages list failed.\n"),
                       component_name->str_value);
            ERR_EXIT();
        }

        for (GNode *component_package = component_packages_table->children; component_package != NULL;
             component_package        = g_node_next_sibling(component_package))
        {
            gboolean ignore_package_by_arch = FALSE;
            if (components_module_is_ignore_package_by_arch(module, component_package, &ignore_package_by_arch, arch)
                < 0)
                ERR_EXIT();

            gboolean ignore_by_lang = FALSE;
            if (components_module_is_ignore_package_by_language(module,
                                                                component_package,
                                                                &ignore_by_lang,
                                                                locales,
                                                                locales_len)
                < 0)
                ERR_EXIT();

            gboolean ignore_by_desktop_env = FALSE;
            if (desktop_environments
                && components_module_is_ignore_package_by_desktop_env(module,
                                                                      component_package,
                                                                      &ignore_by_desktop_env,
                                                                      desktop_environments,
                                                                      TRUE,
                                                                      de_len)
                       < 0)
                ERR_EXIT();

            gchar *package_name = NULL;
            if (components_module_get_package_name(module, component_package, &package_name) < 0)
                ERR_EXIT();

            if (!ignore_package_by_arch && !ignore_by_lang && !ignore_by_desktop_env)
                g_hash_table_add(*result, g_strdup(package_name));
            g_free(package_name);
        }
    }

end:
    if (edition_sections)
        alterator_ctl_module_info_parser_result_trees_free(edition_sections, amounth_of_sections);

    if (components_info)
        alterator_ctl_module_info_parser_result_tree_free(components_info);

    if (base_components)
        g_hash_table_unref(base_components);

    if (components_info_table)
        g_hash_table_destroy(components_info_table);

    g_free(arch);

    g_strfreev(locales);

    g_strfreev(desktop_environments);

    return ret;
}

static int components_module_validate_object_and_iface(AlteratorCtlComponentsModule *module,
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
