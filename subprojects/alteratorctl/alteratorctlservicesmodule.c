#include "alteratorctlservicesmodule.h"
#include "alteratorctldiagmodule.h"

#include <json-glib/json-glib.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define SERVICES_INTERFACE_NAME "org.altlinux.alterator.service1"
#define SERVICES_BACKUP_NAME_METHOD_NAME "Backup"
#define SERVICES_CONFIGURE_NAME_METHOD_NAME "Configure"
#define SERVICES_DEPLOY_METHOD_NAME "Deploy"
#define SERVICES_INFO_METHOD_NAME "Info"
#define SERVICES_RESTORE_NAME_METHOD_NAME "Restore"
#define SERVICES_STATUS_NAME_METHOD_NAME "Status"
#define SERVICES_START_NAME_METHOD_NAME "Start"
#define SERVICES_STOP_NAME_METHOD_NAME "Stop"
#define SERVICES_UNDEPLOY_NAME_METHOD_NAME "Undeploy"

#define DIAG_INTERFACE_NAME "org.altlinux.alterator.diag1"

#define SERVICES_ALTERATOR_ENTRY_SERVICE_NAME_KEY_NAME "name"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_VALUE_KEY_NAME "default"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_TYPE_KEY_NAME "type"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_ARRAY_TYPE_KEY_NAME "array_type"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_PROTOTYPE_KEY_NAME "prototype"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_FORCE_DEPLOY_KEY_NAME "force_deploy"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_DEPLOYED_KEY_NAME "deployed"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_STARTED_KEY_NAME "started"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_PARAM_CONSTANT_KEY_NAME "constant"

#define SERVICES_STDOUT_SIGNAL_NAME "service_stdout_signal"
#define SERVICES_STDERR_SIGNAL_NAME "service_stderr_signal"

#define SERVICES_ALTERATOR_ENTRY_SERVICE_PARAM_REQUIRED_KEY_NAME "required"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_PARAM_CONTEXT_KEY_NAME "context"

#define SERVICES_CONTEXT_DEPLOY "deploy"
#define SERVICES_CONTEXT_UNDEPLOY "undeploy"
#define SERVICES_CONTEXT_CONFIGURE "configure"
#define SERVICES_CONTEXT_BACKUP "backup"
#define SERVICES_CONTEXT_RESTORE "restore"
#define SERVICES_CONTEXT_STATUS "status"
#define SERVICES_CONTEXT_DIAG "diag"

#define SERVICES_ALTERATOR_ENTRY_SERVICE_ENABLE_FORCE_DEPLOY_KEY_NAME "enable_force_deploy"

#define SERVICES_ALTERATOR_ENTRY_SERVICE_DISPLAY_NAME_TABLE_NAME "display_name"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_COMMENT_NAME_TABLE_NAME "comment"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_PARAMETERS_TABLE_NAME "parameters"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_PROPERTIES_TABLE_NAME "properties"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_TYPES_TABLE_NAME "types"

#define SERVICES_ALTERATOR_ENTRY_SERVICE_DIAG_TOOLS_TABLE_NAME "diag_tools"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCES_TABLE_NAME "resources"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCES_PARAMETER_KEY_NAME "parameter"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCES_PATH_TABLE_NAME "path"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCES_INET_TABLE_NAME "inet_service"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCES_UNIT_TABLE_NAME "unit_name"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCE_VALUE_KEY_NAME "value"

#define SERVICES_DIAGNOSE_TESTS_ENV_VALUE_DEPLOY_MODE "service_deploy_mode"
#define SERVICES_DIAGNOSE_TESTS_ENV_VALUE_REQUIRED_ONLY "service_required_only"
#define SERVICES_DIAGNOSE_TESTS_ENV_VALUE_SERVICE_PATH "service_path"

#define SERVICES_ALTERATOR_ENTRY_SERVICE_DIAG_TOOLS_TABLE_NAME "diag_tools"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_DIAG_TOOLS_PATH_KEY_NAME "path"
#define SERVICES_ALTERATOR_ENTRY_SERVICE_DIAG_TOOLS_BUS_KEY_NAME "bus"
#define DIAG_ALTERATOR_ENTRY_TOOLNAME_KEY_NAME "name"
#define DIAG_ALTERATOR_ENTRY_SERVICE_DIAG_TESTS_TABLE_NAME "tests"

#define SERVICES_JSON_PARAMS_INTEGER_PATTERN "^\\d+$"
#define SERVICES_JSON_PARAMS_BOOLEAN_PATTERN "true|false"

#define SERVICES_MODULE_MAX_USAGE_HELP_STRING_LENGTH 80
#define SERVICES_MODULE_MAX_USAGE_HELP_DESCRIPTIONS_MARGIN 30

#define SERVICES_MODULE_MIN_PARAMS_PATH_COLUMN_WIDTH 25
#define SERVICES_MODULE_MIN_PARAMS_VALUE_COLUMN_WIDTH 5

#define EMPTY_JSON "{}"

#define SERVICES_PARAMS_TABLE_DOUBLE_LT "╔"
#define SERVICES_PARAMS_TABLE_DOUBLE_RT "╗"
#define SERVICES_PARAMS_TABLE_DOUBLE_LB "╚"
#define SERVICES_PARAMS_TABLE_DOUBLE_RB "╝"
#define SERVICES_PARAMS_TABLE_DOUBLE_H "═"
#define SERVICES_PARAMS_TABLE_DOUBLE_HT "╦"
#define SERVICES_PARAMS_TABLE_DOUBLE_HB "╩"
#define SERVICES_PARAMS_TABLE_DOUBLE_V "║"
#define SERVICES_PARAMS_TABLE_DOUBLE_VR "╠"
#define SERVICES_PARAMS_TABLE_DOUBLE_VH "╬"
#define SERVICES_PARAMS_TABLE_DOUBLE_VL "╣"

#define SERVICES_PARAMS_TABLE_SINGLE_H_DOUBLE_T "╤"
#define SERVICES_PARAMS_TABLE_SINGLE_H_DOUBLE_B "╧"
#define SERVICES_PARAMS_TABLE_SINGLE_H_SINGLE_B "┴"
#define SERVICES_PARAMS_TABLE_SINGLE_V_DOUBLE_H "╪"
#define SERVICES_PARAMS_TABLE_SINGLE_H_DOUBLE_V_R "╟"
#define SERVICES_PARAMS_TABLE_SINGLE_H_DOUBLE_V_L "╢"

#define SERVICES_PARAMS_TABLE_SINGLE_H "─"
#define SERVICES_PARAMS_TABLE_SINGLE_V "│"

#define streq(str1, str2) (g_strcmp0(str1, str2) == 0)

typedef enum deployment_status
{
    UNDEPLOYED           = 0,
    DEPLOYED             = 127,
    DEPLOYED_AND_STARTED = 128
} deployment_status;

typedef struct param_info
{
    const gchar *name;
    const gchar *linked_resource;
    GHashTable *required;
    GHashTable *usage_ctx;
    gboolean constant;
} param_info;

static gboolean services_module_is_special_param(const gchar *name)
{
    if (!name || !strlen(name))
        return FALSE;

    return streq(name, SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_FORCE_DEPLOY_KEY_NAME)
           || streq(name, SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_DEPLOYED_KEY_NAME)
           || streq(name, SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_STARTED_KEY_NAME);
}

typedef enum services_module_resource_group
{
    SERVICES_RESOURCE_GROUP_DIRECTORIES = 0,
    SERVICES_RESOURCE_GROUP_FILES,
    SERVICES_RESOURCE_GROUP_UNITS,
    SERVICES_RESOURCE_GROUP_PORTS,
    SERVICES_RESOURCE_GROUP_OTHER,
    SERVICES_RESOURCE_GROUP_COUNT
} services_module_resource_group_t;

typedef struct services_module_resource_row
{
    gchar *name;
    gchar *value;
    GPtrArray *details;
} services_module_resource_row_t;

static const gchar *services_module_resource_group_titles[SERVICES_RESOURCE_GROUP_COUNT] = {N_("Directories"),
                                                                                            N_("Files"),
                                                                                            N_("Systemd units"),
                                                                                            N_("Ports"),
                                                                                            N_("Other resources")};

static param_info *services_module_service_info_init(
    const gchar *name, const gchar *linked_resource, GHashTable *required, GHashTable *ctx, gboolean constant);
static void services_module_service_info_free(param_info *info);

typedef enum
{
    PRE_DIAG_MODE,
    POST_DIAG_MODE,
    BOTH_DIAG_MODE
} diag_tool_test_mode;

typedef struct
{
    gchar *name;
    gchar *display_name;
    gchar *description;
    diag_tool_test_mode mode;
    gboolean is_required;
} diag_tool_test_info;

static diag_tool_test_info *services_module_service_diag_tool_test_info_init_from_node(
    AlteratorCtlModuleInfoParser *info_parser, GNode *tool_info, gboolean is_requires, diag_tool_test_mode mode);
static diag_tool_test_info *services_module_service_diag_tool_test_info_init(const gchar *name,
                                                                             const gchar *display_name,
                                                                             const gchar *description,
                                                                             gboolean is_required,
                                                                             diag_tool_test_mode mode);
static void services_module_service_diag_tool_test_info_free(diag_tool_test_info *tool_tests);

typedef struct diag_tool_tests
{
    gchar *path;
    GHashTable *all_tests;
    GHashTable *required_tests;
    GBusType bus_type;
    gchar *entry_name;
    gchar *diag_name;
    gboolean (*is_no_tests)(struct diag_tool_tests *tool);
} diag_tool_tests;

static diag_tool_tests *services_module_service_diag_tool_tests_init(const gchar *path,
                                                                     GHashTable *all_tests,
                                                                     GHashTable *required_only_tests,
                                                                     GBusType bus_type);
static void services_module_service_diag_tool_tests_free(diag_tool_tests *tool_tests);
static gboolean services_module_service_diag_tool_tests_is_no_tests(struct diag_tool_tests *tool);
static GPtrArray *services_module_get_sorted_string_keys(GHashTable *table);

typedef struct
{
    gboolean enabled;
    GHashTable *tool_tests;
} ServicesPlayPlanDiag;

struct ServicesPlayPlan
{
    gchar *service_name;
    gint command_id;
    ServicesPlayPlanDiag prediag;
    ServicesPlayPlanDiag postdiag;
    gboolean has_autostart;
    gboolean autostart;
    gboolean has_force;
    gboolean force;
};

static void services_play_plan_diag_clear(ServicesPlayPlanDiag *diag);
static void services_play_plan_reset(AlteratorCtlServicesModule *module);
static ServicesPlayPlan *services_play_plan_get(AlteratorCtlServicesModule *module,
                                                gint command_id,
                                                const gchar *service_str_id);
static gboolean services_play_plan_requires_diag(const ServicesPlayPlan *plan);
static int services_module_play_enable_force_deploy(AlteratorCtlServicesModule *module, const gchar *service_name);
static gchar *services_module_json_params_to_text(JsonNode *params, const gchar *service_str_id);
static int services_module_get_service_tests(AlteratorCtlServicesModule *module,
                                             const gchar *service_str_id,
                                             GNode *optional_service_info,
                                             const GPtrArray *diag_env_vars,
                                             gboolean all_modes,
                                             GHashTable **result);

static int services_module_is_deployed_from_status_ctx(AlteratorCtlServicesModule *module,
                                                       alteratorctl_ctx_t **status_ctx,
                                                       const gchar *service_str_id,
                                                       deployment_status *result);
static int services_module_run_test(AlteratorCtlServicesModule *module,
                                    const gchar *service_str_id,
                                    const gchar *diag_tool_name,
                                    const gchar *test_name,
                                    deployment_status deploy_mode,
                                    GBusType bus_type);
static gpointer g_strdup_copy_func(gconstpointer src, gpointer data);

static gint services_module_sort_result(gconstpointer a, gconstpointer b);

typedef struct evn_variable_t
{
    gchar *name;
    gchar *value;
} evn_variable_t;
static gchar *services_module_get_service_path(AlteratorCtlServicesModule *module, const gchar *service_str_id)
{
    if (!service_str_id)
        return NULL;

    if (service_str_id[0] == '/')
        return g_strdup(service_str_id);

    return g_strdup(module->gdbus_source->alterator_gdbus_source_get_path_by_name(module->gdbus_source,
                                                                                  service_str_id,
                                                                                  SERVICES_INTERFACE_NAME));
}

static void services_play_plan_diag_clear(ServicesPlayPlanDiag *diag)
{
    if (!diag)
        return;

    diag->enabled = FALSE;
    if (diag->tool_tests)
    {
        g_hash_table_destroy(diag->tool_tests);
        diag->tool_tests = NULL;
    }
}

static void services_play_plan_free(ServicesPlayPlan *plan)
{
    if (!plan)
        return;

    g_free(plan->service_name);
    services_play_plan_diag_clear(&plan->prediag);
    services_play_plan_diag_clear(&plan->postdiag);
    g_free(plan);
}

static void services_play_plan_reset(AlteratorCtlServicesModule *module)
{
    if (!module)
        return;

    if (module->play_plan)
    {
        services_play_plan_free(module->play_plan);
        module->play_plan = NULL;
    }
}

static ServicesPlayPlan *services_play_plan_get(AlteratorCtlServicesModule *module,
                                                gint command_id,
                                                const gchar *service_str_id)
{
    if (!module || !module->play_plan || !service_str_id)
        return NULL;

    if (module->play_plan->command_id != command_id)
        return NULL;

    if (g_strcmp0(module->play_plan->service_name, service_str_id) != 0)
        return NULL;

    return module->play_plan;
}

static gboolean services_play_plan_requires_diag(const ServicesPlayPlan *plan)
{
    return plan && (plan->prediag.enabled || plan->postdiag.enabled);
}

static int services_module_play_enable_force_deploy(AlteratorCtlServicesModule *module, const gchar *service_name)
{
    int ret                       = 0;
    JsonObject *params_object     = NULL;
    alterator_entry_node *entry   = NULL;
    toml_value *force_deploy_flag = NULL;

    if (!module || !service_name)
        ERR_EXIT();

    if (!module->json || json_node_get_node_type(module->json) != JSON_NODE_OBJECT)
    {
        g_printerr(_("Invalid playfile parameters for service \"%s\".\n"), service_name);
        ERR_EXIT();
    }

    params_object = json_node_get_object(module->json);
    if (!params_object)
    {
        g_printerr(_("Invalid playfile parameters for service \"%s\".\n"), service_name);
        ERR_EXIT();
    }

    if (!module->info || !module->info->data)
    {
        g_printerr(_("Failed to parse info for service \"%s\".\n"), service_name);
        ERR_EXIT();
    }

    entry             = (alterator_entry_node *) module->info->data;
    force_deploy_flag = g_hash_table_lookup(entry->toml_pairs,
                                            SERVICES_ALTERATOR_ENTRY_SERVICE_ENABLE_FORCE_DEPLOY_KEY_NAME);
    if (!force_deploy_flag || force_deploy_flag->type != TOML_DATA_BOOL || force_deploy_flag->bool_value != TRUE)
    {
        g_printerr(_("Force-deploy is not available for service \"%s\".\n"), service_name);
        ERR_EXIT();
    }

    json_object_set_boolean_member(params_object,
                                   SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_FORCE_DEPLOY_KEY_NAME,
                                   TRUE);

end:
    return ret;
}
static gboolean services_module_play_parse_diag_section(JsonObject *options_obj,
                                                        const gchar *section_name,
                                                        ServicesPlayPlanDiag *diag)
{
    if (!diag)
        return FALSE;

    services_play_plan_diag_clear(diag);

    if (!options_obj)
        return FALSE;

    JsonNode *section_node = json_object_get_member(options_obj, section_name);
    if (!section_node || json_node_get_node_type(section_node) != JSON_NODE_OBJECT)
        return FALSE;

    JsonObject *section_obj = json_node_get_object(section_node);
    JsonNode *enable_node   = json_object_get_member(section_obj, "enable");
    if (!enable_node || json_node_get_value_type(enable_node) != G_TYPE_BOOLEAN)
        return FALSE;

    diag->enabled = json_node_get_boolean(enable_node);
    if (!diag->enabled)
        return FALSE;

    JsonNode *tests_node = json_object_get_member(section_obj, "options");
    if (!tests_node || json_node_get_node_type(tests_node) != JSON_NODE_OBJECT)
        return TRUE;

    JsonObject *tests_obj = json_node_get_object(tests_node);
    if (!tests_obj)
        return TRUE;

    diag->tool_tests = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_ptr_array_unref);

    JsonObjectIter iter;
    const gchar *tool_name = NULL;
    JsonNode *tests_value  = NULL;
    json_object_iter_init(&iter, tests_obj);
    while (json_object_iter_next(&iter, &tool_name, &tests_value))
    {
        if (!tool_name || !tests_value || json_node_get_node_type(tests_value) != JSON_NODE_ARRAY)
        {
            g_printerr(_("Invalid diagnostics tests list for \"%s\".\n"), section_name);
            continue;
        }

        JsonArray *tests_array = json_node_get_array(tests_value);
        if (!tests_array)
            continue;

        GPtrArray *tests_ptr_array = g_ptr_array_new_with_free_func(g_free);
        for (guint i = 0; i < json_array_get_length(tests_array); i++)
        {
            JsonNode *test_node = json_array_get_element(tests_array, i);
            if (!test_node || json_node_get_value_type(test_node) != G_TYPE_STRING)
                continue;
            const gchar *test_name = json_node_get_string(test_node);
            if (!test_name || !strlen(test_name))
                continue;
            g_ptr_array_add(tests_ptr_array, g_strdup(test_name));
        }

        g_hash_table_insert(diag->tool_tests, g_strdup(tool_name), tests_ptr_array);
    }

    return TRUE;
}

static int services_module_play_prepare_plan(AlteratorCtlServicesModule *module,
                                             const gchar *service_name,
                                             gint command_id,
                                             JsonObject *options_obj)
{
    int ret                 = 0;
    gboolean has_plan_state = FALSE;
    ServicesPlayPlan *plan  = NULL;

    services_play_plan_reset(module);

    if (!service_name || !options_obj)
        goto end;

    plan               = g_new0(ServicesPlayPlan, 1);
    plan->service_name = g_strdup(service_name);
    plan->command_id   = command_id;

    has_plan_state |= services_module_play_parse_diag_section(options_obj, "prediag", &plan->prediag);
    has_plan_state |= services_module_play_parse_diag_section(options_obj, "postdiag", &plan->postdiag);

    if (command_id == SERVICES_DEPLOY)
    {
        JsonNode *autostart_node = json_object_get_member(options_obj, "autostart");
        if (autostart_node)
        {
            if (json_node_get_value_type(autostart_node) != G_TYPE_BOOLEAN)
            {
                g_printerr(_("Invalid input json data. Invalid 'options.autostart'.\n"));
                ERR_EXIT();
            }

            plan->has_autostart = TRUE;
            plan->autostart     = json_node_get_boolean(autostart_node);
            has_plan_state      = TRUE;
        }

        JsonNode *force_node = json_object_get_member(options_obj, "force");
        if (force_node)
        {
            if (json_node_get_value_type(force_node) != G_TYPE_BOOLEAN)
            {
                g_printerr(_("Invalid input json data. Invalid 'options.force'.\n"));
                ERR_EXIT();
            }

            plan->has_force = TRUE;
            plan->force     = json_node_get_boolean(force_node);
            has_plan_state  = TRUE;

            if (plan->force)
            {
                if (services_module_play_enable_force_deploy(module, service_name) < 0)
                    ERR_EXIT();
                services_play_plan_diag_clear(&plan->prediag);
            }
        }
    }

end:
    if (plan)
    {
        if (!ret && has_plan_state)
            module->play_plan = plan;
        else
            services_play_plan_free(plan);
    }

    return ret;
}

static int services_module_play_set_parameter_env(AlteratorCtlServicesModule *module, const gchar *service_str_id)
{
    int ret = 0;

    if (!module || !module->json || json_node_get_node_type(module->json) != JSON_NODE_OBJECT)
        goto end;

    JsonObject *params_object = json_node_get_object(module->json);
    if (!params_object)
        goto end;

    JsonObjectIter iter;
    const gchar *param_name = NULL;
    JsonNode *param_node    = NULL;
    json_object_iter_init(&iter, params_object);
    while (json_object_iter_next(&iter, &param_name, &param_node))
    {
        gchar *param_value = services_module_json_params_to_text(param_node, service_str_id);
        if (!param_value)
            ERR_EXIT();

        if (module->gdbus_source->alterator_gdbus_source_set_env_value(module->gdbus_source, param_name, param_value)
            < 0)
        {
            g_free(param_value);
            ERR_EXIT();
        }
        g_free(param_value);
    }

end:
    return ret;
}

static gboolean yes;

static int services_module_play_run_diag(AlteratorCtlServicesModule *module,
                                         const gchar *service_str_id,
                                         ServicesPlayPlanDiag *diag,
                                         deployment_status status,
                                         gboolean confirm_on_fail,
                                         const gchar *phase_label)
{
    int ret = 0;

    if (!module || !diag || !diag->enabled)
        goto end;

    if (!phase_label)
        phase_label = _("playfile");

    g_print(_("Running %s diagnostics requested in playfile for service \"%s\".\n"), phase_label, service_str_id);

    gchar *service_path = services_module_get_service_path(module, service_str_id);
    if (!service_path || !strlen(service_path))
    {
        g_printerr(_("Failed to determine service path for diagnostics.\n"));
        ERR_EXIT();
    }

    GPtrArray *diag_env_vars        = g_ptr_array_new();
    evn_variable_t deploy_mode_env  = {.name  = SERVICES_DIAGNOSE_TESTS_ENV_VALUE_DEPLOY_MODE,
                                       .value = status != UNDEPLOYED ? "post" : "pre"};
    evn_variable_t service_path_env = {.name = SERVICES_DIAGNOSE_TESTS_ENV_VALUE_SERVICE_PATH, .value = service_path};
    g_ptr_array_add(diag_env_vars, &deploy_mode_env);
    g_ptr_array_add(diag_env_vars, &service_path_env);

    GHashTable *service_tests = NULL;
    if (services_module_get_service_tests(module, service_str_id, NULL, diag_env_vars, FALSE, &service_tests) < 0)
        ERR_EXIT();

    if (services_module_play_set_parameter_env(module, service_str_id) < 0)
        ERR_EXIT();

    GHashTable *selected_tests = g_hash_table_new_full(g_str_hash,
                                                       g_str_equal,
                                                       (GDestroyNotify) g_free,
                                                       (GDestroyNotify) services_module_service_diag_tool_tests_free);

    /*
     * Build an alias map from playfile tool keys to canonical internal tool names.
     * Canonical keys are service diag_tools entry names (hash keys in service_tests).
     * Aliases include:
     *   - the entry name itself (e.g. "tool1"), and
     *   - the diag tool logical name from .diag (e.g. "test_service1"), when available.
     */
    GHashTable *tool_aliases = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);
    if (service_tests && g_hash_table_size(service_tests))
    {
        GHashTableIter alias_iter;
        gpointer alias_key   = NULL;
        gpointer alias_value = NULL;
        g_hash_table_iter_init(&alias_iter, service_tests);
        while (g_hash_table_iter_next(&alias_iter, &alias_key, &alias_value))
        {
            const gchar *entry_name   = (const gchar *) alias_key;
            diag_tool_tests *tool     = (diag_tool_tests *) alias_value;
            const gchar *canonical_id = entry_name;

            if (canonical_id && strlen(canonical_id))
                g_hash_table_insert(tool_aliases, g_strdup(canonical_id), (gpointer) canonical_id);

            if (tool && tool->diag_name && strlen(tool->diag_name)
                && !g_hash_table_contains(tool_aliases, tool->diag_name))
                g_hash_table_insert(tool_aliases, g_strdup(tool->diag_name), (gpointer) canonical_id);
        }
    }

    if (diag->tool_tests && g_hash_table_size(diag->tool_tests))
    {
        GHashTableIter iter;
        gpointer tool_key = NULL;
        gpointer tests    = NULL;
        g_hash_table_iter_init(&iter, diag->tool_tests);
        while (g_hash_table_iter_next(&iter, &tool_key, &tests))
        {
            gchar *canonical_name       = NULL;
            const gchar *requested_tool = (const gchar *) tool_key;

            if (tool_aliases)
                canonical_name = (gchar *) g_hash_table_lookup(tool_aliases, requested_tool);

            if (!canonical_name)
            {
                g_printerr(_("Unknown diagnostic tool \"%s\" for service \"%s\".\n"), requested_tool, service_str_id);
                continue;
            }

            diag_tool_tests *source = g_hash_table_lookup(service_tests, canonical_name);
            if (!source)
            {
                g_printerr(_("Unknown diagnostic tool \"%s\" for service \"%s\".\n"), requested_tool, service_str_id);
                continue;
            }

            GHashTable *selected_all   = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);
            GPtrArray *requested_tests = (GPtrArray *) tests;
            if (requested_tests)
            {
                for (guint i = 0; i < requested_tests->len; i++)
                {
                    const gchar *test_name = requested_tests->pdata[i];
                    if (g_hash_table_contains(source->all_tests, test_name))
                        g_hash_table_add(selected_all, g_strdup(test_name));
                    else
                        g_printerr(_("Unknown diagnostic test \"%s\" in \"%s\" tool.\n"), test_name, requested_tool);
                }
            }

            GHashTable *selected_required = g_hash_table_copy_table(source->required_tests,
                                                                    g_strdup_copy_func,
                                                                    NULL,
                                                                    NULL,
                                                                    NULL);
            diag_tool_tests *clone        = services_module_service_diag_tool_tests_init(source->path,
                                                                                  selected_all,
                                                                                  selected_required,
                                                                                  source->bus_type);
            g_hash_table_insert(selected_tests, g_strdup(canonical_name), clone);
            g_hash_table_unref(selected_all);
            g_hash_table_unref(selected_required);
        }
    }
    else
    {
        GHashTableIter iter;
        gpointer tool_name = NULL;
        gpointer info_ptr  = NULL;
        g_hash_table_iter_init(&iter, service_tests);
        while (g_hash_table_iter_next(&iter, &tool_name, &info_ptr))
        {
            diag_tool_tests *source  = (diag_tool_tests *) info_ptr;
            GHashTable *selected_all = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);
            GHashTable *selected_required = g_hash_table_copy_table(source->required_tests,
                                                                    g_strdup_copy_func,
                                                                    NULL,
                                                                    NULL,
                                                                    NULL);
            diag_tool_tests *clone        = services_module_service_diag_tool_tests_init(source->path,
                                                                                  selected_all,
                                                                                  selected_required,
                                                                                  source->bus_type);
            g_hash_table_insert(selected_tests, g_strdup(tool_name), clone);
            g_hash_table_unref(selected_all);
            g_hash_table_unref(selected_required);
        }
    }

    GHashTableIter required_iter;
    gpointer req_tool_name = NULL;
    gpointer req_tool_ptr  = NULL;
    g_hash_table_iter_init(&required_iter, service_tests);
    while (g_hash_table_iter_next(&required_iter, &req_tool_name, &req_tool_ptr))
    {
        diag_tool_tests *source = (diag_tool_tests *) req_tool_ptr;
        if (!source->required_tests || !g_hash_table_size(source->required_tests))
            continue;

        diag_tool_tests *target = g_hash_table_lookup(selected_tests, req_tool_name);
        if (!target)
        {
            GHashTable *selected_all = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);
            GHashTable *selected_required = g_hash_table_copy_table(source->required_tests,
                                                                    g_strdup_copy_func,
                                                                    NULL,
                                                                    NULL,
                                                                    NULL);
            target                        = services_module_service_diag_tool_tests_init(source->path,
                                                                  selected_all,
                                                                  selected_required,
                                                                  source->bus_type);
            g_hash_table_insert(selected_tests, g_strdup(req_tool_name), target);
            g_hash_table_unref(selected_all);
            g_hash_table_unref(selected_required);
        }

        GPtrArray *required_keys = g_hash_table_get_keys_as_ptr_array(source->required_tests);
        gboolean warned          = FALSE;
        for (gsize i = 0; i < required_keys->len; i++)
        {
            const gchar *test = required_keys->pdata[i];
            if (!g_hash_table_contains(target->all_tests, test))
            {
                if (!warned)
                {
                    g_print(
                        _("Playfile did not list all required tests for \"%s\"; they will be added automatically.\n"),
                        (gchar *) req_tool_name);
                    warned = TRUE;
                }
                g_hash_table_add(target->all_tests, g_strdup(test));
            }
        }
        g_ptr_array_unref(required_keys);
    }

    gboolean has_failures = FALSE;
    GHashTableIter run_iter;
    gpointer run_tool_name = NULL;
    gpointer run_tool_ptr  = NULL;
    g_hash_table_iter_init(&run_iter, selected_tests);
    while (g_hash_table_iter_next(&run_iter, &run_tool_name, &run_tool_ptr))
    {
        const gchar *tool_name     = (const gchar *) run_tool_name;
        diag_tool_tests *selection = (diag_tool_tests *) run_tool_ptr;
        diag_tool_tests *source    = g_hash_table_lookup(service_tests, tool_name);

        if (!selection || !source)
            continue;

        GPtrArray *tests_to_run = NULL;
        if (selection->all_tests && g_hash_table_size(selection->all_tests))
            tests_to_run = services_module_get_sorted_string_keys(selection->all_tests);
        else if (source->required_tests && g_hash_table_size(source->required_tests))
            tests_to_run = services_module_get_sorted_string_keys(source->required_tests);
        else if (source->all_tests)
            tests_to_run = services_module_get_sorted_string_keys(source->all_tests);

        if (!tests_to_run)
            continue;

        g_ptr_array_sort(tests_to_run, services_module_sort_result);

        if (selection->bus_type == G_BUS_TYPE_SYSTEM)
            g_print("%s", _("<<< Tests on the system bus:\n"));
        else if (selection->bus_type == G_BUS_TYPE_SESSION)
            g_print("%s", _("<<< Tests on the session bus:\n"));
        for (gsize i = 0; i < tests_to_run->len; i++)
        {
            const gchar *test_name = (const gchar *) tests_to_run->pdata[i];
            if (!test_name)
                continue;

            g_print(_("Running test \"%s\" from \"%s\".\n"), test_name, tool_name);
            int test_result = services_module_run_test(module,
                                                       service_str_id,
                                                       selection->path ? selection->path : tool_name,
                                                       test_name,
                                                       status,
                                                       selection->bus_type);
            if (test_result < 0)
            {
                g_ptr_array_unref(tests_to_run);
                ERR_EXIT();
            }
            if (test_result > 0)
                has_failures = TRUE;
        }
        g_ptr_array_unref(tests_to_run);
    }

    if (has_failures)
    {
        g_print(_("Some %s diagnostics failed for service \"%s\".\n"), phase_label, service_str_id);

        if (confirm_on_fail)
        {
            if (yes)
            {
                g_print(_("Do you want to continue? [y/N] y\n"));
            }
            else if (isatty_safe(STDIN_FILENO) && isatty_safe(STDOUT_FILENO))
            {
                g_print(_("Do you want to continue? [y/N] "));
                fflush(stdout);

                gchar response_char = 'N';
                if (scanf("%c", &response_char) != 1)
                    response_char = 'N';
                else if (response_char == '\n')
                    response_char = 'N';
                else
                {
                    int flush;
                    while ((flush = getchar()) != '\n' && flush != EOF)
                        ;
                }
                g_print("\n");

                if (response_char != 'y' && response_char != 'Y')
                {
                    g_print(_("Aborted.\n"));
                    ERR_EXIT();
                }
            }
            else
            {
                g_print(_("To confirm the operation, repeat the input with the --yes flag.\n"));
                ERR_EXIT();
            }
        }
        // If confirm_on_fail is FALSE (postdiag), do not prompt or abort; just continue.
    }

end:
    if (diag_env_vars)
        g_ptr_array_unref(diag_env_vars);

    if (service_tests)
        g_hash_table_destroy(service_tests);

    if (selected_tests)
        g_hash_table_destroy(selected_tests);

    if (tool_aliases)
        g_hash_table_destroy(tool_aliases);

    g_free(service_path);

    return ret;
}

typedef struct services_module_subcommands_t
{
    char *subcommand;
    enum services_sub_commands id;
} services_module_subcommands_t;

static services_module_subcommands_t services_module_subcommands_list[] = {{"backup", SERVICES_BACKUP},
                                                                           {"configure", SERVICES_CONFIGURE},
                                                                           {"deploy", SERVICES_DEPLOY},
                                                                           {"diagnose", SERVICES_DIAGNOSE},
                                                                           {"info", SERVICES_INFO},
                                                                           {"resources", SERVICES_RESOURCES},
                                                                           {"restore", SERVICES_RESTORE},
                                                                           {"status", SERVICES_STATUS},
                                                                           {"start", SERVICES_START},
                                                                           {"stop", SERVICES_STOP},
                                                                           {"undeploy", SERVICES_UNDEPLOY},
                                                                           {"list", SERVICES_LIST},
                                                                           {"play", SERVICES_PLAY}};

typedef struct service_names_t
{
    gchar *name;
    gchar *display_name;
} service_names_t;

static service_names_t *service_names_init(const gchar *name, const gchar *display_name);
static void service_names_free(service_names_t *service_names);

typedef struct service_display_entry
{
    gchar *label;
    deployment_status status;
} service_display_entry;

static service_display_entry *service_display_entry_new(gchar *label, deployment_status status);
static void service_display_entry_free(service_display_entry *entry);
static gint services_module_sort_entries(gconstpointer a, gconstpointer b);
static gchar *services_module_build_service_label(AlteratorCtlServicesModule *module,
                                                  const gchar *service_path,
                                                  const service_names_t *service_names);
static const gchar *services_module_status_text(deployment_status status);
static const gchar *services_module_marker_shape_for_status(deployment_status status);
static text_color services_module_color_for_status(deployment_status status);
static gchar *services_module_format_entry(const service_display_entry *entry,
                                           gsize max_label_len,
                                           gboolean show_statuses);
static void services_module_print_status_legend(void);

static GObjectClass *services_module_parent_class = NULL;
static alterator_ctl_module_t services_module     = {0};
gboolean is_dbus_call_error                       = FALSE;

static gboolean lauch_after_deploy    = FALSE;
static gboolean except_status_markers = FALSE;
static gboolean name_only             = FALSE;
static gboolean path_only             = FALSE;
static gboolean display_name_only     = FALSE;
static gboolean no_display_name       = FALSE;
static gboolean no_name               = FALSE;
static gboolean hide_markers_legend   = FALSE;
static gboolean json_status           = FALSE;
static gboolean force_deploy          = FALSE;
static gboolean run_all_tests         = FALSE;
static gboolean no_default            = FALSE;
static gboolean yes                   = FALSE;
static gchar *json_params_filepath    = NULL;
static GPtrArray *diagnostic_tests    = NULL;

// Parsed diagnostics argumnets
static GHashTable *service_tests = NULL;

static void services_module_class_init(AlteratorCtlServicesModuleClass *klass);
static void services_ctl_class_finalize(GObject *klass);

static void services_module_alterator_interface_init(gpointer iface, gpointer iface_data);
static void services_module_alterator_interface_finalize(gpointer iface, gpointer iface_data);

AlteratorCtlServicesModule *services_module_new(gpointer app);
void services_module_free(AlteratorCtlServicesModule *module);

static void fill_command_hash_table(GHashTable *command);

typedef struct
{
    gchar *default_value_str;
    gsize min_array_size; // default 0
    gsize max_array_size; // default 0
    GOptionEntry option_entry;
    gboolean is_constant;
    gboolean is_array;
} ParamEntry;
static void services_module_parameter_entry_clear(ParamEntry *e);

static int services_module_parse_options(AlteratorCtlServicesModule *module, int *argc, char **argv);
static gchar *services_module_get_parameters(gboolean disable_fallback);
static gchar *services_module_parse_stdin_param(gboolean disable_fallback);

static int services_module_parse_diagnose_arguments(AlteratorCtlServicesModule *module,
                                                    int argc,
                                                    char **argv,
                                                    gchar **params);
static int services_module_parse_arguments(
    AlteratorCtlServicesModule *module, int argc, char **argv, alteratorctl_ctx_t **ctx, gboolean *is_accepted);
static int services_module_status_print_params(JsonObject *parameters, GString *prefix);

static int services_module_parametrized_subcommand(AlteratorCtlServicesModule *module,
                                                   alteratorctl_ctx_t **ctx,
                                                   int command,
                                                   gboolean with_signals,
                                                   int required_deploy_state);
static int services_module_diagnose_subcommand(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx);
static int services_module_info_subcommand(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx);
static int services_module_list_subcommand(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx);
static int services_module_resources_subcommand(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx);
static int services_module_status_subcommand(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx);
static int services_module_start_subcommand(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx);
static int services_module_stop_subcommand(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx);

static int services_module_confirm_execution(AlteratorCtlServicesModule *module,
                                             alteratorctl_ctx_t **ctx,
                                             gint selected_subcommand,
                                             const gchar *command_name,
                                             const gchar *service_name,
                                             gchar **argv,
                                             gint actual_argc,
                                             GArray *options,
                                             gboolean *is_accepted,
                                             gboolean *yes_flag);

static int services_module_handle_results(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx);
static int services_module_backup_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx);
static int services_module_configure_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx);
static int services_module_deploy_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx);
static int services_module_info_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx);
static int services_module_resources_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx);
static int services_module_restore_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx);
static int services_module_status_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx);
static int services_module_start_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx);
static int services_module_stop_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx);
static int services_module_undeploy_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx);

static int services_module_status_get_json_node(AlteratorCtlServicesModule *module,
                                                const gchar *service_str_id,
                                                JsonNode **result);

static int services_module_status_params_build_output(AlteratorCtlServicesModule *module,
                                                      gchar *service_str_id,
                                                      JsonObject *parameters,
                                                      GNode *parameters_data,
                                                      GString *prefix,
                                                      GHashTable **result);

// Minimal secrets prefill: top-level required password strings and required
// password children of present object/selected enum variant.
static int services_fill_missing_passwords(AlteratorCtlServicesModule *module,
                                           const gchar *service_name,
                                           const gchar *const *contexts,
                                           gsize contexts_len);

static int services_module_get_required_parameters_paths(AlteratorCtlServicesModule *module,
                                                         const gchar *service_str_id,
                                                         const gchar *context,
                                                         GHashTable **result);

static int services_module_create_params_table(GHashTable *stringified_params_data,
                                               GHashTable *param_name_to_option_entry,
                                               GHashTable *required_params_paths,
                                               gboolean *is_missing_required,
                                               GString **result);

static void services_module_run_stdout_simple_stream_handler(GDBusConnection *connection,
                                                             const gchar *sender_name,
                                                             const gchar *object_path,
                                                             const gchar *interface_name,
                                                             const gchar *signal_name,
                                                             GVariant *parameters,
                                                             gpointer user_data);

static void services_module_run_stderr_simple_stream_handler(GDBusConnection *connection,
                                                             const gchar *sender_name,
                                                             const gchar *object_path,
                                                             const gchar *interface_name,
                                                             const gchar *signal_name,
                                                             GVariant *parameters,
                                                             gpointer user_data);

static int services_module_print_list_with_filters(AlteratorCtlServicesModule *module, GHashTable *services);
static int services_module_validate_object_and_iface(AlteratorCtlServicesModule *module,
                                                     const gchar *object,
                                                     const gchar *iface);
static gint services_module_sort_options(gconstpointer a, gconstpointer b);
static int services_module_get_display_name(AlteratorCtlServicesModule *module,
                                            const gchar *elem_str_id,
                                            GNode *data,
                                            gchar **result);

static int services_module_status_result_get_service_params(AlteratorCtlServicesModule *module,
                                                            alteratorctl_ctx_t **ctx,
                                                            const gchar *service_str_id,
                                                            gboolean initial_params,
                                                            gchar *filtering_ctx,
                                                            JsonNode **result,
                                                            deployment_status *deploy_status,
                                                            gboolean exclude_unset_params);
typedef gchar *(*ServicesModuleTomlValuePrintingMode)(gchar *value_name, gpointer user_data);

static gchar *services_module_print_resource_with_links_to_params(gchar *resource_name,
                                                                  gpointer resource_to_param_table);

static int services_module_printing_resources_value(AlteratorCtlServicesModule *module,
                                                    const gchar *service_str_id,
                                                    const GNode *resources,
                                                    ServicesModuleTomlValuePrintingMode mode,
                                                    gpointer user_data);

static gchar *services_module_get_localized_field(AlteratorCtlModuleInfoParser *info_parser,
                                                  GNode *node,
                                                  const gchar *table_name);
static gchar *services_module_resource_value_to_string(const toml_value *value);
static services_module_resource_group_t services_module_get_resource_group(const gchar *type_str);
static GString *services_module_render_resources_ascii_table(const gchar *left_header,
                                                             const gchar *right_header,
                                                             const GPtrArray *rows);
static void services_module_resource_row_free(gpointer data);
static void services_module_append_ascii_border(GString *buffer, gsize left_width, gsize right_width, gchar fill);
static void services_module_append_ascii_row(
    GString *buffer, gsize left_width, gsize right_width, const gchar *left, const gchar *right);
static void services_module_append_spaces(GString *buffer, gsize count);

static int services_module_get_initial_params(AlteratorCtlServicesModule *module,
                                              const gchar *service_str_id,
                                              JsonNode **result,
                                              gboolean required_only,
                                              gchar *required_only_context,
                                              GHashTable **params_ctx,
                                              gboolean exclude_unset_params);

static int services_module_recursive_get_initial_params(AlteratorCtlServicesModule *module,
                                                        const gchar *service_str_id,
                                                        GNode *service_info,
                                                        GNode *types,
                                                        GNode *parameter,
                                                        JsonNode **result,
                                                        gboolean required_only,
                                                        gchar *required_only_context,
                                                        JsonObject **nested_object,
                                                        GHashTable *params_to_resources,
                                                        gboolean exclude_unset_params,
                                                        gboolean constant_parent);

static int services_module_get_params_context(AlteratorCtlServicesModule *module,
                                              const gchar *service_str_id,
                                              GNode *info,
                                              GHashTable **params_ctx);

static int services_module_get_param_to_resource_table(AlteratorCtlServicesModule *module,
                                                       GNode *resources,
                                                       GHashTable **result);

static JsonNode *services_module_merge_json_data(JsonNode *first, JsonNode *second, const gchar *service_str_id);

static int services_module_recursive_merge_json_data(JsonNode *target, JsonNode *source, const gchar *service_str_id);

static int services_module_recursive_filter_params_by_context(JsonNode *data,
                                                              GHashTable *ctx_data,
                                                              gchar *context_name,
                                                              gboolean constants_only);

static JsonNode *services_module_filter_params_by_context(JsonNode *data,
                                                          GHashTable *ctx_data,
                                                          gchar *context_name,
                                                          gboolean constants_only);

static int service_module_get_list_of_deployed_services_with_params(AlteratorCtlServicesModule *module,
                                                                    GList **result,
                                                                    const gchar *skip_service_name);

static int services_module_is_deployed(AlteratorCtlServicesModule *module,
                                       const gchar *service_str_id,
                                       deployment_status *result);

static int services_module_get_diag_tests(AlteratorCtlServicesModule *module,
                                          const gchar *service_str_id,
                                          const gchar *diag_tool_path,
                                          const gchar *diag_tool_table_name,
                                          GBusType bus_type,
                                          const GPtrArray *diag_env_vars,
                                          gboolean all_modes,
                                          GNode *diag_tests_info,
                                          GHashTable **result);

static int services_module_get_diag_tools_info(AlteratorCtlServicesModule *module,
                                               const gchar *service_str_id,
                                               GHashTable *diag_tools_names,
                                               GHashTable **tools_names_to_nodes);

// ret 1 if failed of getting info
static int services_module_get_diag_info(AlteratorCtlServicesModule *module,
                                         const gchar *service_str_id,
                                         const gchar *service_diag_tool_entry_name,
                                         const gchar *diag_tool_str_id,
                                         GBusType bus_type,
                                         GNode **result);

// retval 1 if a resource of non-existent
static int services_module_get_actual_resourse_value(AlteratorCtlServicesModule *module,
                                                     const gchar *service_str_id,
                                                     GNode *resources,
                                                     JsonNode *parameters,
                                                     const gchar *resource_name,
                                                     toml_value **result);

static gchar *services_module_strignify_toml_value(const toml_value *value);

static int services_module_find_conflicting_resources_within_current_service(AlteratorCtlServicesModule *module,
                                                                             const gchar *service_str_id,
                                                                             JsonNode *parameters_node);
static int services_module_find_conflicting_services(AlteratorCtlServicesModule *module, alteratorctl_ctx_t *ctx);
static int services_module_check_resources_conflicts(AlteratorCtlServicesModule *module,
                                                     const gchar *current_service_str_id,
                                                     const GVariant *deployed_service_data,
                                                     GNode *resources,
                                                     JsonNode *parameters);

static int services_module_validate_json_params(AlteratorCtlServicesModule *module,
                                                JsonNode *params,
                                                const gchar *service_str_id,
                                                GNode *optional_service_info);

GType alterator_ctl_services_module_get_type(void)
{
    static GType services_module_type = 0;

    if (!services_module_type)
    {
        static const GTypeInfo services_module_info
            = {sizeof(AlteratorCtlServicesModuleClass),     /* class structure size */
               NULL,                                        /* base class initializer */
               NULL,                                        /* base class finalizer */
               (GClassInitFunc) services_module_class_init, /* class initializer */
               NULL,                                        /* class finalizer */
               NULL,                                        /* class data */
               sizeof(AlteratorCtlServicesModule),          /* instance structure size */
               1,                                           /* preallocated instances */
               NULL,                                        /* instance initializers */
               NULL};

        const GInterfaceInfo alterator_module_interface_info = {
            (GInterfaceInitFunc) services_module_alterator_interface_init,         /* interface_init */
            (GInterfaceFinalizeFunc) services_module_alterator_interface_finalize, /* interface_finalize */
            NULL                                                                   /* interface_data */
        };

        services_module_type = g_type_register_static(G_TYPE_OBJECT, /* parent class */
                                                      "AlteratorCtlServicesModule",
                                                      &services_module_info,
                                                      0);

        g_type_add_interface_static(services_module_type, TYPE_ALTERATOR_CTL_MODULE, &alterator_module_interface_info);
    }

    return services_module_type;
}

static void services_module_class_init(AlteratorCtlServicesModuleClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->finalize = services_ctl_class_finalize;

    services_module_parent_class = g_type_class_peek_parent(klass);
}

static void services_ctl_class_finalize(GObject *klass)
{
    AlteratorCtlServicesModuleClass *obj = (AlteratorCtlServicesModuleClass *) klass;

    G_OBJECT_CLASS(services_module_parent_class)->finalize(klass);
}

static void services_module_alterator_interface_init(gpointer iface, gpointer iface_data)
{
    AlteratorCtlModuleInterface *interface = iface;

    interface->run_with_args = services_module_run_with_args;

    interface->run = services_module_run;

    interface->print_help = services_module_print_help;
}

static void services_module_alterator_interface_finalize(gpointer iface, gpointer iface_data) {}

alterator_ctl_module_t *get_services_module()
{
    static gsize services_ctl_module_init = 0;
    if (g_once_init_enter(&services_ctl_module_init))
    {
        gsize module_id_size = g_strlcpy(services_module.id,
                                         ALTERATOR_CTL_SERVICES_MODULE_NAME,
                                         strlen(ALTERATOR_CTL_SERVICES_MODULE_NAME) + 1);

        if (module_id_size != strlen(ALTERATOR_CTL_SERVICES_MODULE_NAME))
        {
            g_printerr(_("Internal error in get_services_module: unvaliable id of services module.\n"));
            goto end;
        }

        services_module.new_object_func  = (gpointer) services_module_new;
        services_module.free_object_func = (gpointer) services_module_free;

        gsize tmp = 42;

        g_once_init_leave(&services_ctl_module_init, tmp);
    }

    return &services_module;

end:
    return NULL;
}

AlteratorCtlServicesModule *services_module_new(gpointer app)
{
    AlteratorCtlServicesModule *object = g_object_new(TYPE_ALTERATOR_CTL_SERVICES_MODULE, NULL);

    object->commands = g_hash_table_new(g_str_hash, g_str_equal);
    fill_command_hash_table(object->commands);

    object->alterator_ctl_app = (AlteratorCtlApp *) app;

    object->gdbus_source = alterator_gdbus_source_new(object->alterator_ctl_app->arguments->verbose, G_BUS_TYPE_SYSTEM);

    object->info = NULL;

    object->json                  = NULL;
    object->play_plan             = NULL;
    object->required_params_cache = NULL;

    return object;
}

void services_module_free(AlteratorCtlServicesModule *module)
{
    g_free(json_params_filepath);

    if (diagnostic_tests)
        g_ptr_array_unref(diagnostic_tests);

    if (service_tests)
        g_hash_table_destroy(service_tests);

    g_hash_table_destroy(module->commands);

    if (module->gdbus_source)
    {
        alterator_gdbus_source_free(module->gdbus_source);
    }

    if (module->json)
        json_node_free(module->json);

    if (module->info)
        alterator_ctl_module_info_parser_result_tree_free(module->info);

    services_play_plan_reset(module);

    if (module->required_params_cache)
        g_hash_table_destroy(module->required_params_cache);

    g_object_unref(module);
}

static void fill_command_hash_table(GHashTable *command)
{
    for (int i = 0; i < sizeof(services_module_subcommands_list) / sizeof(services_module_subcommands_t); i++)
        g_hash_table_insert(command,
                            services_module_subcommands_list[i].subcommand,
                            &services_module_subcommands_list[i].id);
}

static service_names_t *service_names_init(const gchar *name, const gchar *display_name)
{
    service_names_t *result = g_new0(service_names_t, 1);
    result->name            = g_strdup(name);
    result->display_name    = g_strdup(display_name);
    return result;
}

static void service_names_free(service_names_t *service_names)
{
    if (!service_names)
        return;
    g_free(service_names->name);
    g_free(service_names->display_name);
    g_free(service_names);
}

int services_module_run_with_args(gpointer self, int argc, char **argv)
{
    int ret                            = 0;
    alteratorctl_ctx_t *ctx            = NULL;
    AlteratorCtlServicesModule *module = ALTERATOR_CTL_SERVICES_MODULE(self);
    gboolean is_accepted               = FALSE;

    if (!module)
    {
        g_printerr(
            _("Internal data error in services module with args: AlteratorCtlServicesModule *module is NULL.\n"));
        ERR_EXIT();
    }

    if (services_module_parse_arguments(module, argc, argv, &ctx, &is_accepted) < 0)
        ERR_EXIT();

    if (!is_accepted)
        goto end;

    if (services_module_run(module, ctx) < 0)
        ERR_EXIT();

    if (!is_dbus_call_error && services_module_handle_results(module, &ctx) < 0)
        ERR_EXIT();

end:
    services_play_plan_reset(module);
    if (ctx)
        alteratorctl_ctx_free(ctx);
    return ret;
}

static int services_module_parse_options(AlteratorCtlServicesModule *module, int *argc, char **argv)
{
    int ret                            = 0;
    GOptionContext *option_context     = NULL;
    AlteratorCtlModuleInterface *iface = GET_ALTERATOR_CTL_MODULE_INTERFACE((void *) module);

    // clang-format off
    static GOptionEntry services_module_options[]
        = {{"all", 'a', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &run_all_tests,
                                                    "Run all diag tests if they are not specified explicitly",
                                                    "ALL"},
           {"autostart", 's', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &lauch_after_deploy,
                                                    "Start service after deploying",
                                                    "AUTOLAUNCH"},
           {"name-only", 'n', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &name_only,
                                                    "List only services names",
                                                    "NAME_ONLY"},
           {"path-only", 'p', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &path_only,
                                                    "List only services dbus paths",
                                                    "PATH_ONLY"},
           {"display-name-only", 'd', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &display_name_only,
                                                    "List only services display names",
                                                    "DISPLAY_NAME_ONLY"},
           {"no-display-name", 'D', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &no_display_name,
                                                    "List services without display names",
                                                    "NO_DISPLAY_NAME"},
           {"except-status-markers", 'E', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &except_status_markers,
                                                    "List services without status markers", "NO_MARKERS"},
           {"hide-markers-legend", 'H', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &hide_markers_legend,
                                                    "Hide status markers legend", "NO_LEGEND"},
           {"no-name", 'N', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &no_name,
                                                    "List services without names", "NO_NAME"},
           {"force-deploy", 'f', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &force_deploy,
                                                    "Force deploy",
                                                    "FORCE_DEPLOY"},
           {"yes", 'y', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &yes,
                                                    "Assume \"yes\" to all queries",
                                                    NULL},
           {NULL}};

    // clang-format on

    GError *error  = NULL;
    option_context = g_option_context_new("Dervices module options");
    g_option_context_set_ignore_unknown_options(option_context, TRUE);
    g_option_context_add_main_entries(option_context, services_module_options, NULL);
    if (!g_option_context_parse(option_context, argc, &argv, &error))
    {
        g_printerr(_("Services module options parsing failed: %s.\n"), error->message);
        g_error_free(error);
        ERR_EXIT();
    }

    if ((*argc < 2))
    {
        g_printerr(_("Wrong arguments in services module.\n"));
        iface->print_help(module);
        ERR_EXIT();
    }

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
    else if (iface->get_command_id(module->commands, argv[2]) != SERVICES_DIAGNOSE
             && module->alterator_ctl_app->arguments->verbose && name_only)
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

    hide_markers_legend = except_status_markers ? FALSE : hide_markers_legend;

end:
    if (option_context)
        g_option_context_free(option_context);

    return ret;
}

static gchar *services_module_get_parameters(gboolean disable_fallback)
{
    gchar *result = NULL;
    if (json_params_filepath)
    {
        result = streq(json_params_filepath, "-") ? services_module_parse_stdin_param(disable_fallback)
                                                  : read_file(json_params_filepath);
    }

    return result;
}

static gchar *services_module_parse_stdin_param(gboolean disable_fallback)
{
    gchar *result             = NULL;
    GIOChannel *stdin_channel = g_io_channel_unix_new(STDIN_FILENO); // stdin
    gsize stdin_input_len     = 0;
    struct pollfd pfd         = {.fd = STDIN_FILENO, .events = POLLIN};
    int poll_result           = poll(&pfd, 1, 0); // non blocking
    if (poll_result == 1 && (pfd.revents & POLLIN))
    {
        g_io_channel_set_encoding(stdin_channel, "UTF-8", NULL); // UTF-8
        if ((g_io_channel_read_to_end(stdin_channel, &result, &stdin_input_len, NULL) == G_IO_STATUS_NORMAL))
        {
            if (stdin_channel)
                g_io_channel_unref(stdin_channel);

            if (!stdin_input_len)
                return disable_fallback ? NULL : g_strdup(EMPTY_JSON);
            return result;
        }
    }

    return disable_fallback ? NULL : g_strdup(EMPTY_JSON);
}

static gpointer g_strdup_copy_func(gconstpointer src, gpointer data)
{
    if (!src)
        return NULL;
    return g_strdup(src);
}

static int services_module_parse_diagnose_arguments(AlteratorCtlServicesModule *module,
                                                    int argc,
                                                    char **argv,
                                                    gchar **params)
{
    int ret               = 0;
    gchar *service_str_id = argv[3];
    gchar *service_path
        = service_str_id[0] != '/'
              ? g_strdup(module->gdbus_source->alterator_gdbus_source_get_path_by_name(module->gdbus_source,
                                                                                       service_str_id,
                                                                                       SERVICES_INTERFACE_NAME))
              : g_strdup(service_str_id);
    GHashTable *all_service_tests = NULL;
    GPtrArray *diag_env_vars      = g_ptr_array_new();

    deployment_status status = UNDEPLOYED;
    if (services_module_is_deployed(module, service_str_id, &status) < 0)
    {
        g_printerr(_("Error of parsing diagnose command arguments. Failed of getting \"%s\" service status.\n"),
                   service_str_id);
        ERR_EXIT();
    }

    // Set env variable for getting diag service test list
    evn_variable_t deploy_mode_env = {.name  = SERVICES_DIAGNOSE_TESTS_ENV_VALUE_DEPLOY_MODE,
                                      .value = status != UNDEPLOYED ? "post" : "pre"};
    g_ptr_array_add(diag_env_vars, &deploy_mode_env);

    evn_variable_t service_path_env = {.name = SERVICES_DIAGNOSE_TESTS_ENV_VALUE_SERVICE_PATH, .value = service_path};
    g_ptr_array_add(diag_env_vars, &service_path_env);

    // Get all test of specified service
    if (services_module_get_service_tests(module,
                                          service_str_id,
                                          NULL,
                                          diag_env_vars,
                                          run_all_tests || module->alterator_ctl_app->arguments->module_help,
                                          &all_service_tests)
        < 0)
        ERR_EXIT();

    service_tests = g_hash_table_new_full(g_str_hash,
                                          g_str_equal,
                                          (GDestroyNotify) g_free,
                                          (GDestroyNotify) services_module_service_diag_tool_tests_free);

    if (run_all_tests || module->alterator_ctl_app->arguments->module_help)
    {
        if (!g_hash_table_size(all_service_tests))
        {
            if (module->alterator_ctl_app->arguments->module_help)
                g_printerr(_("Can't create help for \"services diagnose %s\" with empty dianostic tools tests.\n"),
                           service_str_id);
            else
                g_printerr(_("service \"%s\" doesn't offer diagnostic tests.\n"), service_str_id);
            ERR_EXIT();
        }

        service_tests = all_service_tests;
        goto end;
    }

    if (!g_hash_table_size(all_service_tests) && argc > 4)
    {
        g_printerr(_("No diagnostic tests in \"%s\" service.\n"), service_str_id);
        ERR_EXIT();
    }

    // Validation diagnose arguments
    gchar *last_tool_name = NULL;
    for (gsize i = 4; i < argc; i++)
    {
        // Is a diag tool
        if (g_hash_table_contains(all_service_tests, argv[i]))
        {
            gchar *tool_name                   = argv[i];
            diag_tool_tests *current_tool_info = g_hash_table_lookup(all_service_tests, tool_name);
            if (!current_tool_info)
            {
                g_printerr(_("Error of parsing diagnose command arguments. Diagnostic tool \"%s\" of service \"%s\" "
                             "doesn't exist.\n"),
                           tool_name,
                           service_str_id);
                ERR_EXIT();
            }

            if ((current_tool_info && !current_tool_info->all_tests)
                || (current_tool_info && !g_hash_table_size(current_tool_info->all_tests)))
            {
                g_printerr(_("Error of parsing diagnose command arguments. Diagnostic tool \"%s\" of service \"%s\" "
                             "doesn't have "
                             "tests.\n"),
                           tool_name,
                           service_str_id);
                ERR_EXIT();
            }

            // We add a structure to fill them with selected tests from argv, if structure doesn't exist now
            diag_tool_tests *current_tool_tests = g_hash_table_lookup(service_tests, tool_name);
            if (!current_tool_tests) // Add tool test to global hash-table with tools tests
            {
                GHashTable *current_tool_all_tests_table = g_hash_table_new_full(g_str_hash,
                                                                                 g_str_equal,
                                                                                 (GDestroyNotify) g_free,
                                                                                 NULL);
                GHashTable *current_tool_required_only_tests_table
                    = g_hash_table_copy_table(current_tool_info->required_tests, g_strdup_copy_func, NULL, NULL, NULL);

                current_tool_tests = services_module_service_diag_tool_tests_init(current_tool_info->path,
                                                                                  current_tool_all_tests_table,
                                                                                  current_tool_required_only_tests_table,
                                                                                  current_tool_info->bus_type);
                g_hash_table_insert(service_tests, g_strdup(tool_name), current_tool_tests);
                g_hash_table_unref(current_tool_all_tests_table);
                g_hash_table_unref(current_tool_required_only_tests_table);
            }
            else
            {
                g_printerr(_("The diagnostic tool name \"%s\" has already been specified before.\n"), tool_name);
                ERR_EXIT();
            }

            last_tool_name = tool_name;
        }
        // Is a diag tool test or invalid argument
        else
        {
            // Diagnostic tool must be specified first
            if (!g_hash_table_size(service_tests))
            {
                g_printerr(_("Diagnostic tool must be specified first.\n"));
                ERR_EXIT();
            }

            // The test must be related to the last specified diagnostic tool
            gchar *current_toolname            = last_tool_name;
            gchar *test_name                   = argv[i];
            diag_tool_tests *current_tool_info = g_hash_table_lookup(all_service_tests, current_toolname);
            if (!g_hash_table_contains(current_tool_info->all_tests, test_name))
            {
                g_printerr(_("Test \"%s\" of diagnostic tool \"%s\" either does not apply to \"%s-diag\" mode or does "
                             "not exist.\n"),
                           test_name,
                           current_toolname,
                           deploy_mode_env.value);
                ERR_EXIT();
            }

            diag_tool_tests *current_tool_tests = g_hash_table_lookup(service_tests, current_toolname);
            g_hash_table_add(current_tool_tests->all_tests, g_strdup(test_name));
        }
    }

    // Append required tests
    GHashTableIter iter;
    g_hash_table_iter_init(&iter, all_service_tests);
    gchar *tool_name           = NULL;
    diag_tool_tests *tool_info = NULL;
    while (g_hash_table_iter_next(&iter, (gpointer *) &tool_name, (gpointer) &tool_info))
    {
        // No required tests in current tool
        if (!g_hash_table_size(tool_info->required_tests))
            continue;

        diag_tool_tests *tool_tests = g_hash_table_lookup(service_tests, tool_name);
        if (!tool_tests) // Add tool test to global hash-table with tools tests
        {
            GHashTable *current_tool_all_tests_table           = g_hash_table_new_full(g_str_hash,
                                                                             g_str_equal,
                                                                             (GDestroyNotify) g_free,
                                                                             NULL);
            GHashTable *current_tool_required_only_tests_table = g_hash_table_copy_table(tool_info->required_tests,
                                                                                         g_strdup_copy_func,
                                                                                         NULL,
                                                                                         NULL,
                                                                                         NULL);

            gchar *tool_path = ((diag_tool_tests *) g_hash_table_lookup(all_service_tests, tool_name))->path;
            tool_tests       = services_module_service_diag_tool_tests_init(tool_path,
                                                                      current_tool_all_tests_table,
                                                                      current_tool_required_only_tests_table,
                                                                      tool_info->bus_type);
            g_hash_table_insert(service_tests, g_strdup(tool_name), tool_tests);
            g_hash_table_unref(current_tool_all_tests_table);
            g_hash_table_unref(current_tool_required_only_tests_table);
        }

        GPtrArray *required_tests_array = g_hash_table_get_keys_as_ptr_array(tool_info->required_tests);
        for (gsize i = 0; i < required_tests_array->len; i++)
        {
            gchar *test = (gchar *) required_tests_array->pdata[i];
            if (!g_hash_table_contains(tool_tests->all_tests, test))
            {
                g_print(_("WARNING: The required test \"%s\" of the diagnostic tool \"%s\" was not specified. This "
                          "test will be run.\n"),
                        test,
                        tool_name);
                g_hash_table_add(tool_tests->all_tests, g_strdup(test));
            }
        }
        g_ptr_array_unref(required_tests_array);
    }

end:
    if (service_tests != all_service_tests)
        g_hash_table_destroy(all_service_tests);

    if (diag_env_vars)
        g_ptr_array_unref(diag_env_vars);

    if (service_path)
        g_free(service_path);

    return ret;
}

static int fill_node_recursive(AlteratorCtlServicesModule *module,
                               JsonNode **node,
                               gchar **path,
                               int level,
                               const gchar *value_str,
                               GNode *parameter,
                               gboolean is_array_member,
                               gboolean is_enum_subparameter)
{
    int ret     = 0;
    GValue gval = G_VALUE_INIT;

    if (g_strv_length(path) <= level)
    {
        g_printerr(_("Internal error: failed to process an argument.\n"));
        ERR_EXIT();
    }

    AlteratorCtlModuleInfoParser *info_parser = module->gdbus_source->info_parser;
    GNode *parameters                         = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
        info_parser, module->info, SERVICES_ALTERATOR_ENTRY_SERVICE_PARAMETERS_TABLE_NAME);

    if (!parameter)
        parameter = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser, parameters, path[level]);

    if (!parameter)
    {
        g_printerr(_("Unknown parameter: '%s'.\n"), path[level]);
        ERR_EXIT();
    }

    alterator_entry_node *param_data = parameter->data;
    toml_value *type = g_hash_table_lookup(param_data->toml_pairs, is_array_member ? "array_type" : "type");
    // TODO: check null
    gchar *type_str = type->str_value;

    if (g_strv_length(path + level) == 1)
    {
        if (streq(type_str, "enum"))
        {
            // just create object if not exist and return
            JsonObject *enum_object = NULL;
            if (!*node)
            {
                *node           = json_node_new(JSON_NODE_OBJECT);
                JsonObject *obj = json_object_new();
                json_node_set_object(*node, obj);
                json_object_unref(obj);
            }
            enum_object = json_node_get_object(*node);

            GList *names = json_object_get_members(enum_object);
            if (names)
            {
                for (GList *name = names; name; name = name->next)
                    if (!streq(value_str, name->data))
                        json_object_remove_member(enum_object, name->data);

                g_list_free(names);
            }

            if (json_object_get_size(enum_object) == 0)
            {
                JsonObject *member = json_object_new();
                json_object_set_object_member(enum_object, value_str, member);
            }

            goto end;
        }

        if (!*node)
            *node = json_node_new(JSON_NODE_VALUE);

        if (streq(type_str, "integer"))
        {
            gval.g_type = G_TYPE_INT;
            int value   = atoi(value_str);
            g_value_set_int(&gval, value); //TODO: check int valid
        }
        else if (streq(type_str, "string"))
        {
            gval.g_type = G_TYPE_STRING;
            g_value_set_string(&gval, value_str);
        }
        else if (streq(type_str, "boolean"))
        {
            gval.g_type    = G_TYPE_BOOLEAN;
            gboolean value = streq(value_str, "true");
            g_value_set_boolean(&gval, value);
            if (!value && !streq(value_str, "false"))
            {
                g_printerr(_("Invalid boolean value: %s.\n"), value_str);
                ERR_EXIT();
            }
        }
        else
        {
            g_printerr(_("Internal error: invalid parameter type: '%s'.\n"), type_str);
            ERR_EXIT();
        }

        json_node_set_value(*node, &gval);
    }
    else
    {
        if (is_enum_subparameter || streq(type_str, "object") || streq(type_str, "enum"))
        {
            JsonObject *object = NULL;
            if (!*node)
            {
                *node           = json_node_new(JSON_NODE_OBJECT);
                JsonObject *obj = json_object_new();
                json_node_set_object(*node, obj);
                json_object_unref(obj);
            }

            object = json_node_get_object(*node);

            if (is_enum_subparameter || streq(type_str, "object"))
            {
                toml_value *prototype = g_hash_table_lookup(param_data->toml_pairs, "prototype");
                if (prototype)
                {
                    parameter = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser,
                                                                                               parameters,
                                                                                               prototype->str_value);
                }
            }

            GNode *children = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
                info_parser, parameter, SERVICES_ALTERATOR_ENTRY_SERVICE_PROPERTIES_TABLE_NAME);
            GNode *child = children ? info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser,
                                                                                                     children,
                                                                                                     path[level + 1])
                                    : NULL;

            if (is_enum_subparameter || streq(type_str, "object"))
            {
                JsonNode *member = json_object_get_member(object, path[level + 1]);
                ret = fill_node_recursive(module, &member, path, level + 1, value_str, child, FALSE, FALSE);
                json_object_set_member(object, path[level + 1], member);
            }
            else // enum
            {
                JsonNode *child_node = json_object_get_member(object, path[level + 1]);

                if (!child_node)
                {
                    if (json_object_get_size(object))
                    {
                        GList *names = json_object_get_members(object);
                        g_printerr(
                            _("Selected enum value is '%s', but you are trying to set another subparameter '%s'.\n"),
                            (gchar *) names->data,
                            path[level + 1]);
                        g_list_free(names);
                        ERR_EXIT();
                    }
                    else
                    {
                        child_node = json_node_new(JSON_NODE_OBJECT);
                        json_node_init_object(child_node, json_object_new());
                        json_object_set_member(object, path[level + 1], child_node);
                    }
                }

                if (json_object_get_size(object) == 0)
                    json_object_set_object_member(object, path[level + 1], json_object_new());

                fill_node_recursive(module, &child_node, path, level + 1, value_str, child, FALSE, TRUE);
            }
        }
        else if (streq(type_str, "array"))
        {
            JsonArray *array = NULL;
            if (!*node)
            {
                *node          = json_node_new(JSON_NODE_ARRAY);
                JsonArray *arr = json_array_new();
                json_node_set_array(*node, arr);
                json_array_unref(arr);
            }
            array = json_node_get_array(*node);

            gchar *end = NULL;
            int index  = g_ascii_strtoll(path[level + 1], &end, 10);
            if (end == path[level + 1] || index < 0)
            {
                g_printerr(_("%s: invalid index %s.\n"), g_strjoinv(".", path), path[level + 1]);
                ERR_EXIT();
            }

            JsonNode *member = json_array_get_length(array) > index ? json_array_get_element(array, index) : NULL;
            gboolean add     = !member;

            if ((ret = fill_node_recursive(module, &member, path, level + 1, value_str, parameter, TRUE, FALSE)))
                goto end;

            if (add)
                json_array_add_element(array, member);
        }
        else
        {
            g_printerr(_("Internal error: invalid value type: '%s'.\n"), type_str);
            ERR_EXIT();
        }
    }

end:
    g_value_unset(&gval);
    return ret;
}

// Unset optional param if empty_value (--param_name=) in configure
static int parse_parameter_argument(AlteratorCtlServicesModule *module,
                                    const gchar *option_name,
                                    const gchar *value,
                                    gboolean unset)
{
    int ret = 0;

    AlteratorCtlModuleInfoParser *info_parser = module->gdbus_source->info_parser;
    gchar **path                              = NULL;

    if (strlen(option_name) < 3 || option_name[0] != '-' || option_name[1] != '-')
    {
        g_printerr(_("Invalid parameter: '%s'.\n"), option_name);
        ERR_EXIT();
    }

    GNode *parameters = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
        info_parser, module->info, SERVICES_ALTERATOR_ENTRY_SERVICE_PARAMETERS_TABLE_NAME);

    if (!module->json)
    {
        module->json = json_node_new(JSON_NODE_OBJECT);
        json_node_init_object(module->json, json_object_new());
    }

    gchar *parameter_name = g_utf8_substring(option_name, 2, strlen(option_name));
    path                  = g_strsplit_set(parameter_name, ".", -1);
    g_free(parameter_name);

    if (!g_strv_length(path))
    {
        g_printerr(_("Invalid argument '%s'.\n"), option_name);
        ERR_EXIT();
    }

    JsonNode *member = json_object_get_member(json_node_get_object(module->json), path[0]);
    if ((ret = fill_node_recursive(module, &member, path, 0, value, NULL, FALSE, FALSE)))
        goto end;

    if (member)
    {
        if (unset)
        {
            JsonNode *parent_node = module->json;
            for (gsize i = 1; i < g_strv_length(path); i++)
            {
                parent_node = member;
                member      = json_object_get_member(json_node_get_object(member), path[i]);
            }
            json_object_remove_member(json_node_get_object(parent_node), path[g_strv_length(path) - 1]);
            goto end;
        }
        json_object_set_member(json_node_get_object(module->json), path[0], member);
    }

end:
    g_strfreev(path);
    return ret;
}

static gboolean schema_param_matches_contexts(alterator_entry_node *param_data,
                                              const gchar *const *contexts,
                                              gsize contexts_len);
static gboolean schema_param_required_in_contexts(alterator_entry_node *param_data,
                                                  const gchar *const *contexts,
                                                  gsize contexts_len);
static gboolean schema_property_required_in_contexts(alterator_entry_node *param_data,
                                                     const gchar *const *contexts,
                                                     gsize contexts_len);
static gchar *services_contexts_label(const gchar *const *contexts, gsize contexts_len);

static int validate_json_recursive(AlteratorCtlServicesModule *module,
                                   const gchar *service_name,
                                   JsonNode *node,
                                   GNode *parameter,
                                   const gchar *const *contexts,
                                   gsize contexts_len,
                                   gchar *prefix,
                                   gboolean is_array_member,
                                   gboolean is_enum_subparameter,
                                   gboolean check_required_params)
{
    int ret = 0;

    AlteratorCtlModuleInfoParser *info_parser = module->gdbus_source->info_parser;
    alterator_entry_node *param_data          = parameter->data;
    gchar *path = prefix ? (is_array_member ? g_strdup(prefix) : g_strconcat(prefix, ".", param_data->node_name, NULL))
                         : g_strdup(param_data->node_name);

    gchar *type_str = NULL;
    {
        toml_value *type = NULL;
        info_parser->alterator_ctl_module_info_parser_find_value(info_parser,
                                                                 parameter,
                                                                 &type,
                                                                 is_array_member ? "array_type" : "type",
                                                                 NULL);

        if (type)
            type_str = type->str_value;
    }

    // skip constants
    {
        toml_value *constant = g_hash_table_lookup(param_data->toml_pairs,
                                                   SERVICES_ALTERATOR_ENTRY_SERVICE_PARAM_CONSTANT_KEY_NAME);
        if (constant && constant->bool_value)
            goto end;
    }

    gboolean match               = schema_param_matches_contexts(param_data, contexts, contexts_len);
    gboolean required            = schema_param_required_in_contexts(param_data, contexts, contexts_len);
    const gchar *primary_context = (contexts_len && contexts && contexts[0]) ? contexts[0] : "";

    if (node)
    {
        if (!match)
        {
            gchar *ctx_label = services_contexts_label(contexts, contexts_len);
            g_printerr(_("Parameter '%s' does not belong to context '%s'.\n"), path, ctx_label);
            g_free(ctx_label);
            ERR_EXIT();
        }

        if (json_node_get_node_type(node) == JSON_NODE_NULL)
        {
            g_printerr(_("Parameter '%s' is null.\n"), path);
            ERR_EXIT();
        }
    }
    else
    {
        if (check_required_params && match && required)
        {
            g_printerr(_("Parameter '%s' is required.\n"), path);
            ERR_EXIT();
        }
        goto end;
    }

    if (is_enum_subparameter || streq(type_str, "object"))
    {
        toml_value *prototype = g_hash_table_lookup(param_data->toml_pairs, "prototype");
        if (prototype)
        {
            GNode *parameters = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser,
                                                                                               module->info,
                                                                                               "parameters");
            parameter         = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser,
                                                                                       parameters,
                                                                                       prototype->str_value);
        }

        GNode *children = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
            info_parser, parameter, SERVICES_ALTERATOR_ENTRY_SERVICE_PROPERTIES_TABLE_NAME);

        if (children)
        {
            for (GNode *child = children->children; child != NULL; child = child->next)
            {
                alterator_entry_node *child_data = child->data;
                JsonNode *child_node = json_object_get_member(json_node_get_object(node), child_data->node_name);

                if (validate_json_recursive(module,
                                            service_name,
                                            child_node,
                                            child,
                                            contexts,
                                            contexts_len,
                                            path,
                                            FALSE,
                                            FALSE,
                                            check_required_params)
                    < 0)
                    ERR_EXIT();
            }
        }
        else
        {
            if (json_object_get_size(json_node_get_object(node)))
            {
                g_printerr(_("%s should be empty.\n"), path);
                ERR_EXIT();
            }
        }
    }
    else if (streq(type_str, "array"))
    {
        JsonArray *array = json_node_get_array(node);

        toml_value *min = g_hash_table_lookup(param_data->toml_pairs, "array_min");
        toml_value *max = g_hash_table_lookup(param_data->toml_pairs, "array_max");

        int len = json_array_get_length(array);

        if (min && min->double_value > len)
        {
            g_printerr(_("%s should have at least %i items.\n"), path, (int) min->double_value);
            ERR_EXIT();
        }

        if (max && max->double_value < len)
        {
            g_printerr(_("%s should not have more items than %i.\n"), path, (int) max->double_value);
            ERR_EXIT();
        }

        for (int i = 0; i < len; ++i)
        {
            gchar *suffix = g_strdup_printf(".%i", i);
            gchar *path_  = g_strconcat(path, suffix, NULL);
            g_free(suffix);

            ret = validate_json_recursive(module,
                                          service_name,
                                          json_array_get_element(array, i),
                                          parameter,
                                          contexts,
                                          contexts_len,
                                          path_,
                                          TRUE,
                                          FALSE,
                                          check_required_params);
            g_free(path_);

            if (ret < 0)
                break;
        }
    }
    else if (streq(type_str, "integer"))
    {
        int value = json_node_get_int(node);

        toml_value *min = g_hash_table_lookup(param_data->toml_pairs, "min");
        toml_value *max = g_hash_table_lookup(param_data->toml_pairs, "max");

        if (min && min->double_value > value)
        {
            g_printerr(_("%s should not be less than %i.\n"), path, (int) min->double_value);
            ERR_EXIT();
        }

        if (max && max->double_value < value)
        {
            g_printerr(_("%s should be less than %i.\n"), path, (int) max->double_value);
            ERR_EXIT();
        }
    }
    else if (streq(type_str, "string"))
    {
        const gchar *value_str = json_node_get_string(node);

        toml_value *pattern = g_hash_table_lookup(param_data->toml_pairs, "pattern");

        if (!value_str || !strlen(value_str))
        {
            g_printerr(_("The parameter %s value must not be empty.\n"), path);
            ERR_EXIT();
        }

        if (pattern)
        {
            GRegex *string_param_pattern = g_regex_new(pattern->str_value, 0, 0, NULL);
            GMatchInfo *match_info       = NULL;
            g_regex_match(string_param_pattern, value_str, 0, &match_info);
            if (!g_match_info_matches(match_info))
            {
                g_printerr(_("The parameter %s value doesn't match the pattern %s.\n"), path, pattern->str_value);
                g_match_info_free(match_info);
                g_regex_unref(string_param_pattern);
                ERR_EXIT();
            }
            g_match_info_free(match_info);
            g_regex_unref(string_param_pattern);
        }
    }
    else if (streq(type_str, "enum"))
    {
        JsonObject *enum_object = json_node_get_object(node);

        switch (json_object_get_size(enum_object))
        {
        case 0:
            g_printerr(_("One variant of enum \"%s\" should be selected.\n"), path);
            ERR_EXIT();

        case 1:
            break;

        default:
            g_printerr(_("Only one variant of enum \"%s\" may be selected.\n"), path);
            ERR_EXIT();
        }

        // There will be one element, since enum can only have one value.
        GList *enum_field_name = json_object_get_members(enum_object);
        gchar *field_name      = (gchar *) enum_field_name->data;
        g_list_free(enum_field_name);

        GNode *values = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser, parameter, "values");
        GNode *value = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser, values, field_name);

        if (!value)
        {
            g_printerr(_("The value '%s' for enum '%s' does not exist. "
                         "Run '%s %s --help' to see the list of valid values.\n"),
                       field_name,
                       path,
                       primary_context,
                       service_name);
            ERR_EXIT();
        }

        JsonNode *current_value_node = json_object_get_member(enum_object, field_name);
        if (json_node_get_node_type(current_value_node) != JSON_NODE_OBJECT)
        {
            g_printerr(_("The enum '%s' does not exist. Run '%s %s --help' to see the list of valid parameters.\n"),
                       path,
                       primary_context,
                       service_name);
            ERR_EXIT();
        }

        ret = validate_json_recursive(module,
                                      service_name,
                                      current_value_node,
                                      value,
                                      contexts,
                                      contexts_len,
                                      prefix,
                                      FALSE,
                                      TRUE,
                                      check_required_params);
    }

end:
    g_free(path);
    return ret;
}

static int validate_json(AlteratorCtlServicesModule *module,
                         const gchar *service_name,
                         const gchar *const *contexts,
                         gsize contexts_len,
                         gboolean check_required_params)
{
    int ret = 0;

    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    GNode *parameters = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
        info_parser, module->info, SERVICES_ALTERATOR_ENTRY_SERVICE_PARAMETERS_TABLE_NAME);

    for (GNode *parameter = parameters->children; parameter != NULL; parameter = parameter->next)
    {
        alterator_entry_node *param_data = parameter->data;
        JsonNode *node = json_object_get_member(json_node_get_object(module->json), param_data->node_name);
        if (validate_json_recursive(
                module, service_name, node, parameter, contexts, contexts_len, NULL, FALSE, FALSE, check_required_params)
            < 0)
            ERR_EXIT();
    }

end:
    return ret;
}

static gboolean schema_param_in_context(alterator_entry_node *param_data, const gchar *context)
{
    toml_value *ctx = g_hash_table_lookup(param_data->toml_pairs,
                                          SERVICES_ALTERATOR_ENTRY_SERVICE_PARAM_CONTEXT_KEY_NAME);
    if (ctx && ctx->array_length && ctx->array)
    {
        for (gsize i = 0; i < ctx->array_length; i++)
            if (streq(context, ((gchar **) ctx->array)[i]))
                return TRUE;
        return FALSE;
    }
    return TRUE;
}

static gboolean schema_param_required_in_context(alterator_entry_node *param_data, const gchar *context)
{
    toml_value *required_val = g_hash_table_lookup(param_data->toml_pairs,
                                                   SERVICES_ALTERATOR_ENTRY_SERVICE_PARAM_REQUIRED_KEY_NAME);
    if (!required_val)
        return FALSE;
    if (required_val->array_length && required_val->array)
    {
        for (gsize i = 0; i < required_val->array_length; i++)
            if (streq(context, ((gchar **) required_val->array)[i]))
                return TRUE;
        return FALSE;
    }
    return required_val->type == TOML_DATA_BOOL && required_val->bool_value;
}

static gboolean schema_property_required(alterator_entry_node *param_data, const gchar *context)
{
    toml_value *required_val = g_hash_table_lookup(param_data->toml_pairs,
                                                   SERVICES_ALTERATOR_ENTRY_SERVICE_PARAM_REQUIRED_KEY_NAME);
    if (!required_val)
        return FALSE;
    if (required_val->array_length && required_val->array)
    {
        for (gsize i = 0; i < required_val->array_length; i++)
            if (streq(context, ((gchar **) required_val->array)[i]))
                return TRUE;
        return FALSE;
    }
    return required_val->type == TOML_DATA_BOOL && required_val->bool_value;
}

static gboolean schema_param_matches_contexts(alterator_entry_node *param_data,
                                              const gchar *const *contexts,
                                              gsize contexts_len)
{
    if (!contexts_len)
        return TRUE;

    for (gsize i = 0; i < contexts_len; i++)
        if (schema_param_in_context(param_data, contexts ? contexts[i] : NULL))
            return TRUE;

    return FALSE;
}

static gboolean schema_param_required_in_contexts(alterator_entry_node *param_data,
                                                  const gchar *const *contexts,
                                                  gsize contexts_len)
{
    if (!contexts_len)
        return schema_param_required_in_context(param_data, NULL);

    for (gsize i = 0; i < contexts_len; i++)
        if (schema_param_required_in_context(param_data, contexts ? contexts[i] : NULL))
            return TRUE;

    return FALSE;
}

static gboolean schema_property_required_in_contexts(alterator_entry_node *param_data,
                                                     const gchar *const *contexts,
                                                     gsize contexts_len)
{
    if (!contexts_len)
        return schema_property_required(param_data, NULL);

    for (gsize i = 0; i < contexts_len; i++)
        if (schema_property_required(param_data, contexts ? contexts[i] : NULL))
            return TRUE;

    return FALSE;
}

static gchar *services_contexts_label(const gchar *const *contexts, gsize contexts_len)
{
    if (!contexts_len || !contexts)
        return g_strdup("");

    if (contexts_len == 1)
        return g_strdup(contexts[0] ? contexts[0] : "");

    GString *buffer = g_string_new(NULL);
    for (gsize i = 0; i < contexts_len; i++)
    {
        const gchar *ctx = contexts[i];
        if (!ctx || !strlen(ctx))
            continue;
        if (buffer->len)
            g_string_append(buffer, ", ");
        g_string_append(buffer, ctx);
    }

    if (!buffer->len)
        g_string_append(buffer, "");

    return g_string_free(buffer, FALSE);
}

static gboolean schema_is_constant(alterator_entry_node *param_data)
{
    toml_value *constant = g_hash_table_lookup(param_data->toml_pairs,
                                               SERVICES_ALTERATOR_ENTRY_SERVICE_PARAM_CONSTANT_KEY_NAME);
    return (constant && constant->bool_value);
}

static const gchar *schema_type_name(AlteratorCtlModuleInfoParser *info_parser, GNode *parameter)
{
    toml_value *type = NULL;
    info_parser->alterator_ctl_module_info_parser_find_value(info_parser, parameter, &type, "type", NULL);
    return type ? type->str_value : NULL;
}

static gboolean schema_is_password_string(AlteratorCtlModuleInfoParser *info_parser, GNode *parameter)
{
    alterator_entry_node *param_data = parameter->data;
    const gchar *type_name           = schema_type_name(info_parser, parameter);
    if (!type_name || !streq(type_name, "string"))
        return FALSE;
    toml_value *password = g_hash_table_lookup(param_data->toml_pairs, "password");
    return (password && password->type == TOML_DATA_BOOL && password->bool_value);
}

static gchar *password_compose_label(const gchar *base_path, const gchar *field_name, const gchar *subfield_name)
{
    GString *label_builder = g_string_new(NULL);
    if (base_path && strlen(base_path))
        g_string_append(label_builder, base_path);
    if (field_name && strlen(field_name))
    {
        if (label_builder->len)
            g_string_append_c(label_builder, '.');
        g_string_append(label_builder, field_name);
    }
    if (subfield_name && strlen(subfield_name))
    {
        if (label_builder->len)
            g_string_append_c(label_builder, '.');
        g_string_append(label_builder, subfield_name);
    }
    return g_string_free(label_builder, FALSE);
}

static gboolean prompt_and_set_password(JsonObject *object, const gchar *key, const gchar *label)
{
    JsonNode *node              = json_object_get_member(object, key);
    const gchar *existing_value = (node && json_node_get_node_type(node) == JSON_NODE_VALUE)
                                      ? json_node_get_string(node)
                                      : NULL;
    if (existing_value && strlen(existing_value))
        return TRUE;

    gchar *secret = NULL;
    GError *error = NULL;
    if (!alterator_ctl_read_secret(label, &secret, &error))
    {
        if (error)
        {
            if (error->code == ALTERATOR_CTL_SECRET_ERROR_MISMATCH)
                g_printerr(_("Passwords for '%s' do not match.\n"), label);
            else if (error->code == ALTERATOR_CTL_SECRET_ERROR_EMPTY)
                g_printerr(_("Password for '%s' is required.\n"), label);
            else
                g_printerr(_("Failed to read password for '%s': %s\n"), label, error->message);
            g_error_free(error);
        }
        return FALSE;
    }
    json_object_set_string_member(object, key, secret);
    g_free(secret);
    return TRUE;
}

static gboolean fill_object_passwords(AlteratorCtlModuleInfoParser *info_parser,
                                      GNode *object_schema,
                                      JsonObject *object_json,
                                      const gchar *base_path,
                                      const gchar *const *contexts,
                                      gsize contexts_len)
{
    GNode *children = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
        info_parser, object_schema, SERVICES_ALTERATOR_ENTRY_SERVICE_PROPERTIES_TABLE_NAME);
    for (GNode *child = children ? children->children : NULL; child != NULL; child = child->next)
    {
        alterator_entry_node *child_data = child->data;
        if (schema_is_constant(child_data) || !schema_property_required_in_contexts(child_data, contexts, contexts_len))
            continue;
        if (!schema_is_password_string(info_parser, child))
            continue;

        gchar *label                       = password_compose_label(base_path, child_data->node_name, NULL);
        gboolean password_set_successfully = prompt_and_set_password(object_json, child_data->node_name, label);
        g_free(label);
        if (!password_set_successfully)
            return FALSE;
    }
    return TRUE;
}

static gboolean fill_enum_passwords(AlteratorCtlModuleInfoParser *info_parser,
                                    GNode *enum_schema,
                                    JsonObject *enum_json,
                                    const gchar *param_name,
                                    const gchar *const *contexts,
                                    gsize contexts_len)
{
    if (!json_object_get_size(enum_json))
        return TRUE;

    GList *members = json_object_get_members(enum_json);
    gchar *variant = members ? (gchar *) members->data : NULL;
    g_list_free(members);
    if (!variant)
        return TRUE;

    JsonNode *variant_node = json_object_get_member(enum_json, variant);
    if (!variant_node || json_node_get_node_type(variant_node) != JSON_NODE_OBJECT)
        return TRUE;
    JsonObject *variant_obj = json_node_get_object(variant_node);

    GNode *values = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser, enum_schema, "values");
    GNode *variant_schema = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser, values, variant);
    if (!variant_schema)
        return TRUE;

    gchar *variant_label = password_compose_label(param_name, variant, NULL);
    gboolean passwords_filled_successfully
        = fill_object_passwords(info_parser, variant_schema, variant_obj, variant_label, contexts, contexts_len);
    g_free(variant_label);
    return passwords_filled_successfully;
}

static int services_fill_missing_passwords(AlteratorCtlServicesModule *module,
                                           const gchar *service_name,
                                           const gchar *const *contexts,
                                           gsize contexts_len)
{
    int ret                                   = 0;
    AlteratorCtlModuleInfoParser *info_parser = module->gdbus_source->info_parser;
    if (!module->json || !module->info)
        goto end;

    JsonObject *root  = json_node_get_object(module->json);
    GNode *parameters = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
        info_parser, module->info, SERVICES_ALTERATOR_ENTRY_SERVICE_PARAMETERS_TABLE_NAME);

    for (GNode *parameter = parameters ? parameters->children : NULL; parameter != NULL; parameter = parameter->next)
    {
        alterator_entry_node *param_data = parameter->data;
        if (schema_is_constant(param_data) || !schema_param_matches_contexts(param_data, contexts, contexts_len))
            continue;

        const gchar *type_name = schema_type_name(info_parser, parameter);
        if (!type_name)
            continue;

        if (streq(type_name, "string"))
        {
            if (!schema_is_password_string(info_parser, parameter))
                continue;
            if (!schema_param_required_in_contexts(param_data, contexts, contexts_len))
                continue;
            if (!prompt_and_set_password(root, param_data->node_name, param_data->node_name))
                ERR_EXIT();
            continue;
        }

        if (streq(type_name, "object"))
        {
            JsonNode *obj_node = json_object_get_member(root, param_data->node_name);
            if (!obj_node || json_node_get_node_type(obj_node) != JSON_NODE_OBJECT)
                continue;
            JsonObject *object_json = json_node_get_object(obj_node);
            if (!fill_object_passwords(info_parser, parameter, object_json, param_data->node_name, contexts, contexts_len))
                ERR_EXIT();
            continue;
        }

        if (streq(type_name, "enum"))
        {
            JsonNode *enum_node = json_object_get_member(root, param_data->node_name);
            if (!enum_node || json_node_get_node_type(enum_node) != JSON_NODE_OBJECT)
                continue;
            JsonObject *enum_json = json_node_get_object(enum_node);
            if (!fill_enum_passwords(info_parser, parameter, enum_json, param_data->node_name, contexts, contexts_len))
                ERR_EXIT();
            continue;
        }
    }

end:
    return ret;
}

static int services_module_build_contextual_arguments_recursive(AlteratorCtlServicesModule *module,
                                                                GArray **option_array,
                                                                GNode *parameters,
                                                                GNode *parameter,
                                                                gchar *context,
                                                                gchar *prefix,
                                                                gchar *description_prefix,
                                                                ParamEntry *parent_array_entry,
                                                                gboolean is_array_item,
                                                                gboolean is_enum_subparameter,
                                                                gboolean constant_parent)
{
    int ret                    = 0;
    int enum_subparameters_len = 0;

    AlteratorCtlModuleInfoParser *info_parser = module->gdbus_source->info_parser;
    alterator_entry_node *param_data          = parameter->data;

    ParamEntry param_entry;
    memset(&param_entry, 0, sizeof(param_entry));

    if (!option_array)
    {
        g_printerr(_("Can't save parsed parameters to empty variable.\n"));
        ERR_EXIT();
    }

    gchar *path = prefix ? (is_array_item ? g_strdup(prefix) : g_strconcat(prefix, ".", param_data->node_name, NULL))
                         : g_strdup(param_data->node_name);

    toml_value *constant = g_hash_table_lookup(param_data->toml_pairs,
                                               SERVICES_ALTERATOR_ENTRY_SERVICE_PARAM_CONSTANT_KEY_NAME);

    // filter by context
    if (!prefix)
    {
        gboolean match = FALSE;

        toml_value *context_arr = g_hash_table_lookup(param_data->toml_pairs,
                                                      SERVICES_ALTERATOR_ENTRY_SERVICE_PARAM_CONTEXT_KEY_NAME);

        if (context_arr && context_arr->array_length && context_arr->array)
        {
            for (gsize i = 0; i < context_arr->array_length; i++)
            {
                if (!streq(context, ((gchar **) context_arr->array)[i]))
                    continue;

                match = TRUE;
                break;
            }
        }
        else
            goto end;

        if (!match)
            goto end;
    }

    gchar *type_str = NULL;
    {
        toml_value *type = NULL;
        info_parser->alterator_ctl_module_info_parser_find_value(
            info_parser,
            parameter,
            &type,
            is_array_item ? SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_ARRAY_TYPE_KEY_NAME
                          : SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_TYPE_KEY_NAME,
            NULL);
        if (type)
            type_str = type->str_value;
    }

    gchar *description     = g_strdup(description_prefix);
    gchar *comment         = NULL;
    gchar *arg_description = NULL;

    gboolean required = FALSE;
    // TODO: add to description
    {
        toml_value *required_val = g_hash_table_lookup(param_data->toml_pairs,
                                                       SERVICES_ALTERATOR_ENTRY_SERVICE_PARAM_REQUIRED_KEY_NAME);

        if (required_val)
        {
            if (required_val->array_length && required_val->array)
            {
                for (gsize i = 0; i < required_val->array_length; i++)
                {
                    if (!streq(context, ((gchar **) required_val->array)[i]))
                        continue;

                    required = TRUE;
                    break;
                }
            }
            else
                required = required_val->bool_value;
        }
    }

    // Add default values info into description
    toml_value *default_value = g_hash_table_lookup(param_data->toml_pairs,
                                                    SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_VALUE_KEY_NAME);

    if (is_enum_subparameter || streq(type_str, "object"))
    {
        {
            toml_value *prototype = g_hash_table_lookup(param_data->toml_pairs, "prototype");
            if (prototype)
                parameter = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser,
                                                                                           parameters,
                                                                                           prototype->str_value);
        }

        GNode *children = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
            info_parser, parameter, SERVICES_ALTERATOR_ENTRY_SERVICE_PROPERTIES_TABLE_NAME);

        if (!children)
            goto end;

        for (GNode *child = children->children; child != NULL; child = child->next)
            if (services_module_build_contextual_arguments_recursive(module,
                                                                     option_array,
                                                                     parameters,
                                                                     child,
                                                                     context,
                                                                     path,
                                                                     description,
                                                                     NULL,
                                                                     FALSE,
                                                                     FALSE,
                                                                     constant && constant->bool_value || constant_parent)
                < 0)
                ret = -1;

        goto end;
    }
    else if (streq(type_str, "array"))
    {
        int min = 0;
        int max = INT_MAX;

        toml_value *min_node = g_hash_table_lookup(param_data->toml_pairs, "array_min");
        if (min_node)
            min = min_node->double_value;

        toml_value *max_node = g_hash_table_lookup(param_data->toml_pairs, "array_max");
        if (max_node)
            max = max_node->double_value;

        gchar *max_str          = max == INT_MAX ? g_strdup("...") : g_strdup_printf("%i", max - 1);
        gchar *path_with_suffix = g_strdup_printf("%s.<%i-%s>", path, 0, max_str);
        g_free(max_str);

        param_entry.is_array       = TRUE;
        param_entry.min_array_size = min;
        param_entry.max_array_size = max;

        { // Pack default values into ParamEntry
            if (default_value)
            {
                toml_value *default_values_type
                    = g_hash_table_lookup(param_data->toml_pairs,
                                          SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_ARRAY_TYPE_KEY_NAME);

                GString *default_value_str = g_string_new(NULL);
                for (gsize i = 0; i < default_value->array_length; i++)
                {
                    if (i == 0)
                        g_string_append(default_value_str, default_value->array_length ? "[" : "");
                    if (!default_values_type->str_value || !strlen(default_values_type->str_value))
                        break;

                    if (streq(default_values_type->str_value, "integer"))
                    {
                        gint *array = (gint *) default_value->array;
                        g_string_append_printf(default_value_str,
                                               "%i%s",
                                               array[i],
                                               i != default_value->array_length - 1 ? ", " : "]");
                    }
                    else if (streq(default_values_type->str_value, "boolean"))
                    {
                        gboolean *array = (gboolean *) default_value->array;
                        g_string_append_printf(default_value_str,
                                               "%s%s",
                                               array[i] ? "true" : "false",
                                               i != default_value->array_length - 1 ? ",\n" : "]");
                    }
                    else if (streq(default_values_type->str_value, "string")
                             || streq(default_values_type->str_value, "enum"))
                    {
                        gchar **array = (gchar **) default_value->array;
                        g_string_append_printf(default_value_str,
                                               "\"%s\"%s",
                                               array[i],
                                               i != default_value->array_length - 1 ? ", " : "]");
                    }
                }
                param_entry.default_value_str = g_string_free(default_value_str, FALSE);
            }
        }

        services_module_build_contextual_arguments_recursive(module,
                                                             option_array,
                                                             parameters,
                                                             parameter,
                                                             context,
                                                             path_with_suffix,
                                                             description,
                                                             &param_entry,
                                                             TRUE,
                                                             FALSE,
                                                             constant && constant->bool_value || constant_parent);
        g_free(path_with_suffix);
        goto end;
    }
    else if (streq(type_str, "integer"))
    {
        int min = INT_MIN;
        int max = INT_MAX;

        toml_value *min_node = g_hash_table_lookup(param_data->toml_pairs, "min");
        if (min_node)
            min = min_node->double_value;

        toml_value *max_node = g_hash_table_lookup(param_data->toml_pairs, "max");
        if (max_node)
            max = max_node->double_value;

        if (min_node || max_node)
        {
            gchar *min_str = min_node ? g_strdup_printf("%i", min) : g_strdup("...");
            gchar *max_str = max_node ? g_strdup_printf("%i", max) : g_strdup("...");

            arg_description = g_strdup_printf("<%s-%s>", min_str, max_str);

            g_free(min_str);
            g_free(max_str);
        }

        param_entry.is_array          = FALSE;
        param_entry.default_value_str = default_value && default_value->type == TOML_DATA_DOUBLE
                                            ? g_strdup_printf("%i", (gint) default_value->double_value)
                                            : NULL;
        arg_description               = g_strdup("<number>");
    }
    else if (streq(type_str, "string"))
    {
        param_entry.is_array          = FALSE;
        param_entry.default_value_str = default_value && default_value->type == TOML_DATA_STRING
                                            ? g_strdup(default_value->str_value)
                                            : NULL;
        arg_description               = g_strdup("<string>");
    }
    else if (streq(type_str, "enum"))
    {
        param_entry.is_array = FALSE;
        GArray *values_array = g_array_new(TRUE, FALSE, sizeof(gchar *));
        GNode *values = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser, parameter, "values");

        enum_subparameters_len = (*option_array)->len;

        for (GNode *value = values->children; value != NULL; value = value->next)
        {
            gchar *value_name = ((alterator_entry_node *) value->data)->node_name;
            g_array_append_val(values_array, value_name);

            if (services_module_build_contextual_arguments_recursive(module,
                                                                     option_array,
                                                                     parameters,
                                                                     value,
                                                                     context,
                                                                     path,
                                                                     description,
                                                                     NULL,
                                                                     FALSE,
                                                                     TRUE,
                                                                     constant && constant->bool_value || constant_parent)
                < 0)
                ret = -1;
        }

        gchar *values_joined = g_strjoinv("|", (gchar **) values_array->data);
        arg_description      = g_strconcat("<", values_joined, ">", NULL);
        g_free(values_joined);

        g_array_free(values_array, TRUE);

        param_entry.default_value_str = default_value && default_value->type == TOML_DATA_STRING
                                            ? g_strdup(default_value->str_value)
                                            : NULL;
    }
    else if (streq(type_str, "boolean"))
    {
        param_entry.is_array          = FALSE;
        param_entry.default_value_str = default_value && default_value->type == TOML_DATA_BOOL
                                            ? g_strdup_printf("%s", default_value->bool_value ? "true" : "false")
                                            : NULL;
        arg_description               = g_strdup("<true|false>");
    }
    else
    {
        g_printerr(_("Service returned invalid parameter type: '%s'.\n"), type_str);
        ERR_EXIT();
    }

    { // get localized fields
        gchar *locale = alterator_ctl_get_language();

        GHashTable *display_name = NULL;
        if (info_parser
                ->alterator_ctl_module_info_parser_find_table(info_parser,
                                                              parameter,
                                                              &display_name,
                                                              -1,
                                                              SERVICES_ALTERATOR_ENTRY_SERVICE_DISPLAY_NAME_TABLE_NAME,
                                                              NULL))
        {
            toml_value *display_name_locale_value = g_hash_table_lookup(display_name, locale);
            if (!display_name_locale_value)
                display_name_locale_value = g_hash_table_lookup(display_name, LOCALE_FALLBACK);

            if (description && g_utf8_strlen(description, -1))
            {
                gchar *tmp  = description;
                description = g_strconcat(tmp, display_name_locale_value->str_value, "\n", NULL);
                g_free(tmp);
            }
            else
                description = g_strdup_printf("%s\n", display_name_locale_value->str_value);
        }

        GHashTable *comment_table = NULL;
        if (info_parser
                ->alterator_ctl_module_info_parser_find_table(info_parser,
                                                              parameter,
                                                              &comment_table,
                                                              -1,
                                                              SERVICES_ALTERATOR_ENTRY_SERVICE_COMMENT_NAME_TABLE_NAME,
                                                              NULL))
        {
            toml_value *comment_locale_value = g_hash_table_lookup(comment_table, locale);
            if (!comment_locale_value)
                comment_locale_value = g_hash_table_lookup(comment_table, LOCALE_FALLBACK);

            comment = comment_locale_value->str_value;

            if (description && g_utf8_strlen(description, -1))
            {
                gchar *tmp  = description;
                description = g_strconcat(tmp, comment, "\n", NULL);
                g_free(tmp);
            }
            else
                description = g_strdup(comment);
        }

        g_free(locale);
    } // get localized fields
    // TODO: append value constraints & localized enum values

    GOptionEntry option_entry
        = {g_strdup(path), 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, NULL, description, arg_description};

    if (parent_array_entry)
    {
        param_entry.default_value_str = g_strdup(parent_array_entry->default_value_str);
        param_entry.min_array_size    = parent_array_entry->min_array_size;
        param_entry.max_array_size    = parent_array_entry->max_array_size;
        services_module_parameter_entry_clear(parent_array_entry);
    }
    param_entry.option_entry = option_entry;
    param_entry.is_constant  = constant && constant->bool_value || constant_parent;

    if (streq(type_str, "enum"))
        g_array_insert_val(*option_array, enum_subparameters_len, param_entry);
    else
        g_array_append_val(*option_array, param_entry);

end:
    if (ret < 0)
    {
        g_printerr(_("Parsing '%s' failed.\n"), path);

        if (arg_description)
            g_free(arg_description);

        if (description)
            g_free(description);
    }

    g_free(path);
    return ret;
}

static void services_module_parameter_entry_clear(ParamEntry *e)
{
    g_free(e->default_value_str);
    g_free((gchar *) e->option_entry.long_name);
    g_free((gchar *) e->option_entry.arg_description);
    g_free((gchar *) e->option_entry.description);
}

static gchar *get_context_from_command_id(enum services_sub_commands command)
{
    switch (command)
    {
    case SERVICES_DEPLOY:
        return SERVICES_CONTEXT_DEPLOY;
    case SERVICES_UNDEPLOY:
        return SERVICES_CONTEXT_UNDEPLOY;
    case SERVICES_BACKUP:
        return SERVICES_CONTEXT_BACKUP;
    case SERVICES_RESTORE:
        return SERVICES_CONTEXT_RESTORE;
    case SERVICES_CONFIGURE:
        return SERVICES_CONTEXT_CONFIGURE;
    case SERVICES_DIAGNOSE:
        return SERVICES_CONTEXT_DIAG;
    default:
        return "";
    }
}

static gint get_command_id_from_context(const gchar *context)
{
    if (!context)
        return -1;

    if (streq(context, SERVICES_CONTEXT_DEPLOY))
        return SERVICES_DEPLOY;
    else if (streq(context, SERVICES_CONTEXT_UNDEPLOY))
        return SERVICES_UNDEPLOY;
    else if (streq(context, SERVICES_CONTEXT_BACKUP))
        return SERVICES_BACKUP;
    else if (streq(context, SERVICES_CONTEXT_RESTORE))
        return SERVICES_RESTORE;
    else if (streq(context, SERVICES_CONTEXT_CONFIGURE))
        return SERVICES_CONFIGURE;

    return -1;
}

static int services_module_parse_contextual_arguments(AlteratorCtlServicesModule *module,
                                                      const gchar *service_name,
                                                      int command,
                                                      int *argc,
                                                      char **argv,
                                                      alteratorctl_ctx_t *ctx,
                                                      GArray **result)
{
    int ret                                   = 0;
    GString *params_options                   = g_string_new("");
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;
    gchar *context                            = NULL;
    GError *error                             = NULL;
    GOptionContext *option_context            = NULL;
    GOptionGroup *contextual_params           = NULL;
    JsonNode *defaults                        = NULL;
    JsonNode *current_defaults                = NULL;
    JsonNode *defaults_filtered               = NULL;
    JsonNode *current_defaults_filtered       = NULL;
    alteratorctl_ctx_t *service_status_ctx    = NULL;

    context = get_context_from_command_id(command);

    if (!result)
    {
        g_printerr(_("Can't save parsed parameters to empty variable.\n"));
        ERR_EXIT();
    }

    *result = g_array_new(TRUE, FALSE, sizeof(ParamEntry));
    g_array_set_clear_func(*result, (GDestroyNotify) services_module_parameter_entry_clear);

    if (!module->info)
    {
        g_printerr(_("Error processing service %s parameters: empty service info.\n"), service_name);
        ERR_EXIT();
    }

    GNode *parameters = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
        info_parser, module->info, SERVICES_ALTERATOR_ENTRY_SERVICE_PARAMETERS_TABLE_NAME);

    if (command == SERVICES_DEPLOY)
    {
        toml_value *force_deploy_allowed = NULL;
        info_parser
            ->alterator_ctl_module_info_parser_find_value(info_parser,
                                                          module->info,
                                                          &force_deploy_allowed,
                                                          SERVICES_ALTERATOR_ENTRY_SERVICE_ENABLE_FORCE_DEPLOY_KEY_NAME,
                                                          NULL);
        if (force_deploy_allowed && force_deploy_allowed->bool_value)
        {
            // Allocating memory on the heap for services_module_parameter_entry_clear
            GOptionEntry force_e = {g_strdup("force-deploy"), // Move to option from parameter section!!!!
                                    'f',
                                    G_OPTION_FLAG_NONE,
                                    G_OPTION_ARG_NONE,
                                    &force_deploy,
                                    g_strdup("Force deploy"),  // rewrite description
                                    g_strdup("FORCE_DEPLOY")}; // rewrite arg-description
            ParamEntry force     = {NULL, 0, 0, force_e, FALSE, FALSE};

            g_array_append_val(*result, force);
        }
    }

    for (GNode *parameter = parameters->children; parameter != NULL; parameter = parameter->next)
        services_module_build_contextual_arguments_recursive(
            module, result, parameters, parameter, context, NULL, NULL, NULL, FALSE, FALSE, FALSE);

    option_context = g_option_context_new(context);
    g_option_context_set_ignore_unknown_options(option_context, TRUE);

    gchar *contextual_params_description = g_strconcat("'", context, "' parameters:", NULL);
    contextual_params                    = g_option_group_new(context, contextual_params_description, "", module, NULL);
    g_free(contextual_params_description);

    GArray *option_entries = g_array_new(TRUE, FALSE, sizeof(GOptionEntry));
    for (gsize i = 0; i < (*result)->len; i++)
    {
        GOptionEntry entry = ((ParamEntry *) (*result)->data)[i].option_entry;
        GOptionEntry copy  = {g_strdup(entry.long_name),
                              entry.short_name,
                              entry.flags,
                              entry.arg,
                              entry.arg_data,
                              g_strdup(entry.description),
                              g_strdup(entry.arg_description)};
        g_array_append_val(option_entries, copy);
    }
    g_option_group_add_entries(contextual_params, (GOptionEntry *) option_entries->data);
    g_array_unref(option_entries);

    GOptionEntry main_entries[]
        = {{"help", 'h', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, NULL, "Show usage help\n", NULL},
           {"no-default",
            0,
            G_OPTION_FLAG_NONE,
            G_OPTION_ARG_NONE,
            &no_default,
            "do not use default value if parameter is not set",
            NULL},
           {"yes", 'y', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &yes, "Assume \"yes\" to all queries\n", NULL},
           {NULL}};

    g_option_context_add_main_entries(option_context, main_entries, NULL);

    if (module->alterator_ctl_app->arguments->module_help)
    {
        g_option_context_add_group(option_context, contextual_params);
        goto end;
    }
    else
    {
        g_option_group_unref(contextual_params);
        contextual_params = NULL;
    }

    if (!g_option_context_parse(option_context, argc, &argv, &error))
    {
        g_printerr("%s.\n", error->message);
        ERR_EXIT();
    }

    // merge defaults
    {
        GHashTable *params_ctx = NULL;

        defaults = json_node_new(JSON_NODE_OBJECT);
        {
            JsonObject *obj = json_object_new();
            json_node_set_object(defaults, obj);
            json_object_unref(obj);
        }

        if (services_module_get_initial_params(
                module, service_name, &defaults, TRUE, get_context_from_command_id(command), &params_ctx, TRUE)
            < 0)
            ERR_EXIT();

        defaults_filtered = services_module_filter_params_by_context(defaults, params_ctx, context, no_default);

        service_status_ctx = alteratorctl_ctx_init_services(SERVICES_STATUS, service_name, NULL, NULL, NULL);

        if (no_default)
        {
            JsonNode *status_node = NULL;
            if (services_module_status_get_json_node(module, service_name, &status_node) < 0)
                ERR_EXIT();

            module->json = services_module_filter_params_by_context(status_node, params_ctx, context, no_default);
            json_node_unref(status_node);
        }
        else
        {
            if (services_module_status_subcommand(module, &service_status_ctx) < 0)
                ERR_EXIT();

            current_defaults = json_node_new(JSON_NODE_OBJECT);
            {
                JsonObject *obj = json_object_new();
                json_node_set_object(current_defaults, obj);
                json_object_unref(obj);
            }

            if (services_module_status_result_get_service_params(module,
                                                                 &service_status_ctx,
                                                                 service_name,
                                                                 command != SERVICES_CONFIGURE,
                                                                 context,
                                                                 &current_defaults,
                                                                 NULL,
                                                                 TRUE)
                < 0)
                ERR_EXIT();

            current_defaults_filtered = services_module_filter_params_by_context(current_defaults,
                                                                                 params_ctx,
                                                                                 context,
                                                                                 no_default);

            if (!(module->json = services_module_merge_json_data(defaults_filtered,
                                                                 current_defaults_filtered,
                                                                 service_name)))
                ERR_EXIT();
        }

        // checking and approving force deploy mode
        if (force_deploy)
        {
            toml_value *is_enable_force_deploy = NULL;
            if (!(is_enable_force_deploy
                  = g_hash_table_lookup(((alterator_entry_node *) module->info->data)->toml_pairs,
                                        SERVICES_ALTERATOR_ENTRY_SERVICE_ENABLE_FORCE_DEPLOY_KEY_NAME))
                || is_enable_force_deploy->type != TOML_DATA_BOOL || is_enable_force_deploy->bool_value != TRUE)
            {
                g_printerr(_("Force-deploy is not available for service \"%s\".\n"), service_name);
                ERR_EXIT();
            }

            json_object_set_boolean_member(json_node_get_object(module->json),
                                           SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_FORCE_DEPLOY_KEY_NAME,
                                           TRUE);
        }

        g_hash_table_destroy(params_ctx);
    }

    // TODO: rework input usage
    if (json_params_filepath)
    {
        if (*argc > 4)
        {
            g_printerr(_("Mixing --input with other parameters is not supported.\n"));
            ERR_EXIT();
        }

        gchar *input_json_parameters = services_module_get_parameters(TRUE);

        if (input_json_parameters)
        {
            JsonParser *parser = json_parser_new();
            if (!json_parser_load_from_data(parser, input_json_parameters, -1, &error))
            {
                g_printerr(_("Invalid input json data.\n"));
                g_free(input_json_parameters);
                ERR_EXIT();
            }

            JsonNode *input_json     = json_parser_get_root(parser);
            JsonNode *parameters_tmp = NULL;

            if (!(parameters_tmp = services_module_merge_json_data(module->json, input_json, service_name)))
                ERR_EXIT();
            else
            {
                json_node_unref(module->json);
                module->json = parameters_tmp;
            }

            g_object_unref(parser);
            g_free(input_json_parameters);
        }
    }
    else
    {
        for (int i = 4; i < *argc; ++i)
        {
            gchar **data = g_strsplit_set(argv[i], "=", 2);
            if (parse_parameter_argument(module,
                                         data[0],
                                         data[1],
                                         command == SERVICES_CONFIGURE && !g_utf8_strlen(data[1], -1))
                < 0)
            {
                g_strfreev(data);
                ERR_EXIT();
            }

            g_strfreev(data);
        }
    }

    const gchar *validation_contexts[] = {context};
    if (services_fill_missing_passwords(module, service_name, validation_contexts, G_N_ELEMENTS(validation_contexts))
        < 0)
        ERR_EXIT();

    if ((ret = validate_json(module,
                             service_name,
                             validation_contexts,
                             G_N_ELEMENTS(validation_contexts),
                             command != SERVICES_CONFIGURE ? FALSE : TRUE)))
        goto end;

    g_variant_unref(ctx->parameters);

    gchar *input_json_parameters = services_module_json_params_to_text(module->json, service_name);
    gchar *tmp                   = input_json_parameters;
    input_json_parameters        = g_strconcat(input_json_parameters, "\n", NULL);
    g_free(tmp);

    ctx->parameters = g_variant_new("(msms)", service_name, input_json_parameters);
    g_free(input_json_parameters);

end:
    if (option_context)
        g_option_context_free(option_context);

    if (contextual_params)
        g_option_group_unref(contextual_params);

    if (params_options)
        g_string_free(params_options, TRUE);

    if (defaults)
        json_node_free(defaults);

    if (current_defaults)
        json_node_free(current_defaults);

    if (defaults_filtered)
        json_node_free(defaults_filtered);

    if (current_defaults_filtered)
        json_node_free(current_defaults_filtered);

    if (service_status_ctx)
        alteratorctl_ctx_free(service_status_ctx);

    return ret;
}

// TODO: Apply wherever there is similar logic!
static gchar *make_description_readable(gchar *oririnal, gsize margin_from_border, gsize max_length)
{
    if (!oririnal || !strlen(oririnal))
        return NULL;

    GString *result       = g_string_new(NULL);
    gchar **words         = g_strsplit(oririnal, " ", -1);
    gsize current_row_len = margin_from_border;
    for (gsize w = 0; g_strv_length(words); w++)
    {
        gchar *word = words[w];
        if (!word)
            break;

        if (streq(word, "  "))
            continue;

        gsize possible_length = current_row_len + strlen(" ") + g_utf8_strlen(word, -1);
        if (possible_length == max_length)
        {
            g_string_append_printf(result, " %s\n", word);
            current_row_len = margin_from_border;
        }
        else if (possible_length > max_length)
        {
            gchar *margin_str = g_strnfill(margin_from_border, ' ');
            g_string_append_printf(result, "\n%s%s", margin_str, word);
            current_row_len = margin_from_border + g_utf8_strlen(word, -1);
            g_free(margin_str);
        }
        else
        {
            if (current_row_len == margin_from_border && result->len != 0)
            {
                gchar *margin_str = g_strnfill(margin_from_border, ' ');
                g_string_append_printf(result, "%s%s", margin_str, word);
                g_free(margin_str);
            }
            else if (result->len != 0)
                g_string_append_printf(result, " %s", word);
            else
                g_string_append(result, word);
            current_row_len += strlen(" ") + g_utf8_strlen(word, -1);
        }
    }
    g_strfreev(words);

    return g_string_free(result, FALSE);
}

static int services_module_create_command_usage_help(const gchar *service_name,
                                                     const gchar *context,
                                                     GArray *options,
                                                     GString **result)
{
    int ret                 = 0;
    GString *options_string = g_string_new("");
    if (!options)
    {
        g_printerr(_("Failed to create service \"%s %s\" usage help. Empty options data.\n"), service_name, context);
        ERR_EXIT();
    }

    if (!result)
    {
        g_printerr(_("Failed to create service \"%s %s\" usage help. Nowhere to save the result.\n"),
                   service_name,
                   context);
        ERR_EXIT();
    }

    g_array_sort(options, services_module_sort_options);

    for (gsize i = 0; i < options->len; i++)
    {
        ParamEntry param_entry = ((ParamEntry *) options->data)[i];
        if (param_entry.is_constant)
            continue;

        if (streq(param_entry.option_entry.long_name, "force-deploy"))
            continue;

        GOptionEntry entry           = param_entry.option_entry;
        const gchar *path            = entry.long_name;
        const gchar *arg_description = entry.arg_description;
        const gchar *description     = g_strdup(entry.description);

        { // Append new data in description
            const gchar *default_value_str = param_entry.default_value_str;
            gsize min_size                 = param_entry.min_array_size;
            gsize max_size                 = param_entry.max_array_size;
            GString *new_description       = g_string_new(NULL);

            if (min_size > 0)
                g_string_append_printf(new_description, _("Minimum number of array elements: %lu\n"), min_size);

            if (max_size != 0 && max_size != INT_MAX)
                g_string_append_printf(new_description, _("Maximum number of array elements: %lu\n"), max_size);

            if (default_value_str && g_utf8_strlen(default_value_str, -1))
                g_string_append_printf(new_description, _("Default value: %s\n"), default_value_str);
            g_string_append(new_description, description);
            g_free((gchar *) description);
            description = g_string_free(new_description, FALSE);
        }

        gchar *header       = g_strdup_printf("  --%s=%s", path, arg_description);
        gsize header_length = g_utf8_strlen(header, -1);
        g_string_append(options_string, header);

        if (header_length >= SERVICES_MODULE_MAX_USAGE_HELP_DESCRIPTIONS_MARGIN)
        {
            g_string_append_c(options_string, '\n');
            gchar *margin = g_strnfill(SERVICES_MODULE_MAX_USAGE_HELP_DESCRIPTIONS_MARGIN - 1, ' ');
            g_string_append(options_string, margin);
            g_free(margin);
        }
        else
        {
            gchar *margin_str = g_strnfill(SERVICES_MODULE_MAX_USAGE_HELP_DESCRIPTIONS_MARGIN - header_length - 1, ' ');
            g_string_append(options_string, margin_str);
            g_free(margin_str);
        }

        gchar **description_parts = g_strsplit(description, "\n", -1);
        for (gsize part_idx = 0; part_idx < g_strv_length(description_parts); part_idx++)
        {
            gchar **words = g_strsplit(description_parts[part_idx], " ", -1);

            gsize current_col = (header_length >= SERVICES_MODULE_MAX_USAGE_HELP_DESCRIPTIONS_MARGIN)
                                    ? SERVICES_MODULE_MAX_USAGE_HELP_DESCRIPTIONS_MARGIN
                                    : SERVICES_MODULE_MAX_USAGE_HELP_DESCRIPTIONS_MARGIN;

            // Have a comment
            if (part_idx > 0)
            {
                gchar *margin_str = g_strnfill(SERVICES_MODULE_MAX_USAGE_HELP_DESCRIPTIONS_MARGIN - 1, ' ');
                g_string_append(options_string, margin_str);
                g_free(margin_str);
            }

            for (gsize w = 0; g_strv_length(words); w++)
            {
                if (!words[w])
                    break;

                gsize word_length = g_utf8_strlen(words[w], -1);
                if (streq(words[w], "  "))
                    continue;

                gsize break_line_idx = (options_string->len && options_string->str[options_string->len - 1] != '\n')
                                           ? 1
                                           : 0;

                if (current_col + break_line_idx + word_length > SERVICES_MODULE_MAX_USAGE_HELP_STRING_LENGTH)
                {
                    g_string_append_c(options_string, '\n');
                    gchar *margin = g_strnfill(SERVICES_MODULE_MAX_USAGE_HELP_DESCRIPTIONS_MARGIN, ' ');
                    g_string_append(options_string, margin);
                    g_free(margin);
                    current_col    = SERVICES_MODULE_MAX_USAGE_HELP_DESCRIPTIONS_MARGIN;
                    break_line_idx = 0;
                }

                if (break_line_idx)
                    g_string_append_c(options_string, ' ');
                g_string_append(options_string, words[w]);
                current_col += break_line_idx + word_length;
            }
            if (i != options->len - 1)
                g_string_append_c(options_string, '\n');
            else if (part_idx != g_strv_length(description_parts) - 1)
                g_string_append_c(options_string, '\n');
            g_strfreev(words);
        }
        g_strfreev(description_parts);

        g_free(header);
    }

    *result = g_string_new("");
    g_string_append(*result, _("Usage:\n"));
    if (streq(context, "diagnose")) // TODO: `service_tests` NULL-guard
    {
        g_string_append_printf(*result,
                               _("  alteratorctl diagnose %s\n"
                                 "                              (<diagnostic tool> [test...])...\n"
                                 "                              [PARAMETERS [arguments]]\n\n"),
                               service_name);

        g_string_append_printf(*result, _("Diagnostics tools of %s service:\n"), service_name);

        // Printing tools with tests section
        gchar *margin_from_border = g_strnfill(SERVICES_MODULE_MAX_USAGE_HELP_DESCRIPTIONS_MARGIN, ' ');
        GHashTableIter tools_iter;
        g_hash_table_iter_init(&tools_iter, service_tests);
        gchar *tool_name            = NULL;
        diag_tool_tests *tool_tests = NULL;
        while (g_hash_table_iter_next(&tools_iter, (gpointer) &tool_name, (gpointer) &tool_tests))
        {
            g_string_append_printf(*result, "  %s\n", tool_name);
            g_string_append_printf(*result,
                                   _("  %s diagnostics tests in %s bus:\n"),
                                   tool_name,
                                   tool_tests->bus_type == G_BUS_TYPE_SYSTEM ? _("system") : _("serssion"));

            GPtrArray *test_names = g_hash_table_get_keys_as_ptr_array(tool_tests->all_tests);
            g_ptr_array_sort(test_names, services_module_sort_result);
            for (gsize i = 0; i < test_names->len; i++)
            {
                gchar *test_name               = ((gchar **) test_names->pdata)[i];
                diag_tool_test_info *tool_test = g_hash_table_lookup(tool_tests->all_tests, test_name);

                gchar *margin_from_test = NULL;
                {
                    gint64 margin_from_test_length = SERVICES_MODULE_MAX_USAGE_HELP_DESCRIPTIONS_MARGIN - strlen("    ")
                                                     - strlen(test_name);

                    if (margin_from_test_length > 0)
                        margin_from_test = g_strnfill(margin_from_test_length, ' ');
                    else
                    {
                        gchar *tmp       = g_strnfill(SERVICES_MODULE_MAX_USAGE_HELP_DESCRIPTIONS_MARGIN, ' ');
                        margin_from_test = g_strconcat("\n", tmp, NULL);
                        g_free(tmp);
                    }
                }
                gchar *formated_display_name
                    = make_description_readable(tool_test->display_name,
                                                SERVICES_MODULE_MAX_USAGE_HELP_DESCRIPTIONS_MARGIN,
                                                SERVICES_MODULE_MAX_USAGE_HELP_STRING_LENGTH);
                g_string_append_printf(*result, "    %s%s%s\n", test_name, margin_from_test, formated_display_name);
                g_free(formated_display_name);
                if (tool_test->description)
                {
                    gchar *formated_description
                        = make_description_readable(tool_test->description,
                                                    SERVICES_MODULE_MAX_USAGE_HELP_DESCRIPTIONS_MARGIN,
                                                    SERVICES_MODULE_MAX_USAGE_HELP_STRING_LENGTH);
                    g_string_append_printf(*result, "%s%s\n", margin_from_border, formated_description);
                    g_free(formated_description);
                }
                gchar *avaliable_modes = g_strconcat(tool_test->is_required ? _("Required") : _("Optional"),
                                                     ". ",
                                                     tool_test->mode == PRE_DIAG_MODE
                                                         ? _("Can be run in pre-diagnostic mode\n")
                                                         : (tool_test->mode == POST_DIAG_MODE
                                                                ? _("Can be run in post-diagnostic mode\n")
                                                                : _("Can be run in pre- and post-diagnostic modes\n")),
                                                     NULL);
                gchar *formated_additional_info
                    = make_description_readable(avaliable_modes,
                                                SERVICES_MODULE_MAX_USAGE_HELP_DESCRIPTIONS_MARGIN,
                                                SERVICES_MODULE_MAX_USAGE_HELP_STRING_LENGTH);
                g_string_append_printf(*result, "%s%s\n", margin_from_border, formated_additional_info);
                g_free(avaliable_modes);
            }
            g_ptr_array_unref(test_names);
        }
        g_free(margin_from_border);
    }
    else
        g_string_append_printf(*result, _("  alteratorctl %s [OPTIONS] [PARAMETERS [arguments]]\n\n"), context);

    if (options_string->len)
    {
        g_string_append_printf(*result, _("Parameters of \"%s\" service:\n"), service_name);
        g_string_append_printf(*result, "%s\n", options_string->str);
    }

    g_string_append(*result, _("Options:\n"));
    if (streq(context, "diagnose"))
        g_string_append(*result,
                        _("  -a, --all                   Run all diagnostics tests in current mode\n"
                          "                              (pre- or postdiagnostic)\n"));
    for (gsize i = 0; i < options->len; i++)
        if (streq(context, "deploy") && streq(((ParamEntry *) options->data)[i].option_entry.long_name, "force-deploy"))
            g_string_append(*result, _("  -f, --force-deploy          Force deploy service\n"));
    g_string_append(*result, _("  -y, --yes                   Assume Yes to all queries and do not prompt\n"));
    g_string_append(*result, _("  --no-default                Do not use default value if parameter is not set\n"));
    g_string_append_printf(*result,
                           _("  -h, --help                  Show \"%s\" command of\n"
                             "                              \"%s\" service usage help\n"),
                           context,
                           service_name);
    if (streq(context, "configure"))
    {
        g_string_append_c(*result, '\n');
        g_string_append(*result,
                        _("If you want to reset an optional parameter that has been set, simply omit it's value:\n"));
        g_string_append(*result, "  alteratorctl services configure <deployed service> --param=\n");
    }

end:
    g_string_free(options_string, TRUE);

    return ret;
}

static GOptionEntry *g_param_option_entry_dup(GOptionEntry *entry)
{
    if (!entry)
        return NULL;
    GOptionEntry *result    = g_new0(GOptionEntry, 1);
    result->long_name       = g_strdup(entry->long_name);
    result->description     = g_strdup(entry->description);
    result->arg_description = g_strdup(entry->arg_description);
    return result;
}

static void g_param_option_entry_free(GOptionEntry *entry)
{
    if (!entry)
        return;
    g_free((gchar *) entry->long_name);
    g_free((gchar *) entry->description);
    g_free((gchar *) entry->arg_description);
    g_free(entry);
}

static int services_module_confirm_execution(AlteratorCtlServicesModule *module,
                                             alteratorctl_ctx_t **ctx,
                                             gint selected_subcommand,
                                             const gchar *command_name,
                                             const gchar *service_name,
                                             gchar **argv,
                                             gint actual_argc,
                                             GArray *options,
                                             gboolean *is_accepted,
                                             gboolean *yes_flag)
{
    int ret                              = 0;
    GHashTable *parameters_output_data   = NULL;
    GHashTable *required_params_paths    = NULL;
    GHashTable *long_name_to_description = NULL;
    GString *options_line                = NULL;
    GString *output_str                  = NULL;
    GString *header_text                 = NULL;
    GString *display_text                = NULL;
    gboolean assume_yes                  = (yes_flag && *yes_flag) ? TRUE : FALSE;

    if (!module || !ctx || !*ctx || !command_name || !service_name || !is_accepted || !yes_flag)
    {
        g_printerr(_("Internal error: invalid arguments for services confirmation.\n"));
        ERR_EXIT();
    }

    AlteratorCtlModuleInfoParser *info_parser = module->gdbus_source->info_parser;
    if (!info_parser)
    {
        g_printerr(_("Internal error: services info parser is not initialized.\n"));
        ERR_EXIT();
    }

    GNode *parameters_data = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
        info_parser, module->info, SERVICES_ALTERATOR_ENTRY_SERVICE_PARAMETERS_TABLE_NAME);
    if (!parameters_data)
    {
        g_printerr(_("Error processing service %s parameters: empty service info.\n"), service_name);
        ERR_EXIT();
    }

    if (!module->json || json_node_get_node_type(module->json) != JSON_NODE_OBJECT)
    {
        g_printerr(_("Invalid input json data.\n"));
        ERR_EXIT();
    }

    JsonNode *parameters_node     = module->json;
    JsonObject *parameters_object = json_node_get_object(parameters_node);

    if (services_module_status_params_build_output(module,
                                                   (gchar *) service_name,
                                                   parameters_object,
                                                   parameters_data,
                                                   NULL,
                                                   &parameters_output_data)
        < 0)
        ERR_EXIT();

    if (options && options->len)
    {
        long_name_to_description = g_hash_table_new_full(g_str_hash,
                                                         g_str_equal,
                                                         NULL,
                                                         (GDestroyNotify) g_param_option_entry_free);
        for (gsize i = 0; i < options->len; i++)
        {
            GOptionEntry entry = ((ParamEntry *) options->data)[i].option_entry;
            g_hash_table_insert(long_name_to_description, (gchar *) entry.long_name, g_param_option_entry_dup(&entry));
        }
    }

    if (services_module_get_required_parameters_paths(module,
                                                      service_name,
                                                      get_context_from_command_id(selected_subcommand),
                                                      &required_params_paths)
        < 0)
        ERR_EXIT();

    gboolean is_missing_required = FALSE;
    if (services_module_create_params_table(parameters_output_data,
                                            long_name_to_description,
                                            required_params_paths,
                                            &is_missing_required,
                                            &output_str)
        < 0)
        ERR_EXIT();

    if (services_module_find_conflicting_resources_within_current_service(module, service_name, parameters_node) < 0)
        ERR_EXIT();

    if (selected_subcommand == SERVICES_CONFIGURE || selected_subcommand == SERVICES_DEPLOY)
    {
        g_print(_("Checking resources conflicts...\n"));
        if (services_module_find_conflicting_services(module, *ctx) < 0)
            ERR_EXIT();
    }

    options_line = g_string_new("");
    if (argv && actual_argc > 4)
        for (gint i = 4; i < actual_argc; i++)
            g_string_append_printf(options_line, " %s", argv[i]);
    const gchar *options_suffix = options_line->len ? options_line->str : "";

    header_text         = g_string_new(NULL);
    display_text        = g_string_new(NULL);
    gchar response_char = assume_yes ? 'y' : 'N';

    if (!is_missing_required && !assume_yes)
    {
        if (output_str)
            g_string_append_printf(header_text,
                                   _("Continue executing \"%s %s%s\" with these parameters?\n\n"),
                                   command_name,
                                   service_name,
                                   options_suffix);
        else
            g_string_append_printf(header_text,
                                   _("Continue executing \"%s %s%s\"?\n"),
                                   command_name,
                                   service_name,
                                   options_suffix);
    }
    else if (is_missing_required)
    {
        g_printerr(_("Check the specified parameters for the %s service. Required parameters that are not set "
                     "are marked with \"*\" and must be provided when running the %s command.\n%s\n"),
                   service_name,
                   command_name,
                   output_str ? output_str->str : "");
        ERR_EXIT();
    }

    g_string_append(display_text, header_text->str);
    if (output_str)
        g_string_append(display_text, output_str->str);

    if (!assume_yes)
    {
        g_string_append(display_text, _("Do you want to continue? [y/N] "));
        g_print("%s", display_text->str);

        gchar is_accept_symbol = 'N';
        if (isatty_safe(STDOUT_FILENO))
        {
            if (!isatty_safe(STDIN_FILENO))
            {
                FILE *tty = fopen("/dev/tty", "r");
                if (tty)
                {
                    is_accept_symbol = getc(tty);
                    g_print("\n");
                    fclose(tty);
                }
            }
            else
            {
                if (scanf("%c", &is_accept_symbol) != 1)
                    is_accept_symbol = 'N';
                else if (is_accept_symbol == '\n')
                    is_accept_symbol = 'N';
                else
                {
                    int flush;
                    while ((flush = getchar()) != '\n' && flush != EOF)
                        ;
                }
                g_print("\n");
            }
        }
        else
            g_print(_("To confirm the operation, repeat the input with the --yes flag.\n"));

        response_char = is_accept_symbol;
        g_string_append_c(display_text, response_char);
        g_string_append_c(display_text, '\n');
    }
    else
    {
        g_string_append(display_text, _("Do you want to continue? [y/N] y\n"));
        g_print("%s", display_text->str);
    }

    gboolean accepted = assume_yes;
    if (!assume_yes)
        accepted = (response_char == 'Y' || response_char == 'y');

    if (accepted)
        *is_accepted = TRUE;
    else
    {
        *is_accepted = FALSE;
        g_print(_("Aborted.\n\n"));
    }

end:
    if (display_text)
        g_string_free(display_text, TRUE);
    if (header_text)
        g_string_free(header_text, TRUE);
    if (options_line)
        g_string_free(options_line, TRUE);
    if (output_str)
        g_string_free(output_str, TRUE);
    if (long_name_to_description)
        g_hash_table_destroy(long_name_to_description);
    if (parameters_output_data)
        g_hash_table_destroy(parameters_output_data);
    if (required_params_paths)
        g_hash_table_destroy(required_params_paths);

    return ret;
}

static int services_module_parse_arguments(
    AlteratorCtlServicesModule *module, int argc, char **argv, alteratorctl_ctx_t **ctx, gboolean *is_accepted)
{
    int ret                                   = 0;
    GString *command_help                     = NULL;
    gchar *input                              = NULL;
    gchar *play_raw_json                      = NULL;
    gchar *play_json_text_parameters          = NULL;
    const gchar *play_service_from_json       = NULL;
    AlteratorCtlModuleInterface *iface        = GET_ALTERATOR_CTL_MODULE_INTERFACE((void *) module);
    AlteratorCtlModuleInfoParser *info_parser = module->gdbus_source->info_parser;
    GArray *options                           = NULL;
    JsonParser *play_parser                   = NULL;

    gboolean help = module->alterator_ctl_app->arguments->module_help;

    if (!iface)
    {
        g_printerr(_("Internal error in services module while parsing arguments: *iface is NULL.\n"));
        ERR_EXIT();
    }

    if (argc < 3 && help)
    {
        iface->print_help(module);
        *is_accepted = TRUE;
        goto end;
    }

    if (services_module_parse_options(module, &argc, argv) < 0)
        ERR_EXIT();

    // -1 - default command
    int selected_subcommand = iface->get_command_id(module->commands, argv[2]);

    if (selected_subcommand == -1)
    {
        if (argc == 2)
            selected_subcommand = SERVICES_LIST;
        else
        {
            g_printerr(_("Unknown services module command.\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
    }

    gboolean command_in_context = FALSE;

    // Special handling for "play" command: read JSON plan and map to deploy
    if (selected_subcommand == SERVICES_PLAY)
    {
        if (help)
        {
            g_print(_("Usage: services play <file|->\n"));
            g_print(_("  Execute action from JSON file\n\n"));
            *is_accepted = TRUE;
            goto end;
        }

        if (argc < 4)
        {
            g_printerr(_("No input file specified.\n"));
            ERR_EXIT();
        }

        gchar *filepath               = argv[3];
        JsonNode *root                = NULL;
        JsonObject *root_obj          = NULL;
        JsonNode *svc_node            = NULL; // service
        JsonNode *act_node            = NULL; // action
        JsonNode *vers_node           = NULL; // version
        JsonNode *params_node         = NULL; // parameters
        JsonNode *options_node        = NULL; // options
        const gchar *action_from_json = NULL;
        gint playfile_version         = 1;

        // read JSON from file or stdin
        play_raw_json = streq(filepath, "-") ? services_module_parse_stdin_param(TRUE) : read_file(filepath);
        if (!play_raw_json)
        {
            g_printerr(_("Invalid or empty input json data.\n"));
            ERR_EXIT();
        }

        play_parser = json_parser_new();
        if (!json_parser_load_from_data(play_parser, play_raw_json, -1, NULL))
        {
            g_printerr(_("Invalid input json data.\n"));
            ERR_EXIT();
        }

        root = json_parser_get_root(play_parser);
        if (!root || json_node_get_node_type(root) != JSON_NODE_OBJECT)
        {
            g_printerr(_("Invalid input json data. Object is expected.\n"));
            ERR_EXIT();
        }

        root_obj     = json_node_get_object(root);
        svc_node     = json_object_get_member(root_obj, "service");
        act_node     = json_object_get_member(root_obj, "action");
        vers_node    = json_object_get_member(root_obj, "version");
        params_node  = json_object_get_member(root_obj, "parameters");
        options_node = json_object_get_member(root_obj, "options");

        if (!svc_node || json_node_get_value_type(svc_node) != G_TYPE_STRING)
        {
            g_printerr(_("Invalid input json data. Missing or invalid 'service'.\n"));
            ERR_EXIT();
        }
        if (!act_node || json_node_get_value_type(act_node) != G_TYPE_STRING)
        {
            if (json_object_has_member(root_obj, "context"))
                g_printerr(_("This playfile uses outdated file format that's unsupported. Please export the playfile "
                             "from a newer version of Alt-Services.\n"));
            else
                g_printerr(_("Invalid input json data. Missing or invalid 'action'.\n"));
            ERR_EXIT();
        }
        if (vers_node && json_node_get_value_type(vers_node) != G_TYPE_INT64)
        {
            g_printerr(_("Invalid input json data. Invalid 'version'.\n"));
            ERR_EXIT();
        }
        if (vers_node)
        {
            playfile_version = (gint) json_node_get_int(vers_node);
            if (playfile_version != 1)
            {
                g_printerr(_("Unsupported playfile version: %d.\n"), playfile_version);
                ERR_EXIT();
            }
        }
        if (!params_node || json_node_get_node_type(params_node) != JSON_NODE_OBJECT)
        {
            g_printerr(_("Invalid input json data. Missing or invalid 'parameters'.\n"));
            ERR_EXIT();
        }

        play_service_from_json = json_node_get_string(svc_node);
        action_from_json       = json_node_get_string(act_node);

        selected_subcommand = get_command_id_from_context(action_from_json);
        if (selected_subcommand == -1)
        {
            g_printerr(_("Unsupported action '%s' in services play. Only the following actions are supported: "
                         "'deploy', 'undeploy', 'configure', 'backup' and 'restore'.\n"),
                       action_from_json ? action_from_json : "");
            ERR_EXIT();
        }

        // Load service info for validation
        if (!module->info
            && info_parser->alterator_ctl_module_info_parser_get_specified_object_data(info_parser,
                                                                                       module->gdbus_source,
                                                                                       play_service_from_json,
                                                                                       SERVICES_INTERFACE_NAME,
                                                                                       &module->info)
                   < 0)
        {
            g_printerr(_("Failed of parsing service \"%s\" info.\n"), play_service_from_json);
            ERR_EXIT();
        }

        // Build parameters JSON, fill required passwords, then validate
        if (module->json)
        {
            json_node_free(module->json);
            module->json = NULL;
        }
        module->json            = json_node_copy(params_node);
        JsonObject *options_obj = (options_node && json_node_get_node_type(options_node) == JSON_NODE_OBJECT)
                                      ? json_node_get_object(options_node)
                                      : NULL;
        if (services_module_play_prepare_plan(module, play_service_from_json, selected_subcommand, options_obj) < 0)
            ERR_EXIT();
        {
            const gchar *context_for_validation = get_context_from_command_id(selected_subcommand);
            const gchar *validation_contexts[2];
            gsize validation_contexts_len                  = 0;
            validation_contexts[validation_contexts_len++] = context_for_validation;
            if (services_play_plan_requires_diag(module->play_plan))
                validation_contexts[validation_contexts_len++] = SERVICES_CONTEXT_DIAG;

            if (services_fill_missing_passwords(module,
                                                play_service_from_json,
                                                validation_contexts,
                                                validation_contexts_len)
                < 0)
                ERR_EXIT();
            if (validate_json(module, play_service_from_json, validation_contexts, validation_contexts_len, TRUE) < 0)
                ERR_EXIT();
        }

        // Stringify parameters JSON (with passwords injected)
        play_json_text_parameters = services_module_json_params_to_text(module->json, play_service_from_json);
        if (!play_json_text_parameters)
            ERR_EXIT();

        // Map play action to command and create context
        *ctx = alteratorctl_ctx_init_services(selected_subcommand,
                                              play_service_from_json,
                                              play_json_text_parameters,
                                              NULL,
                                              NULL);
        if (!*ctx)
            ERR_EXIT();

        if (services_module_confirm_execution(module,
                                              ctx,
                                              selected_subcommand,
                                              services_module_subcommands_list[selected_subcommand].subcommand,
                                              play_service_from_json,
                                              argv,
                                              argc,
                                              options,
                                              is_accepted,
                                              &yes)
            < 0)
            ERR_EXIT();

        if (!(*is_accepted))
            goto end;

        // cleanup temporary buffers we own
        g_clear_pointer(&play_raw_json, g_free);
        g_clear_pointer(&play_json_text_parameters, g_free);

        goto end;
    }

    switch (selected_subcommand)
    {
    case SERVICES_DEPLOY:
    case SERVICES_UNDEPLOY:
    case SERVICES_CONFIGURE:
    case SERVICES_BACKUP:
    case SERVICES_RESTORE:
    case SERVICES_DIAGNOSE:
        command_in_context = TRUE;
        break;
    default:
        break;
    }

    gchar *service = NULL;

    if (selected_subcommand == SERVICES_LIST)
    {
        if (help)
        {
            g_print(_("List services\n"));
            g_print(_("Usage:\n  services list\n"));
            goto end;
        }

        if (argc > 3)
        {
            g_printerr(_("Unknown arguments for list command.\n\n"));
            iface->print_help(module);
            ERR_EXIT();
        }
    }
    else
    {
        if (help)
        {
            switch (selected_subcommand)
            {
            case SERVICES_INFO:
                g_print(_("Get info of specified service\n"));
                g_print(_("Usage:\n  services info <service>\n"));
                goto end;
            case SERVICES_RESOURCES:
                g_print(_("Show resources of specified service\n"));
                g_print(_("Usage:\n  services resources <service>\n"));
                goto end;
            case SERVICES_STATUS:
                g_print(_("Get current state and parameters of specified service\n"));
                g_print(_("Usage:\n  services status <service>\n"));
                goto end;
            case SERVICES_START:
                g_print(_("Start specified service\n"));
                g_print(_("Usage:\n  services start <service>\n"));
                goto end;
            case SERVICES_STOP:
                g_print(_("Stop specified service\n"));
                g_print(_("Usage:\n  services stop <service>\n"));
                goto end;
            default:
                break;
            }
        }

        if (argc >= 4)
            service = argv[3];
        else if (command_in_context || selected_subcommand == SERVICES_INFO || selected_subcommand == SERVICES_STATUS
                 || selected_subcommand == SERVICES_START || selected_subcommand == SERVICES_STOP
                 || selected_subcommand == SERVICES_RESOURCES)
        {
            g_printerr(_("No service specified.\n"));
            ERR_EXIT();
        }

        { // Checking required state
            if (!help)
            {
                deployment_status deploy_status = UNDEPLOYED;
                if (services_module_is_deployed(module, service, &deploy_status) < 0)
                    ERR_EXIT();

                switch (selected_subcommand)
                {
                case SERVICES_DEPLOY:
                    if (!force_deploy && deploy_status != UNDEPLOYED)
                    {
                        g_printerr(_("This command can not be used on a deployed service.\n"));
                        ERR_EXIT();
                    }
                    break;
                case SERVICES_UNDEPLOY:
                case SERVICES_CONFIGURE:
                case SERVICES_BACKUP:
                case SERVICES_RESTORE:
                    if (deploy_status == UNDEPLOYED)
                    {
                        g_printerr(_("This command can only be used on a deployed service.\n"));
                        ERR_EXIT();
                    }
                    break;
                default:
                    break;
                };
            }
        }

        if (command_in_context)
        {
            if (!module->info
                && info_parser->alterator_ctl_module_info_parser_get_specified_object_data(info_parser,
                                                                                           module->gdbus_source,
                                                                                           service,
                                                                                           SERVICES_INTERFACE_NAME,
                                                                                           &module->info)
                       < 0)
            {
                g_printerr(_("Failed of parsing service \"%s\" info.\n"), service);
                ERR_EXIT();
            }

            if (selected_subcommand == SERVICES_DIAGNOSE)
            {
                if (!help && !run_all_tests)
                {
                    // Create argv without params, only tools and tests
                    gint tools_tests_argc                  = 0;
                    GStrvBuilder *tools_tests_argv_builder = g_strv_builder_new();
                    for (gsize i = 0; i < argc; i++)
                    {
                        if (argv[i][0] == '-')
                            break;
                        g_strv_builder_add(tools_tests_argv_builder, argv[i]);
                        tools_tests_argc++;
                    }
                    gchar **tools_tests_argv = g_strv_builder_unref_to_strv(tools_tests_argv_builder);
                    if (services_module_parse_diagnose_arguments(module, tools_tests_argc, tools_tests_argv, &input) < 0)
                    {
                        g_strfreev(tools_tests_argv);
                        ERR_EXIT();
                    }
                    g_strfreev(tools_tests_argv);
                }
                else if (services_module_parse_diagnose_arguments(module, argc, argv, &input) < 0)
                    ERR_EXIT();
            }

            if (!input)
                input = services_module_get_parameters(selected_subcommand == SERVICES_CONFIGURE);
        }

        else if (selected_subcommand == SERVICES_STATUS)
        {
            if (!module->info
                && info_parser->alterator_ctl_module_info_parser_get_specified_object_data(info_parser,
                                                                                           module->gdbus_source,
                                                                                           service,
                                                                                           SERVICES_INTERFACE_NAME,
                                                                                           &module->info))
            {
                g_printerr(_("Failed to execute \"%s\" status. Error getting service alterator entry info.\n"), service);
                ERR_EXIT();
            }
        }
    }

    *ctx = alteratorctl_ctx_init_services(selected_subcommand, service, input, NULL, NULL);
    if (command_in_context)
    {
        gint actual_argc = argc;
        if (!help && selected_subcommand == SERVICES_DIAGNOSE)
        {
            // Create argv without tools and tests, only params
            actual_argc                            = 0;
            GStrvBuilder *tools_tests_argv_builder = g_strv_builder_new();
            gboolean has_parameters                = FALSE;
            for (gsize i = 0; i < argc; i++)
            {
                has_parameters = argv[i][0] == '-' && !has_parameters ? TRUE : has_parameters;
                if (i > 3 && argv[i][0] != '-')
                {
                    if (!has_parameters)
                        continue;
                    else
                    {
                        g_printerr(_("\"%s\" is not a parameter of diagnostic tests.\n"), argv[i]);
                        g_strv_builder_unref(tools_tests_argv_builder);
                        ERR_EXIT();
                    }
                }

                g_strv_builder_add(tools_tests_argv_builder, argv[i]);
                actual_argc++;
            }
            gchar **tools_tests_argv = g_strv_builder_unref_to_strv(tools_tests_argv_builder);
            if (services_module_parse_contextual_arguments(
                    module, service, selected_subcommand, &actual_argc, tools_tests_argv, *ctx, &options)
                < 0)
            {
                g_strfreev(tools_tests_argv);
                ERR_EXIT();
            }
            g_strfreev(tools_tests_argv);
        }
        else if (services_module_parse_contextual_arguments(
                     module, service, selected_subcommand, &actual_argc, argv, *ctx, &options)
                 < 0)
            ERR_EXIT();

        if (help)
        {
            if (services_module_create_command_usage_help(service, argv[2], options, &command_help) < 0)
                ERR_EXIT();
            g_print("%s\n", command_help->str);
            goto end;
        }

        if (services_module_confirm_execution(
                module, ctx, selected_subcommand, argv[2], service, argv, actual_argc, options, is_accepted, &yes)
            < 0)
            ERR_EXIT();

        goto end;
    }
    else
        *is_accepted = TRUE;

end:
    if (command_help)
        g_string_free(command_help, TRUE);

    g_free(input);
    g_free(play_raw_json);
    g_free(play_json_text_parameters);
    if (play_parser)
        g_object_unref(play_parser);

    if (options)
        g_array_free(options, TRUE);

    return ret;
}

int services_module_run(gpointer self, gpointer data)
{
    int ret                            = 0;
    AlteratorCtlModuleInterface *iface = GET_ALTERATOR_CTL_MODULE_INTERFACE(self);
    AlteratorCtlServicesModule *module = ALTERATOR_CTL_SERVICES_MODULE(self);

    if (module->alterator_ctl_app->arguments->module_help)
        goto end;

    if (!self || !data)
    {
        g_printerr(_("Internal error in services module run: *module or *context is NULL.\n"));
        ERR_EXIT();
    }

    alteratorctl_ctx_t *ctx = (alteratorctl_ctx_t *) data;

    int command_id = g_variant_get_int32(ctx->subcommands_ids);
    switch (command_id)
    {
    case SERVICES_LIST:
        if (services_module_list_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;

    case SERVICES_INFO:
        if (services_module_info_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;

    case SERVICES_RESOURCES:
        if (services_module_resources_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;

    case SERVICES_STATUS:
        if (services_module_status_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;

    case SERVICES_START:
        if (services_module_start_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;

    case SERVICES_STOP:
        if (services_module_stop_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;

    case SERVICES_DEPLOY:
    case SERVICES_CONFIGURE:
    case SERVICES_UNDEPLOY:
    case SERVICES_BACKUP:
    case SERVICES_RESTORE: {
        gboolean with_signals     = command_id == SERVICES_DEPLOY || command_id == SERVICES_UNDEPLOY;
        int required_deploy_state = command_id == SERVICES_DEPLOY ? (force_deploy ? -1 : 0) : 1;

        if (services_module_parametrized_subcommand(module, &ctx, command_id, with_signals, required_deploy_state) < 0)
            ERR_EXIT();
        break;
    }

    case SERVICES_DIAGNOSE:
        if (services_module_diagnose_subcommand(module, &ctx) < 0)
            ERR_EXIT();
        break;

    default:
        g_printerr(_("Unknown services module command.\n"));
        ERR_EXIT();
    }

end:
    return ret;
}

/*
 * Executes a corresponding DBus method, passing json parameters.
 *  required_deploy_state:
 *    > -1 - do not check
 *    >  0 - service should be undeployed
 *    >  1 - service should be deployed
 */
static int services_module_parametrized_subcommand(AlteratorCtlServicesModule *module,
                                                   alteratorctl_ctx_t **ctx,
                                                   int command,
                                                   gboolean with_signals,
                                                   int required_deploy_state)
{
    int ret                                = 0;
    gchar *service_str_id                  = NULL;
    gchar *input_json_parameters           = NULL;
    dbus_ctx_t *dbus_ctx                   = NULL;
    GError *dbus_call_error                = NULL;
    JsonParser *parser                     = NULL;
    alteratorctl_ctx_t *service_status_ctx = NULL;
    GVariant *exit_code                    = NULL;
    GPtrArray *signals                     = NULL;
    subscribe_signals_t *stdout_signal     = NULL;
    subscribe_signals_t *stderr_signal     = NULL;

    if (!module)
    {
        g_printerr(_("Internal error: Module doesn't exist.\n"));
        ERR_EXIT();
    }

    g_variant_get((*ctx)->parameters, "(msms)", &service_str_id, &input_json_parameters);
    if (!service_str_id)
    {
        g_printerr(_("Service is not specified.\n"));
        ERR_EXIT();
    }

    ServicesPlayPlan *play_plan      = NULL;
    gboolean effective_force         = force_deploy;
    int effective_required_state     = required_deploy_state;
    deployment_status current_status = UNDEPLOYED;

    play_plan = services_play_plan_get(module, command, service_str_id);
    if (play_plan && play_plan->has_force)
        effective_force = play_plan->force;

    if (command == SERVICES_DEPLOY)
        effective_required_state = effective_force ? -1 : 0;

    if (services_module_is_deployed(module, service_str_id, &current_status) < 0)
        ERR_EXIT();

    if (effective_required_state != -1)
    {
        gboolean service_deployed = current_status != UNDEPLOYED;

        if (effective_required_state != service_deployed)
        {
            g_printerr(service_deployed ? _("This command can not be used on a deployed service.\n")
                                        : _("This command can only be used on a deployed service.\n"));
            ERR_EXIT();
        }
    }

    if (play_plan && effective_force)
        services_play_plan_diag_clear(&play_plan->prediag);

    gboolean need_prediag  = play_plan && play_plan->prediag.enabled && command == SERVICES_DEPLOY;
    gboolean need_postdiag = play_plan && play_plan->postdiag.enabled
                             && (command == SERVICES_DEPLOY || command == SERVICES_CONFIGURE);

    if (need_prediag)
        if (services_module_play_run_diag(module,
                                          service_str_id,
                                          &play_plan->prediag,
                                          current_status,
                                          TRUE,
                                          _("pre-deployment"))
            < 0)
            ERR_EXIT();

    if (!input_json_parameters)
    {
        g_printerr(_("Internal error: Parameters missing.\n"));
        ERR_EXIT();
    }
    else
    {
        gchar *tmp            = input_json_parameters;
        input_json_parameters = g_strconcat(input_json_parameters, "\n", NULL);
        g_free(tmp);
    }

    // JSON-format validation
    parser = json_parser_new();
    if (!json_parser_load_from_data(parser, input_json_parameters, -1, NULL))
    {
        g_printerr(_("Internal error: invalid JSON.\n"));
        ERR_EXIT();
    }

    gchar *method = NULL;

    switch (command)
    {
    case SERVICES_DEPLOY:
        method = SERVICES_DEPLOY_METHOD_NAME;
        break;
    case SERVICES_UNDEPLOY:
        method = SERVICES_UNDEPLOY_NAME_METHOD_NAME;
        break;
    case SERVICES_CONFIGURE:
        method = SERVICES_CONFIGURE_NAME_METHOD_NAME;
        break;
    case SERVICES_BACKUP:
        method = SERVICES_BACKUP_NAME_METHOD_NAME;
        break;
    case SERVICES_RESTORE:
        method = SERVICES_RESTORE_NAME_METHOD_NAME;
        break;
    default:
        g_printerr(_("Internal error: unknown method.\n"));
        ERR_EXIT();
    }

    dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                             service_str_id,
                             SERVICES_INTERFACE_NAME,
                             method,
                             module->alterator_ctl_app->arguments->verbose);

    dbus_ctx->parameters = g_variant_new("(s)", input_json_parameters);
    dbus_ctx->reply_type = G_VARIANT_TYPE("(i)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    if (with_signals)
    {
        signals = g_ptr_array_new();

        stdout_signal = g_malloc0(sizeof(subscribe_signals_t));
        stderr_signal = g_malloc0(sizeof(subscribe_signals_t));

        if (!stdout_signal)
        {
            g_printerr(_("Internal error: Can't allocate memory for stdout signal.\n"));
            ERR_EXIT();
        }

        stdout_signal->signal_name = SERVICES_STDOUT_SIGNAL_NAME;
        stdout_signal->callback    = &services_module_run_stdout_simple_stream_handler;
        stdout_signal->user_data   = dbus_ctx;

        if (!stderr_signal)
        {
            g_printerr(_("Internal error: Can't allocate memory for stderr signal.\n"));
            ERR_EXIT();
        }

        stderr_signal->signal_name = SERVICES_STDERR_SIGNAL_NAME;
        stderr_signal->callback    = &services_module_run_stderr_simple_stream_handler;
        stderr_signal->user_data   = dbus_ctx;

        g_ptr_array_add(signals, stdout_signal);
        g_ptr_array_add(signals, stderr_signal);

        module->gdbus_source->call_with_signals(module->gdbus_source, dbus_ctx, signals, &dbus_call_error);
    }
    else
    {
        module->gdbus_source->call(module->gdbus_source, dbus_ctx, &dbus_call_error);
    }

    if (dbus_call_error)
    {
        g_printerr("%s.\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    if (dbus_ctx->result)
        (*ctx)->results = (gpointer) g_variant_ref(dbus_ctx->result);

    if (need_postdiag)
    {
        deployment_status updated_status = current_status;
        if (services_module_is_deployed(module, service_str_id, &updated_status) < 0)
            ERR_EXIT();
        const gchar *phase = command == SERVICES_CONFIGURE ? _("post-configuration") : _("post-deployment");
        if (services_module_play_run_diag(module, service_str_id, &play_plan->postdiag, updated_status, FALSE, phase)
            < 0)
            ERR_EXIT();
    }

end:
    g_free(service_str_id);

    g_free(input_json_parameters);

    if (dbus_ctx)
        dbus_ctx_free(dbus_ctx);

    if (stdout_signal)
        g_free(stdout_signal);

    if (stderr_signal)
        g_free(stderr_signal);

    if (signals)
        g_ptr_array_unref(signals);

    g_clear_error(&dbus_call_error);

    if (service_status_ctx)
        alteratorctl_ctx_free(service_status_ctx);

    if (parser)
        g_object_unref(parser);

    if (exit_code)
        g_variant_unref(exit_code);

    return ret;
}

static int services_module_is_deployed_from_status_ctx(AlteratorCtlServicesModule *module,
                                                       alteratorctl_ctx_t **status_ctx,
                                                       const gchar *service_str_id,
                                                       deployment_status *result)
{
    int ret = 0;

    if (!module)
    {
        if (service_str_id && strlen(service_str_id))
            g_printerr(_("Can't get deploy status of %s service. The services module doesn't exist.\n"), service_str_id);
        else
            g_printerr(_("Can't get deploy status of unknown service. The services module doesn't exist.\n"));
        ERR_EXIT();
    }

    if (!service_str_id || !strlen(service_str_id))
    {
        g_printerr(_("Can't get deploy status of unspecified service.\n"));
        ERR_EXIT();
    }

    if (!result)
    {
        g_printerr(_("Can't get deploy status of %s service. There is nowhere to save the result.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!status_ctx || !*status_ctx || !(*status_ctx)->results)
    {
        g_printerr(_("Failed to get %s service status. No result of services status command.\n"), service_str_id);
        ERR_EXIT();
    }

    if (services_module_status_result_get_service_params(
            module, status_ctx, service_str_id, FALSE, NULL, NULL, result, FALSE)
        < 0)
        ERR_EXIT();

end:
    return ret;
}

static int services_module_diagnose_subcommand(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                                = 0;
    dbus_ctx_t *dbus_ctx                   = NULL;
    gchar *service_str_id                  = NULL;
    gchar *service_path                    = NULL;
    gchar *diag_input_params               = NULL;
    alteratorctl_ctx_t *service_status_ctx = NULL;
    JsonParser *parser                     = NULL;
    JsonNode *parsed_parameters            = NULL;
    JsonNode *current_parameters_tmp       = NULL;
    JsonNode *service_current_params       = json_node_new(JSON_NODE_OBJECT);
    JsonObject *current_parameters_object  = json_object_new();
    json_node_set_object(service_current_params, current_parameters_object);

    alteratorctl_ctx_t *diag_ctx = NULL;

    if (!module)
    {
        g_printerr(_("Can't run services diagnostics tests. *AlteratorCtlServicesModule is NULL.\n"));
        ERR_EXIT();
    }

    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    g_variant_get((*ctx)->parameters, "(msms)", &service_str_id, &diag_input_params);
    if (!service_str_id)
    {
        g_printerr(_("Unable to run diagnostics tests on unspecified service.\n"));
        ERR_EXIT();
    }

    if (!module->info)
    {
        g_printerr(_("Can't run diagnostics tests in \"%s\" service. Service info is empty.\n"), service_str_id);
        ERR_EXIT();
    }

    service_path = service_str_id[0] != '/'
                       ? g_strdup(module->gdbus_source->alterator_gdbus_source_get_path_by_name(module->gdbus_source,
                                                                                                service_str_id,
                                                                                                SERVICES_INTERFACE_NAME))
                       : g_strdup(service_path);

    if (!diag_input_params)
    {
        g_printerr(_("Empty input params for configure \"%s\" service.\n"), service_str_id);
        ERR_EXIT();
    }
    else
    {
        gchar *tmp        = diag_input_params;
        diag_input_params = g_strconcat(tmp, "\n", NULL);
        g_free(tmp);
    }

    // JSON-format validation
    parser = json_parser_new();
    if (!json_parser_load_from_data(parser, diag_input_params, -1, NULL))
    {
        g_printerr(_("Invalid input json data for run \"%s\" service diagnostics tests.\n"), service_str_id);
        ERR_EXIT();
    }

    service_status_ctx = alteratorctl_ctx_init_services(SERVICES_STATUS, service_str_id, NULL, NULL, NULL);

    if (services_module_status_subcommand(module, &service_status_ctx) < 0)
        ERR_EXIT();

    deployment_status status = UNDEPLOYED;
    if (services_module_status_result_get_service_params(
            module, &service_status_ctx, service_str_id, FALSE, "diag", &service_current_params, &status, TRUE)
        < 0)
        ERR_EXIT();

    parsed_parameters = json_parser_get_root(parser);
    if (!(current_parameters_tmp = services_module_merge_json_data(service_current_params,
                                                                   parsed_parameters,
                                                                   service_str_id)))
        ERR_EXIT();
    else if (current_parameters_object)
        parsed_parameters = current_parameters_tmp;

    if (status != DEPLOYED && status != DEPLOYED_AND_STARTED && status != UNDEPLOYED)
    {
        g_printerr(_("Incorrect deploy status of \"%s\" service.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!JSON_NODE_HOLDS_OBJECT(parsed_parameters))
    {
        g_printerr(_("Failed to send parameters to diagnostic tests of \"%s\" service.\n"), service_str_id);
        ERR_EXIT();
    }

    JsonObjectIter object_json_iter;
    JsonObject *params_object           = json_node_get_object(parsed_parameters);
    const gchar *param_member_node_name = NULL;
    JsonNode *param_member_node         = NULL;
    json_object_iter_init(&object_json_iter, params_object);
    while (json_object_iter_next(&object_json_iter, &param_member_node_name, &param_member_node))
    {
        gchar *param_member_node_str = NULL;
        if (!(param_member_node_str = services_module_json_params_to_text(param_member_node, service_str_id)))
        {
            g_printerr(_("Failed to send %s parameter to diagnostic tests in \"%s\" service.\n"),
                       param_member_node_name,
                       service_str_id);
            ERR_EXIT();
        }

        if (module->gdbus_source->alterator_gdbus_source_set_env_value(module->gdbus_source,
                                                                       param_member_node_name,
                                                                       param_member_node_str)
            < 0)
        {
            g_printerr(_("Failed to set diagnostic regime in \"%s\" service.\n"), service_str_id);
            g_free(param_member_node_str);
            ERR_EXIT();
        }
        g_free(param_member_node_str);
    }

    GHashTableIter iter;
    g_hash_table_iter_init(&iter, service_tests);
    gpointer tool_name = NULL, tool_tests = NULL;
    while (g_hash_table_iter_next(&iter, &tool_name, &tool_tests))
    {
        if (!tool_name || !strlen(tool_name) || !tool_tests)
        {
            g_printerr(_("Failed to run \"%s\" service diagnostics tests. Invalid tests data.\n"), service_str_id);
            ERR_EXIT();
        }

        diag_tool_tests *test = (diag_tool_tests *) tool_tests;
        if (!test || !test->all_tests || !g_hash_table_size(test->all_tests))
        {
            g_printerr(_("Failed to run \"%s\" service diagnostics tests. Empty tests data.\n"), service_str_id);
            ERR_EXIT();
        }

        GNode *tool_info = NULL;
        if (!name_only
            && services_module_get_diag_info(module, service_str_id, tool_name, test->path, test->bus_type, &tool_info)
                   < 0)
            ERR_EXIT();

        GNode *tests_node = NULL;
        if (!name_only
            && !(tests_node = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
                     info_parser, tool_info, DIAG_ALTERATOR_ENTRY_SERVICE_DIAG_TESTS_TABLE_NAME)))
        {
            g_printerr(_("Failed to get tests display names in \"%s\" diagnostic tool.\n"), (gchar *) tool_name);
            ERR_EXIT();
        }

        if (diagnostic_tests && diagnostic_tests->len)
        {
            g_ptr_array_sort(diagnostic_tests, services_module_sort_result);

            if (test->bus_type == G_BUS_TYPE_SYSTEM)
                g_print("%s", _("<<< Tests on the system bus:\n"));
            else if (test->bus_type == G_BUS_TYPE_SESSION)
                g_print("%s", _("<<< Tests on the session bus:\n"));
            for (gsize i = 0; i < diagnostic_tests->len; i++)
            {
                if (!g_hash_table_contains(test->all_tests, diagnostic_tests->pdata[i]))
                {
                    g_printerr(_("Can't run non-existent \"%s\" test in \"%s\" service.\n"),
                               (gchar *) diagnostic_tests->pdata[i],
                               service_str_id);
                    ERR_EXIT();
                }

                gchar *test_display_name = NULL;

                GNode *test_node = NULL;
                if (!name_only
                    && !(test_node = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
                             info_parser, tests_node, diagnostic_tests->pdata[i])))
                {
                    g_printerr(_("Failed to get tests display names in \"%s\" diagnostic tool.\n"), (gchar *) tool_name);
                    ERR_EXIT();
                }

                if (!name_only
                    && services_module_get_display_name(module, diagnostic_tests->pdata[i], test_node, &test_display_name)
                           < 0)
                    ERR_EXIT();

                if (name_only)
                    g_print(_("Running test \"%s\"\n"), (gchar *) diagnostic_tests->pdata[i]);
                else
                    g_print(_("Running test \"%s\"\n"), test_display_name);

                if (services_module_run_test(module,
                                             service_str_id,
                                             tool_name,
                                             diagnostic_tests->pdata[i],
                                             status,
                                             test->bus_type)
                    < 0)
                {
                    g_free(test_display_name);
                    ERR_EXIT();
                }
                g_free(test_display_name);

                if (i != diagnostic_tests->len - 1)
                    g_print("\n");
            }

            goto end;
        }

        // Run all tests
        if (diagnostic_tests)
            g_ptr_array_unref(diagnostic_tests);

        diagnostic_tests = services_module_get_sorted_string_keys(test->all_tests);
        if (test->bus_type == G_BUS_TYPE_SYSTEM)
            g_print("%s", _("<<< Tests on the system bus:\n"));
        else if (test->bus_type == G_BUS_TYPE_SESSION)
            g_print("%s", _("<<< Tests on the session bus:\n"));

        for (gsize i = 0; i < diagnostic_tests->len; i++)
            if (services_module_run_test(module,
                                         service_str_id,
                                         test->path,
                                         diagnostic_tests->pdata[i],
                                         status,
                                         test->bus_type)
                < 0)
                ERR_EXIT();
    }

end:
    g_free(service_str_id);

    g_free(service_path);

    g_free(diag_input_params);

    if (dbus_ctx)
        dbus_ctx_free(dbus_ctx);

    if (service_status_ctx)
        alteratorctl_ctx_free(service_status_ctx);

    if (service_current_params)
        json_node_free(service_current_params);

    if (current_parameters_object)
        json_object_unref(current_parameters_object);

    if (current_parameters_tmp)
        json_node_free(current_parameters_tmp);

    if (diag_ctx)
        alteratorctl_ctx_free(diag_ctx);

    if (parser)
        g_object_unref(parser);

    return ret;
}

static int services_module_info_subcommand(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                 = 0;
    gchar *service_str_id   = NULL;
    dbus_ctx_t *d_ctx       = NULL;
    GError *dbus_call_error = NULL;

    g_variant_get((*ctx)->parameters, "(msms)", &service_str_id, NULL);
    if (!service_str_id)
    {
        g_printerr(_("Can't get info of unspecified service.\n"));
        ERR_EXIT();
    }

    d_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                          service_str_id,
                          SERVICES_INTERFACE_NAME,
                          SERVICES_INFO_METHOD_NAME,
                          module->alterator_ctl_app->arguments->verbose);

    d_ctx->reply_type = G_VARIANT_TYPE("(ayi)");

    (*ctx)->free_results = (void (*)(gpointer)) g_variant_unref;

    module->gdbus_source->call(module->gdbus_source, d_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("It is not possible to obtain information on a non-existent service.\n"));
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (dbus_call_error)
    {
        g_printerr("%s.\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (!d_ctx->result
             && services_module_validate_object_and_iface(module, service_str_id, SERVICES_INTERFACE_NAME) < 0)
    {
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    (*ctx)->results = (gpointer) g_variant_ref(d_ctx->result);

end:
    g_free(service_str_id);

    g_clear_error(&dbus_call_error);

    if (d_ctx)
        dbus_ctx_free(d_ctx);

    return ret;
}

static int services_module_list_subcommand(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                      = 0;
    GPtrArray *wrong_info_errors = g_ptr_array_new_full(0, (GDestroyNotify) g_free);
    GHashTable *services_table   = NULL;
    GHashTable *result_table = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify) service_names_free);
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    module->gdbus_source->alterator_gdbus_source_get_iface_objects(module->gdbus_source,
                                                                   SERVICES_INTERFACE_NAME,
                                                                   &services_table);
    if (!services_table)
    {
        g_printerr(_("Services list is empty.\n"));
        ERR_EXIT();
    }

    GHashTableIter iter;
    g_hash_table_iter_init(&iter, services_table);
    gchar *service_path = NULL;
    while (g_hash_table_iter_next(&iter, (gpointer *) &service_path, NULL))
    {
        gchar *alterator_entry_text = NULL;
        if (module->gdbus_source->alterator_gdbus_source_get_text_of_alterator_entry_by_path(module->gdbus_source,
                                                                                             service_path,
                                                                                             SERVICES_INTERFACE_NAME,
                                                                                             &alterator_entry_text)
            < 0)
        {
            g_ptr_array_add(wrong_info_errors,
                            g_strconcat(_("Error while getting info of "),
                                        service_path,
                                        _(" on Alterator Entry format.\n"),
                                        NULL));
            continue;
        }

        GNode *parsed_info = NULL;
        if (info_parser->alterator_ctl_module_info_parser_get_data_from_text(info_parser,
                                                                             module->gdbus_source,
                                                                             alterator_entry_text,
                                                                             &parsed_info)
            < 0)
        {
            g_ptr_array_add(wrong_info_errors,
                            g_strconcat(_("Parsing of alterator entry service "), service_path, _(" failed.\n"), NULL));
            ret = -1;
            g_free(alterator_entry_text);
            continue;
        }

        toml_value *toml_name = g_hash_table_lookup(((alterator_entry_node *) parsed_info->data)->toml_pairs,
                                                    SERVICES_ALTERATOR_ENTRY_SERVICE_NAME_KEY_NAME);

        gchar *display_name = NULL;
        services_module_get_display_name(module, service_path, parsed_info, &display_name);
        service_names_t *service_names = service_names_init(toml_name->str_value, display_name);
        g_free(display_name);
        g_hash_table_insert(result_table, service_path, service_names);

        alterator_ctl_module_info_parser_result_tree_free(parsed_info);
        g_free(alterator_entry_text);
    }

    g_ptr_array_sort(wrong_info_errors, services_module_sort_result);
    for (gsize i = 0; i < wrong_info_errors->len; i++)
        g_printerr("%s", (gchar *) wrong_info_errors->pdata[i]);

    services_module_print_list_with_filters(module, result_table);

end:
    if (wrong_info_errors)
        g_ptr_array_unref(wrong_info_errors);

    if (services_table)
        g_hash_table_unref(services_table);

    if (result_table)
        g_hash_table_unref(result_table);

    return ret;
}

static int services_module_resources_subcommand(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                                   = 0;
    alteratorctl_ctx_t *service_info_ctx      = NULL;
    gchar *service_str_id                     = NULL;
    GVariant *info_array                      = NULL;
    GVariant *exit_code                       = NULL;
    gchar *info_str                           = NULL;
    GNode *parsed_info                        = NULL;
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    g_variant_get((*ctx)->parameters, "(msms)", &service_str_id, NULL);
    if (!service_str_id)
    {
        g_printerr(_("Can't get info of unspecified service.\n"));
        ERR_EXIT();
    }

    service_info_ctx = alteratorctl_ctx_init_services(SERVICES_INFO, service_str_id, NULL, NULL, NULL);
    if (services_module_info_subcommand(module, &service_info_ctx) < 0)
        ERR_EXIT();

    if (!service_info_ctx->results)
    {
        g_printerr(_("Empty info of service %s in resources command.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!g_variant_is_of_type(service_info_ctx->results, G_VARIANT_TYPE("(ayi)")))
    {
        g_printerr(_("Wrong answer services info type in services resources command.\n"));
        ERR_EXIT();
    }

    info_array = g_variant_get_child_value(service_info_ctx->results, 0);
    exit_code  = g_variant_get_child_value(service_info_ctx->results, 1);

    if (!exit_code)
    {
        g_printerr(_("Can't get service %s info result: exit code is empty.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!info_array)
        ERR_EXIT();

    gsize array_size;
    gconstpointer gvar_info = g_variant_get_fixed_array(info_array, &array_size, sizeof(guint8));

    info_str = g_malloc(array_size + 1);
    if (!info_str)
        ERR_EXIT();
    memset(info_str, 0, array_size + 1);
    memcpy(info_str, gvar_info, array_size);

    if (info_parser->alterator_ctl_module_info_parser_get_data_from_text(module,
                                                                         module->gdbus_source,
                                                                         info_str,
                                                                         &parsed_info)
        < 0)
    {
        g_printerr(_("Parsing of service %s information failed.\n"), service_str_id);
        ERR_EXIT();
    }

    GNode *resources = NULL;
    if (!(resources = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
              module, parsed_info, SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCES_TABLE_NAME)))
    {
        g_print(_("No resources of service %s.\n"), service_str_id);
        goto end;
    }

    (*ctx)->results      = (gpointer) alterator_ctl_module_info_parser_tree_deep_copy(resources);
    (*ctx)->free_results = (gpointer) alterator_ctl_module_info_parser_result_tree_free;

    // Parse JSON parameters of deployed service for getting resource linked params
    {
        alteratorctl_ctx_t *status_ctx = alteratorctl_ctx_init_services(SERVICES_STATUS,
                                                                        service_str_id,
                                                                        NULL,
                                                                        NULL,
                                                                        NULL);

        if (services_module_status_subcommand(module, &status_ctx) < 0)
            ERR_EXIT();

        module->json = json_node_new(JSON_NODE_OBJECT);
        json_node_init_object(module->json, json_object_new());

        if (services_module_status_result_get_service_params(
                module, &status_ctx, service_str_id, FALSE, NULL, &module->json, NULL, FALSE)
            < 0)
            ERR_EXIT();
    }

end:
    if (service_info_ctx)
        alteratorctl_ctx_free(service_info_ctx);

    g_free(service_str_id);

    if (info_array)
        g_variant_unref(info_array);

    if (exit_code)
        g_variant_unref(exit_code);

    g_free(info_str);

    if (parsed_info)
        alterator_ctl_module_info_parser_result_tree_free(parsed_info);

    return ret;
}

static int services_module_status_subcommand(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                 = 0;
    gchar *service_str_id   = NULL;
    dbus_ctx_t *dbus_ctx    = NULL;
    GError *dbus_call_error = NULL;

    if (!module)
    {
        g_printerr(_("The call to the services status method failed. The module doesn't exist.\n"));
        ERR_EXIT();
    }

    g_variant_get((*ctx)->parameters, "(msms)", &service_str_id, NULL);
    if (!service_str_id)
    {
        g_printerr(_("Can't get status of unspecified service.\n"));
        ERR_EXIT();
    }

    dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                             service_str_id,
                             SERVICES_INTERFACE_NAME,
                             SERVICES_STATUS_NAME_METHOD_NAME,
                             module->alterator_ctl_app->arguments->verbose);

    if (!dbus_ctx)
    {
        g_printerr(_("Can't allcate dbus_ctx_t in services_module_status_subcommand.\n"));
        ERR_EXIT();
    }

    dbus_ctx->reply_type = G_VARIANT_TYPE("(ayi)");

    (*ctx)->free_results = (gpointer) g_variant_unref;
    module->gdbus_source->call(module->gdbus_source, dbus_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Current edition is not set.\n"));
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (dbus_call_error)
    {
        g_printerr("%s.\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (!dbus_ctx->result
             && services_module_validate_object_and_iface(module, service_str_id, SERVICES_INTERFACE_NAME) < 0)
    {
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    (*ctx)->results = (gpointer) g_variant_ref(dbus_ctx->result);

end:
    g_free(service_str_id);

    if (dbus_ctx)
        dbus_ctx_free(dbus_ctx);

    g_clear_error(&dbus_call_error);

    return ret;
}

static int services_module_start_subcommand(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                                = 0;
    gchar *service_str_id                  = NULL;
    dbus_ctx_t *dbus_ctx                   = NULL;
    GError *dbus_call_error                = NULL;
    alteratorctl_ctx_t *service_status_ctx = NULL;
    GVariant *exit_code                    = NULL;

    if (!module)
    {
        g_printerr(_("The call to the services start method failed. The module doesn't exist.\n"));
        ERR_EXIT();
    }

    g_variant_get((*ctx)->parameters, "(msms)", &service_str_id, NULL);
    if (!service_str_id)
    {
        g_printerr(_("Can't start of unspecified service.\n"));
        ERR_EXIT();
    }

    service_status_ctx = alteratorctl_ctx_init_services(SERVICES_STATUS, service_str_id, NULL, NULL, NULL);

    if (services_module_status_subcommand(module, &service_status_ctx) < 0)
        ERR_EXIT();

    if (!service_status_ctx->results)
    {
        g_printerr(_("Empty status of service %s in start command.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!g_variant_is_of_type(service_status_ctx->results, G_VARIANT_TYPE("(ayi)")))
    {
        g_printerr(_("Wrong answer services status type in services start command.\n"));
        ERR_EXIT();
    }

    exit_code = g_variant_get_child_value(service_status_ctx->results, 1);
    if (!exit_code)
    {
        g_printerr(_("Can't check service %s status in start command.\n"), service_str_id);
        ERR_EXIT();
    }

    deployment_status service_status = UNDEPLOYED;
    if (services_module_is_deployed_from_status_ctx(module, &service_status_ctx, service_str_id, &service_status) < 0)
        ERR_EXIT();

    if (service_status == UNDEPLOYED)
    {
        g_printerr(_("Can't start undeployed service %s.\n"), service_str_id);
        ERR_EXIT();
    }
    else if (service_status == DEPLOYED_AND_STARTED)
    {
        g_printerr(_("Can't start an already running %s service.\n"), service_str_id);
        ERR_EXIT();
    }

    dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                             service_str_id,
                             SERVICES_INTERFACE_NAME,
                             SERVICES_START_NAME_METHOD_NAME,
                             module->alterator_ctl_app->arguments->verbose);

    if (!dbus_ctx)
    {
        g_printerr(_("Can't allcate dbus_ctx_t in services_module_start_subcommand.\n"));
        ERR_EXIT();
    }
    dbus_ctx->reply_type = G_VARIANT_TYPE("(i)");

    (*ctx)->free_results = (gpointer) g_variant_unref;
    module->gdbus_source->call(module->gdbus_source, dbus_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Service %s doesn't exist.\n"), service_str_id);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (dbus_call_error)
    {
        g_printerr("%s.\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (!dbus_ctx->result
             && services_module_validate_object_and_iface(module, service_str_id, SERVICES_INTERFACE_NAME) < 0)
    {
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    (*ctx)->results = (gpointer) g_variant_ref(dbus_ctx->result);

end:
    g_free(service_str_id);

    if (dbus_ctx)
        dbus_ctx_free(dbus_ctx);

    g_clear_error(&dbus_call_error);

    if (service_status_ctx)
        alteratorctl_ctx_free(service_status_ctx);

    if (exit_code)
        g_variant_unref(exit_code);

    return ret;
}

static int services_module_stop_subcommand(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                                = 0;
    gchar *service_str_id                  = NULL;
    dbus_ctx_t *dbus_ctx                   = NULL;
    GError *dbus_call_error                = NULL;
    alteratorctl_ctx_t *service_status_ctx = NULL;
    GVariant *exit_code                    = NULL;

    if (!module)
    {
        g_printerr(_("The call to the services stop method failed. The module doesn't exist.\n"));
        ERR_EXIT();
    }

    g_variant_get((*ctx)->parameters, "(msms)", &service_str_id, NULL);
    if (!service_str_id)
    {
        g_printerr(_("Can't stop of unspecified service.\n"));
        ERR_EXIT();
    }

    service_status_ctx = alteratorctl_ctx_init_services(SERVICES_STATUS, service_str_id, NULL, NULL, NULL);

    if (services_module_status_subcommand(module, &service_status_ctx) < 0)
        ERR_EXIT();

    if (!service_status_ctx->results)
    {
        g_printerr(_("Empty status of service %s in stop command.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!g_variant_is_of_type(service_status_ctx->results, G_VARIANT_TYPE("(ayi)")))
    {
        g_printerr(_("Wrong answer services status type in services stop command.\n"));
        ERR_EXIT();
    }

    exit_code = g_variant_get_child_value(service_status_ctx->results, 1);
    if (!exit_code)
    {
        g_printerr(_("Can't check service %s status in stop command.\n"), service_str_id);
        ERR_EXIT();
    }

    deployment_status service_status = UNDEPLOYED;
    if (services_module_is_deployed_from_status_ctx(module, &service_status_ctx, service_str_id, &service_status) < 0)
        ERR_EXIT();

    if (service_status == UNDEPLOYED)
    {
        g_printerr(_("Can't start undeployed service %s.\n"), service_str_id);
        ERR_EXIT();
    }
    else if (service_status != DEPLOYED_AND_STARTED)
    {
        g_printerr(_("Can't stop a %s service that is not running.\n"), service_str_id);
        ERR_EXIT();
    }

    dbus_ctx = dbus_ctx_init(ALTERATOR_SERVICE_NAME,
                             service_str_id,
                             SERVICES_INTERFACE_NAME,
                             SERVICES_STOP_NAME_METHOD_NAME,
                             module->alterator_ctl_app->arguments->verbose);

    if (!dbus_ctx)
    {
        g_printerr(_("Can't allcate dbus_ctx_t in services_module_stop_subcommand.\n"));
        ERR_EXIT();
    }
    dbus_ctx->reply_type = G_VARIANT_TYPE("(i)");

    (*ctx)->free_results = (gpointer) g_variant_unref;
    module->gdbus_source->call(module->gdbus_source, dbus_ctx, &dbus_call_error);

    if (dbus_call_error && !alterator_ctl_check_object_is_exist(dbus_call_error))
    {
        g_printerr(_("Service %s doesn't exist.\n"), service_str_id);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (dbus_call_error)
    {
        g_printerr("%s.\n", dbus_call_error->message);
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }
    else if (!dbus_ctx->result
             && services_module_validate_object_and_iface(module, service_str_id, SERVICES_INTERFACE_NAME) < 0)
    {
        is_dbus_call_error = TRUE;
        ERR_EXIT();
    }

    (*ctx)->results = (gpointer) g_variant_ref(dbus_ctx->result);

end:
    g_free(service_str_id);

    if (dbus_ctx)
        dbus_ctx_free(dbus_ctx);

    g_clear_error(&dbus_call_error);

    if (service_status_ctx)
        alteratorctl_ctx_free(service_status_ctx);

    if (exit_code)
        g_variant_unref(exit_code);

    return ret;
}

static int services_module_handle_results(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx)
{
    int ret = 0;
    if (!module)
    {
        g_printerr(_("Internal error in services module when handle result: module is NULL.\n"));
        ERR_EXIT();
    }

    if (module->alterator_ctl_app->arguments->module_help)
        goto end;

    if (!(*ctx))
    {
        g_printerr(_("Internal error in services module when handle results: *ctx is NULL.\n"));
        ERR_EXIT();
    }

    switch (g_variant_get_int32((*ctx)->subcommands_ids))
    {
    case SERVICES_BACKUP:
        if (services_module_backup_handle_result(module, ctx) < 0)
            ERR_EXIT();
        break;

    case SERVICES_CONFIGURE:
        if (services_module_configure_handle_result(module, ctx) < 0)
            ERR_EXIT();
        break;

    case SERVICES_DEPLOY:
        if (services_module_deploy_handle_result(module, ctx) < 0)
            ERR_EXIT();
        break;

    case SERVICES_DIAGNOSE:
        break;

    case SERVICES_INFO:
        if (services_module_info_handle_result(module, ctx) < 0)
            ERR_EXIT();
        break;

    case SERVICES_LIST:
        break;

    case SERVICES_RESOURCES:
        if (services_module_resources_handle_result(module, ctx) < 0)
            ERR_EXIT();
        break;

    case SERVICES_RESTORE:
        if (services_module_restore_handle_result(module, ctx) < 0)
            ERR_EXIT();
        break;

    case SERVICES_STATUS:
        if (services_module_status_handle_result(module, ctx) < 0)
            ERR_EXIT();
        break;

    case SERVICES_START:
        if (services_module_start_handle_result(module, ctx) < 0)
            ERR_EXIT();
        break;

    case SERVICES_STOP:
        if (services_module_stop_handle_result(module, ctx) < 0)
            ERR_EXIT();
        break;

    case SERVICES_UNDEPLOY:
        if (services_module_undeploy_handle_result(module, ctx) < 0)
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

static int services_module_backup_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                     = 0;
    const gchar *service_str_id = NULL;
    GVariant *exit_code         = NULL;
    GError *error               = NULL;

    g_variant_get((*ctx)->parameters, "(msms)", &service_str_id, NULL);
    if (!service_str_id)
    {
        g_printerr(_("Can't backup of unspecified service.\n"));
        ERR_EXIT();
    }

    if (!(*ctx)->results)
    {
        g_printerr(_("DBus error in service %s backup.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(i)")))
    {
        g_printerr(_("Wrong answer type in backup of %s service.\n"), service_str_id);
        ERR_EXIT();
    }

    exit_code = g_variant_get_child_value((*ctx)->results, 0);

    if (!exit_code)
    {
        g_printerr(_("Can't get service %s backup result: exit code is empty.\n"), service_str_id);
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);
    if (ret)
    {
        g_printerr(_("Failed to backup %s service. Exit code: %i.\n"), service_str_id, ret);
        ERR_EXIT();
    }

end:
    g_free((gchar *) service_str_id);

    if (exit_code)
        g_variant_unref(exit_code);

    g_clear_error(&error);

    return ret;
}

static int services_module_configure_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                     = 0;
    const gchar *service_str_id = NULL;
    GVariant *exit_code         = NULL;
    GError *error               = NULL;

    g_variant_get((*ctx)->parameters, "(msms)", &service_str_id, NULL);
    if (!service_str_id)
    {
        g_printerr(_("Can't configure of unspecified service.\n"));
        ERR_EXIT();
    }

    if (!(*ctx)->results)
    {
        g_printerr(_("DBus error in service %s configure.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(i)")))
    {
        g_printerr(_("Wrong answer type in configure of %s service.\n"), service_str_id);
        ERR_EXIT();
    }

    exit_code = g_variant_get_child_value((*ctx)->results, 0);

    if (!exit_code)
    {
        g_printerr(_("Can't get service %s configure result: exit code is empty.\n"), service_str_id);
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);
    if (ret)
    {
        g_printerr(_("Failed to configure %s service. Exit code: %i.\n"), service_str_id, ret);
        ERR_EXIT();
    }

end:
    g_free((gchar *) service_str_id);

    if (exit_code)
        g_variant_unref(exit_code);

    g_clear_error(&error);

    return ret;
}

static int services_module_deploy_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx)
{
    int ret               = 0;
    GVariant *exit_code   = NULL;
    gchar *service_str_id = NULL;

    alteratorctl_ctx_t *service_start_ctx = NULL;
    GVariant *service_start_exit_code     = NULL;

    g_variant_get((*ctx)->parameters, "(msms)", &service_str_id, NULL);
    if (!service_str_id)
    {
        g_printerr(_("Can't deploy unspecified service.\n"));
        ERR_EXIT();
    }

    if (!(*ctx)->results)
    {
        g_printerr(_("Empty result of deploy %s service command.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(i)")))
    {
        g_printerr(_("Wrong answer type in deploy of %s service.\n"), service_str_id);
        ERR_EXIT();
    }

    exit_code = g_variant_get_child_value((*ctx)->results, 0);
    if (!exit_code)
    {
        g_printerr(_("Can't get service %s deploy result: exit code is empty.\n"), service_str_id);
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);

    if (ret != 0)
    {
        g_printerr(_("Failed to deploy service '%s'. Exit code: %i.\n"), service_str_id, ret);
        ERR_EXIT();
    }

    {
        ServicesPlayPlan *play_plan = services_play_plan_get(module, SERVICES_DEPLOY, service_str_id);
        gboolean should_autostart   = lauch_after_deploy;
        if (play_plan && play_plan->has_autostart)
            should_autostart = play_plan->autostart;

        if (!should_autostart)
            goto end;
    }

    service_start_ctx = alteratorctl_ctx_init_services(SERVICES_START, service_str_id, NULL, NULL, NULL);
    if (services_module_start_subcommand(module, &service_start_ctx) < 0)
        ERR_EXIT();

    if (!service_start_ctx->results)
    {
        g_printerr(_("No exit code from %s service autolaunch after deploying.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!g_variant_is_of_type(service_start_ctx->results, G_VARIANT_TYPE("(i)")))
    {
        g_printerr(_("Wrong answer services start type in services deploy command.\n"));
        ERR_EXIT();
    }

    service_start_exit_code = g_variant_get_child_value((GVariant *) service_start_ctx->results, 0);
    if (!service_start_exit_code)
    {
        g_printerr(_("No exit code from %s service autolaunch after deploying.\n"), service_str_id);
        ERR_EXIT();
    }

    ret = g_variant_get_int32(service_start_exit_code);
    if (ret)
    {
        g_printerr(_("Failed to start %s service after deploying. Exit code: %i.\n"), service_str_id, ret);
        ERR_EXIT();
    }

end:
    if (exit_code)
        g_variant_unref(exit_code);

    g_free(service_str_id);

    if (service_start_ctx)
        alteratorctl_ctx_free(service_start_ctx);

    if (service_start_exit_code)
        g_variant_unref(service_start_exit_code);

    return ret;
}

static int services_module_info_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx)
{
    int ret               = 0;
    gchar *service_str_id = NULL;
    gchar *result         = NULL;
    GVariant *exit_code   = NULL;
    GVariant *info_array  = NULL;

    g_variant_get((*ctx)->parameters, "(msms)", &service_str_id, NULL);
    if (!service_str_id)
    {
        g_printerr(_("Can't get info of unspecified service.\n"));
        ERR_EXIT();
    }

    if (!(*ctx)->results)
    {
        g_printerr(_("Empty info of service %s.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(ayi)")))
    {
        g_printerr(_("Wrong answer type in %s service info.\n"), service_str_id);
        ERR_EXIT();
    }

    info_array = g_variant_get_child_value((*ctx)->results, 0);
    exit_code  = g_variant_get_child_value((*ctx)->results, 1);

    if (!exit_code)
    {
        g_printerr(_("Can't get service %s info result: exit code is empty.\n"), service_str_id);
        ERR_EXIT();
    }
    if (!info_array)
        ERR_EXIT();

    gsize array_size;
    gconstpointer gvar_info = g_variant_get_fixed_array(info_array, &array_size, sizeof(guint8));

    result = g_malloc(array_size + 1);
    if (!result)
        ERR_EXIT();
    memset(result, 0, array_size + 1);
    memcpy(result, gvar_info, array_size);

    print_with_pager(result);

end:
    g_free(service_str_id);

    g_free(result);

    return ret;
}

static int services_module_resources_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                         = 0;
    gchar *service_str_id           = NULL;
    GNode *resources                = NULL;
    GHashTable *params_to_resources = NULL;
    GHashTable *resources_to_params = NULL;

    g_variant_get((*ctx)->parameters, "(msms)", &service_str_id, NULL);
    if (!service_str_id)
    {
        g_printerr(_("Can't get info of unspecified service.\n"));
        ERR_EXIT();
    }

    if (!(*ctx)->results)
        goto end;

    resources = (GNode *) (*ctx)->results;

    if (services_module_get_param_to_resource_table(module, resources, &params_to_resources) < 0)
        ERR_EXIT();

    resources_to_params = hash_table_str2str_invert(params_to_resources);

    if (services_module_printing_resources_value(module,
                                                 service_str_id,
                                                 resources,
                                                 services_module_print_resource_with_links_to_params,
                                                 resources_to_params)
        < 0)
        ERR_EXIT();

end:
    if (params_to_resources)
        g_hash_table_destroy(params_to_resources);

    if (resources_to_params)
        g_hash_table_destroy(resources_to_params);

    g_free(service_str_id);

    return ret;
}

static int services_module_restore_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                     = 0;
    const gchar *service_str_id = NULL;
    GVariant *exit_code         = NULL;
    GError *error               = NULL;

    g_variant_get((*ctx)->parameters, "(msms)", &service_str_id, NULL);
    if (!service_str_id)
    {
        g_printerr(_("Can't restore of unspecified service.\n"));
        ERR_EXIT();
    }

    if (!(*ctx)->results)
    {
        g_printerr(_("DBus error in service %s restore.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(i)")))
    {
        g_printerr(_("Wrong answer type in restore of %s service.\n"), service_str_id);
        ERR_EXIT();
    }

    exit_code = g_variant_get_child_value((*ctx)->results, 0);

    if (!exit_code)
    {
        g_printerr(_("Can't get service %s restore result: exit code is empty.\n"), service_str_id);
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);
    if (ret)
    {
        g_printerr(_("Failed to restore %s service. Exit code: %i.\n"), service_str_id, ret);
        ERR_EXIT();
    }

end:
    g_free((gchar *) service_str_id);

    if (exit_code)
        g_variant_unref(exit_code);

    g_clear_error(&error);

    return ret;
}

static int services_module_status_value_to_str(JsonNode *value, gchar **result)
{
    int ret = 0;
    if (!result)
        ERR_EXIT();

    switch (json_node_get_value_type(value))
    {
    case G_TYPE_STRING:
        *result = g_strdup_printf("\"%s\"", json_node_get_string(value));
        break;

    case G_TYPE_INT64:
        *result = g_strdup_printf("%ld", json_node_get_int(value));
        break;

    case G_TYPE_DOUBLE:
        *result = g_strdup_printf("%lf", json_node_get_double(value));
        break;

    case G_TYPE_BOOLEAN:
        *result = g_strdup_printf("%s", (json_node_get_boolean(value) ? "true" : "false"));
        break;

    default:
        g_printerr(_("Unknown parameter value type.\n"));
        ERR_EXIT();
        break;
    }

end:
    return ret;
}

static GNode *get_direct_child_by_name(GNode *parent, const char *name)
{
    if (!parent || !name)
        return NULL;

    for (GNode *child = parent->children; child != NULL; child = child->next)
    {
        const gchar *child_name = child ? ((alterator_entry_node *) child->data)->node_name : NULL;

        if (child_name && streq(child_name, name))
            return child;
    }
    return NULL;
}

static GNode *find_properties_on_node(GNode *node)
{
    return get_direct_child_by_name(node, SERVICES_ALTERATOR_ENTRY_SERVICE_PROPERTIES_TABLE_NAME);
}

static GNode *resolve_parameter_properties(AlteratorCtlModuleInfoParser *info_parser,
                                           GNode *module_info,
                                           GNode *param_node)
{
    if (!param_node)
        return NULL;

    GNode *props = find_properties_on_node(param_node);
    if (props)
        return props;

    toml_value *prototype = g_hash_table_lookup(((alterator_entry_node *) param_node->data)->toml_pairs,
                                                SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_PROTOTYPE_KEY_NAME);

    if (prototype && prototype->type == TOML_DATA_STRING)
    {
        GNode *all_params = get_direct_child_by_name(module_info,
                                                     SERVICES_ALTERATOR_ENTRY_SERVICE_PARAMETERS_TABLE_NAME);
        if (!all_params)
            return NULL;

        GNode *proto = get_direct_child_by_name(all_params, prototype->str_value);
        if (!proto)
            return NULL;

        return find_properties_on_node(proto);
    }

    return NULL;
}

static int services_module_status_params_build_path(AlteratorCtlServicesModule *module,
                                                    gchar *service_str_id,
                                                    const gchar *key,
                                                    JsonNode *value,
                                                    GNode *parameter_node,
                                                    gboolean is_enum_subparam,
                                                    GString *prefix,
                                                    GHashTable **result)
{
    int ret                                   = 0;
    GString *path                             = g_string_new("");
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    gboolean is_simple_enum_field = FALSE;
    {
        alterator_entry_node *parent_parameter_data = parameter_node->parent->data;
        is_simple_enum_field                        = streq(parent_parameter_data->node_name, "values");
    }

    g_string_assign(path, prefix ? prefix->str : "");
    if (!is_simple_enum_field)
    {
        if (prefix)
            path = g_string_append_c(path, '.');
        g_string_append(path, key);
    }

    JsonNodeType node_type = json_node_get_node_type(value);
    switch (node_type)
    {
    case JSON_NODE_ARRAY: {
        JsonArray *array = json_node_get_array(value);
        int length       = json_array_get_length(array);
        if (!length)
        {
            g_hash_table_insert(*result, g_strconcat(prefix->str, ".", key, NULL), g_strdup("empty array"));
            break;
        }

        for (int i = 0; i < length; ++i)
        {
            JsonNode *item         = json_array_get_element(array, i);
            JsonNodeType item_type = json_node_get_node_type(item);

            GString *path_i = g_string_new(path->str);
            g_string_append_c(path_i, '.');
            g_string_append_printf(path_i, "%i", i);

            if (item_type == JSON_NODE_OBJECT)
            {
                GNode *children = resolve_parameter_properties(info_parser, module->info, parameter_node);
                if (!children)
                    children = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser,
                                                                                              parameter_node,
                                                                                              "values");

                if (children)
                {
                    if (services_module_status_params_build_output(module,
                                                                   service_str_id,
                                                                   json_node_get_object(item),
                                                                   children,
                                                                   path_i,
                                                                   result)
                        < 0)
                        ERR_EXIT();
                }
            }
            else
            {
                gchar *value_str = NULL;
                if (services_module_status_value_to_str(item, &value_str) < 0)
                    ERR_EXIT();
                g_hash_table_insert(*result, g_strdup(path_i->str), g_strdup(value_str));
                g_free(value_str);
            }

            g_string_free(path_i, TRUE);
        }
        break;
    }

    case JSON_NODE_OBJECT: {
        GNode *values_tbl = NULL;
        if (!is_enum_subparam)
            values_tbl = get_direct_child_by_name(parameter_node, "values");

        if (values_tbl)
        {
            JsonObject *obj = json_node_get_object(value);
            JsonObjectIter it;
            json_object_iter_init(&it, obj);

            const gchar *enum_value_name;
            JsonNode *enum_value_payload;

            GPtrArray *selected = g_ptr_array_new_with_free_func(g_free);

            while (json_object_iter_next(&it, &enum_value_name, &enum_value_payload))
            {
                GNode *enum_value_node = get_direct_child_by_name(values_tbl, enum_value_name);
                if (!enum_value_node)
                {
                    g_printerr(_("Enum value '%s' is not defined in schema for '%s'.\n"), enum_value_name, key);
                    continue;
                }

                g_ptr_array_add(selected, g_strdup(enum_value_name));

                GNode *enum_props = find_properties_on_node(enum_value_node);
                if (enum_props && JSON_NODE_OBJECT == json_node_get_node_type(enum_value_payload))
                {
                    GString *path_val = g_string_new(path->str);
                    g_string_append_c(path_val, '.');
                    g_string_append(path_val, enum_value_name);

                    if (services_module_status_params_build_output(module,
                                                                   service_str_id,
                                                                   json_node_get_object(enum_value_payload),
                                                                   enum_props,
                                                                   path_val,
                                                                   result)
                        < 0)
                        ERR_EXIT();

                    g_string_free(path_val, TRUE);
                }
            }

            if (selected->len == 1)
            {
                const char *sel = (const char *) g_ptr_array_index(selected, 0);
                g_hash_table_insert(*result, g_strdup(path->str), g_strdup_printf("\"%s\"", sel));
            }
            else if (selected->len > 1)
            {
                GString *arr = g_string_new("[");
                for (guint i = 0; i < selected->len; ++i)
                {
                    const char *sel = (const char *) g_ptr_array_index(selected, i);
                    if (i)
                        g_string_append(arr, ",");
                    g_string_append_printf(arr, "\"%s\"", sel);
                }
                g_string_append(arr, "]");
                g_hash_table_insert(*result, g_strdup(path->str), g_string_free(arr, FALSE));
            }
            g_ptr_array_free(selected, TRUE);

            break;
        }

        // Getting properties
        {
            if (!is_simple_enum_field)
            {
                GNode *children = resolve_parameter_properties(info_parser, module->info, parameter_node);
                if (!children)
                {
                    g_printerr(_("Internal error: can't resolve properties for '%s'.\n"), key);
                    ERR_EXIT();
                }

                if (services_module_status_params_build_output(module,
                                                               service_str_id,
                                                               json_node_get_object(value),
                                                               children,
                                                               path,
                                                               result)
                    < 0)
                    ERR_EXIT();
            }
            else
                g_hash_table_insert(*result, g_strdup(path->str), g_strdup(key));
        }
        break;
    }

    case JSON_NODE_VALUE: {
        gchar *value_str = NULL;
        if (services_module_status_value_to_str(value, &value_str) < 0)
            ERR_EXIT();

        if (schema_is_password_string(info_parser, parameter_node))
        {
            g_free(value_str);
            value_str = g_strdup("*******");
        }

        g_hash_table_insert(*result, g_strdup(path->str), g_strdup(value_str));
        g_free(value_str);
        break;
    }

    case JSON_NODE_NULL:
        break;
    }

end:
    g_string_free(path, TRUE);
    return ret;
}

static int services_module_status_params_build_output(AlteratorCtlServicesModule *module,
                                                      gchar *service_str_id,
                                                      JsonObject *parameters,
                                                      GNode *parameters_data,
                                                      GString *prefix,
                                                      GHashTable **result)
{
    int ret = 0;

    if (!result)
        ERR_EXIT();

    if (!(*result))
        *result = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, (GDestroyNotify) g_free);

    if (!module->info)
    {
        g_printerr(_("Error processing service %s parameters: empty service info.\n"), service_str_id);
        ERR_EXIT();
    }

    JsonObjectIter iter;
    json_object_iter_init(&iter, parameters);

    const gchar *key;
    JsonNode *value;
    while (json_object_iter_next(&iter, &key, &value))
    {
        if (services_module_is_special_param(key))
            continue;

        GNode *parameter = get_direct_child_by_name(parameters_data, key);
        if (!parameter)
        {
            g_printerr(_("Parameter data not found for %s.\n"), key);
            continue;
        }

        alterator_entry_node *parameter_data = parameter->data;

        toml_value *constant = g_hash_table_lookup(parameter_data->toml_pairs,
                                                   SERVICES_ALTERATOR_ENTRY_SERVICE_PARAM_CONSTANT_KEY_NAME);
        if (constant && constant->bool_value)
            continue;

        toml_value *context = g_hash_table_lookup(parameter_data->toml_pairs,
                                                  SERVICES_ALTERATOR_ENTRY_SERVICE_PARAM_CONTEXT_KEY_NAME);
        if (context && !context->array_length)
            continue; // For example, object prototypes

        if (services_module_status_params_build_path(module, service_str_id, key, value, parameter, FALSE, prefix, result)
            < 0)
            ERR_EXIT();
    }

end:
    return ret;
}

static gboolean services_module_get_required_parameters_paths_recursive(AlteratorCtlServicesModule *module,
                                                                        const gchar *context,
                                                                        gchar *prefix,
                                                                        GNode *parameter,
                                                                        JsonNode *value,
                                                                        gboolean is_array_member,
                                                                        gboolean is_enum_value,
                                                                        GHashTable **result)
{
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;
    gchar *param_name                         = ((alterator_entry_node *) parameter->data)->node_name;

    gchar *path = prefix ? (is_array_member ? g_strdup(prefix) : g_strconcat(prefix, ".", param_name, NULL))
                         : g_strdup(param_name);

    { // skip constants
        toml_value *constant = g_hash_table_lookup(((alterator_entry_node *) parameter->data)->toml_pairs, "constant");
        ;
        if (constant && constant->bool_value)
            goto end;
    }

    gboolean is_required = FALSE;
    {
        toml_value *required = g_hash_table_lookup(((alterator_entry_node *) parameter->data)->toml_pairs, "required");
        if (required)
        {
            if (required->type == TOML_DATA_BOOL)
                is_required = required->bool_value;
            else if (required->type == TOML_DATA_ARRAY_OF_STRING)
            {
                for (gsize context_idx = 0; context_idx < required->array_length; context_idx++)
                    if (streq(context, ((gchar **) required->array)[context_idx]))
                    {
                        is_required = TRUE;
                        break;
                    }
            }
        }
    }

    gchar *type = NULL;
    {
        toml_value *type_name
            = g_hash_table_lookup(((alterator_entry_node *) parameter->data)->toml_pairs,
                                  is_array_member ? SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_ARRAY_TYPE_KEY_NAME
                                                  : SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_TYPE_KEY_NAME);

        if (type_name)
            type = type_name->str_value;
    }

    if (!is_required && !is_enum_value)
        goto end;

    if (is_enum_value || streq(type, "object"))
    {
        { // check prototype and repoint "parameter" to a corresponding table
            toml_value *prototype
                = g_hash_table_lookup(((alterator_entry_node *) parameter->data)->toml_pairs,
                                      SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_PROTOTYPE_KEY_NAME);

            if (prototype)
            {
                GNode *parameters = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
                    info_parser, module->info, SERVICES_ALTERATOR_ENTRY_SERVICE_PARAMETERS_TABLE_NAME);

                parameter = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser,
                                                                                           parameters,
                                                                                           prototype->str_value);
            }
        }

        GNode *parameters = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
            info_parser, parameter, SERVICES_ALTERATOR_ENTRY_SERVICE_PROPERTIES_TABLE_NAME);

        if (parameters)
        {
            JsonObject *object = value ? json_node_get_object(value) : NULL;

            for (GNode *child = parameters->children; child != NULL; child = child->next)
            {
                JsonNode *child_value = object
                                            ? json_object_get_member(object,
                                                                     ((alterator_entry_node *) child->data)->node_name)
                                            : NULL;
                services_module_get_required_parameters_paths_recursive(module,
                                                                        context,
                                                                        path,
                                                                        child,
                                                                        child_value,
                                                                        FALSE,
                                                                        FALSE,
                                                                        result);
            }
        }

        // we prefer full paths like in --help instead of only parent object,
        // so we'll skip current path.
        goto end;
    }
    else if (streq(type, "enum"))
    {
        if (is_required)
            g_hash_table_add(*result, g_strdup(path));

        JsonObject *enum_object = value ? json_node_get_object(value) : NULL;

        if (enum_object && json_object_get_size(enum_object)) // append child and its subparameters
        {
            gchar *enum_value_name = NULL;
            {
                GList *members  = json_object_get_members(enum_object);
                enum_value_name = members->data;
                g_list_free(members);
            }

            GNode *enum_values = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser,
                                                                                                parameter,
                                                                                                "values");

            GNode *value_data    = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser,
                                                                                               enum_values,
                                                                                               enum_value_name);
            JsonNode *enum_value = json_object_get_member(enum_object, enum_value_name);

            services_module_get_required_parameters_paths_recursive(module,
                                                                    context,
                                                                    path,
                                                                    value_data,
                                                                    enum_value,
                                                                    FALSE,
                                                                    TRUE,
                                                                    result);
        }

        goto end;
    }
    else if (streq(type, "array"))
    {
        JsonArray *array = value ? json_node_get_array(value) : NULL;

        int minsize = 0;
        {
            toml_value *min_size_toml = g_hash_table_lookup(((alterator_entry_node *) parameter->data)->toml_pairs,
                                                            "array_min");
            ;
            if (min_size_toml)
                minsize = min_size_toml->double_value;
        }

        int array_len = array ? json_array_get_length(array) : 0;

        if (minsize > array_len)
        {
            GString *path_i = g_string_new(path);
            if (minsize - array_len == 1)
                g_string_append_printf(path_i, ".%i", minsize - 1);
            else
                g_string_append_printf(path_i, ".<%i-%i>", array_len, minsize - 1);

            services_module_get_required_parameters_paths_recursive(module,
                                                                    context,
                                                                    path_i->str,
                                                                    parameter,
                                                                    NULL,
                                                                    TRUE,
                                                                    FALSE,
                                                                    result);
            g_string_free(path_i, TRUE);
        }

        // skip array path since we got subpaths already
        goto end;
    }

    if (is_required)
        g_hash_table_add(*result, g_strdup(path));

end:
    g_free(path);
    return is_required;
}

static int services_module_get_required_parameters_paths(AlteratorCtlServicesModule *module,
                                                         const gchar *service_str_id,
                                                         const gchar *context,
                                                         GHashTable **result)
{
    int ret                                   = 0;
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;
    if (!service_str_id || !strlen(service_str_id))
    {
        g_printerr(_("Can't get required params paths. Unknown service name.\n"));
        ERR_EXIT();
    }

    if (!context || !strlen(context))
    {
        g_printerr(_("Can't get required params paths. Unknown service \"%s\" context.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!result)
    {
        g_printerr(_("Can't get required params paths of \"%s %s\". There is nowhere to save the result.\n"),
                   service_str_id,
                   context);
        ERR_EXIT();
    }

    if (!module->info)
        ERR_EXIT();
    if (!module->required_params_cache)
        module->required_params_cache = g_hash_table_new_full(g_str_hash,
                                                              g_str_equal,
                                                              (GDestroyNotify) g_free,
                                                              (GDestroyNotify) g_hash_table_destroy);

    gchar *cache_key = g_strdup_printf("%s|%s", service_str_id, context);

    GHashTable *cached = g_hash_table_lookup(module->required_params_cache, cache_key);
    if (!cached)
    {
        GHashTable *fresh = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);

        GNode *parameters = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
            info_parser, module->info, SERVICES_ALTERATOR_ENTRY_SERVICE_PARAMETERS_TABLE_NAME);

        if (!module->json)
            json_node_init_object(module->json, json_object_new());

        JsonObject *root = json_node_get_object(module->json);

        for (GNode *parameter = parameters->children; parameter != NULL; parameter = parameter->next)
        {
            JsonNode *value = json_object_get_member(root, ((alterator_entry_node *) parameter->data)->node_name);
            services_module_get_required_parameters_paths_recursive(module,
                                                                    context,
                                                                    NULL,
                                                                    parameter,
                                                                    value,
                                                                    FALSE,
                                                                    FALSE,
                                                                    &fresh);
        }

        g_hash_table_insert(module->required_params_cache, g_strdup(cache_key), fresh);
        cached = fresh;
    }

    *result = g_hash_table_copy_table(cached, g_strdup_copy_func, NULL, NULL, NULL);

    g_free(cache_key);

end:
    return ret;
}

static int services_module_create_params_table(GHashTable *stringified_params_data,
                                               GHashTable *param_name_to_option_entry,
                                               GHashTable *required_params_paths,
                                               gboolean *is_missing_required,
                                               GString **result)
{
    int ret                                  = 0;
    gsize max_path_length                    = 0;
    gsize max_value_length                   = 0;
    GHashTable *stringified_params_data_copy = g_hash_table_copy_table(stringified_params_data,
                                                                       g_strdup_copy_func,
                                                                       NULL,
                                                                       g_strdup_copy_func,
                                                                       NULL);
    GPtrArray *params_paths                  = NULL;
    GPtrArray *params_values                 = g_hash_table_get_values_as_ptr_array(stringified_params_data_copy);
    if (!g_hash_table_size(stringified_params_data_copy) && !g_hash_table_size(required_params_paths))
        goto end;

    // Calculate missed required parameters
    if (required_params_paths && g_hash_table_size(required_params_paths))
    {
        GPtrArray *required_params_paths_array = g_hash_table_get_keys_as_ptr_array(required_params_paths);
        for (gsize i = 0; i < required_params_paths_array->len; i++)
            if (!g_hash_table_contains(stringified_params_data_copy, (gchar *) required_params_paths_array->pdata[i]))
            {
                if (is_missing_required)
                    *is_missing_required = TRUE;
                g_hash_table_insert(stringified_params_data_copy,
                                    g_strdup_printf("* %s", (gchar *) required_params_paths_array->pdata[i]),
                                    g_strdup("none"));
            }
        g_ptr_array_unref(required_params_paths_array);
    }

    params_paths = g_hash_table_get_keys_as_ptr_array(stringified_params_data_copy);
    g_ptr_array_sort(params_paths, services_module_sort_result);

    *result = g_string_new("");

    for (gsize i = 0; i < params_paths->len; i++)
    {
        glong path_len = g_utf8_strlen((gchar *) params_paths->pdata[i], -1);
        if (path_len > max_path_length)
            max_path_length = path_len;
    }
    if (max_path_length < SERVICES_MODULE_MIN_PARAMS_PATH_COLUMN_WIDTH)
        max_path_length = SERVICES_MODULE_MIN_PARAMS_PATH_COLUMN_WIDTH;

    for (gsize i = 0; i < params_values->len; i++)
    {
        glong value_len = g_utf8_strlen((gchar *) params_values->pdata[i], -1);
        if (value_len > max_value_length)
            max_value_length = value_len;
    }
    if (max_value_length < SERVICES_MODULE_MIN_PARAMS_VALUE_COLUMN_WIDTH)
        max_value_length = SERVICES_MODULE_MIN_PARAMS_VALUE_COLUMN_WIDTH;

    GString *horizontal_single_line_left = g_string_new("");
    for (gsize i = 0; i < max_path_length + 2; i++)
        g_string_append(horizontal_single_line_left, SERVICES_PARAMS_TABLE_SINGLE_H);

    GString *horizontal_single_line_right = g_string_new("");
    for (gsize i = 0; i < max_value_length + 2; i++)
        g_string_append(horizontal_single_line_right, SERVICES_PARAMS_TABLE_SINGLE_H);

    GString *horizontal_double_line_left = g_string_new("");
    for (gsize i = 0; i < max_path_length + 2; i++)
        g_string_append(horizontal_double_line_left, SERVICES_PARAMS_TABLE_DOUBLE_H);

    GString *horizontal_double_line_right = g_string_new("");
    for (gsize i = 0; i < max_value_length + 2; i++)
        g_string_append(horizontal_double_line_right, SERVICES_PARAMS_TABLE_DOUBLE_H);

    // Top of the table
    g_string_append_printf(*result,
                           "%s%s%s%s%s\n",
                           SERVICES_PARAMS_TABLE_DOUBLE_LT,
                           horizontal_double_line_left->str,
                           SERVICES_PARAMS_TABLE_SINGLE_H_DOUBLE_T,
                           horizontal_double_line_right->str,
                           SERVICES_PARAMS_TABLE_DOUBLE_RT);

    for (gsize i = 0; i < params_paths->len; i++)
    {
        gsize path_length = g_utf8_strlen((gchar *) params_paths->pdata[i], -1);
        gchar *path       = g_strdup((gchar *) params_paths->pdata[i]);
        gchar *value      = g_hash_table_lookup(stringified_params_data_copy, path); // HANDLE NULL
        if (((gchar *) params_paths->pdata[i])[0] == '*' && isatty_safe(STDOUT_FILENO))
        {
            gchar *tmp = path;
            path       = colorize_text(tmp, RED);
            g_free(tmp);
        }

        gchar *first_col_spaces  = g_strnfill(max_path_length - path_length + 2, ' ');
        gchar *second_col_spaces = g_strnfill(max_value_length - (value ? g_utf8_strlen(value, -1) : 0) + 2, ' ');

        GRegex *regex    = g_regex_new("\\.(\\d+)(\\.|$)", 0, 0, NULL);
        gchar *help_path = g_regex_replace(regex, path, -1, 0, "<num>", 0, NULL);
        g_regex_unref(regex);

        GOptionEntry *param_entry = param_name_to_option_entry
                                        ? (GOptionEntry *) g_hash_table_lookup(param_name_to_option_entry, help_path)
                                        : NULL;
        const gchar *description  = param_entry ? param_entry->description : NULL;

        g_free(help_path);

        gchar **description_parts       = description ? g_strsplit(description, "\n", -1) : NULL;
        gsize descriprion_subrow_length = description_parts ? max_path_length + g_utf8_strlen("  ", -1)
                                                                  + max_value_length + g_utf8_strlen("  ", -1)
                                                                  + g_utf8_strlen(SERVICES_PARAMS_TABLE_DOUBLE_V, -1)
                                                            : 0;

        // Top of the table
        g_string_append_printf(*result,
                               "%s%s%s%s%s%s%s\n",
                               SERVICES_PARAMS_TABLE_DOUBLE_V,
                               path,
                               first_col_spaces,
                               SERVICES_PARAMS_TABLE_SINGLE_V,
                               value,
                               second_col_spaces,
                               SERVICES_PARAMS_TABLE_DOUBLE_V);

        if (description)
        {
            g_string_append_printf(*result,
                                   "%s%s%s%s%s\n",
                                   SERVICES_PARAMS_TABLE_SINGLE_H_DOUBLE_V_R,
                                   horizontal_single_line_left->str,
                                   SERVICES_PARAMS_TABLE_SINGLE_H_SINGLE_B,
                                   horizontal_single_line_right->str,
                                   SERVICES_PARAMS_TABLE_SINGLE_H_DOUBLE_V_L);

            for (gsize desc_part_idx = 0; desc_part_idx < g_strv_length(description_parts) && desc_part_idx < 2;
                 desc_part_idx++)
            {
                gchar *description_part = description_parts[desc_part_idx];

                // Adding '.' at the end

                description_part = g_strconcat(description_part, ".", NULL);

                gchar **words  = g_strsplit(description_part, " ", -1);
                GString *line  = g_string_new("");
                gsize line_len = 0;
                for (gsize w = 0; w < g_strv_length(words); w++)
                {
                    gchar *word = words[w];
                    if (!word || !g_utf8_strlen(word, -1))
                        continue;

                    gsize word_need_len = (line_len ? 1 : 0) + g_utf8_strlen(word, -1);
                    if (line_len + word_need_len <= descriprion_subrow_length)
                    {
                        if (line_len)
                            g_string_append_c(line, ' ');
                        g_string_append(line, word);
                        line_len += word_need_len;
                    }
                    else
                    {
                        gsize margin  = (line_len < descriprion_subrow_length) ? (descriprion_subrow_length - line_len)
                                                                               : 0;
                        gchar *spaces = g_strnfill(margin, ' ');
                        g_string_append_printf(*result,
                                               "%s%s%s%s\n",
                                               SERVICES_PARAMS_TABLE_DOUBLE_V,
                                               line->str,
                                               spaces,
                                               SERVICES_PARAMS_TABLE_DOUBLE_V);
                        g_free(spaces);
                        g_string_assign(line, word);
                        line_len = g_utf8_strlen(word, -1);
                    }
                }
                if (line_len > 0)
                {
                    gsize margin  = (line_len < descriprion_subrow_length) ? (descriprion_subrow_length - line_len) : 0;
                    gchar *spaces = g_strnfill(margin, ' ');
                    g_string_append_printf(*result,
                                           "%s%s%s%s\n",
                                           SERVICES_PARAMS_TABLE_DOUBLE_V,
                                           line->str,
                                           spaces,
                                           SERVICES_PARAMS_TABLE_DOUBLE_V);
                    g_free(spaces);
                }
                g_string_free(line, TRUE);
                g_strfreev(words);
                g_free(description_part);
            }

            // g_free(path);
        }

        if (i != params_paths->len - 1)
            g_string_append_printf(*result,
                                   "%s%s%s%s%s\n",
                                   SERVICES_PARAMS_TABLE_DOUBLE_VR,
                                   horizontal_double_line_left->str,
                                   description ? SERVICES_PARAMS_TABLE_SINGLE_H_DOUBLE_T
                                               : SERVICES_PARAMS_TABLE_SINGLE_V_DOUBLE_H,
                                   horizontal_double_line_right->str,
                                   SERVICES_PARAMS_TABLE_DOUBLE_VL);
        else
            // Bottom of the table
            g_string_append_printf(*result,
                                   "%s%s%s%s%s\n",
                                   SERVICES_PARAMS_TABLE_DOUBLE_LB,
                                   horizontal_double_line_left->str,
                                   description ? SERVICES_PARAMS_TABLE_DOUBLE_H
                                               : SERVICES_PARAMS_TABLE_SINGLE_H_DOUBLE_B,
                                   horizontal_double_line_right->str,
                                   SERVICES_PARAMS_TABLE_DOUBLE_RB);

        if (first_col_spaces)
            g_free(first_col_spaces);

        if (second_col_spaces)
            g_free(second_col_spaces);

        if (path)
            g_free(path);

        if (description_parts)
            g_strfreev(description_parts);
    }

    g_string_free(horizontal_single_line_left, TRUE);
    g_string_free(horizontal_single_line_right, TRUE);
    g_string_free(horizontal_double_line_left, TRUE);
    g_string_free(horizontal_double_line_right, TRUE);

end:
    if (params_paths)
        g_ptr_array_unref(params_paths);

    if (params_values)
        g_ptr_array_unref(params_values);

    if (stringified_params_data_copy)
        g_hash_table_destroy(stringified_params_data_copy);

    return ret;
}

static int services_module_status_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                       = 0;
    gchar *service_str_id         = NULL;
    JsonNode *parameters_node     = json_node_new(JSON_NODE_OBJECT);
    JsonObject *parameters_object = json_object_new();
    GHashTable *output_data       = NULL;
    json_node_set_object(parameters_node, parameters_object);
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;
    GNode *parameters_data                    = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
        info_parser, module->info, SERVICES_ALTERATOR_ENTRY_SERVICE_PARAMETERS_TABLE_NAME);

    g_variant_get((*ctx)->parameters, "(msms)", &service_str_id, NULL);
    if (!service_str_id)
    {
        g_printerr(_("Can't get status of unspecified service.\n"));
        ERR_EXIT();
    }

    if (!(*ctx)->results)
    {
        g_printerr(_("DBus error in service %s status.\n"), service_str_id);
        ERR_EXIT();
    }

    deployment_status deploy_status = UNDEPLOYED;
    if (services_module_status_result_get_service_params(
            module, ctx, service_str_id, FALSE, NULL, &parameters_node, &deploy_status, FALSE)
        < 0)
        ERR_EXIT();

    if (!except_status_markers)
        switch (deploy_status)
        {
        case UNDEPLOYED:
            g_print(_("Service %s is undeployed\n"), service_str_id);
            break;

        case DEPLOYED:
            g_print(_("Service %s is deployed, but not launched\n\n"), service_str_id);
            break;

        case DEPLOYED_AND_STARTED:
            g_print(_("Service %s is deployed and running\n\n"), service_str_id);
            break;
        default:
            g_print(_("Unknown status of %s service: %i\n\n"), service_str_id, deploy_status);
            break;
        }

    if (deploy_status == UNDEPLOYED)
        goto end;

    if (services_module_status_params_build_output(module,
                                                   service_str_id,
                                                   parameters_object,
                                                   parameters_data,
                                                   NULL,
                                                   &output_data)
        < 0)
        ERR_EXIT();

    GString *output_str = NULL;
    if (services_module_create_params_table(output_data, NULL, NULL, NULL, &output_str) < 0)
        ERR_EXIT();

    if (!output_str)
    {
        g_printerr("No parameters of service \"%s\"\n", service_str_id);
        ERR_EXIT();
    }

    g_print("%s\n", output_str->str);
    g_string_free(output_str, TRUE);

end:
    g_free((gchar *) service_str_id);

    if (parameters_node)
        json_node_unref(parameters_node);

    if (parameters_object)
        json_object_unref(parameters_object);

    if (output_data)
        g_hash_table_destroy(output_data);

    return ret;
}

static int services_module_start_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                     = 0;
    const gchar *service_str_id = NULL;
    GVariant *exit_code         = NULL;
    GError *error               = NULL;

    g_variant_get((*ctx)->parameters, "(msms)", &service_str_id, NULL);
    if (!service_str_id)
    {
        g_printerr(_("Can't start of unspecified service.\n"));
        ERR_EXIT();
    }

    if (!(*ctx)->results)
    {
        g_printerr(_("DBus error in service %s start.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(i)")))
    {
        g_printerr(_("Wrong answer type in start of %s service.\n"), service_str_id);
        ERR_EXIT();
    }

    exit_code = g_variant_get_child_value((*ctx)->results, 0);

    if (!exit_code)
    {
        g_printerr(_("Can't get service %s start result: exit code is empty.\n"), service_str_id);
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);
    if (ret)
    {
        g_printerr(_("Failed to start %s service. Exit code: %i.\n"), service_str_id, ret);
        ERR_EXIT();
    }

end:
    g_free((gchar *) service_str_id);

    if (exit_code)
        g_variant_unref(exit_code);

    g_clear_error(&error);

    return ret;
}

static int services_module_stop_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx)
{
    int ret                     = 0;
    const gchar *service_str_id = NULL;
    GVariant *exit_code         = NULL;
    GError *error               = NULL;

    g_variant_get((*ctx)->parameters, "(msms)", &service_str_id, NULL);
    if (!service_str_id)
    {
        g_printerr(_("Can't stop of unspecified service.\n"));
        ERR_EXIT();
    }

    if (!(*ctx)->results)
    {
        g_printerr(_("DBus error in service %s stop.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(i)")))
    {
        g_printerr(_("Wrong answer type in stop of %s service.\n"), service_str_id);
        ERR_EXIT();
    }

    exit_code = g_variant_get_child_value((*ctx)->results, 0);

    if (!exit_code)
    {
        g_printerr(_("Can't get service %s stop result: exit code is empty.\n"), service_str_id);
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);
    if (ret)
    {
        g_printerr(_("Failed to stop %s service. Exit code: %i.\n"), service_str_id, ret);
        ERR_EXIT();
    }

end:
    g_free((gchar *) service_str_id);

    if (exit_code)
        g_variant_unref(exit_code);

    g_clear_error(&error);

    return ret;
}

static int services_module_undeploy_handle_result(AlteratorCtlServicesModule *module, alteratorctl_ctx_t **ctx)
{
    int ret               = 0;
    GVariant *exit_code   = NULL;
    gchar *service_str_id = NULL;

    g_variant_get((*ctx)->parameters, "(msms)", &service_str_id, NULL);
    if (!service_str_id)
    {
        g_printerr(_("Can't undeploy unspecified service.\n"));
        ERR_EXIT();
    }

    if (!(*ctx)->results)
    {
        g_printerr(_("Empty result of undeploy %s service command.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(i)")))
    {
        g_printerr(_("Wrong answer type in uneploy of %s service.\n"), service_str_id);
        ERR_EXIT();
    }

    exit_code = g_variant_get_child_value((*ctx)->results, 0);
    if (!exit_code)
    {
        g_printerr(_("Can't get service %s undeploy result: exit code is empty.\n"), service_str_id);
        ERR_EXIT();
    }

    ret = g_variant_get_int32(exit_code);

    if (ret != UNDEPLOYED)
    {
        g_printerr(_("Failed to undeploy %s service. Exit code: %i.\n"), service_str_id, ret);
        ERR_EXIT();
    }

end:
    if (exit_code)
        g_variant_unref(exit_code);

    g_free(service_str_id);

    return ret;
}

static int services_module_status_get_json_node(AlteratorCtlServicesModule *module,
                                                const gchar *service_str_id,
                                                JsonNode **result)
{
    int ret                        = 0;
    JsonParser *parser             = json_parser_new();
    GVariant *node_array           = NULL;
    gchar *node_str                = NULL;
    alteratorctl_ctx_t *status_ctx = NULL;
    if (!module)
    {
        g_printerr(_("The call to the services status method failed. The module doesn't exist.\n"));
        ERR_EXIT();
    }

    if (!service_str_id)
    {
        g_printerr(_("Can't get json params via \"status\" command. empty service name.\n"));
        ERR_EXIT();
    }

    if (!result)
    {
        g_printerr(_("Can't get json params via \"status\" command. Nowhere to save the result.\n"));
        ERR_EXIT();
    }

    status_ctx = alteratorctl_ctx_init_services(SERVICES_STATUS, service_str_id, NULL, NULL, NULL);

    if (services_module_status_subcommand(module, &status_ctx) < 0)
        ERR_EXIT();

    GVariant *exit_code = g_variant_get_child_value(status_ctx->results, 1);
    if (!exit_code || g_variant_get_int32(exit_code) != 0)
    {
        g_printerr(_("Can't get service \"%s\" status: error %i.\n"), service_str_id, g_variant_get_int32(exit_code));
        ERR_EXIT();
    }

    node_array = g_variant_get_child_value(status_ctx->results, 0);
    if (!node_array)
        ERR_EXIT();

    gsize array_size;
    gconstpointer gvar_info = g_variant_get_fixed_array(node_array, &array_size, sizeof(guint8));

    node_str = g_malloc(array_size + 1);
    if (!node_str)
        ERR_EXIT();
    memset(node_str, 0, array_size + 1);
    memcpy(node_str, gvar_info, array_size);

    if (!json_parser_load_from_data(parser, node_str, -1, NULL))
    {
        g_printerr(_("Invalid json data.\n"));
        ERR_EXIT();
    }

    *result = json_node_copy(json_parser_get_root(parser));

end:
    if (parser)
        g_object_unref(parser);

    if (node_array)
        g_variant_unref(node_array);

    g_free(node_str);

    if (status_ctx)
        alteratorctl_ctx_free(status_ctx);

    return ret;
}

static void services_module_run_stdout_simple_stream_handler(GDBusConnection *connection,
                                                             const gchar *sender_name,
                                                             const gchar *object_path,
                                                             const gchar *interface_name,
                                                             const gchar *signal_name,
                                                             GVariant *parameters,
                                                             gpointer user_data)
{
    for (int i = 0; i < g_variant_n_children(parameters); i++)
    {
        GVariant *tmp = g_variant_get_child_value(parameters, i);
        const gchar *str;
        str = g_variant_get_string(tmp, NULL);
        g_print("%s\n", str);
        g_variant_unref(tmp);
    }

    return;
}

static void services_module_run_stderr_simple_stream_handler(GDBusConnection *connection,
                                                             const gchar *sender_name,
                                                             const gchar *object_path,
                                                             const gchar *interface_name,
                                                             const gchar *signal_name,
                                                             GVariant *parameters,
                                                             gpointer user_data)
{
    for (int i = 0; i < g_variant_n_children(parameters); i++)
    {
        GVariant *tmp = g_variant_get_child_value(parameters, i);
        const gchar *str;
        str = g_variant_get_string(tmp, NULL);
        g_printerr("%s\n", str);
        g_variant_unref(tmp);
    }

    return;
}

int services_module_print_help(gpointer self)
{
    int ret = 0;

    g_print(_("Usage:\n"));
    g_print(_("  alteratorctl services [OPTIONS]\n"
              "                              List services\n"));
    g_print(_("  alteratorctl services [COMMAND] [OPTIONS] [PARAMETERS <argument>] [service]\n\n"));
    g_print(_("Commands:\n"));
    g_print(_("  list                        List services (default command)\n"));
    g_print(_("  status <service>            Get current state and parameters of specified\n"
              "                              service\n"));
    g_print(_("  info <service>              Get info about specified service\n"));
    g_print(_("  resources <service>         Show resources of specified service\n"));
    g_print(_("  play <file>                 Deploy service using parameters from JSON file\n"
              "                              exported from Alt-Services\n"));
    g_print(_("  diagnose <service> (<diagnostic tool> [test...])... [PARAMETERS]\n"
              "                              Run diagnostics tests for specified service\n"));
    g_print(_("  deploy <service> [PARAMETERS]\n"
              "                              Deploy specified service\n"));
    g_print(_("  configure <service> <PARAMETERS>\n"
              "                              Configure specified service\n"));
    g_print(_("  start <service>             Start service\n"));
    g_print(_("  stop <service>              Stop service\n"));
    g_print(_("  backup <service> [PARAMETERS]\n"
              "                              Backup specified service\n"));
    g_print(_("  restore <service> [PARAMETERS]\n"
              "                              Restore specified service\n"));
    g_print(_("  undeploy <service> [PARAMETERS]\n"
              "                              Undeploy specified service\n\n"));
    g_print(_("Options:\n"));
    g_print(_("  -v, --verbose               Add services paths to services list output\n"));
    g_print(_("  -d, --display-name-only     Show only services display names\n"));
    g_print(_("  -D, --no-display-name       Show services without display names\n"));
    g_print(_("  -p, --path-only             Show only services objects paths on D-Bus\n"));
    g_print(_("  -n, --name-only             Show only services objects names\n"));
    g_print(_("  -a, --all                   Run all diagnostics tests of current mode\n"
              "                              (pre-diagnostic or post-diagnostic)\n"));
    g_print(_("  -f, --force-deploy          Force deploy service\n"));
    g_print(_("  -y, --yes                   Assume Yes to all queries and do not prompt\n"));
    g_print(_("  -s, --autostart             Start the service immediately after deployment\n"));
    g_print(_("  -E, --except-status-markers Show services list without deploy and running\n"
              "                              state markers\n"));
    g_print(_("  -N, --no-name               Hide services names\n"));
    g_print(_("  -H, --hide-markers-legend   Hide the legend of status tokens\n"));
    g_print(_("  -h, --help                  Show services module usage help\n\n"));

end:
    return ret;
}

static int services_module_print_list_with_filters(AlteratorCtlServicesModule *module, GHashTable *services)
{
    int ret              = 0;
    GPtrArray *entries   = g_ptr_array_new_with_free_func((GDestroyNotify) service_display_entry_free);
    GArray *status_array = !except_status_markers ? g_array_new(0, 0, sizeof(gint)) : NULL;

    GHashTableIter status_iter;
    g_hash_table_iter_init(&status_iter, services);
    gchar *service_path            = NULL;
    service_names_t *service_names = NULL;
    while (g_hash_table_iter_next(&status_iter, (gpointer) &service_path, (gpointer) &service_names))
    {
        deployment_status status = UNDEPLOYED;
        if (!except_status_markers)
        {
            JsonNode *params_node     = json_node_new(JSON_NODE_OBJECT);
            JsonObject *params_object = json_object_new();
            json_node_set_object(params_node, params_object);

            alteratorctl_ctx_t *status_ctx = alteratorctl_ctx_init_services(SERVICES_STATUS,
                                                                            service_path,
                                                                            NULL,
                                                                            NULL,
                                                                            NULL);

            if (services_module_status_subcommand(module, &status_ctx) < 0)
            {
                if (status_ctx)
                    alteratorctl_ctx_free(status_ctx);

                json_node_free(params_node);
                json_object_unref(params_object);

                ERR_EXIT();
            }

            if (services_module_status_result_get_service_params(
                    module, &status_ctx, service_path, FALSE, NULL, &params_node, &status, FALSE)
                < 0)
                ERR_EXIT();

            json_node_free(params_node);
            json_object_unref(params_object);

            g_array_append_val(status_array, status);

            alteratorctl_ctx_free(status_ctx);
        }
    }

    gboolean show_statuses = !except_status_markers;

    GHashTableIter iter;
    g_hash_table_iter_init(&iter, services);
    service_path  = NULL;
    service_names = NULL;
    for (gsize i = 0; g_hash_table_iter_next(&iter, (gpointer) &service_path, (gpointer) &service_names); i++)
    {
        deployment_status status = UNDEPLOYED;
        if (status_array && i < status_array->len)
            status = g_array_index(status_array, gint, i);

        gchar *label = services_module_build_service_label(module, service_path, service_names);

        service_display_entry *entry = service_display_entry_new(label, status);
        g_ptr_array_add(entries, entry);
    }

    g_ptr_array_sort(entries, services_module_sort_entries);

    gsize max_label_len = 0;
    if (show_statuses)
    {
        for (gsize i = 0; i < entries->len; i++)
        {
            service_display_entry *entry = g_ptr_array_index(entries, i);
            if (!entry || !entry->label)
                continue;

            gsize label_len = g_utf8_strlen(entry->label, -1);
            if (label_len > max_label_len)
                max_label_len = label_len;
        }
    }

    for (gsize i = 0; i < entries->len; i++)
    {
        service_display_entry *entry = g_ptr_array_index(entries, i);
        gchar *line                  = services_module_format_entry(entry, max_label_len, show_statuses);
        if (line)
        {
            g_print("%s\n", line);
            g_free(line);
        }
    }

    if (show_statuses && !hide_markers_legend && entries->len)
        services_module_print_status_legend();

end:
    if (entries)
        g_ptr_array_unref(entries);

    if (status_array)
        g_array_unref(status_array);

    return ret;
}

static int services_module_validate_object_and_iface(AlteratorCtlServicesModule *module,
                                                     const gchar *object,
                                                     const gchar *iface)
{
    int ret                   = 0;
    int object_exist          = 0;
    int iface_exists          = 0;
    const gchar *service_path = object[0] != '/'
                                    ? module->gdbus_source->alterator_gdbus_source_get_path_by_name(module->gdbus_source,
                                                                                                    object,
                                                                                                    iface)
                                    : object;

    //Check object
    if (module->gdbus_source->alterator_gdbus_source_check_object_by_path(module->gdbus_source,
                                                                          service_path,
                                                                          &object_exist)
        < 0)
    {
        g_printerr(_("The service %s doesn't exist.\n"), object);
        ERR_EXIT();
    }

    if (object_exist == 0)
    {
        g_printerr(_("The service %s doesn't exist.\n"), object);
        ERR_EXIT();
    }

    //check interface of the object
    if (module->gdbus_source->alterator_gdbus_source_check_object_by_iface(module->gdbus_source,
                                                                           service_path,
                                                                           iface,
                                                                           &iface_exists)
        < 0)
    {
        if (iface_exists == 0)
        {
            g_printerr(_("Service %s has no interface %s.\n"), object, iface);
            ERR_EXIT();
        }

        g_printerr(_("Error when checking if an service: %s has an interface %s.\n"), object, iface);
        ERR_EXIT();
    }

end:
    return ret;
}

static int services_module_get_display_name(AlteratorCtlServicesModule *module,
                                            const gchar *elem_str_id,
                                            GNode *data,
                                            gchar **result)
{
    int ret                                   = 0;
    GNode *elem_data                          = data;
    gchar *locale                             = NULL;
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    if (!elem_data)
    {
        if (info_parser->alterator_ctl_module_info_parser_get_specified_object_data(info_parser,
                                                                                    module->gdbus_source,
                                                                                    elem_str_id,
                                                                                    SERVICES_INTERFACE_NAME,
                                                                                    &elem_data)
            < 0)
        {
            g_printerr(_("Can't get display name of service %s. Parsing of service data failed.\n"), elem_str_id);
            ERR_EXIT();
        }
    }

    if (!(locale = alterator_ctl_get_language()))
        ERR_EXIT();

    GHashTable *display_name = NULL;
    if (!info_parser->alterator_ctl_module_info_parser_find_table(
            info_parser, elem_data, &display_name, -1, SERVICES_ALTERATOR_ENTRY_SERVICE_DISPLAY_NAME_TABLE_NAME, NULL))
    {
        g_printerr(_("Can't get display name of service %s. Display name data by key %s is empty.\n"),
                   elem_str_id,
                   SERVICES_ALTERATOR_ENTRY_SERVICE_DISPLAY_NAME_TABLE_NAME);
        ERR_EXIT();
    }

    toml_value *display_name_locale_value = g_hash_table_lookup(display_name, locale);
    if (!display_name_locale_value)
        display_name_locale_value = g_hash_table_lookup(display_name, LOCALE_FALLBACK);

    (*result) = g_strdup(display_name_locale_value->str_value);

end:
    if (locale)
        g_free(locale);

    return ret;
}

static int services_module_status_params_get_state(JsonNode *params, deployment_status *deploy_status)
{
    int ret = 0;
    if (!params)
        ERR_EXIT();

    JsonObject *params_object = json_node_get_object(params);

    JsonNode *deployed_field = json_object_get_member(params_object,
                                                      SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_DEPLOYED_KEY_NAME);
    gboolean is_deployed     = deployed_field ? json_node_get_boolean(deployed_field) : FALSE;

    JsonNode *started_field = json_object_get_member(params_object,
                                                     SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_STARTED_KEY_NAME);
    gboolean is_started     = deployed_field ? json_node_get_boolean(started_field) : FALSE;

    if (is_deployed && is_started)
        *deploy_status = DEPLOYED_AND_STARTED;
    else if (is_deployed && !is_started)
        *deploy_status = DEPLOYED;
    else if (!is_deployed && !is_started)
        *deploy_status = UNDEPLOYED;
    else
        ERR_EXIT();

end:
    return ret;
}

static int services_module_status_result_get_service_params(AlteratorCtlServicesModule *module,
                                                            alteratorctl_ctx_t **ctx,
                                                            const gchar *service_str_id,
                                                            gboolean initial_params,
                                                            gchar *filtering_ctx,
                                                            JsonNode **result,
                                                            deployment_status *deploy_status,
                                                            gboolean exclude_unset_params)
{
    int ret                = 0;
    GVariant *exit_code    = NULL;
    GVariant *status_array = NULL;
    gchar *status          = NULL;
    GError *error          = NULL;
    GNode *service_info    = NULL;
    JsonParser *parser     = NULL;
    GHashTable *params_ctx = NULL;

    if (!service_str_id)
    {
        g_printerr(_("Can't get status of unspecified service.\n"));
        ERR_EXIT();
    }

    if (!result && !deploy_status)
    {
        g_printerr(_("Can't get status %s service result: invalid variable for saving the result.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!(*ctx)->results)
    {
        g_printerr(_("DBus error in service %s status.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!g_variant_is_of_type((*ctx)->results, G_VARIANT_TYPE("(ayi)")))
    {
        g_printerr(_("Wrong answer type in %s service status.\n"), service_str_id);
        ERR_EXIT();
    }

    status_array = g_variant_get_child_value((*ctx)->results, 0);
    exit_code    = g_variant_get_child_value((*ctx)->results, 1);

    if (!exit_code)
    {
        g_printerr(_("Can't get service %s status result: exit code is empty.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!status_array)
        ERR_EXIT();

    gsize array_size;
    gconstpointer g_var_status = g_variant_get_fixed_array(status_array, &array_size, sizeof(guint8));

    status = g_malloc0(array_size + 1);
    if (!status)
        ERR_EXIT();
    memcpy(status, g_var_status, array_size);

    if (!result && deploy_status)
    {
        if (status && strlen(status))
        {
            parser = json_parser_new();
            json_parser_load_from_data(parser, status, array_size, &error);
            if (error)
            {
                g_printerr(_("Service '%s' returned invalid status: %s (%s, %d).\n"),
                           service_str_id,
                           error->message,
                           g_quark_to_string(error->domain),
                           error->code);
                ERR_EXIT();
            }

            JsonNode *deployed_params = json_parser_get_root(parser);
            if (services_module_status_params_get_state(deployed_params, deploy_status) < 0)
                ERR_EXIT();
        }
        else
        {
            *deploy_status = UNDEPLOYED;
        }

        goto end;
    }

    gboolean required_only = TRUE;
    if (!filtering_ctx && exclude_unset_params)
        required_only = FALSE;

    if (services_module_get_initial_params(
            module, service_str_id, result, required_only, filtering_ctx, &params_ctx, exclude_unset_params)
        < 0)
        ERR_EXIT();

    if (!initial_params && status && strlen(status))
    {
        parser = json_parser_new();
        json_parser_load_from_data(parser, status, array_size, &error);
        if (error)
            g_printerr(_("Service '%s' returned invalid status: %s (%s, %d).\n"),
                       service_str_id,
                       error->message,
                       g_quark_to_string(error->domain),
                       error->code);

        JsonNode *deployed_params = json_parser_get_root(parser);
        JsonNode *parameters_tmp  = NULL;
        if (!(parameters_tmp = services_module_merge_json_data(*result, deployed_params, service_str_id)))
            ERR_EXIT();
        else
        {
            json_node_unref(*result);
            *result = parameters_tmp;
        }
    }

    if (!streq(filtering_ctx, "status") && filtering_ctx && strlen(filtering_ctx))
    {
        JsonNode *filtered_node = services_module_filter_params_by_context(*result, params_ctx, filtering_ctx, FALSE);
        json_node_unref(*result);
        *result = filtered_node;
    }

    if (deploy_status && services_module_status_params_get_state(*result, deploy_status) < 0)
        ERR_EXIT();

end:
    g_free(status);

    g_clear_error(&error);

    if (service_info)
        alterator_ctl_module_info_parser_result_tree_free(service_info);

    if (parser)
        g_object_unref(parser);

    if (params_ctx)
        g_hash_table_destroy(params_ctx);

    return ret;
}

static gchar *services_module_json_params_to_text(JsonNode *params, const gchar *service_str_id)
{
    gchar *result = NULL;
    if (!params)
    {
        g_printerr(_("Can't get text of unspecified json parameters of %s service.\n"), service_str_id);
        return result;
    }

    JsonGenerator *gen = json_generator_new();
    json_generator_set_root(gen, params);
    json_generator_set_pretty(gen, TRUE);
    json_generator_set_indent(gen, 2);
    result = json_generator_to_data(gen, NULL);
    g_object_unref(gen);

    return result;
}

static gint services_module_sort_result(gconstpointer a, gconstpointer b)
{
    const gchar *first_comparable_data  = (const gchar *) ((GPtrArray *) a)->pdata;
    const gchar *second_comparable_data = (const gchar *) ((GPtrArray *) b)->pdata;
    return g_utf8_collate(first_comparable_data, second_comparable_data);
}

static GPtrArray *services_module_get_sorted_string_keys(GHashTable *table)
{
    if (!table || !g_hash_table_size(table))
        return NULL;

    GPtrArray *keys = g_hash_table_get_keys_as_ptr_array(table);
    g_ptr_array_sort(keys, services_module_sort_result);
    return keys;
}

static gint services_module_sort_options(gconstpointer a, gconstpointer b)
{
    const ParamEntry *pa = a;
    const ParamEntry *pb = b;
    return g_utf8_collate(pa->option_entry.long_name, pb->option_entry.long_name);
}

static service_display_entry *service_display_entry_new(gchar *label, deployment_status status)
{
    service_display_entry *entry = g_new0(service_display_entry, 1);
    entry->label                 = label ? label : g_strdup("");
    entry->status                = status;
    return entry;
}

static void service_display_entry_free(service_display_entry *entry)
{
    if (!entry)
        return;

    g_free(entry->label);
    g_free(entry);
}

static gint services_module_sort_entries(gconstpointer a, gconstpointer b)
{
    const service_display_entry *first  = *(const service_display_entry *const *) a;
    const service_display_entry *second = *(const service_display_entry *const *) b;
    const gchar *first_label            = first && first->label ? first->label : "";
    const gchar *second_label           = second && second->label ? second->label : "";
    return g_utf8_collate(first_label, second_label);
}

static gchar *services_module_build_service_label(AlteratorCtlServicesModule *module,
                                                  const gchar *service_path,
                                                  const service_names_t *service_names)
{
    const gchar *name = service_names && service_names->name ? service_names->name : (service_path ? service_path : "");
    const gchar *display_name = service_names && service_names->display_name ? service_names->display_name : name;
    const gchar *path         = service_path ? service_path : "";

    if (module->alterator_ctl_app && module->alterator_ctl_app->arguments
        && module->alterator_ctl_app->arguments->verbose)
    {
        if (!no_display_name)
            return g_strdup_printf("%s (%s: %s)", display_name, name, path);

        return g_strdup_printf("%s: %s", name, path);
    }

    if (display_name_only || no_name)
        return g_strdup(display_name);

    if (no_display_name || name_only)
        return g_strdup(name);

    if (path_only)
        return g_strdup(path);

    return g_strdup_printf("%s (%s)", display_name, name);
}

static const gchar *services_module_status_text(deployment_status status)
{
    switch (status)
    {
    case UNDEPLOYED:
        return _("not deployed");
    case DEPLOYED:
        return _("deployed, stopped");
    case DEPLOYED_AND_STARTED:
        return _("deployed, running");
    default:
        return _("unknown status");
    }
}

static const gchar *services_module_marker_shape_for_status(deployment_status status)
{
    switch (status)
    {
    case DEPLOYED_AND_STARTED:
        return "■"; // running
    case DEPLOYED:
        return "◆"; // deployed, stopped
    case UNDEPLOYED:
    default:
        return "▲"; // not deployed
    }
}

static text_color services_module_color_for_status(deployment_status status)
{
    switch (status)
    {
    case DEPLOYED_AND_STARTED:
        return GREEN;
    case DEPLOYED:
        return YELLOW;
    case UNDEPLOYED:
    default:
        return RED;
    }
}

static gchar *services_module_format_entry(const service_display_entry *entry,
                                           gsize max_label_len,
                                           gboolean show_statuses)
{
    if (!entry)
        return NULL;

    if (!show_statuses)
        return g_strdup(entry->label ? entry->label : "");

    const gchar *label       = entry->label ? entry->label : "";
    const gchar *shape       = services_module_marker_shape_for_status(entry->status);
    text_color color         = services_module_color_for_status(entry->status);
    gboolean colorize        = isatty_safe(STDOUT_FILENO) == TTY;
    gchar *marker            = colorize ? colorize_text(shape, color) : g_strdup(shape);
    const gchar *status_text = services_module_status_text(entry->status);

    gsize label_len    = g_utf8_strlen(label, -1);
    gsize padding_size = max_label_len > label_len ? max_label_len - label_len : 0;

    GString *line = g_string_new(NULL);
    g_string_append(line, marker);
    g_string_append_c(line, ' ');
    g_string_append(line, label);
    for (gsize i = 0; i < padding_size; i++)
        g_string_append_c(line, ' ');
    g_string_append_c(line, ' ');
    g_string_append(line, status_text);

    g_free(marker);
    return g_string_free(line, FALSE);
}

static void services_module_print_status_legend(void)
{
    g_print("\n%s\n", _("Status legend:"));
    gboolean colorize = isatty_safe(STDOUT_FILENO) == TTY;

    const gchar *shape_running    = services_module_marker_shape_for_status(DEPLOYED_AND_STARTED);
    const gchar *shape_deployed   = services_module_marker_shape_for_status(DEPLOYED);
    const gchar *shape_undeployed = services_module_marker_shape_for_status(UNDEPLOYED);

    gchar *mark_running    = colorize ? colorize_text(shape_running, GREEN) : g_strdup(shape_running);
    gchar *mark_deployed   = colorize ? colorize_text(shape_deployed, YELLOW) : g_strdup(shape_deployed);
    gchar *mark_undeployed = colorize ? colorize_text(shape_undeployed, RED) : g_strdup(shape_undeployed);

    g_print("  %s %s\n", mark_running, services_module_status_text(DEPLOYED_AND_STARTED));
    g_print("  %s %s\n", mark_deployed, services_module_status_text(DEPLOYED));
    g_print("  %s %s\n", mark_undeployed, services_module_status_text(UNDEPLOYED));

    g_free(mark_running);
    g_free(mark_deployed);
    g_free(mark_undeployed);
}

static void print_json_indent(gsize indent)
{
    for (int i = 0; i < indent; i++)
        g_print("  ");
}

static int print_json_node(JsonNode *node, const gchar *key, GHashTable *custom_types, gchar *end_of_line, gint indent)
{
    int ret           = 0;
    JsonNodeType type = json_node_get_node_type(node);

    switch (type)
    {
    case JSON_NODE_VALUE: {
        GType nodetype = json_node_get_value_type(node);
        print_json_indent(indent);
        if (key && strlen(key))
            g_print("%s: ", key);

        if (nodetype == G_TYPE_STRING)
            g_print("\"%s\"%s\n", json_node_get_string(node), end_of_line);
        else if (nodetype == G_TYPE_INT64)
            g_print("%ld%s\n", json_node_get_int(node), end_of_line);
        else if (nodetype == G_TYPE_BOOLEAN)
            g_print("%s%s\n", json_node_get_boolean(node) ? "true" : "false", end_of_line);
        else
            g_print("(unsupported)\n");
        break;
    }

    case JSON_NODE_ARRAY: {
        JsonArray *arr = json_node_get_array(node);
        gsize length   = json_array_get_length(arr);

        print_json_indent(indent);
        if (key && strlen(key))
            g_print("%s: [\n", key);
        else
            g_print("[\n");

        for (gsize i = 0; i < length; i++)
        {
            JsonNode *elem = json_array_get_element(arr, i);
            print_json_node(elem, "", custom_types, ",", indent + 1);
        }

        print_json_indent(indent);
        g_print("]\n");
        break;
    }

    case JSON_NODE_OBJECT: {
        JsonObject *obj = json_node_get_object(node);
        GList *members  = json_object_get_members(obj);

        print_json_indent(indent);
        if (key && strlen(key))
            g_print("%s: {\n", key);
        else
            g_print("{\n");

        for (GList *l = members; l; l = l->next)
        {
            const gchar *subkey = l->data;
            JsonNode *subn      = json_object_get_member(obj, subkey);
            print_json_node(subn, subkey, custom_types, "", indent + 1);
        }
        g_list_free(members);

        print_json_indent(indent);
        g_print("}%s\n", end_of_line);
        break;
    }

    default:
        print_json_indent(indent);
        if (key && strlen(key))
            g_print("%s: (unknown type)\n", key);
        else
            g_print("(unknown type)\n");
    }

end:
    return ret;
}

static gchar *services_module_print_resource_with_links_to_params(gchar *resource_name, gpointer resource_to_param_table)
{
    GHashTable *resource_to_params = (GHashTable *) resource_to_param_table;
    return g_hash_table_lookup(resource_to_params, resource_name);
}

static int services_module_printing_resources_value(AlteratorCtlServicesModule *module,
                                                    const gchar *service_str_id,
                                                    const GNode *resources,
                                                    ServicesModuleTomlValuePrintingMode mode,
                                                    gpointer user_data)
{
    int ret                                   = 0;
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;
    GHashTable *resources_to_params           = (GHashTable *) user_data;
    GNode *service_info                       = NULL;
    GNode *parameters_node                    = NULL;
    GPtrArray *groups[SERVICES_RESOURCE_GROUP_COUNT] = {0};
    gboolean printed_any_group                       = FALSE;
    const gchar *header_left                         = _("Name");
    const gchar *header_right                        = _("Value");
    GString *output                                  = g_string_new(NULL);

    if (!service_str_id)
    {
        g_printerr(_("Can't get info of unspecified service.\n"));
        ERR_EXIT();
    }

    if (!resources)
    {
        g_printerr(_("Can't save %s service resources to unvaliable container.\n"), service_str_id);
        ERR_EXIT();
    }

    if (info_parser->alterator_ctl_module_info_parser_get_specified_object_data(info_parser,
                                                                                module->gdbus_source,
                                                                                service_str_id,
                                                                                SERVICES_INTERFACE_NAME,
                                                                                &service_info)
        < 0)
    {
        g_printerr(_("Can't get service %s data for resources output.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!service_info)
    {
        g_printerr(_("Can't print resources of service %s: empty service info.\n"), service_str_id);
        ERR_EXIT();
    }

    parameters_node = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
        info_parser, service_info, SERVICES_ALTERATOR_ENTRY_SERVICE_PARAMETERS_TABLE_NAME);

    for (gsize i = 0; i < SERVICES_RESOURCE_GROUP_COUNT; i++)
        groups[i] = g_ptr_array_new_with_free_func(services_module_resource_row_free);

    for (GNode *resource = resources->children; resource != NULL; resource = resource->next)
    {
        alterator_entry_node *resource_data = (alterator_entry_node *) resource->data;
        gchar *resource_name                = resource_data->node_name;
        toml_value *resource_initial_value  = NULL;
        if (info_parser->alterator_ctl_module_info_parser_find_value(
                info_parser,
                resource,
                &resource_initial_value,
                SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCE_VALUE_KEY_NAME,
                NULL)
                < 0
            && !resource_initial_value)
        {
            g_printerr(_("Can't get non-existance value of resource %s in %s service.\n"),
                       resource_name,
                       service_str_id);
            ERR_EXIT();
        }

        toml_value *resource_type = NULL;
        if (!info_parser
                 ->alterator_ctl_module_info_parser_find_value(info_parser, resource, &resource_type, "type", NULL))
            resource_type = NULL;

        const gchar *type_str = resource_type ? resource_type->str_value : NULL;

        services_module_resource_group_t resource_group = services_module_get_resource_group(type_str);
        if (resource_group < 0 || resource_group >= SERVICES_RESOURCE_GROUP_COUNT)
            resource_group = SERVICES_RESOURCE_GROUP_OTHER;

        gchar *resource_display
            = services_module_get_localized_field(info_parser,
                                                  resource,
                                                  SERVICES_ALTERATOR_ENTRY_SERVICE_DISPLAY_NAME_TABLE_NAME);
        if (!resource_display)
            resource_display = g_strdup(resource_name);

        gchar *resource_comment
            = services_module_get_localized_field(info_parser,
                                                  resource,
                                                  SERVICES_ALTERATOR_ENTRY_SERVICE_COMMENT_NAME_TABLE_NAME);

        gchar *value_string = NULL;
        {
            deployment_status deploy_status = UNDEPLOYED;
            if (module->json && services_module_status_params_get_state(module->json, &deploy_status) < 0)
                ERR_EXIT();

            gchar *linked_param = g_hash_table_lookup(resources_to_params, resource_name);

            if (deploy_status == UNDEPLOYED || !linked_param || !strlen(linked_param))
                value_string = services_module_resource_value_to_string(resource_initial_value);
            else
            {
                JsonObject *object              = json_node_get_object(module->json);
                JsonNode *resource_linked_param = json_object_get_member(object,
                                                                         g_hash_table_lookup(resources_to_params,
                                                                                             resource_name));
                if (!resource_linked_param)
                    continue;

                JsonNodeType node_type = json_node_get_node_type(resource_linked_param);
                if (node_type != JSON_NODE_VALUE)
                {
                    g_printerr(_("Failed of getting resource \"%s\" linked param: it isn't json simple value type.\n"),
                               resource_name);
                    ERR_EXIT();
                }

                if (services_module_status_value_to_str(resource_linked_param, &value_string) < 0)
                    ERR_EXIT();
            }

            if (!value_string)
                value_string = g_strdup("");
        }

        if (type_str && streq(type_str, "port"))
        {
            gboolean has_tcp      = FALSE;
            gboolean has_udp      = FALSE;
            toml_value *tcp_value = NULL;
            toml_value *udp_value = NULL;

            if (!info_parser->alterator_ctl_module_info_parser_find_value(info_parser, resource, &tcp_value, "tcp", NULL)
                || !tcp_value || tcp_value->type != TOML_DATA_BOOL)
            {
                g_printerr(_("Can't get required TCP flag of port resource %s in %s service.\n"),
                           resource_name,
                           service_str_id);
                ERR_EXIT();
            }
            has_tcp = tcp_value->bool_value;

            if (!info_parser->alterator_ctl_module_info_parser_find_value(info_parser, resource, &udp_value, "udp", NULL)
                || !udp_value || udp_value->type != TOML_DATA_BOOL)
            {
                g_printerr(_("Can't get required UDP flag of port resource %s in %s service.\n"),
                           resource_name,
                           service_str_id);
                ERR_EXIT();
            }
            has_udp = udp_value->bool_value;

            if (has_tcp || has_udp)
            {
                gchar *value_with_proto = NULL;
                if (has_tcp && has_udp)
                    value_with_proto = g_strdup_printf("%s TCP/UDP", value_string);
                else if (has_tcp)
                    value_with_proto = g_strdup_printf("%s TCP", value_string);
                else if (has_udp)
                    value_with_proto = g_strdup_printf("%s UDP", value_string);

                if (value_with_proto)
                {
                    g_free(value_string);
                    value_string = value_with_proto;
                }
            }
        }

        services_module_resource_row_t *row = g_new0(services_module_resource_row_t, 1);
        row->name                           = resource_display;
        row->value                          = value_string;
        row->details                        = g_ptr_array_new_with_free_func(g_free);

        if (resource_comment && strlen(resource_comment))
        {
            gchar *comment_text = g_strdup_printf(_("Comment: %s"), resource_comment);
            gchar *detail       = g_strdup_printf("- %s", comment_text);
            g_free(comment_text);
            g_ptr_array_add(row->details, detail);
        }
        g_free(resource_comment);

        const gchar *linked_param_name = mode && resources_to_params ? mode(resource_name, resources_to_params) : NULL;

        if (linked_param_name)
        {
            gchar *param_display = NULL;
            gchar *param_comment = NULL;

            if (parameters_node)
            {
                GNode *parameter_node = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser,
                                                                                                       parameters_node,
                                                                                                       linked_param_name);

                if (parameter_node)
                {
                    param_display
                        = services_module_get_localized_field(info_parser,
                                                              parameter_node,
                                                              SERVICES_ALTERATOR_ENTRY_SERVICE_DISPLAY_NAME_TABLE_NAME);
                    param_comment
                        = services_module_get_localized_field(info_parser,
                                                              parameter_node,
                                                              SERVICES_ALTERATOR_ENTRY_SERVICE_COMMENT_NAME_TABLE_NAME);
                }
            }

            const gchar *param_display_effective = param_display ? param_display : linked_param_name;

            gchar *param_line_text = g_strdup_printf(_("Param: %s (%s)"), param_display_effective, linked_param_name);
            gchar *param_detail    = g_strdup_printf("- %s", param_line_text);
            g_free(param_line_text);
            g_ptr_array_add(row->details, param_detail);

            if (param_comment && strlen(param_comment))
            {
                gchar *param_comment_text = g_strdup_printf(_("Param comment: %s"), param_comment);
                gchar *param_comment_line = g_strdup_printf("- %s", param_comment_text);
                g_free(param_comment_text);
                g_ptr_array_add(row->details, param_comment_line);
            }

            g_free(param_display);
            g_free(param_comment);
        }

        g_ptr_array_add(groups[resource_group], row);
    }

    for (gsize group_index = 0; group_index < SERVICES_RESOURCE_GROUP_COUNT; group_index++)
    {
        GPtrArray *rows = groups[group_index];
        if (!rows || !rows->len)
            continue;

        if (printed_any_group)
            g_string_append_c(output, '\n');

        g_string_append_printf(output, "[%s]\n", _(services_module_resource_group_titles[group_index]));

        GString *table = services_module_render_resources_ascii_table(header_left, header_right, rows);
        if (table)
        {
            g_string_append(output, table->str);
            g_string_free(table, TRUE);
        }

        printed_any_group = TRUE;
    }

end:
    if (!ret && output && output->len)
        print_with_pager(output->str);

    if (output)
        g_string_free(output, TRUE);

    for (gsize i = 0; i < SERVICES_RESOURCE_GROUP_COUNT; i++)
        if (groups[i])
            g_ptr_array_unref(groups[i]);

    if (service_info)
        alterator_ctl_module_info_parser_result_tree_free(service_info);

    return ret;
}

static gchar *services_module_get_localized_field(AlteratorCtlModuleInfoParser *info_parser,
                                                  GNode *node,
                                                  const gchar *table_name)
{
    gchar *result = NULL;
    gchar *locale = NULL;

    if (!info_parser || !node || !table_name)
        return NULL;

    locale = alterator_ctl_get_language();

    GHashTable *table = NULL;
    if (info_parser->alterator_ctl_module_info_parser_find_table(info_parser, node, &table, -1, table_name, NULL))
    {
        toml_value *value = NULL;
        if (locale)
            value = g_hash_table_lookup(table, locale);
        if (!value)
            value = g_hash_table_lookup(table, LOCALE_FALLBACK);

        if (value && value->str_value)
            result = g_strdup(value->str_value);
    }

    if (locale)
        g_free(locale);

    return result;
}

static gchar *services_module_resource_value_to_string(const toml_value *value)
{
    if (!value)
        return NULL;

    gchar *stringified = services_module_strignify_toml_value(value);
    if (!stringified)
        return NULL;

    g_strchomp(stringified);

    if (value->type == TOML_DATA_STRING)
    {
        gsize len = strlen(stringified);
        if (len >= 2 && stringified[0] == '"' && stringified[len - 1] == '"')
        {
            gchar *unquoted = g_strndup(stringified + 1, len - 2);
            g_free(stringified);
            return unquoted;
        }
    }

    return stringified;
}

static services_module_resource_group_t services_module_get_resource_group(const gchar *type_str)
{
    if (!type_str)
        return SERVICES_RESOURCE_GROUP_OTHER;

    if (streq(type_str, "directory"))
        return SERVICES_RESOURCE_GROUP_DIRECTORIES;
    if (streq(type_str, "file"))
        return SERVICES_RESOURCE_GROUP_FILES;
    if (streq(type_str, "systemd_unit"))
        return SERVICES_RESOURCE_GROUP_UNITS;
    if (streq(type_str, "port"))
        return SERVICES_RESOURCE_GROUP_PORTS;

    return SERVICES_RESOURCE_GROUP_OTHER;
}

static void services_module_resource_row_free(gpointer data)
{
    services_module_resource_row_t *row = data;
    if (!row)
        return;

    g_free(row->name);
    g_free(row->value);
    if (row->details)
        g_ptr_array_unref(row->details);
    g_free(row);
}

static void services_module_append_spaces(GString *buffer, gsize count)
{
    for (gsize i = 0; i < count; i++)
        g_string_append_c(buffer, ' ');
}

static void services_module_append_ascii_border(GString *buffer, gsize left_width, gsize right_width, gchar fill)
{
    g_string_append_c(buffer, '+');
    for (gsize i = 0; i < left_width + 2; i++)
        g_string_append_c(buffer, fill);
    g_string_append_c(buffer, '+');
    for (gsize i = 0; i < right_width + 2; i++)
        g_string_append_c(buffer, fill);
    g_string_append_c(buffer, '+');
    g_string_append_c(buffer, '\n');
}

static void services_module_append_ascii_row(
    GString *buffer, gsize left_width, gsize right_width, const gchar *left, const gchar *right)
{
    const gchar *left_text  = left ? left : "";
    const gchar *right_text = right ? right : "";

    gsize left_len  = g_utf8_strlen(left_text, -1);
    gsize right_len = g_utf8_strlen(right_text, -1);

    g_string_append_c(buffer, '|');
    g_string_append_c(buffer, ' ');
    g_string_append(buffer, left_text);
    if (left_width > left_len)
        services_module_append_spaces(buffer, left_width - left_len);
    g_string_append_c(buffer, ' ');
    g_string_append_c(buffer, '|');
    g_string_append_c(buffer, ' ');
    g_string_append(buffer, right_text);
    if (right_width > right_len)
        services_module_append_spaces(buffer, right_width - right_len);
    g_string_append_c(buffer, ' ');
    g_string_append_c(buffer, '|');
    g_string_append_c(buffer, '\n');
}

static GString *services_module_render_resources_ascii_table(const gchar *left_header,
                                                             const gchar *right_header,
                                                             const GPtrArray *rows)
{
    if (!rows || !rows->len)
        return NULL;

    gsize left_width  = g_utf8_strlen(left_header, -1);
    gsize right_width = g_utf8_strlen(right_header, -1);

    for (gsize i = 0; i < rows->len; i++)
    {
        services_module_resource_row_t *row = g_ptr_array_index(rows, i);
        if (!row)
            continue;

        if (row->name)
        {
            gsize len = g_utf8_strlen(row->name, -1);
            if (len > left_width)
                left_width = len;
        }

        if (row->value)
        {
            gsize len = g_utf8_strlen(row->value, -1);
            if (len > right_width)
                right_width = len;
        }

        if (row->details)
        {
            for (gsize d = 0; d < row->details->len; d++)
            {
                gchar *detail = g_ptr_array_index(row->details, d);
                if (!detail)
                    continue;
                gsize len = g_utf8_strlen(detail, -1);
                if (len > left_width)
                    left_width = len;
            }
        }
    }

    if (left_width < SERVICES_MODULE_MIN_PARAMS_PATH_COLUMN_WIDTH)
        left_width = SERVICES_MODULE_MIN_PARAMS_PATH_COLUMN_WIDTH;
    if (right_width < SERVICES_MODULE_MIN_PARAMS_VALUE_COLUMN_WIDTH)
        right_width = SERVICES_MODULE_MIN_PARAMS_VALUE_COLUMN_WIDTH;

    GString *buffer = g_string_new(NULL);

    services_module_append_ascii_border(buffer, left_width, right_width, '-');
    services_module_append_ascii_row(buffer, left_width, right_width, left_header, right_header);
    services_module_append_ascii_border(buffer, left_width, right_width, '=');

    for (gsize i = 0; i < rows->len; i++)
    {
        services_module_resource_row_t *row = g_ptr_array_index(rows, i);
        if (!row)
            continue;

        services_module_append_ascii_row(buffer, left_width, right_width, row->name, row->value);

        if (row->details)
        {
            for (gsize d = 0; d < row->details->len; d++)
            {
                gchar *detail = g_ptr_array_index(row->details, d);
                services_module_append_ascii_row(buffer, left_width, right_width, detail, "");
            }
        }

        if (i + 1 < rows->len)
            services_module_append_ascii_border(buffer, left_width, right_width, '-');
    }

    services_module_append_ascii_border(buffer, left_width, right_width, '-');

    return buffer;
}

// TODO: add optional GNode* argument for optimise getting service info
static int services_module_get_initial_params(AlteratorCtlServicesModule *module,
                                              const gchar *service_str_id,
                                              JsonNode **result,
                                              gboolean required_only,
                                              gchar *required_only_context,
                                              GHashTable **params_ctx,
                                              gboolean exclude_unset_params)
{
    int ret                                   = 0;
    GNode *service_info                       = NULL;
    GHashTable *params_to_resources           = NULL;
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    if (!service_str_id)
    {
        g_printerr(_("Can't get initial parameters of unspecified service.\n"));
        ERR_EXIT();
    }

    if (info_parser->alterator_ctl_module_info_parser_get_specified_object_data(info_parser,
                                                                                module->gdbus_source,
                                                                                service_str_id,
                                                                                SERVICES_INTERFACE_NAME,
                                                                                &service_info)
        < 0)
    {
        g_printerr(_("Can't get service %s default value's.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!service_info)
    {
        g_printerr(_("Can't get initial params with empty info of %s service.\n"), service_str_id);
        ERR_EXIT();
    }

    if (params_ctx && services_module_get_params_context(module, service_str_id, service_info, params_ctx) < 0)
        ERR_EXIT();

    GNode *parameters = NULL;
    if (!(parameters = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
              info_parser, service_info, SERVICES_ALTERATOR_ENTRY_SERVICE_PARAMETERS_TABLE_NAME)))
    {
        g_printerr(_("No service %s initial parameters info.\n"), service_str_id);
        ERR_EXIT();
    }

    GNode *resources = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
        info_parser, service_info, SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCES_TABLE_NAME);

    if (resources && services_module_get_param_to_resource_table(module, resources, &params_to_resources) < 0)
        ERR_EXIT();

    for (GNode *parameter = parameters->children; parameter != NULL; parameter = parameter->next)
    {
        if (services_module_recursive_get_initial_params(module,
                                                         service_str_id,
                                                         service_info,
                                                         parameters,
                                                         parameter,
                                                         result,
                                                         required_only,
                                                         required_only_context,
                                                         NULL,
                                                         params_to_resources,
                                                         exclude_unset_params,
                                                         FALSE)
            < 0)
            ERR_EXIT();
    }

end:
    if (service_info)
        alterator_ctl_module_info_parser_result_tree_free(service_info);

    if (params_to_resources)
        g_hash_table_unref(params_to_resources);

    return ret;
}

static int services_module_recursive_get_initial_params(AlteratorCtlServicesModule *module,
                                                        const gchar *service_str_id,
                                                        GNode *service_info,
                                                        GNode *types,
                                                        GNode *parameter,
                                                        JsonNode **result,
                                                        gboolean required_only,
                                                        gchar *required_only_context,
                                                        JsonObject **nested_object,
                                                        GHashTable *params_to_resources,
                                                        gboolean exclude_unset_params,
                                                        gboolean constant_parent)
{
    int ret                                   = 0;
    JsonObject *root_object                   = json_node_get_object(*result);
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    if (!service_str_id)
    {
        g_printerr(_("Can't get initial parameters of unspecified service.\n"));
        ERR_EXIT();
    }

    const gchar *param_name = ((alterator_entry_node *) parameter->data)->node_name;
    GHashTable *param_data  = ((alterator_entry_node *) parameter->data)->toml_pairs;

    toml_value *type_val = g_hash_table_lookup(param_data, SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_TYPE_KEY_NAME);
    gboolean is_enum_field = !type_val
                             && streq(((alterator_entry_node *) parameter->parent->data)->node_name, "values");

    toml_value *is_constant_toml = g_hash_table_lookup(param_data,
                                                       SERVICES_ALTERATOR_ENTRY_SERVICE_PARAM_CONSTANT_KEY_NAME);
    gboolean is_constant = is_constant_toml && is_constant_toml->type == TOML_DATA_BOOL && is_constant_toml->bool_value
                           || constant_parent;

    // Skip optional parameters (exclude constants)
    if (!is_enum_field && !is_constant && required_only)
    {
        toml_value *is_required = g_hash_table_lookup(param_data,
                                                      SERVICES_ALTERATOR_ENTRY_SERVICE_PARAM_REQUIRED_KEY_NAME);
        if (!is_required)
            goto end;
        else if (is_required->type == TOML_DATA_BOOL)
        {
            if (!is_required->bool_value)
                goto end;
        }
        else if (is_required->type == TOML_DATA_ARRAY_OF_STRING)
        {
            if (!required_only_context || !strlen(required_only_context))
                goto end;

            gboolean has_context = FALSE;
            for (gsize i = 0; i < is_required->array_length; i++)
                has_context = has_context ? has_context
                                          : streq(required_only_context, ((gchar **) is_required->array)[i]);

            if (!has_context)
                goto end;
        }
        else
        {
            g_printerr(_("Incorrect key 'required' type in '%s' alterator entry.\n"), service_str_id);
            ERR_EXIT();
        }
    }

    gboolean is_enum_simple_field = is_enum_field
                                    && !info_parser->alterator_ctl_module_info_parser_get_node_by_name(
                                        info_parser, parameter, SERVICES_ALTERATOR_ENTRY_SERVICE_PROPERTIES_TABLE_NAME);
    gboolean is_composite_parameter = type_val ? streq(type_val->str_value, "object") : FALSE;

    gboolean is_enum = type_val ? streq(type_val->str_value, "enum") : FALSE;
    gboolean is_enum_with_subparams
        = is_enum ? info_parser->alterator_ctl_module_info_parser_find_table(
                        info_parser, parameter, NULL, -1, SERVICES_ALTERATOR_ENTRY_SERVICE_PROPERTIES_TABLE_NAME, NULL)
                  : FALSE;

    gboolean is_composite_array = FALSE;
    gboolean is_enum_array      = FALSE;
    if (type_val && streq(type_val->str_value, "array"))
    {
        toml_value *arr_type = (toml_value *) g_hash_table_lookup(param_data, "array_type");
        if (arr_type && arr_type->type == TOML_DATA_STRING)
        {
            if (streq(arr_type->str_value, "object"))
                is_composite_array = TRUE;
            else if (streq(arr_type->str_value, "enum"))
                is_enum_array = TRUE;
        }
    }

    JsonObject *target = (nested_object && *nested_object) ? *nested_object : root_object;

    if (is_enum)
    {
        GNode *values = NULL;
        if (!(values = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser, parameter, "values")))
            ERR_EXIT();

        toml_value *default_field_name
            = g_hash_table_lookup(param_data, SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_VALUE_KEY_NAME);

        if (default_field_name && default_field_name->type == TOML_DATA_STRING && strlen(default_field_name->str_value))
        {
            JsonObject *subobject = json_object_new();

            for (GNode *value = values->children; value != NULL; value = g_node_next_sibling(value))
            {
                if (streq(((alterator_entry_node *) value->data)->node_name, default_field_name->str_value))
                {
                    if (services_module_recursive_get_initial_params(module,
                                                                     service_str_id,
                                                                     service_info,
                                                                     types,
                                                                     value,
                                                                     result,
                                                                     required_only,
                                                                     required_only_context,
                                                                     &subobject,
                                                                     params_to_resources,
                                                                     FALSE,
                                                                     is_constant)
                        < 0)
                    {
                        json_object_unref(subobject);
                        ERR_EXIT();
                    }
                    break;
                }
            }

            json_object_set_object_member(target, param_name, subobject);
            return ret;
        }
    }

    if (is_composite_parameter || is_composite_array)
    {
        toml_value *prototype_value
            = g_hash_table_lookup(param_data, SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_PROTOTYPE_KEY_NAME);

        GNode *prototypes = prototype_value
                                ? info_parser
                                      ->alterator_ctl_module_info_parser_get_node_by_name(info_parser,
                                                                                          types,
                                                                                          prototype_value->str_value)
                                : parameter;

        GNode *properties = prototypes
                                ? info_parser->alterator_ctl_module_info_parser_get_node_by_name(
                                      info_parser, prototypes, SERVICES_ALTERATOR_ENTRY_SERVICE_PROPERTIES_TABLE_NAME)
                                : NULL;

        if ((properties != NULL) && g_node_n_children(properties) > 0)
        {
            if (is_enum_with_subparams)
            {
                toml_value *default_child_name
                    = g_hash_table_lookup(param_data, SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_VALUE_KEY_NAME);
                if (!default_child_name || !default_child_name->str_value)
                    goto end;

                JsonObject *subobject = json_object_new();

                for (GNode *prop = properties->children; prop != NULL; prop = prop->next)
                {
                    alterator_entry_node *child = prop->data;
                    if (streq(child->node_name, default_child_name->str_value))
                    {
                        if (services_module_recursive_get_initial_params(module,
                                                                         service_str_id,
                                                                         service_info,
                                                                         types,
                                                                         prop,
                                                                         result,
                                                                         required_only,
                                                                         required_only_context,
                                                                         &subobject,
                                                                         params_to_resources,
                                                                         FALSE,
                                                                         is_constant)
                            < 0)
                        {
                            json_object_unref(subobject);
                            ERR_EXIT();
                        }
                        break;
                    }
                }

                json_object_set_object_member(target, param_name, subobject);
                return ret;
            }
            else if (!is_composite_array)
            {
                JsonObject *subobject = json_object_new();
                for (GNode *prop = properties->children; prop != NULL; prop = prop->next)
                {
                    if (services_module_recursive_get_initial_params(module,
                                                                     service_str_id,
                                                                     service_info,
                                                                     types,
                                                                     prop,
                                                                     result,
                                                                     required_only,
                                                                     required_only_context,
                                                                     &subobject,
                                                                     params_to_resources,
                                                                     exclude_unset_params,
                                                                     is_constant)
                        < 0)
                    {
                        ERR_EXIT();
                    }
                }

                json_object_set_object_member(target, param_name, subobject);
                return ret;
            }
            else
            {
                JsonArray *array     = json_array_new();
                toml_value *min_toml = (toml_value *) g_hash_table_lookup(param_data, "array_min");
                gsize min = min_toml && min_toml->type == TOML_DATA_DOUBLE ? (gsize) min_toml->double_value : 0;
                for (gsize i = 0; i < min; i++)
                {
                    JsonObject *elem = json_object_new();
                    for (GNode *prop = properties->children; prop != NULL; prop = prop->next)
                    {
                        if (services_module_recursive_get_initial_params(module,
                                                                         service_str_id,
                                                                         service_info,
                                                                         types,
                                                                         prop,
                                                                         result,
                                                                         required_only,
                                                                         required_only_context,
                                                                         &elem,
                                                                         params_to_resources,
                                                                         exclude_unset_params,
                                                                         is_constant)
                            < 0)
                        {
                            ERR_EXIT();
                        }
                    }
                    json_array_add_object_element(array, elem);
                }

                json_object_set_array_member(target, param_name, array);
                return ret;
            }
        }
    }

    if (is_enum_array)
    {
        GNode *values = NULL;
        if (!(values = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser, parameter, "values")))
        {
            g_printerr(_("Failed to get enum fields of \"%s\" array elements.\n"), param_name);
            ERR_EXIT();
        }

        JsonArray *array = json_array_new();

        toml_value *default_fields_array = (toml_value *)
            g_hash_table_lookup(param_data, SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_VALUE_KEY_NAME);

        if (default_fields_array && default_fields_array->type == TOML_DATA_ARRAY_OF_STRING
            && default_fields_array->array_length)
        {
            for (gsize i = 0; i < default_fields_array->array_length; i++)
            {
                JsonObject *elem = json_object_new();

                GNode *default_field_node = NULL;
                {
                    gchar *default_field_name = ((gchar **) default_fields_array->array)[i];
                    if (!default_field_name || !strlen(default_field_name))
                    {
                        g_printerr(_("Failed to get enum fields of \"%s\" array element with index %lu.\n"),
                                   param_name,
                                   i);
                        ERR_EXIT();
                    }

                    if (!(default_field_node = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
                              info_parser, values, default_field_name)))
                    {
                        g_printerr(_("Failed to get enum fields of \"%s\" array element with index %lu.\n"),
                                   param_name,
                                   i);
                        ERR_EXIT();
                    }
                }

                if (services_module_recursive_get_initial_params(module,
                                                                 service_str_id,
                                                                 service_info,
                                                                 types,
                                                                 default_field_node,
                                                                 result,
                                                                 required_only,
                                                                 required_only_context,
                                                                 &elem,
                                                                 params_to_resources,
                                                                 FALSE,
                                                                 is_constant)
                    < 0)
                {
                    json_object_unref(elem);
                    ERR_EXIT();
                }

                json_array_add_object_element(array, elem);
            }

            json_object_set_array_member(target, param_name, array);
            return ret;
        }
    }

    toml_value *value = g_hash_table_lookup(param_data, SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_VALUE_KEY_NAME);
    if (!value) // Get value of param which linked with some resource
    {
        gchar *resource_name = NULL;
        if (params_to_resources && (resource_name = g_hash_table_lookup(params_to_resources, param_name)))
        {
            GNode *resources = NULL;
            if (!(resources = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
                      info_parser, service_info, SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCES_TABLE_NAME)))
                return ret;

            GNode *resource = NULL;
            if (!(resource = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser,
                                                                                            resources,
                                                                                            resource_name)))
            {
                g_printerr(_("No service %s resources info.\n"), service_str_id);
                ERR_EXIT();
            }

            if (!info_parser->alterator_ctl_module_info_parser_find_value(
                    info_parser, resource, &value, SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCE_VALUE_KEY_NAME, NULL))
                return ret;
        }
    }

    if (!value)
    {
        if (is_enum_simple_field)
        {
            JsonObject *object = json_object_new();
            json_object_set_object_member(target, param_name, object);
        }
        else if (!exclude_unset_params)
        {
            if (!is_enum_field)
                json_object_set_null_member(target, param_name);
            else
            {
                JsonObject *empty_subparam = json_object_new();
                json_object_set_object_member(target, param_name, empty_subparam);
            }
        }
        return ret;
    }

    switch (value->type)
    {
    case TOML_DATA_STRING: {
        const gchar *parent_name = ((alterator_entry_node *) parameter->parent->data)->node_name;
        if (streq(parent_name, "values"))
        {
            JsonObject *object = json_object_new();
            json_object_set_object_member(target, param_name, object);
        }
        else
        {
            json_object_set_string_member(target, param_name, value->str_value);
        }
        break;
    }
    case TOML_DATA_DOUBLE:
        if (value->double_value != (gint64) value->double_value)
            json_object_set_double_member(target, param_name, value->double_value);
        else
            json_object_set_int_member(target, param_name, (gint64) value->double_value);
        break;
    case TOML_DATA_BOOL:
        json_object_set_boolean_member(target, param_name, value->bool_value);
        break;
    case TOML_DATA_ARRAY_OF_INT: {
        JsonArray *arr = json_array_new();
        for (gsize i = 0; i < value->array_length; i++)
            json_array_add_int_element(arr, ((gint64 *) value->array)[i]);
        json_object_set_array_member(target, param_name, arr);
    }
    break;
    case TOML_DATA_ARRAY_OF_DOUBLE: {
        JsonArray *arr = json_array_new();
        for (gsize i = 0; i < value->array_length; i++)
            json_array_add_double_element(arr, ((gdouble *) value->array)[i]);
        json_object_set_array_member(target, param_name, arr);
    }
    break;
    case TOML_DATA_ARRAY_OF_BOOL: {
        JsonArray *arr = json_array_new();
        for (gsize i = 0; i < value->array_length; i++)
            json_array_add_boolean_element(arr, ((gint *) value->array)[i]);
        json_object_set_array_member(target, param_name, arr);
    }
    break;
    case TOML_DATA_ARRAY_OF_STRING: {
        JsonArray *arr = json_array_new();
        for (gsize i = 0; i < value->array_length; i++)
            json_array_add_string_element(arr, ((gchar **) value->array)[i]);
        json_object_set_array_member(target, param_name, arr);
    }
    break;
    default:
        break;
    }

end:
    return ret;
}

static int services_module_get_params_context(AlteratorCtlServicesModule *module,
                                              const gchar *service_str_id,
                                              GNode *info,
                                              GHashTable **params_ctx)
{
    int ret                                   = 0;
    *params_ctx                               = g_hash_table_new_full(g_str_hash,
                                        g_str_equal,
                                        (GDestroyNotify) g_free,
                                        (GDestroyNotify) services_module_service_info_free);
    GHashTable *params_to_resources           = NULL;
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    GNode *parameters = NULL;
    if (!(parameters = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
              info_parser, info, SERVICES_ALTERATOR_ENTRY_SERVICE_PARAMETERS_TABLE_NAME)))
    {
        g_printerr(_("No service %s initial parameters info.\n"), service_str_id);
        ERR_EXIT();
    }

    GNode *resources = NULL;
    if ((resources = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
             info_parser, info, SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCES_TABLE_NAME)))
    {
        if (services_module_get_param_to_resource_table(module, resources, &params_to_resources) < 0)
            ERR_EXIT();
    }

    for (GNode *parameter = parameters->children; parameter != NULL; parameter = parameter->next)
    {
        alterator_entry_node *param_data = (alterator_entry_node *) parameter->data;
        const gchar *param_name          = param_data->node_name;
        GNode *current_param_ctx         = NULL;
        if (!(current_param_ctx = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
                  info_parser, info, SERVICES_ALTERATOR_ENTRY_SERVICE_PARAMETERS_TABLE_NAME)))
        {
            g_printerr(_("No service %s context usage of %s parameter.\n"), service_str_id, param_name);
            ERR_EXIT();
        }

        toml_value *required_arr = g_hash_table_lookup(param_data->toml_pairs,
                                                       SERVICES_ALTERATOR_ENTRY_SERVICE_PARAM_REQUIRED_KEY_NAME);

        GHashTable *
            requires
        = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);
        if (required_arr && required_arr->array_length && required_arr->array)
            for (gsize i = 0; i < required_arr->array_length; i++)
                g_hash_table_add(requires, g_strdup(((gchar **) required_arr->array)[i]));

        GHashTable *contexts    = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);
        toml_value *context_arr = g_hash_table_lookup(param_data->toml_pairs,
                                                      SERVICES_ALTERATOR_ENTRY_SERVICE_PARAM_CONTEXT_KEY_NAME);

        if (context_arr && context_arr->array_length && context_arr->array)
            for (gsize i = 0; i < context_arr->array_length; i++)
                g_hash_table_add(contexts, g_strdup(((gchar **) context_arr->array)[i]));

        toml_value *constant_val = g_hash_table_lookup(param_data->toml_pairs,
                                                       SERVICES_ALTERATOR_ENTRY_SERVICE_PARAM_CONSTANT_KEY_NAME);
        gboolean constant        = constant_val && constant_val->bool_value;

        gchar *linked_resource     = params_to_resources ? g_hash_table_lookup(params_to_resources, param_name) : NULL;
        param_info *param_contexts = services_module_service_info_init(param_name,
                                                                       linked_resource,
                                                                       requires,
                                                                       contexts,
                                                                       constant);
        g_hash_table_insert(*params_ctx, g_strdup((gchar *) param_name), param_contexts);
    }

end:
    if (params_to_resources)
        g_hash_table_unref(params_to_resources);

    return ret;
}

static int services_module_get_param_to_resource_table(AlteratorCtlServicesModule *module,
                                                       GNode *resources,
                                                       GHashTable **result)
{
    int ret = 0;
    if (!resources)
        return ret;

    *result                                   = g_hash_table_new(g_str_hash, g_str_equal);
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    for (GNode *resource = resources->children; resource != NULL; resource = resource->next)
    {
        toml_value *resource_val = NULL;
        gchar *resource_name     = ((alterator_entry_node *) resource->data)->node_name;
        if (info_parser->alterator_ctl_module_info_parser_find_value(
                info_parser,
                resource,
                &resource_val,
                SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCES_PARAMETER_KEY_NAME,
                NULL))
        {
            if (resource_val && resource_val->str_value)
                g_hash_table_insert(*result, resource_val->str_value, resource_name);
        }
    }

end:
    return ret;
}

static JsonNode *services_module_merge_json_data(JsonNode *first, JsonNode *second, const gchar *service_str_id)
{
    JsonNode *result = NULL;
    if (!JSON_NODE_HOLDS_OBJECT(first))
    {
        g_printerr(_("Invalid json data of service %s to update current data.\n"), service_str_id);
        return NULL;
    }

    if (!JSON_NODE_HOLDS_OBJECT(second))
    {
        g_printerr(_("Invalid json data of service %s to update current data.\n"), service_str_id);
        return NULL;
    }

    result = json_node_copy(first);
    services_module_recursive_merge_json_data(result, second, service_str_id);
    return result;
}

static int services_module_recursive_merge_json_data(JsonNode *target, JsonNode *source, const gchar *service_str_id)
{
    int ret        = 0;
    GList *members = NULL;

    if (!JSON_NODE_HOLDS_OBJECT(target) || !JSON_NODE_HOLDS_OBJECT(source))
    {
        g_printerr(_("Invalid json data of service %s to update current data.\n"), service_str_id);
        ERR_EXIT();
    }

    JsonObject *target_object = json_node_get_object(target);
    JsonObject *source_object = json_node_get_object(source);
    members                   = json_object_get_members(source_object);

    for (GList *member = members; member != NULL; member = member->next)
    {
        const gchar *key       = (gchar *) member->data;
        JsonNode *source_value = json_object_get_member(source_object, key);

        if (json_object_has_member(target_object, key))
        {
            JsonNode *target_value = json_object_get_member(target_object, key);

            if (JSON_NODE_HOLDS_OBJECT(target_value) && JSON_NODE_HOLDS_OBJECT(source_value))
                services_module_recursive_merge_json_data(target_value, source_value, service_str_id);
            else
            {
                JsonNode *copy = json_node_copy(source_value);
                json_object_set_member(target_object, key, copy);
            }
        }
        else
        {
            JsonNode *copy = json_node_copy(source_value);
            json_object_set_member(target_object, key, copy);
        }
    }

end:
    if (members)
        g_list_free(members);

    return ret;
}

static int services_module_recursive_filter_params_by_context(JsonNode *data,
                                                              GHashTable *ctx_data,
                                                              gchar *context_name,
                                                              gboolean constants_only)
{
    int ret            = 0;
    gboolean is_status = streq(context_name, "status");
    switch (json_node_get_node_type(data))
    {
    case JSON_NODE_OBJECT: {
        JsonObject *object = json_node_get_object(data);
        GList *members     = json_object_get_members(object);
        for (GList *member = members; member != NULL; member = member->next)
        {
            const gchar *member_name = (gchar *) member->data;
            JsonNode *child          = json_object_get_member(object, member_name);
            param_info *info         = g_hash_table_lookup(ctx_data, member_name);
            if (info)
            {
                gboolean allowed = constants_only ? info->constant : TRUE;

                if (allowed)
                {
                    if (info->usage_ctx && !g_hash_table_contains(info->usage_ctx, context_name))
                        allowed = FALSE;
                    if (!is_status && g_hash_table_size(info->required)
                        && !g_hash_table_contains(info->required, context_name))
                        allowed = FALSE;
                }

                if (!allowed)
                {
                    json_object_remove_member(object, member_name);
                    continue;
                }
            }

            services_module_recursive_filter_params_by_context(child, ctx_data, context_name, FALSE);
        }
        g_list_free(members);
        break;
    }
    case JSON_NODE_ARRAY: {
        JsonArray *arr = json_node_get_array(data);
        guint len      = json_array_get_length(arr);
        for (guint i = 0; i < len; i++)
        {
            JsonNode *elem = json_array_get_element(arr, i);
            services_module_recursive_filter_params_by_context(elem, ctx_data, context_name, FALSE);
        }
        break;
    }
    default:
        break;
    }

end:
    return ret;
}

static JsonNode *services_module_filter_params_by_context(JsonNode *data,
                                                          GHashTable *ctx_data,
                                                          gchar *context_name,
                                                          gboolean constants_only)
{
    JsonNode *root = json_node_copy(data);
    json_object_remove_member(json_node_get_object(root),
                              SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_DEPLOYED_KEY_NAME);
    json_object_remove_member(json_node_get_object(root),
                              SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_STARTED_KEY_NAME);
    services_module_recursive_filter_params_by_context(root, ctx_data, context_name, constants_only);
    return root;
}

static param_info *services_module_service_info_init(
    const gchar *name, const gchar *linked_resource, GHashTable *required, GHashTable *ctx, gboolean constant)
{
    param_info *result      = g_new0(param_info, 1);
    result->name            = g_strdup(name);
    result->linked_resource = g_strdup(linked_resource);
    result->required        = required;
    result->usage_ctx       = ctx;
    result->constant        = constant;
    return result;
}

static void services_module_service_info_free(param_info *info)
{
    if (!info)
        return;

    g_free((gchar *) info->name);
    g_free((gchar *) info->linked_resource);

    if (info->usage_ctx)
        g_hash_table_destroy(info->usage_ctx);

    if (info->required)
        g_hash_table_destroy(info->required);

    g_free(info);
}

static diag_tool_test_info *services_module_service_diag_tool_test_info_init_from_node(
    AlteratorCtlModuleInfoParser *info_parser, GNode *tool_info, gboolean is_required, diag_tool_test_mode mode)
{
    gchar *lang = NULL;
    if (!tool_info)
        return NULL;

    alterator_entry_node *data = tool_info->data;
    if (!data)
        return NULL;

    if (!(lang = alterator_ctl_get_language()))
        return NULL;

    GHashTable *display_name_table = NULL;
    if (!info_parser
             ->alterator_ctl_module_info_parser_find_table(info_parser,
                                                           tool_info,
                                                           &display_name_table,
                                                           -1,
                                                           SERVICES_ALTERATOR_ENTRY_SERVICE_DISPLAY_NAME_TABLE_NAME,
                                                           NULL))
    {
        g_free(lang);
        return NULL;
    }

    toml_value *display_name = g_hash_table_lookup(display_name_table, lang);
    if (!display_name)
        display_name = g_hash_table_lookup(display_name_table, "en");

    GHashTable *comment_table = NULL;
    if (!info_parser->alterator_ctl_module_info_parser_find_table(
            info_parser, tool_info, &comment_table, -1, SERVICES_ALTERATOR_ENTRY_SERVICE_COMMENT_NAME_TABLE_NAME, NULL))
    {
        g_free(lang);
        return NULL;
    }

    toml_value *comment = g_hash_table_lookup(comment_table, lang);
    if (!comment)
        comment = g_hash_table_lookup(comment_table, "en");

    g_free(lang);

    return services_module_service_diag_tool_test_info_init(data->node_name,
                                                            display_name ? display_name->str_value : NULL,
                                                            comment ? comment->str_value : NULL,
                                                            is_required,
                                                            mode);
}

static diag_tool_test_info *services_module_service_diag_tool_test_info_init(const gchar *name,
                                                                             const gchar *display_name,
                                                                             const gchar *description,
                                                                             gboolean is_required,
                                                                             diag_tool_test_mode mode)
{
    diag_tool_test_info *result = g_new0(diag_tool_test_info, 1);
    result->name                = g_strdup(name);
    result->display_name        = g_strdup(display_name);
    result->description         = g_strdup(description);
    result->is_required         = is_required;
    result->mode                = mode;
    return result;
}

static void services_module_service_diag_tool_test_info_free(diag_tool_test_info *tool_tests)
{
    if (!tool_tests)
        return;

    g_free(tool_tests->name);
    g_free(tool_tests->description);
    g_free(tool_tests->display_name);
    g_free(tool_tests);
}

static diag_tool_tests *services_module_service_diag_tool_tests_init(const gchar *path,
                                                                     GHashTable *all_tests,
                                                                     GHashTable *required_only_tests,
                                                                     GBusType bus_type)
{
    diag_tool_tests *result = g_new0(diag_tool_tests, 1);
    result->path            = g_strdup(path);
    result->all_tests       = all_tests ? g_hash_table_ref(all_tests) : NULL;
    result->required_tests  = required_only_tests ? g_hash_table_ref(required_only_tests) : NULL;
    result->bus_type        = bus_type;
    result->is_no_tests     = services_module_service_diag_tool_tests_is_no_tests;
    return result;
}

static void services_module_service_diag_tool_tests_free(diag_tool_tests *tool_tests)
{
    if (!tool_tests)
        return;

    g_free(tool_tests->path);

    g_free(tool_tests->entry_name);
    g_free(tool_tests->diag_name);

    if (tool_tests->all_tests)
        g_hash_table_unref(tool_tests->all_tests);

    if (tool_tests->required_tests)
        g_hash_table_unref(tool_tests->required_tests);

    g_free(tool_tests);
}

static gboolean services_module_service_diag_tool_tests_is_no_tests(struct diag_tool_tests *tool)
{
    gboolean all_tests_is_empty = !tool->all_tests || !g_hash_table_size(tool->all_tests);
    gboolean required_is_empty  = !tool->required_tests || !g_hash_table_size(tool->required_tests);
    return all_tests_is_empty && required_is_empty;
}

static int service_module_get_list_of_deployed_services_with_params(AlteratorCtlServicesModule *module,
                                                                    GList **result,
                                                                    const gchar *skip_service_name)
{
    int ret                    = 0;
    GHashTable *services_table = NULL;
    const gchar *skip_service_path
        = skip_service_name && skip_service_name[0] != '/'
              ? g_strdup(module->gdbus_source->alterator_gdbus_source_get_path_by_name(module->gdbus_source,
                                                                                       skip_service_name,
                                                                                       SERVICES_INTERFACE_NAME))
              : g_strdup(skip_service_name);
    GList *all_services_list = NULL;

    module->gdbus_source->alterator_gdbus_source_get_iface_objects(module->gdbus_source,
                                                                   SERVICES_INTERFACE_NAME,
                                                                   &services_table);
    if (!services_table)
    {
        g_printerr(_("Services list is empty.\n"));
        ERR_EXIT();
    };

    if (skip_service_path && strlen(skip_service_path) && g_hash_table_contains(services_table, skip_service_path))
        g_hash_table_remove(services_table, skip_service_path);

    if (!(all_services_list = g_hash_table_get_keys(services_table)))
        ERR_EXIT();

    for (GList *service = all_services_list; service != NULL; service = service->next)
    {
        alteratorctl_ctx_t *status_ctx = alteratorctl_ctx_init_services(SERVICES_STATUS,
                                                                        (gchar *) service->data,
                                                                        NULL,
                                                                        NULL,
                                                                        NULL);
        if (services_module_status_subcommand(module, &status_ctx) < 0)
            ERR_EXIT();

        if (!status_ctx->results)
        {
            g_printerr(_("Empty status of service %s in getting list of deployed services.\n"), (gchar *) service->data);
            ERR_EXIT();
        }

        if (!g_variant_is_of_type(status_ctx->results, G_VARIANT_TYPE("(ayi)")))
        {
            g_printerr(_("Wrong answer services status type in getting list of deployed services.\n"));
            ERR_EXIT();
        }

        JsonNode *parameters_node     = json_node_new(JSON_NODE_OBJECT);
        JsonObject *parameters_object = json_object_new();
        json_node_set_object(parameters_node, parameters_object);
        json_object_unref(parameters_object);

        deployment_status deploy_status = UNDEPLOYED;
        if (services_module_status_result_get_service_params(
                module, &status_ctx, (gchar *) service->data, FALSE, NULL, &parameters_node, &deploy_status, TRUE)
            < 0)
        {
            json_node_unref(parameters_node);
            g_printerr(_("Failed to determine service '%s' state.\n"), (gchar *) service->data);
            ERR_EXIT();
        }

        if (deploy_status == UNDEPLOYED)
        {
            alteratorctl_ctx_free(status_ctx);
            json_node_unref(parameters_node);
            continue;
        }

        if (deploy_status == DEPLOYED || deploy_status == DEPLOYED_AND_STARTED)
        {
            gchar *output = services_module_json_params_to_text(parameters_node, (gchar *) service->data);
            if (!output)
            {
                json_node_unref(parameters_node);
                alteratorctl_ctx_free(status_ctx);
                ERR_EXIT();
            }

            GVariant *deployed_service_data = g_variant_new("(ss)", (gchar *) service->data, output);
            (*result)                       = g_list_append(*result, deployed_service_data);
            g_free(output);
        }

        json_node_unref(parameters_node);
        alteratorctl_ctx_free(status_ctx);
    }

end:
    g_free((gchar *) skip_service_path);

    if (services_table)
        g_hash_table_unref(services_table);

    if (all_services_list)
        g_list_free(all_services_list);

    return ret;
}

static int services_module_is_deployed(AlteratorCtlServicesModule *module,
                                       const gchar *service_str_id,
                                       deployment_status *result)
{
    int ret                        = 0;
    alteratorctl_ctx_t *status_ctx = NULL;
    GVariant *exit_code            = NULL;

    if (!module)
    {
        if (service_str_id && strlen(service_str_id))
            g_printerr(_("Can't get deploy status of %s service. The services module doesn't exist.\n"), service_str_id);
        else
            g_printerr(_("Can't get deploy status of unknown service. The services module doesn't exist.\n"));
        ERR_EXIT();
    }

    if (!service_str_id || !strlen(service_str_id))
    {
        g_printerr(_("Can't get deploy status of unspecified service.\n"));
        ERR_EXIT();
    }

    if (!result)
    {
        g_printerr(_("Can't get deploy status of %s service. There is nowhere to save the result.\n"), service_str_id);
        ERR_EXIT();
    }

    status_ctx = alteratorctl_ctx_init_services(SERVICES_STATUS, service_str_id, NULL, NULL, NULL);
    if (!status_ctx)
    {
        g_printerr(_("Can't get service %s deploy status. Unspecified service status context.\n"), service_str_id);
        ERR_EXIT();
    }

    if (services_module_status_subcommand(module, &status_ctx))
        ERR_EXIT();

    if (!status_ctx->results)
    {
        g_printerr(_("Failed to get %s service status. No result of services status command.\n"), service_str_id);
        ERR_EXIT();
    }

    exit_code = g_variant_get_child_value(status_ctx->results, 1);
    if (!exit_code)
    {
        g_printerr(_("Can't get service %s status.\n"), service_str_id);
        ERR_EXIT();
    }

    if (services_module_status_result_get_service_params(
            module, &status_ctx, service_str_id, FALSE, NULL, NULL, result, FALSE)
        < 0)
        ERR_EXIT();

end:
    if (status_ctx)
        alteratorctl_ctx_free(status_ctx);

    if (exit_code)
        g_variant_unref(exit_code);

    return ret;
}

static int services_module_get_service_tests(AlteratorCtlServicesModule *module,
                                             const gchar *service_str_id,
                                             GNode *optional_service_info,
                                             const GPtrArray *diag_env_vars,
                                             gboolean all_modes,
                                             GHashTable **result)
{
    int ret                      = 0;
    GHashTable *service_ifaces   = NULL;
    gchar *service_path          = NULL;
    GNode *service_info          = NULL;
    GHashTable *tests_info_table = NULL;

    if (!module)
    {
        if (service_str_id && strlen(service_str_id))
            g_printerr(_("Can't get diagnostic tests of \"%s\" service. The services module doesn't exist.\n"),
                       service_str_id);
        else
            g_printerr(_("Can't get diagnostic tests of unknown service. The services module doesn't exist.\n"));
        ERR_EXIT();
    }

    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;
    service_info                              = optional_service_info ? optional_service_info : module->info;

    if (!service_str_id || !strlen(service_str_id))
    {
        g_printerr(_("Can't get diagnostic tests of unspecified service.\n"));
        ERR_EXIT();
    }

    service_path = service_str_id[0] != '/'
                       ? g_strdup(module->gdbus_source->alterator_gdbus_source_get_path_by_name(module->gdbus_source,
                                                                                                service_str_id,
                                                                                                SERVICES_INTERFACE_NAME))
                       : g_strdup(service_str_id);

    if (!service_info && !module->info
        && info_parser->alterator_ctl_module_info_parser_get_specified_object_data(info_parser,
                                                                                   module->gdbus_source,
                                                                                   service_path,
                                                                                   SERVICES_INTERFACE_NAME,
                                                                                   &service_info)
               < 0)
        ERR_EXIT();

    if (!service_info)
    {
        g_printerr(_("Can't get diagnostic tests. No service \"%s\" alterator entry.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!result)
    {
        g_printerr(_("Can't get diagnostic tests of \"%s\" service. There is nowhere to save the result.\n"),
                   service_str_id);
        ERR_EXIT();
    }

    *result = g_hash_table_new_full(g_str_hash,
                                    g_str_equal,
                                    (GDestroyNotify) g_free,
                                    (GDestroyNotify) services_module_service_diag_tool_tests_free);

    if (module->gdbus_source->alterator_gdbus_source_get_object_ifaces(module->gdbus_source,
                                                                       service_path,
                                                                       &service_ifaces)
        < 0)
    {
        g_printerr(_("Failed to get \"%s\" service D-Bus interfaces.\n"), service_str_id);
        ERR_EXIT();
    }

    // No diag tool in service Alterator Entry
    GNode *diag_service_tools_node = NULL;
    if (!(diag_service_tools_node = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
              info_parser, service_info, SERVICES_ALTERATOR_ENTRY_SERVICE_DIAG_TOOLS_TABLE_NAME)))
        goto end;

    GHashTable *tools_names = g_hash_table_new(g_str_hash, g_str_equal);
    for (GNode *tool = diag_service_tools_node->children; tool != NULL; tool = g_node_next_sibling(tool))
        g_hash_table_add(tools_names, ((alterator_entry_node *) tool->data)->node_name);

    gint tools_info_result = services_module_get_diag_tools_info(module, service_str_id, tools_names, &tests_info_table);
    if (tools_info_result < 0 || (tools_info_result > 1 && !g_hash_table_size(tools_names)))
    {
        g_printerr(_("Can't get test list of \"%s\" service: failed of getting diag tools info.\n"), service_str_id);
        ERR_EXIT();
    }

    for (GNode *tool = diag_service_tools_node->children; tool != NULL; tool = g_node_next_sibling(tool))
    {
        gchar *tool_name = ((alterator_entry_node *) tool->data)->node_name;
        if (!g_hash_table_contains(tools_names, tool_name))
            continue;

        toml_value *path = g_hash_table_lookup(((alterator_entry_node *) tool->data)->toml_pairs,
                                               SERVICES_ALTERATOR_ENTRY_SERVICE_DIAG_TOOLS_PATH_KEY_NAME);

        if (!path)
        {
            g_printerr(_("Can't get test list of \"%s\" service \"%s\" diag tool with unspecified DBus path.\n"),
                       service_str_id,
                       tool_name);
            ERR_EXIT();
        }

        if (path && (path->type != TOML_DATA_STRING || !strlen(path->str_value)))
        {
            g_printerr(_("Can't get test list of \"%s\" service \"%s\" diag tool with invalid DBus path.\n"),
                       service_str_id,
                       tool_name);
            ERR_EXIT();
        }

        toml_value *bus = g_hash_table_lookup(((alterator_entry_node *) tool->data)->toml_pairs,
                                              SERVICES_ALTERATOR_ENTRY_SERVICE_DIAG_TOOLS_BUS_KEY_NAME);

        if (!bus)
        {
            g_printerr(_("Can't get test list of \"%s\" service \"%s\" diag tool with unspecified bus type.\n"),
                       service_str_id,
                       tool_name);
            ERR_EXIT();
        }

        if (bus && (bus->type != TOML_DATA_STRING || !strlen(bus->str_value)))
        {
            g_printerr(_("Can't get test list of \"%s\" service: \"%s\" diag tool has invalid bus type.\n"),
                       service_str_id,
                       tool_name);
            ERR_EXIT();
        }

        GBusType bus_type = streq(bus->str_value, "system")
                                ? G_BUS_TYPE_SYSTEM
                                : (streq(bus->str_value, "session") ? G_BUS_TYPE_SESSION : G_BUS_TYPE_NONE);
        if (bus_type == G_BUS_TYPE_NONE)
        {
            g_printerr(_("Can't get test list of \"%s\" service: \"%s\" diag tool has invalid bus type.\n"),
                       service_str_id,
                       tool_name);
            ERR_EXIT();
        }

        int is_exist = 0;

        // Check object by iface in bus from alterator entry
        {
            AlteratorGDBusSource *check_source = bus_type == G_BUS_TYPE_SYSTEM
                                                     ? module->gdbus_source
                                                     : alterator_gdbus_source_new(FALSE, G_BUS_TYPE_SESSION);

            if (module->gdbus_source->alterator_gdbus_source_check_object_by_iface(check_source,
                                                                                   path->str_value,
                                                                                   DIAG_INTERFACE_NAME,
                                                                                   &is_exist)
                < 0)
            {
                if (bus_type == G_BUS_TYPE_SESSION)
                    alterator_gdbus_source_free(check_source);
                ERR_EXIT();
            }

            if (bus_type == G_BUS_TYPE_SESSION)
                alterator_gdbus_source_free(check_source);
        }

        // Current diag tool from service Alterator Entry doesn't exist. Skip
        if (is_exist == 0)
            continue;

        GNode *diag_tool_info       = g_hash_table_lookup(tests_info_table, path->str_value);
        GNode *diag_tool_tests_info = NULL;
        if (!(diag_tool_tests_info = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
                  info_parser, diag_tool_info, DIAG_ALTERATOR_ENTRY_SERVICE_DIAG_TESTS_TABLE_NAME)))
        {
            g_printerr(_("Failed to get \"%s\" diag tool tests info.\n"), tool_name);
            ERR_EXIT();
        }

        if (services_module_get_diag_tests(module,
                                           service_str_id,
                                           path->str_value,
                                           tool_name,
                                           streq(bus->str_value, "system")
                                               ? G_BUS_TYPE_SYSTEM
                                               : (streq(bus->str_value, "session") ? G_BUS_TYPE_SESSION
                                                                                   : G_BUS_TYPE_NONE),
                                           diag_env_vars,
                                           all_modes,
                                           diag_tool_tests_info,
                                           result)
            < 0)
            ERR_EXIT();

        /*
         * Enrich per-tool tests info with stable identifiers:
         *  - entry_name: service diag_tools table entry name (e.g. "tool1")
         *  - diag_name:  logical diagnostic tool name from .diag (e.g. "test_service1")
         */
        {
            diag_tool_tests *tool_tests = g_hash_table_lookup(*result, tool_name);
            if (tool_tests)
            {
                if (!tool_tests->entry_name && tool_name)
                    tool_tests->entry_name = g_strdup(tool_name);

                toml_value *tool_diag_name_val = NULL;
                if (diag_tool_info
                    && info_parser->alterator_ctl_module_info_parser_find_value(info_parser,
                                                                                diag_tool_info,
                                                                                &tool_diag_name_val,
                                                                                DIAG_ALTERATOR_ENTRY_TOOLNAME_KEY_NAME,
                                                                                NULL)
                    && tool_diag_name_val && tool_diag_name_val->type == TOML_DATA_STRING
                    && tool_diag_name_val->str_value && strlen(tool_diag_name_val->str_value))
                {
                    if (!tool_tests->diag_name)
                        tool_tests->diag_name = g_strdup(tool_diag_name_val->str_value);
                }
            }
        }
    }

    if (!g_hash_table_size(*result))
    {
        g_printerr(_("No diagnostic tests in \"%s\" service.\n"), service_str_id);
        ERR_EXIT();
    }

    // Validate result tools table
    GHashTableIter iter;
    g_hash_table_iter_init(&iter, *result);
    diag_tool_tests *tool = NULL;
    gchar *tool_name      = NULL;
    while (g_hash_table_iter_next(&iter, (gpointer *) &tool_name, (gpointer *) &tool))
    {
        if (tool->is_no_tests(tool))
        {
            g_printerr(_("No diagnostic tests in diagnostic tool \"%s\" of \"%s\" service.\n"),
                       tool_name,
                       service_str_id);
            ERR_EXIT();
        }
    }

end:
    if (tests_info_table)
        g_hash_table_destroy(tests_info_table);

    if (service_ifaces)
        g_hash_table_destroy(service_ifaces);

    g_free(service_path);

    return ret;
}

static int diag_list_tests_call(AlteratorCtlModule *diag_module,
                                const gchar *service_str_id,
                                const gchar *diag_tool_name,
                                GBusType bus_type,
                                gboolean get_required_only,
                                gboolean all_modes,
                                GNode *tests_info,
                                alteratorctl_ctx_t **diag_ctx)
{
    int ret                                      = 0;
    AlteratorCtlDiagModule *diag_module_instance = (AlteratorCtlDiagModule *) diag_module->module_instance;
    AlteratorGDBusSource *diag_gdbus_source      = bus_type == G_BUS_TYPE_SYSTEM
                                                       ? diag_module_instance->gdbus_system_source
                                                       : diag_module_instance->gdbus_session_source;

    if (diag_gdbus_source->alterator_gdbus_source_set_env_value(diag_gdbus_source,
                                                                SERVICES_DIAGNOSE_TESTS_ENV_VALUE_REQUIRED_ONLY,
                                                                get_required_only ? "true" : "")
        < 0)
    {
        g_printerr(_("Failed to set environment variable (\"%s\": \"%s\") for getting diagnostic tests of service "
                     "\"%s\".\n"),
                   SERVICES_DIAGNOSE_TESTS_ENV_VALUE_REQUIRED_ONLY,
                   get_required_only ? "true" : "",
                   service_str_id);
        ERR_EXIT();
    }

    if (!diag_tool_name || !strlen(diag_tool_name))
    {
        g_printerr(_("Failed to get diagnostic tests of unspecified diagnostic tool.\n"));
        ERR_EXIT();
    }

    if (all_modes && !tests_info)
    {
        g_printerr(_("Empty info of diagnostic tests of \"%s\" diagnostic tool.\n"), diag_tool_name);
        ERR_EXIT();
    }

    gchar *current_diag_mode_str = NULL;
    diag_gdbus_source->alterator_gdbus_source_get_env_value(diag_gdbus_source,
                                                            SERVICES_DIAGNOSE_TESTS_ENV_VALUE_DEPLOY_MODE,
                                                            &current_diag_mode_str);
    diag_tool_test_mode mode = streq(current_diag_mode_str, "pre") ? PRE_DIAG_MODE : POST_DIAG_MODE;

    // Backup help options and run
    {
        gboolean help_backup                         = diag_module_instance->alterator_ctl_app->arguments->module_help;
        enum alteratorctl_help_type help_type_backup = diag_module_instance->alterator_ctl_app->arguments->help_type;

        diag_module_instance->alterator_ctl_app->arguments->module_help = FALSE;
        diag_module_instance->alterator_ctl_app->arguments->help_type   = ALTERATORCTL_NONE_HELP;

        if (diag_module->module_iface->run(diag_module->module_instance, *diag_ctx) < 0)
        {
            diag_module_instance->alterator_ctl_app->arguments->module_help = help_backup;
            diag_module_instance->alterator_ctl_app->arguments->help_type   = help_type_backup;
            ERR_EXIT();
        }

        diag_module_instance->alterator_ctl_app->arguments->module_help = help_backup;
        diag_module_instance->alterator_ctl_app->arguments->help_type   = help_type_backup;
    }

    if (!(*diag_ctx)->results || !g_hash_table_size((GHashTable *) (*diag_ctx)->results))
    {
        if (streq(service_str_id, diag_tool_name))
            g_printerr(_("Failed to get \"%s\" diagnostic tool tests of \"%s\" service. Empty result.\n"),
                       diag_tool_name,
                       service_str_id);
        else
            g_printerr(_("Failed to get diagnostic tests of \"%s\" service. Empty result.\n"), service_str_id);
        ERR_EXIT();
    }

    AlteratorCtlModuleInfoParser *info_parser = diag_gdbus_source->info_parser;
    // Set table values as a parser tests info
    {
        GHashTable *tests_table = g_hash_table_lookup((GHashTable *) (*diag_ctx)->results,
                                                      bus_type == G_BUS_TYPE_SYSTEM ? "system_bus" : "session_bus");
        GHashTableIter iter;
        g_hash_table_iter_init(&iter, tests_table);
        gchar *test_name = NULL;
        while (g_hash_table_iter_next(&iter, (gpointer *) &test_name, NULL))
        {
            GNode *test_node = NULL;
            if (!(test_node = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser,
                                                                                             tests_info,
                                                                                             test_name)))
                ERR_EXIT();

            g_hash_table_insert(tests_table,
                                g_strdup(test_name),
                                services_module_service_diag_tool_test_info_init_from_node(info_parser,
                                                                                           test_node,
                                                                                           get_required_only,
                                                                                           mode));
        }
    }

    if (all_modes)
    {
        GHashTable *backup_result = (*diag_ctx)->results;
        (*diag_ctx)->results      = NULL;

        // Get test from another mode
        diag_gdbus_source->alterator_gdbus_source_set_env_value(diag_gdbus_source,
                                                                SERVICES_DIAGNOSE_TESTS_ENV_VALUE_DEPLOY_MODE,
                                                                mode == PRE_DIAG_MODE ? "post" : "pre");
        if (diag_list_tests_call(
                diag_module, service_str_id, diag_tool_name, bus_type, get_required_only, FALSE, tests_info, diag_ctx)
            < 0)
            ERR_EXIT();

        // Return old value
        diag_gdbus_source->alterator_gdbus_source_set_env_value(diag_gdbus_source,
                                                                SERVICES_DIAGNOSE_TESTS_ENV_VALUE_DEPLOY_MODE,
                                                                mode == PRE_DIAG_MODE ? "pre" : "post");

        GHashTable *original_mode_tests_table = g_hash_table_lookup(backup_result,
                                                                    bus_type == G_BUS_TYPE_SYSTEM ? "system_bus"
                                                                                                  : "session_bus");

        GHashTable *another_mode_tests_table = g_hash_table_lookup((GHashTable *) (*diag_ctx)->results,
                                                                   bus_type == G_BUS_TYPE_SYSTEM ? "system_bus"
                                                                                                 : "session_bus");

        GPtrArray *another_mode_tests_array = g_hash_table_get_keys_as_ptr_array(another_mode_tests_table);
        for (gsize i = 0; i < another_mode_tests_array->len; i++)
        {
            gchar *another_mode_test_name = ((gchar **) another_mode_tests_array->pdata)[i];
            GNode *test_node              = NULL;
            if (!(test_node = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser,
                                                                                             tests_info,
                                                                                             another_mode_test_name)))
                ERR_EXIT();

            if (!g_hash_table_contains(original_mode_tests_table, another_mode_test_name))
                g_hash_table_insert(original_mode_tests_table,
                                    g_strdup(another_mode_test_name),
                                    services_module_service_diag_tool_test_info_init_from_node(info_parser,
                                                                                               test_node,
                                                                                               get_required_only,
                                                                                               mode == PRE_DIAG_MODE
                                                                                                   ? POST_DIAG_MODE
                                                                                                   : PRE_DIAG_MODE));
            else
            {
                diag_tool_test_info *existing_info = g_hash_table_lookup(original_mode_tests_table,
                                                                         another_mode_test_name);
                existing_info->mode                = BOTH_DIAG_MODE;
            }
        }
        g_ptr_array_unref(another_mode_tests_array);
        g_hash_table_destroy((*diag_ctx)->results);
        (*diag_ctx)->results = backup_result;
    }

end:
    return ret;
}

static int services_module_pack_diag_tests(const gchar *service_str_id,
                                           const gchar *diag_tool_name,
                                           GBusType bus_type,
                                           alteratorctl_ctx_t **diag_ctx,
                                           GHashTable *tests_data)
{
    int ret = 0;
    GHashTableIter iter;
    GHashTable *tests_table = g_hash_table_lookup((GHashTable *) (*diag_ctx)->results,
                                                  bus_type == G_BUS_TYPE_SYSTEM ? "system_bus" : "session_bus");
    g_hash_table_iter_init(&iter, tests_table);
    gpointer test_name        = NULL;
    gpointer test_parsed_info = NULL;
    while (g_hash_table_iter_next(&iter, &test_name, &test_parsed_info))
        g_hash_table_insert(tests_data, g_strdup((gchar *) test_name), test_parsed_info);

end:
    return ret;
}

static int services_module_get_diag_tests(AlteratorCtlServicesModule *module,
                                          const gchar *service_str_id,
                                          const gchar *diag_tool_path,
                                          const gchar *diag_tool_table_name,
                                          GBusType bus_type,
                                          const GPtrArray *diag_env_vars,
                                          gboolean all_modes,
                                          GNode *diag_tests_info,
                                          GHashTable **result)
{
    int ret                         = 0;
    GVariant *diag_tool_tests_var   = NULL;
    GVariant *diag_exit_code        = NULL;
    alteratorctl_ctx_t *diag_ctx    = NULL;
    AlteratorCtlModule *diag_module = NULL;

    if (!service_str_id || !strlen(service_str_id))
    {
        g_printerr(_("Can't get diagnostic tests of unspecified service.\n"));
        ERR_EXIT();
    }

    if ((!diag_tool_path || !strlen(diag_tool_path)) || (!diag_tool_table_name || !strlen(diag_tool_table_name)))
    {
        g_printerr(_("Can't get diagnostic test of unspecified diagnostic tool.\n"));
        ERR_EXIT();
    }

    if (!module)
    {
        if (service_str_id && strlen(service_str_id))
        {
            if (streq(service_str_id, diag_tool_path))
                g_printerr(_("Can't get \"%s\" diagnostic tool tests of \"%s\" service. The services module "
                             "doesn't exist.\n"),
                           diag_tool_table_name,
                           service_str_id);
            else
                g_printerr(_("Can't get diagnostic tests of unknown service. The services module doesn't exist.\n"));
        }
        ERR_EXIT();
    }

    if (bus_type != G_BUS_TYPE_SYSTEM && bus_type != G_BUS_TYPE_SESSION)
    {
        if (streq(service_str_id, diag_tool_path))
            g_printerr(_("Can't get \"%s\" diagnostic tool tests of \"%s\" service. Incorrect bus type.\n"),
                       diag_tool_table_name,
                       service_str_id);
        else
            g_printerr(_("Can't get diagnostic tests of unknown service. Incorrect bus type.\n"));
        ERR_EXIT();
    }

    if (!result)
    {
        if (streq(service_str_id, diag_tool_path))
            g_printerr(_("Can't get diagnostic tests of \"%s\" service. There is nowhere to save the result.\n"),
                       service_str_id);
        else
            g_printerr(_("Can't get \"%s\" diagnostic tool tests of \"%s\" service. There is nowhere to save the "
                         "result.\n"),
                       diag_tool_table_name,
                       service_str_id);
        ERR_EXIT();
    }

    if (alterator_ctl_get_registered_module(module->alterator_ctl_app, ALTERATOR_CTL_DIAG_MODULE_NAME, &diag_module) < 0)
        ERR_EXIT();

    AlteratorCtlDiagModule *diag_module_instance = (AlteratorCtlDiagModule *) diag_module->module_instance;
    AlteratorGDBusSource *diag_gdbus_source      = bus_type == G_BUS_TYPE_SYSTEM
                                                       ? diag_module_instance->gdbus_system_source
                                                       : diag_module_instance->gdbus_session_source;

    for (gsize i = 0; i < diag_env_vars->len; i++)
    {
        evn_variable_t *var = (evn_variable_t *) (diag_env_vars->pdata[i]);
        if (diag_gdbus_source->alterator_gdbus_source_set_env_value(diag_gdbus_source, var->name, var->value) < 0)
        {
            g_print("Failed to set environment value (\"%s\": \"%s\") for getting diagnostic tests of service "
                    "\"%s\".\n",
                    var->name,
                    var->value,
                    service_str_id);
            ERR_EXIT();
        }
    }

    diag_ctx = alteratorctl_ctx_init_diag(DIAG_LIST_TESTS_PRIV, diag_tool_path, NULL, NULL, NULL, NULL);
    if (diag_list_tests_call(
            diag_module, service_str_id, diag_tool_path, bus_type, FALSE, all_modes, diag_tests_info, &diag_ctx)
        < 0)
        ERR_EXIT();

    GHashTable *all_tests_data = g_hash_table_new_full(g_str_hash,
                                                       g_str_equal,
                                                       (GDestroyNotify) g_free,
                                                       (GDestroyNotify)
                                                           services_module_service_diag_tool_test_info_free);
    if (services_module_pack_diag_tests(service_str_id, diag_tool_path, bus_type, &diag_ctx, all_tests_data) < 0)
        ERR_EXIT();

    diag_ctx->free_results(diag_ctx->results);

    if (diag_list_tests_call(
            diag_module, service_str_id, diag_tool_path, bus_type, TRUE, all_modes, diag_tests_info, &diag_ctx)
        < 0)
        ERR_EXIT();

    GHashTable *required_tests_data = g_hash_table_new_full(g_str_hash,
                                                            g_str_equal,
                                                            (GDestroyNotify) g_free,
                                                            (GDestroyNotify)
                                                                services_module_service_diag_tool_test_info_free);
    if (services_module_pack_diag_tests(service_str_id, diag_tool_path, bus_type, &diag_ctx, required_tests_data) < 0)
        ERR_EXIT();

    if (diag_gdbus_source->alterator_gdbus_source_set_env_value(diag_gdbus_source,
                                                                SERVICES_DIAGNOSE_TESTS_ENV_VALUE_REQUIRED_ONLY,
                                                                "")
        < 0)
    {
        g_printerr(_("Failed to unset environment variable \"%s\" for getting diagnostic tests of service \"%s\".\n"),
                   SERVICES_DIAGNOSE_TESTS_ENV_VALUE_REQUIRED_ONLY,
                   service_str_id);
        ERR_EXIT();
    }

    GPtrArray *all_tests_names = g_hash_table_get_keys_as_ptr_array(required_tests_data);
    for (gsize i = 0; i < all_tests_names->len; i++)
    {
        gchar *test_name          = ((gchar **) all_tests_names->pdata)[i];
        diag_tool_test_info *test = g_hash_table_lookup(all_tests_data, test_name);
        test->is_required         = TRUE;
    }
    g_ptr_array_unref(all_tests_names);

    g_hash_table_insert(*result,
                        g_strdup(diag_tool_table_name),
                        services_module_service_diag_tool_tests_init(diag_tool_path,
                                                                     all_tests_data,
                                                                     required_tests_data,
                                                                     bus_type));

    g_hash_table_unref(all_tests_data);
    g_hash_table_unref(required_tests_data);

end:
    if (diag_ctx)
        alteratorctl_ctx_free(diag_ctx);

    if (diag_tool_tests_var)
        g_variant_unref(diag_tool_tests_var);

    if (diag_exit_code)
        g_variant_unref(diag_exit_code);

    if (diag_module)
        diag_module->free_module_func(diag_module->module_instance);

    return ret;
}

static int services_module_get_diag_tools_info(AlteratorCtlServicesModule *module,
                                               const gchar *service_str_id,
                                               GHashTable *diag_tools_names,
                                               GHashTable **tools_names_to_nodes)
{
    int ret                           = 0;
    GPtrArray *diag_tools_names_arr   = NULL;
    GHashTable *diag_tools_names_copy = g_hash_table_copy_table(diag_tools_names, g_strdup_copy_func, NULL, NULL, NULL);
    AlteratorCtlModuleInfoParser *info_parser = NULL;
    if (!service_str_id)
    {
        g_printerr(_("Can't get diagnostics tests info of unspecified service.\n"));
        ERR_EXIT();
    }

    if (!module)
    {
        g_printerr(_("Can't get diagnostics tests of \"%s\" service.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!diag_tools_names)
    {
        g_printerr("Failed to get diagnostic tools info's: empty tools list.\n");
        ERR_EXIT();
    }

    if (!tools_names_to_nodes)
    {
        g_printerr("Can't save diagnostic tools info's in empty store.\n");
        ERR_EXIT();
    }

    info_parser = module->gdbus_source->info_parser;

    if (!module->info
        && info_parser->alterator_ctl_module_info_parser_get_specified_object_data(info_parser,
                                                                                   module->gdbus_source,
                                                                                   service_str_id,
                                                                                   SERVICES_INTERFACE_NAME,
                                                                                   &module->info))
    {
        g_printerr(_("Failed to get all service \"%s\" tools info. Error getting service alterator entry info.\n"),
                   service_str_id);
        ERR_EXIT();
    }

    GNode *diag_tools_node = NULL;
    if (!(diag_tools_node = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
              info_parser, module->info, SERVICES_ALTERATOR_ENTRY_SERVICE_DIAG_TOOLS_TABLE_NAME)))
    {
        g_printerr(_("Failed to get all service \"%s\" tools info. No diagnostics tools.\n"), service_str_id);
        ERR_EXIT();
    }

    info_parser           = module->gdbus_source->info_parser;
    *tools_names_to_nodes = g_hash_table_new_full(g_str_hash,
                                                  g_str_equal,
                                                  (GDestroyNotify) g_free,
                                                  (GDestroyNotify) alterator_ctl_module_info_parser_result_tree_free);

    diag_tools_names_arr = g_hash_table_get_keys_as_ptr_array(diag_tools_names_copy);
    for (gsize i = 0; i < diag_tools_names_arr->len; i++)
    {
        gchar *tool_name          = ((gchar **) (diag_tools_names_arr->pdata))[i];
        GNode *tool_metadata_node = NULL;
        if (!(tool_metadata_node = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser,
                                                                                                  module->info,
                                                                                                  tool_name)))
        {
            g_printerr(_("Failed to get all service \"%s\" tools info. Tool \"%s\" doesn't exists.\n"),
                       service_str_id,
                       tool_name);
            ERR_EXIT();
        }

        alterator_entry_node *tool_metadata = tool_metadata_node->data;
        toml_value *bus                     = g_hash_table_lookup(tool_metadata->toml_pairs,
                                              SERVICES_ALTERATOR_ENTRY_SERVICE_DIAG_TOOLS_BUS_KEY_NAME);
        if (!bus || bus->type != TOML_DATA_STRING || !bus->str_value || !strlen(bus->str_value))
        {
            g_printerr(_("Failed to get all service \"%s\" tools info. The \"%s\" tool bus isn't specified.\n"),
                       service_str_id,
                       tool_name);
            ERR_EXIT();
        }

        toml_value *tool_path = g_hash_table_lookup(tool_metadata->toml_pairs,
                                                    SERVICES_ALTERATOR_ENTRY_SERVICE_DIAG_TOOLS_PATH_KEY_NAME);
        if (!tool_path || tool_path->type != TOML_DATA_STRING || !tool_path->str_value || !strlen(tool_path->str_value))
        {
            g_printerr(_("Failed to get all service \"%s\" tools info. The \"%s\" tool path isn't specified.\n"),
                       service_str_id,
                       tool_name);
            ERR_EXIT();
        }

        GNode *tool_info     = NULL;
        gint get_info_result = services_module_get_diag_info(module,
                                                             service_str_id,
                                                             tool_name,
                                                             tool_path->str_value,
                                                             streq(bus->str_value, "system")
                                                                 ? G_BUS_TYPE_SYSTEM
                                                                 : (streq(bus->str_value, "session") ? G_BUS_TYPE_SESSION
                                                                                                     : G_BUS_TYPE_NONE),
                                                             &tool_info);

        if (get_info_result < 0)
            ERR_EXIT();
        else if (get_info_result > 0)
        {
            // Remove not founded tools
            g_hash_table_remove(diag_tools_names, tool_name);
        }
        else
            g_hash_table_insert(*tools_names_to_nodes, g_strdup(tool_path->str_value), tool_info);
    }

end:
    if (diag_tools_names_arr)
        g_ptr_array_unref(diag_tools_names_arr);

    g_hash_table_destroy(diag_tools_names_copy);

    return ret;
}

// ret 1 if failed of getting info
static int services_module_get_diag_info(AlteratorCtlServicesModule *module,
                                         const gchar *service_str_id,
                                         const gchar *service_diag_tool_entry_name,
                                         const gchar *diag_tool_str_id,
                                         GBusType bus_type,
                                         GNode **result)
{
    int ret                           = 0;
    AlteratorCtlModule *diag_module   = NULL;
    alteratorctl_ctx_t *diag_info_ctx = NULL;

    if (!service_str_id)
    {
        g_printerr(_("Can't get diagnostics tests info of unspecified service.\n"));
        ERR_EXIT();
    }

    if (!diag_tool_str_id)
    {
        g_printerr(_("Can't get diagnostics tests info of \"%s\" service with unspecified diagnostic tool.\n"),
                   service_str_id);
        ERR_EXIT();
    }

    if (!module)
    {
        g_printerr(_("Can't get diagnostics tests of \"%s\" service.\n"), service_str_id);
        ERR_EXIT();
    }

    if (bus_type != G_BUS_TYPE_SYSTEM && bus_type != G_BUS_TYPE_SESSION)
    {
        g_printerr(_("Can't get \"%s\" diagnostic tool info of \"%s\" service. Invalid bus type.\n"),
                   diag_tool_str_id,
                   service_str_id);
        ERR_EXIT();
    }

    if (!result)
    {
        g_printerr(_("Can't save \"%s\" diagnostic tool info of \"%s\" service.\n"), diag_tool_str_id, service_str_id);
        ERR_EXIT();
    }

    if (alterator_ctl_get_registered_module(module->alterator_ctl_app, ALTERATOR_CTL_DIAG_MODULE_NAME, &diag_module) < 0)
    {
        g_printerr(_("Failed to run getting diagnostic tests info in \"%s\" service.\n"), service_str_id);
        ERR_EXIT();
    }

    diag_info_ctx = alteratorctl_ctx_init_diag(DIAG_INFO, diag_tool_str_id, NULL, NULL, NULL, NULL);

    disable_output();
    diag_module->module_iface->run(diag_module->module_instance, diag_info_ctx);
    enable_output();

    AlteratorGDBusSource *diag_gdbus_source
        = bus_type == G_BUS_TYPE_SYSTEM
              ? ((AlteratorCtlDiagModule *) diag_module->module_instance)->gdbus_system_source
              : (bus_type == G_BUS_TYPE_SESSION
                     ? ((AlteratorCtlDiagModule *) diag_module->module_instance)->gdbus_session_source
                     : NULL);

    if (!diag_gdbus_source)
    {
        g_printerr(_("Can't get dbus source.\n"));
        ERR_EXIT();
    }

    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) diag_gdbus_source->info_parser;
    disable_output();
    gint getting_data_result = info_parser->alterator_ctl_module_info_parser_get_specified_object_data(
        info_parser, diag_gdbus_source, diag_tool_str_id, DIAG_INTERFACE_NAME, result);
    enable_output();
    if (getting_data_result < 0)
    {
        g_print(_("WARNING: Diagnostic tool \"%s\" in service \"%s\" cannot be used - D-Bus object \"%s\" not "
                  "found.\n"),
                service_diag_tool_entry_name,
                service_str_id,
                diag_tool_str_id);
        ret = 1;
        goto end;
    }

end:
    if (diag_module)
        diag_module->free_module_func(diag_module->module_instance);

    if (diag_info_ctx)
        alteratorctl_ctx_free(diag_info_ctx);

    return ret;
}

static int services_module_run_test(AlteratorCtlServicesModule *module,
                                    const gchar *service_str_id,
                                    const gchar *diag_tool_name,
                                    const gchar *test_name,
                                    deployment_status deploy_mode,
                                    GBusType bus_type)
{
    int ret                                 = 0;
    alteratorctl_ctx_t *diag_ctx            = NULL;
    AlteratorCtlModule *diag_module         = NULL;
    AlteratorGDBusSource *diag_gdbus_source = NULL;

    if (!service_str_id)
    {
        g_printerr(_("Can't run diagnostics tests of unspecified service.\n"));
        ERR_EXIT();
    }

    if (!diag_tool_name)
    {
        g_printerr(_("Can't run diagnostics tests of %s service with unspecified diagnostic tool.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!test_name)
    {
        if (streq(service_str_id, diag_tool_name))
            g_printerr(_("Can't run unspecified diagnostics test of %s service.\n"), service_str_id);
        else
            g_printerr(_("Can't run unspecified diagnostics test of %s diagnostic tool of %s service.\n"),
                       diag_tool_name,
                       service_str_id);
        ERR_EXIT();
    }

    if (!module)
    {
        g_printerr(_("Can't run diagnostics tests of %s service.\n"), service_str_id);
        ERR_EXIT();
    }

    if (alterator_ctl_get_registered_module(module->alterator_ctl_app, ALTERATOR_CTL_DIAG_MODULE_NAME, &diag_module) < 0)
        ERR_EXIT();
    AlteratorCtlDiagModule *diag_module_instance = (AlteratorCtlDiagModule *) diag_module->module_instance;
    diag_gdbus_source                            = bus_type == G_BUS_TYPE_SYSTEM
                                                       ? diag_module_instance->gdbus_system_source
                                                       : (bus_type == G_BUS_TYPE_SESSION ? diag_module_instance->gdbus_session_source : NULL);

    if (diag_gdbus_source->alterator_gdbus_source_set_env_value(diag_gdbus_source,
                                                                SERVICES_DIAGNOSE_TESTS_ENV_VALUE_DEPLOY_MODE,
                                                                deploy_mode != UNDEPLOYED ? "post" : "pre")
        < 0)
        ERR_EXIT();

    gchar *additional_str_data = g_strdup(bus_type == G_BUS_TYPE_SYSTEM ? "system" : "session");
    diag_ctx                   = alteratorctl_ctx_init_diag(DIAG_RUN_TOOL_TEST_PRIV,
                                          diag_tool_name,
                                          test_name,
                                          NULL,
                                          NULL,
                                          additional_str_data);

    if (!diag_gdbus_source)
    {
        if (streq(service_str_id, diag_tool_name))
            g_printerr(_("Failed to run diagnostics tests. Unknown diagnostic tool bus type in %s service.\n"),
                       service_str_id);
        else
            g_printerr(_("Failed to run diagnostics tests. Unknown %s diagnostic tool bus type in %s service.\n"),
                       diag_tool_name,
                       service_str_id);
        ERR_EXIT();
    }

    int result = diag_module->module_iface->run(diag_module->module_instance, diag_ctx);
    if (result > 0)
        ret = 1;

    if (result < 0)
        ERR_EXIT();

end:
    if (diag_ctx)
    {
        diag_ctx->additional_data = NULL;
        alteratorctl_ctx_free(diag_ctx);
    }

    if (diag_module)
        diag_module->free_module_func(diag_module->module_instance);

    return ret;
}

static toml_value *services_module_toml_value_dup(toml_value *value)
{
    if (!value)
        return NULL;

    toml_value *result = g_new0(toml_value, 1);
    result->type       = value->type;
    if (value->array)
    {
        result->array_length = value->array_length;
        if (value->type == TOML_DATA_ARRAY_OF_STRING)
        {
            result->array = (gpointer) g_new0(gchar *, value->array_length);
            for (gsize i = 0; i < value->array_length; i++)
                ((gchar **) result->array)[i] = g_strdup(((gchar **) value->array)[i]);
            return result;
        }
        else if (value->type == TOML_DATA_ARRAY_OF_INT)
            result->array = (gpointer) g_new0(gint, value->array_length);
        else if (value->type == TOML_DATA_ARRAY_OF_DOUBLE)
            result->array = (gpointer) g_new0(gdouble, value->array_length);
        else if (value->type == TOML_DATA_ARRAY_OF_BOOL)
            result->array = (gpointer) g_new0(gboolean, value->array_length);
        memcpy(result->array, value->array, value->array_length);
        return result;
    }
    else
    {
        if (value->type == TOML_DATA_STRING)
            result->str_value = g_strdup(value->str_value);
        else if (value->type == TOML_DATA_DOUBLE)
            result->double_value = value->double_value;
        else if (value->type == TOML_DATA_BOOL)
            result->bool_value = value->bool_value;

        return result;
    }
}

// ret = 1: The resource does not exist or does not have an associated parameter
static int services_module_get_actual_resourse_value(AlteratorCtlServicesModule *module,
                                                     const gchar *service_str_id,
                                                     GNode *resources,
                                                     JsonNode *parameters,
                                                     const gchar *resource_name,
                                                     toml_value **result)
{
    int ret                                   = 0;
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;
    if (!service_str_id)
    {
        g_printerr(_("Can't get resource %s value of unspecified service.\n"), resource_name ? resource_name : "");
        ERR_EXIT();
    }

    if (!resource_name)
    {
        g_printerr(_("Can't get unspecified resource value of %s service.\n"), service_str_id);
        ERR_EXIT();
    }

    if (!resources)
    {
        g_printerr(_("Can't get %s resource value of %s service: no resources data.\n"), resource_name, service_str_id);
        ERR_EXIT();
    }

    if (!result)
    {
        g_printerr(_("No container for saving resource %s value of %s service.\n"), resource_name, service_str_id);
        ERR_EXIT();
    }

    GNode *resource = NULL;
    if (!(resource = info_parser->alterator_ctl_module_info_parser_get_node_by_name(info_parser,
                                                                                    resources,
                                                                                    resource_name)))
        goto end;

    toml_value *resource_value = NULL;
    if (info_parser->alterator_ctl_module_info_parser_find_value(
            info_parser, resource, &resource_value, SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCE_VALUE_KEY_NAME, NULL)
        && resource_value)
        *result = resource_value;

    toml_value *parameter = NULL;
    if (!info_parser->alterator_ctl_module_info_parser_find_value(
            info_parser, resource, &parameter, SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCES_PARAMETER_KEY_NAME, NULL)
        || !parameter)
    {
        if (!resource_value)
            ret = 1;
        goto end;
    }

    // There is no point in looking for a parameter in JSON with default parameters if they are in TOML
    // if (parameter && only_initial_parameters && !parameters)
    //     *result = parameter;

    toml_value *param_name_val = NULL;
    if (!info_parser->alterator_ctl_module_info_parser_find_value(
            info_parser, resource, &param_name_val, SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCES_PARAMETER_KEY_NAME, NULL)
        || param_name_val->type != TOML_DATA_STRING || !param_name_val->str_value)
    {
        ret = 1;
        goto end;
    }

    const gchar *param_name      = param_name_val->str_value;
    JsonObject *parameter_object = json_node_get_object(parameters);

    if (!json_object_has_member(parameter_object, param_name))
    {
        *result = resource_value;
        switch ((*result)->type)
        {
        case TOML_DATA_ARRAY_OF_STRING: {
            JsonArray *array = json_array_new();
            for (gsize i = 0; i < (*result)->array_length; i++)
                json_array_add_string_element(array, ((gchar **) (*result)->array)[i]);
            json_object_set_array_member(parameter_object, param_name, array);
        }
        break;
        case TOML_DATA_ARRAY_OF_INT: {
            JsonArray *array = json_array_new();
            for (gsize i = 0; i < (*result)->array_length; i++)
                json_array_add_int_element(array, ((gint64 *) (*result)->array)[i]);
            json_object_set_array_member(parameter_object, param_name, array);
        }
        break;
        case TOML_DATA_ARRAY_OF_DOUBLE: {
            JsonArray *array = json_array_new();
            for (gsize i = 0; i < (*result)->array_length; i++)
                json_array_add_double_element(array, ((gdouble *) (*result)->array)[i]);
            json_object_set_array_member(parameter_object, param_name, array);
        }
        break;
        case TOML_DATA_ARRAY_OF_BOOL: {
            JsonArray *array = json_array_new();
            for (gsize i = 0; i < (*result)->array_length; i++)
                json_array_add_boolean_element(array, ((gboolean *) (*result)->array)[i]);
            json_object_set_array_member(parameter_object, param_name, array);
        }
        break;
        case TOML_DATA_STRING:
            json_object_set_string_member(parameter_object, param_name, (*result)->str_value);
            break;
        case TOML_DATA_DOUBLE:
            json_object_set_double_member(parameter_object, param_name, (*result)->double_value);
            break;
        case TOML_DATA_BOOL:
            json_object_set_boolean_member(parameter_object, param_name, (*result)->bool_value);
            break;
        default:
            break;
        };
        goto end;
    }

    JsonNode *param_node    = json_object_get_member(parameter_object, param_name);
    JsonNodeType value_type = json_node_get_node_type(param_node);
    resource_value          = g_new0(toml_value, 1);

    if (value_type == JSON_NODE_VALUE)
    {
        switch (json_node_get_value_type(param_node))
        {
        case G_TYPE_STRING:
            resource_value->type      = TOML_DATA_STRING;
            resource_value->str_value = g_strdup(json_node_get_string(param_node));
            break;

        case G_TYPE_INT64:
            resource_value->type         = TOML_DATA_DOUBLE;
            resource_value->double_value = (gdouble) json_node_get_int(param_node);
            break;

        case G_TYPE_DOUBLE:
            resource_value->type         = TOML_DATA_DOUBLE;
            resource_value->double_value = json_node_get_double(param_node);
            break;

        case G_TYPE_BOOLEAN:
            resource_value->type       = TOML_DATA_BOOL;
            resource_value->bool_value = json_node_get_boolean(param_node);
            break;
        default:
            g_printerr(_("Unknown resourse %s value type in %s service.\n"), resource_name, service_str_id);
            ERR_EXIT();
            break;
        }
    }
    else if (value_type == JSON_NODE_ARRAY)
    {
        JsonArray *arr = json_node_get_array(param_node);
        guint len      = json_array_get_length(arr);
        if (len == 0)
        {
            g_printerr(_("Invalid resource %s value type as a empty array in %s service.\n"),
                       resource_name,
                       service_str_id);
            ERR_EXIT();
        }
        else
        {
            switch (json_node_get_value_type(param_node))
            {
            case G_TYPE_STRING:
                resource_value->type         = TOML_DATA_ARRAY_OF_STRING;
                resource_value->array        = (gpointer) g_new0(gchar *, len);
                resource_value->array_length = len;
                for (guint i = 0; i < len; i++)
                    ((gchar **) resource_value->array)[i] = g_strdup(
                        json_node_get_string(json_array_get_element(arr, i)));
                break;
            case G_TYPE_INT64:
                resource_value->type         = TOML_DATA_ARRAY_OF_INT;
                resource_value->array        = (gpointer) g_new0(gint, len);
                resource_value->array_length = len;
                for (guint i = 0; i < len; i++)
                    ((gint *) resource_value->array)[i] = (gint) json_node_get_int(json_array_get_element(arr, i));
                break;
            case G_TYPE_DOUBLE:
                resource_value->type         = TOML_DATA_ARRAY_OF_DOUBLE;
                resource_value->array        = (gpointer) g_new0(double, len);
                resource_value->array_length = len;
                for (guint i = 0; i < len; i++)
                    ((double *) resource_value->array)[i] = json_node_get_double(json_array_get_element(arr, i));
                break;
            case G_TYPE_BOOLEAN:
                resource_value->type         = TOML_DATA_ARRAY_OF_BOOL;
                resource_value->array        = (gpointer) g_new0(gboolean, len);
                resource_value->array_length = len;
                for (guint i = 0; i < len; i++)
                    ((gboolean *) resource_value->array)[i] = json_node_get_boolean(json_array_get_element(arr, i));
                break;
            default:
                g_printerr(_("Unknown resourse %s value type in %s service.\n"), resource_name, service_str_id);
                ERR_EXIT();
                break;
            }
        }
    }
    else if (value_type == JSON_NODE_OBJECT)
    {
        g_printerr(_("Unsupported resourse %s value type as a composite param in %s service.\n"),
                   resource_name,
                   service_str_id);
        ERR_EXIT();
    }
    else
    {
        g_printerr(_("Unknown resourse %s value type in %s service.\n"), resource_name, service_str_id);
        ERR_EXIT();
    }
    *result = resource_value;

end:
    if (ret < 0)
        g_free(resource_value);

    return ret;
}

static gchar *services_module_strignify_toml_value(const toml_value *value)
{
    gchar *result = NULL;

    if (!value)
    {
        g_printerr(_("Can't strignify toml value.\n"));
        return NULL;
    }

    switch (value->type)
    {
    case TOML_DATA_STRING:
        result = g_strconcat("\"", value->str_value, "\"", NULL);
        break;

    case TOML_DATA_DOUBLE:
        if (value->double_value == (gint64) value->double_value)
            result = g_strdup_printf("%ld", (gint64) value->double_value);
        else
            result = g_strdup_printf("%f", value->double_value);
        break;

    case TOML_DATA_BOOL:
        if (value->bool_value)
            result = g_strdup("true\n");
        else
            result = g_strdup("false\n");
        break;

    case TOML_DATA_ARRAY_OF_INT: {
        result = g_strdup("[");
        for (gsize i = 0; i < value->array_length; i++)
        {
            gchar *tmp     = result;
            gchar *int_str = g_strdup_printf("%d", ((gint *) value->array)[i]);
            result         = g_strconcat(tmp, int_str, i + 1 != value->array_length ? ", " : "]", NULL);
            g_free(tmp);
            g_free(int_str);
        }
        break;
    }

    case TOML_DATA_ARRAY_OF_DOUBLE: {
        result = g_strdup("[");
        for (gsize i = 0; i < value->array_length; i++)
        {
            gchar *tmp        = result;
            gchar *double_str = g_strdup_printf("%f", ((gdouble *) value->array)[i]);
            result            = g_strconcat(tmp, double_str, i + 1 != value->array_length ? ", " : "]", NULL);
            g_free(tmp);
            g_free(double_str);
        }
        break;
    }

    case TOML_DATA_ARRAY_OF_BOOL: {
        result = g_strdup("[");
        for (gsize i = 0; i < value->array_length; i++)
        {
            gchar *tmp = result;
            result     = g_strconcat(tmp,
                                 ((gint *) value->array)[i] ? "true" : "false",
                                 i + 1 != value->array_length ? ", " : "]",
                                 NULL);
            g_free(tmp);
        }
        break;
    }

    case TOML_DATA_ARRAY_OF_STRING: {
        result = g_strdup("[");
        for (gsize i = 0; i < value->array_length; i++)
        {
            gchar *tmp = result;
            result     = g_strconcat(tmp,
                                 "\"",
                                 ((gchar **) value->array)[i],
                                 i + 1 != value->array_length ? "\", " : "]",
                                 NULL);
            g_free(tmp);
        }
        break;

    default:
        break;
    }
    }

    return result;
}

static int services_module_find_conflicting_resources_within_current_service(AlteratorCtlServicesModule *module,
                                                                             const gchar *service_str_id,
                                                                             JsonNode *parameters_node)
{
    int ret                                   = 0;
    AlteratorCtlModuleInfoParser *info_parser = module->gdbus_source->info_parser;
    GString *error_message                    = g_string_new(NULL);
    //<(services_module_strignify_toml_value(toml_value), GHashTable -> <resource_type, GArray (resources_names)>>
    GHashTable *actual_resources_data = g_hash_table_new_full(g_str_hash,
                                                              g_str_equal,
                                                              g_free,
                                                              (GDestroyNotify) g_hash_table_destroy);
    GNode *resources                  = NULL;
    if (!(resources = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
              info_parser, module->info, SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCES_TABLE_NAME)))
        goto end;

    for (GNode *resource = resources->children; resource != NULL; resource = g_node_next_sibling(resource))
    {
        alterator_entry_node *resource_data = (alterator_entry_node *) resource->data;
        gchar *type                         = g_hash_table_lookup(resource_data->toml_pairs,

                                          SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_TYPE_KEY_NAME);
        gchar *name                         = resource_data->node_name;
        toml_value *value                   = NULL;
        int actual_resource_ret             = services_module_get_actual_resourse_value(module,
                                                                            service_str_id,
                                                                            resources,
                                                                            parameters_node,
                                                                            name,
                                                                            &value);
        if (actual_resource_ret == 1)
        {
            if (info_parser->alterator_ctl_module_info_parser_find_value(
                    info_parser, resource, &value, SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCE_VALUE_KEY_NAME, NULL)
                < 0)
            {
                g_printerr(_("Can't get non-existance value of resource \"%s\" in \"%s\" service.\n"),
                           name,
                           service_str_id);
                ERR_EXIT();
            }
        }

        gchar *strinified_toml_value        = services_module_strignify_toml_value(value);
        GHashTable *current_resources_types = g_hash_table_lookup(actual_resources_data, strinified_toml_value);
        gboolean resources_types_contains   = current_resources_types != NULL;
        current_resources_types             = !resources_types_contains ? g_hash_table_new_full(g_str_hash,
                                                                                    g_str_equal,
                                                                                    NULL,
                                                                                    (GDestroyNotify) g_ptr_array_unref)
                                                                        : current_resources_types;

        GPtrArray *current_type_names = g_hash_table_lookup(current_resources_types, type);
        gboolean array_contains       = current_type_names != NULL;
        current_type_names            = !array_contains ? g_ptr_array_new_with_free_func(g_free) : current_type_names;

        gchar *display_name = NULL;
        if (services_module_get_display_name(module, name, resource, &display_name) < 0)
            g_ptr_array_add(current_type_names, g_strdup(name));
        else
            g_ptr_array_add(current_type_names, g_strdup(display_name));
        g_free(display_name);

        if (!array_contains)
            g_hash_table_insert(current_resources_types, type, current_type_names);

        if (!resources_types_contains)
            g_hash_table_insert(actual_resources_data, strinified_toml_value, current_resources_types);
    }

    GHashTableIter iter;
    g_hash_table_iter_init(&iter, actual_resources_data);
    gchar *resource_value         = NULL;
    GHashTable *resources_by_type = NULL;
    while (g_hash_table_iter_next(&iter, (gpointer *) &resource_value, (gpointer *) &resources_by_type))
    {
        GHashTableIter types_iter;
        g_hash_table_iter_init(&types_iter, resources_by_type);
        gchar *type   = NULL;
        GArray *names = NULL;
        while (g_hash_table_iter_next(&types_iter, (gpointer *) &type, (gpointer *) &names))
            if (names->len > 1)
            {
                gchar **names_str = (gchar **) names->data;
                if (names->len == 2)
                {
                    g_string_append_printf(error_message,
                                           _("A resource conflict was detected within the \"%s\" service: it attempts "
                                             "to acquire the same resources \"%s\" and \"%s\""),
                                           service_str_id,
                                           names_str[0],
                                           names_str[1]);
                }
                else
                {
                    g_string_append_printf(error_message,
                                           _("A resource conflict was detected within the service \"%s\": the service "
                                             "it attempts to acquire the same resources: "),
                                           service_str_id);
                    for (gsize i = 0; i < names->len; i++)
                        g_string_append_printf(error_message, i != names->len - 1 ? "\"%s\", " : "\"%s\"", names_str[i]);
                }
            }
    }

    if (error_message->len)
    {
        g_printerr("%s.\n", error_message->str);
        ERR_EXIT();
    }

end:
    g_string_free(error_message, TRUE);
    g_hash_table_destroy(actual_resources_data);

    return ret;
}

static int services_module_find_conflicting_services(AlteratorCtlServicesModule *module, alteratorctl_ctx_t *ctx)
{
    int ret                          = 0;
    GList *deployed_services         = NULL;
    GNode *service_info              = NULL;
    gchar *current_service_str_id    = NULL;
    gboolean all_services_undeployed = FALSE;

    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    g_variant_get(ctx->parameters, "(msms)", &current_service_str_id, NULL);

    if (service_module_get_list_of_deployed_services_with_params(module, &deployed_services, current_service_str_id) < 0)
        ERR_EXIT();

    if (info_parser->alterator_ctl_module_info_parser_get_specified_object_data(info_parser,
                                                                                module->gdbus_source,
                                                                                current_service_str_id,
                                                                                SERVICES_INTERFACE_NAME,
                                                                                &service_info)
        < 0)
    {
        g_printerr(_("Internal error: empty service info.\n"));
        ERR_EXIT();
    }

    GNode *resources = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
        info_parser, service_info, SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCES_TABLE_NAME);

    if (!resources || g_node_n_children(resources) == 0)
        goto end;

    all_services_undeployed = !g_list_length(deployed_services);
    if (!all_services_undeployed && resources)
        for (GList *deployed_service = deployed_services; deployed_service != NULL;
             deployed_service        = deployed_service->next)
            if (services_module_check_resources_conflicts(module,
                                                          current_service_str_id,
                                                          (GVariant *) deployed_service->data,
                                                          resources,
                                                          module->json)
                < 0)
                ERR_EXIT();

end:
    if (service_info)
        alterator_ctl_module_info_parser_result_tree_free(service_info);

    g_list_free_full(deployed_services, (GDestroyNotify) g_variant_unref);

    if (current_service_str_id)
        g_free(current_service_str_id);

    return ret;
}

static int services_module_check_resources_conflicts(AlteratorCtlServicesModule *module,
                                                     const gchar *current_service_str_id,
                                                     const GVariant *deployed_service_data,
                                                     GNode *resources,
                                                     JsonNode *parameters)
{
    int ret                                   = 0;
    gchar *deployed_service_str_id            = NULL;
    gchar *deployed_service_params            = NULL;
    GString *conflicts                        = g_string_new("");
    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;
    if (!deployed_service_data)
    {
        g_printerr(_("Internal error: failed to check resources conflicts with other services.\n"));
        ERR_EXIT();
    }

    g_variant_get((GVariant *) deployed_service_data, "(ss)", &deployed_service_str_id, &deployed_service_params);

    if (!deployed_service_str_id || !deployed_service_params)
    {
        g_printerr(_("Internal error: failed to check conflicts with other services.\n"));
        ERR_EXIT();
    }

    GNode *deployed_service_info = NULL;
    if (info_parser->alterator_ctl_module_info_parser_get_specified_object_data(info_parser,
                                                                                module->gdbus_source,
                                                                                deployed_service_str_id,
                                                                                SERVICES_INTERFACE_NAME,
                                                                                &deployed_service_info)
        < 0)
    {
        g_printerr(_("Internal error: failed to check conflicts with other services.\n"));
        ERR_EXIT();
    }

    GNode *deployed_service_resources = NULL;
    if (!(deployed_service_resources = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
              info_parser, deployed_service_info, SERVICES_ALTERATOR_ENTRY_SERVICE_RESOURCES_TABLE_NAME)))
        goto end;

    JsonParser *deployed_service_parser    = NULL;
    JsonNode *deployed_service_params_node = NULL;

    gsize conflicts_count = 0;
    for (GNode *resource = resources->children; resource != NULL; resource = resource->next)
    {
        const gchar *resource_name           = ((alterator_entry_node *) resource->data)->node_name;
        toml_value *deployable_service_value = NULL;
        gint get_actual_ret                  = services_module_get_actual_resourse_value(module,
                                                                        current_service_str_id,
                                                                        resources,
                                                                        parameters,
                                                                        resource_name,
                                                                        &deployable_service_value);
        if (get_actual_ret < 0)
            ERR_EXIT();
        else if (get_actual_ret == 1)
            continue;

        if (!deployed_service_parser)
        {
            deployed_service_parser = json_parser_new();
            if (!json_parser_load_from_data(deployed_service_parser, deployed_service_params, -1, NULL))
            {
                g_printerr(_("Invalid json parameters.\n"));
                g_object_unref(deployed_service_parser);
                deployed_service_parser = NULL;
                ERR_EXIT();
            }

            deployed_service_params_node = json_parser_get_root(deployed_service_parser);
        }

        toml_value *deployed_service_value = NULL;
        get_actual_ret                     = services_module_get_actual_resourse_value(module,
                                                                   current_service_str_id,
                                                                   deployed_service_resources,
                                                                   deployed_service_params_node,
                                                                   resource_name,
                                                                   &deployed_service_value);

        if (get_actual_ret < 0)
        {
            ERR_EXIT();
        }
        else if (get_actual_ret == 1)
        {
            continue;
        }

        if (alterator_ctl_compare_toml_values(deployable_service_value, deployed_service_value, NULL))
        {
            gchar *display_name = NULL;
            services_module_get_display_name(module, resource_name, resource, &display_name);

            gchar *strinified_value = services_module_strignify_toml_value(deployable_service_value);
            g_string_append_printf(conflicts,
                                   _("Resource \"%s\" with value %s is occupied by another "
                                     "service \"%s\".\n"),
                                   display_name && g_utf8_strlen(display_name, -1) ? display_name : resource_name,
                                   strinified_value,
                                   deployed_service_str_id);
            g_free(display_name);

            g_free(strinified_value);
            conflicts_count++;
        }
    }

    if (conflicts_count)
    {
        g_string_prepend(conflicts,
                         conflicts_count == 1 ? _("Resource conflict detected.\n")
                                              : _("Resources conflicts detected.\n"));
        g_printerr("%s", conflicts->str);
        ERR_EXIT();
    }

end:
    if (conflicts)
        g_string_free(conflicts, TRUE);

    if (deployed_service_info)
        alterator_ctl_module_info_parser_result_tree_free(deployed_service_info);

    if (deployed_service_parser)
        g_object_unref(deployed_service_parser);

    g_free(deployed_service_str_id);

    g_free(deployed_service_params);

    return ret;
}

static int services_module_validate_json_params(AlteratorCtlServicesModule *module,
                                                JsonNode *params,
                                                const gchar *service_str_id,
                                                GNode *optional_service_info)
{
    int ret                        = 0;
    GNode *service_alterator_entry = NULL;

    if (!module)
    {
        if (service_str_id && strlen(service_str_id))
            g_printerr(_("Can't validate json params in %s service. The services module doesn't exist.\n"),
                       service_str_id);
        else
            g_printerr(_("Can't validate json params. The services module doesn't exist.\n"));
        ERR_EXIT();
    }

    AlteratorCtlModuleInfoParser *info_parser = (AlteratorCtlModuleInfoParser *) module->gdbus_source->info_parser;

    if (!service_str_id)
    {
        g_printerr(_("Can't validate json params in unspecified service.\n"));
        ERR_EXIT();
    }

    JsonObject *params_object = params ? json_node_get_object(params) : NULL;
    if (!params_object)
    {
        g_printerr(_("Can't validate json params in %s service. Parameters is NULL.\n"), service_str_id);
        ERR_EXIT();
    }

    if (optional_service_info)
        service_alterator_entry = optional_service_info;
    else if (info_parser->alterator_ctl_module_info_parser_get_specified_object_data(info_parser,
                                                                                     module->gdbus_source,
                                                                                     service_str_id,
                                                                                     SERVICES_INTERFACE_NAME,
                                                                                     &service_alterator_entry)
             < 0)
    {
        g_printerr(_("Failed to validate %s service parameters. Can't get service info.\n"), service_str_id);
        ERR_EXIT();
    }

    GNode *service_alterator_entry_params = NULL;
    if (!(service_alterator_entry_params = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
              info_parser, service_alterator_entry, SERVICES_ALTERATOR_ENTRY_SERVICE_PARAMETERS_TABLE_NAME)))
    {
        g_printerr(_("It is not possible to validate parameters for a service that does not contain them.\n"));
        ERR_EXIT();
    }

    JsonObjectIter json_iter;
    const gchar *member_name = NULL;
    JsonNode *member_node    = NULL;
    json_object_iter_init(&json_iter, params_object);
    while (json_object_iter_next(&json_iter, &member_name, &member_node))
    {
        JsonNodeType member_node_type = json_node_get_node_type(member_node);
        GNode *parameter_info         = NULL;
        if (!(parameter_info = info_parser->alterator_ctl_module_info_parser_get_node_by_name(
                  info_parser, service_alterator_entry_params, member_name)))
        {
            g_printerr(_("Service %s validation parameters failed. The service doesn't have \"%s\" parameter.\n"),
                       service_str_id,
                       member_name);
            ERR_EXIT();
        }

        alterator_entry_node *parameter = (alterator_entry_node *) parameter_info->data;
        toml_value *parameter_type
            = parameter ? g_hash_table_lookup(parameter->toml_pairs,
                                              SERVICES_ALTERATOR_ENTRY_SERVICE_DEFAULT_PARAM_TYPE_KEY_NAME)
                        : NULL;
        if (!parameter_type || parameter_type->type != TOML_DATA_STRING)
        {
            g_printerr(_("Service %s validation parameters failed. The parameter %s data type isn't defined in the "
                         "service "
                         "alterator entry.\n"),
                       service_str_id,
                       parameter->node_name);
            ERR_EXIT();
        }

        if (member_node_type == JSON_NODE_NULL)
        {
            g_printerr(_("Service %s validation parameters failed. Services don't support null parameters.\n"),
                       service_str_id);
            ERR_EXIT();
        }
        else if (member_node_type == JSON_NODE_ARRAY || member_node_type == JSON_NODE_VALUE)
        {
            JsonNode *node_value = NULL;
            if (member_node_type == JSON_NODE_ARRAY)
            {
                if (!streq(parameter_type->str_value, "array"))
                {
                    g_printerr(_("Service %s validation parameters failed. The parameter %s must have an \"%s\" data "
                                 "type, not a "
                                 "\"array\".\n"),
                               service_str_id,
                               parameter->node_name,
                               parameter_type->str_value);
                    ERR_EXIT();
                }

                toml_value *min_size = g_hash_table_lookup(parameter->toml_pairs, "array_min");
                if (min_size && min_size->type != TOML_DATA_DOUBLE)
                {
                    g_printerr(
                        _("Service %s validation parameters failed. Wrong type of array min length field of \"%s\" "
                          "array in %s service alterator entry.\n"),
                        service_str_id,
                        parameter->node_name,
                        service_str_id);
                }

                toml_value *max_size = g_hash_table_lookup(parameter->toml_pairs, "array_max");
                if (max_size && max_size->type != TOML_DATA_DOUBLE)
                {
                    g_printerr(
                        _("Service %s validation parameters failed. Wrong type of array max length field of \"%s\" "
                          "array in %s service alterator entry.\n"),
                        service_str_id,
                        parameter->node_name,
                        service_str_id);
                }

                JsonArray *arr  = json_node_get_array(member_node);
                gint arr_length = json_array_get_length(arr);
                if (min_size && arr_length < min_size->double_value)
                {
                    g_printerr(
                        _("Service %s validation parameters failed. The size of the array parameter \"%s\" must be "
                          "greater than %d, while it is %d.\n"),
                        service_str_id,
                        parameter->node_name,
                        (gint) min_size->double_value,
                        (gint) json_array_get_length(arr));
                    ERR_EXIT();
                }

                if (max_size && arr_length > max_size->double_value)
                {
                    g_printerr(
                        _("Service %s validation parameters failed. The size of the array parameter \"%s\" must be "
                          "less than %d, while it is %d.\n"),
                        service_str_id,
                        parameter->node_name,
                        (gint) max_size->double_value,
                        (gint) json_array_get_length(arr));
                    ERR_EXIT();
                }

                node_value = !json_array_get_length(arr) ? NULL : json_array_get_element(arr, 0);
                if (!node_value)
                    continue; // empty array
            }
            else
                node_value = member_node;

            toml_value *pattern   = g_hash_table_lookup(parameter->toml_pairs, "pattern");
            GType node_value_type = json_node_get_value_type(node_value);
            switch (node_value_type)
            {
            case G_TYPE_STRING:
                if (pattern)
                {
                    GRegex *string_param_pattern = g_regex_new(pattern->str_value, 0, 0, NULL);
                    GMatchInfo *match            = NULL;
                    g_regex_match(string_param_pattern, json_node_get_string(member_node), 0, &match);
                    if (!g_match_info_matches(match))
                    {
                        g_printerr(_("The parameter %s value doesn't match the pattern %s.\n"),
                                   parameter->node_name,
                                   pattern->str_value);
                        g_match_info_free(match);
                        g_regex_unref(string_param_pattern);
                        ERR_EXIT();
                    }
                    g_match_info_free(match);
                    g_regex_unref(string_param_pattern);
                }
                break;

            case G_TYPE_INT64: {
                GRegex *int_param_pattern
                    = g_regex_new(pattern ? pattern->str_value : SERVICES_JSON_PARAMS_INTEGER_PATTERN, 0, 0, NULL);
                GMatchInfo *match = NULL;
                g_regex_match(int_param_pattern, json_node_get_string(member_node), 0, &match);
                if (!g_match_info_matches(match))
                {
                    g_printerr(_("The parameter %s value doesn't match the pattern %s.\n"),
                               parameter->node_name,
                               pattern ? pattern->str_value : SERVICES_JSON_PARAMS_INTEGER_PATTERN);
                    g_match_info_free(match);
                    g_regex_unref(int_param_pattern);
                    ERR_EXIT();
                }
                g_match_info_free(match);
                g_regex_unref(int_param_pattern);
            }
            break;

            case G_TYPE_BOOLEAN: {
                GRegex *boolean_param_pattern = g_regex_new(SERVICES_JSON_PARAMS_BOOLEAN_PATTERN, 0, 0, NULL);
                GMatchInfo *match             = NULL;
                g_regex_match(boolean_param_pattern, json_node_get_string(member_node), 0, &match);
                if (!g_match_info_matches(match))
                {
                    g_printerr(_("The parameter %s value doesn't match the pattern %s.\n"),
                               parameter->node_name,
                               SERVICES_JSON_PARAMS_BOOLEAN_PATTERN);
                    g_match_info_free(match);
                    g_regex_unref(boolean_param_pattern);
                    ERR_EXIT();
                }
                g_match_info_free(match);
                g_regex_unref(boolean_param_pattern);
                break;
            }

            default: {
                GTypeQuery *type_info = NULL;
                g_type_query(node_value_type, type_info);
                if (member_node_type == JSON_NODE_ARRAY)
                    g_printerr(_("Wrong array %s parameter elem type %s.\n"),
                               parameter->node_name,
                               type_info->type_name);
                else
                    g_printerr(_("Wrong %s parameter type %s.\n"), parameter->node_name, type_info->type_name);
                g_free(type_info);
                ERR_EXIT();
            }
            break;
            }
        }
        else if (member_node_type == JSON_NODE_OBJECT)
        {
            if (!streq(parameter_type->str_value, "object"))
            {
                g_printerr(_("Service %s validation parameters failed. The parameter %s must have an \"%s\" data "
                             "type, not a "
                             "\"object\".\n"),
                           service_str_id,
                           parameter->node_name,
                           parameter_type->str_value);
                ERR_EXIT();
            }

            JsonObject *subobject = json_node_get_object(member_node);
            JsonObjectIter subobject_json_iter;
            const gchar *subobject_member_name = NULL;
            JsonNode *subobject_member_node    = NULL;
            json_object_iter_init(&json_iter, subobject);
            while (json_object_iter_next(&subobject_json_iter, &subobject_member_name, &subobject_member_node))
                if (services_module_validate_json_params(module, member_node, service_str_id, service_alterator_entry)
                    < 0)
                    ERR_EXIT();
        }
    }

end:
    if (!optional_service_info && service_alterator_entry)
        alterator_ctl_module_info_parser_result_tree_free(service_alterator_entry);

    return ret;
}
