/**/

#include "alterator_manager_dev_tool.h"

static const gchar policy_top[] =
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
  "<!DOCTYPE policyconfig PUBLIC\n"
  "\"-//freedesktop//DTD PolicyKit Policy Configuration 1.0//EN\"\n"
  "\"http://www.freedesktop.org/standards/PolicyKit/1/policyconfig.dtd\">\n"
  "<policyconfig>\n";

static const gchar policy_action[] =
  "  <action id=\"";

static const gchar policy_message[] =
  "\">\n"
  "    <description>Alterator Authentication dialogue</description>\n"
  "    <message>";

static const gchar policy_action_end[] =
  " requires authentication.</message>\n"
  "    <defaults>\n"
  "      <allow_any>auth_admin_keep</allow_any>\n"
  "      <allow_inactive>auth_admin_keep</allow_inactive>\n"
  "      <allow_active>auth_admin_keep</allow_active>\n"
  "    </defaults>\n"
  "  </action>\n";

static const gchar policy_bottom[] =
  "</policyconfig>\n";

static gboolean quiet;
static gchar *gen_policy = NULL;
static gchar *validate = NULL;
static gchar *output = NULL;


static GOptionEntry entries[] =
{
    {
        "gen-policy-template",
        'g',
        0,
        G_OPTION_ARG_STRING,
        &gen_policy,
        "Generate content for the policy file based on the backend file",
        NULL
    },
    {
        "validate",
        't',
        0,
        G_OPTION_ARG_STRING,
        &validate,
        "Validate the backend file",
        NULL
    },
    {
        "quiet",
        'q',
        0,
        G_OPTION_ARG_NONE,
        &quiet,
        "Suppress output",
        NULL
    },
    {
        "output",
        'o',
        0,
        G_OPTION_ARG_STRING,
        &output,
        "Write the generated policy file contents to the file",
        NULL
    },
    { NULL }
};

static void quiet_print_handler(const gchar *string) {
}

static gboolean write_to_file(gchar *path_to_file, gchar *content) {
    FILE *file;

    file = fopen(path_to_file, "w");
    if (file == NULL) {
        g_warning("fopen returned NULL (%s).", path_to_file);
        return FALSE;
    }

    fprintf(file, "%s", content);
    fclose(file);

    return TRUE;
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
contain letters, numbers, and underscores.
It is assumed that the length of the name cannot be greater than 100 and less
than 2. */
static gboolean is_correct_name(gchar* name) {
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

/* Do not forget to call g_key_file_free() for GKeyFile *backend_file. */
static GKeyFile *load_file(gchar *path) {
    GKeyFile *backend_file = NULL;
    GError *error = NULL;

    backend_file = g_key_file_new();

    g_key_file_load_from_file(backend_file,
                              path,
                              G_KEY_FILE_NONE,
                              &error);

    if (error) {
        g_print("File load error(%s): %s.\n", path, error->message);
        g_key_file_free(backend_file);
        g_error_free(error);
        return NULL;
    }

    return backend_file;
}

/* Checking the Alterator Entry section. Field validation: Type, Name, Module
   and Interface. For the executor module, checking section names. */
static gboolean validation(GKeyFile *backend_file) {
    gboolean result = TRUE;
    gchar *type = NULL;
    gchar *module = NULL;
    gchar *node = NULL;
    gchar *interface_short = NULL;
    gchar *interface_full = NULL;
    gchar *default_action_id = NULL;
    gchar **sections = NULL;
    gsize length = 0;

    if (backend_file == NULL) {
        g_print("validation(): passed argument is NULL.\n");
        return FALSE;
    }

    if (!g_key_file_has_group(backend_file, TOP_SECTION)) {
        g_print("The backend file doesn't have group '%s'.\n", TOP_SECTION);
        return FALSE;
    }

    type = g_key_file_get_string(backend_file,
                                 TOP_SECTION,
                                 TYPE_NAME,
                                 NULL);

    if (type == NULL) {
        g_print("The '%s' field in the '%s' section is not specified.\n",
                TYPE_NAME, TOP_SECTION);
        result = FALSE;
    } else if (g_strcmp0(type, TYPE_BACKEND)) {
        g_print("The '%s' field in the '%s' section is not equal '%s'.\n",
                TYPE_NAME, TOP_SECTION, TYPE_BACKEND);
        result = FALSE;
    }

    module = g_key_file_get_string(backend_file,
                                   TOP_SECTION,
                                   MODULE_NAME,
                                   NULL);

    if (module == NULL) {
        g_print("The '%s' field in the '%s' section is not specified.\n",
                MODULE_NAME, TOP_SECTION);
        result = FALSE;
    }

    node = g_key_file_get_string(backend_file,
                                 TOP_SECTION,
                                 NODE_NAME,
                                 NULL);

    if (node == NULL) {
        g_print("The '%s' field in the '%s' section is not specified.\n",
                NODE_NAME, TOP_SECTION);
        result = FALSE;
    } else if (!is_correct_name(node)) {
        g_print("The '%s' field in the '%s' section is not correct.\n",
                NODE_NAME, TOP_SECTION);
        result = FALSE;
    }

    interface_short = g_key_file_get_string(backend_file,
                                            TOP_SECTION,
                                            INTERFACE_NAME,
                                            NULL);

    if (interface_short == NULL) {
        g_print("The '%s' field in the '%s' section is not specified.\n",
                INTERFACE_NAME, TOP_SECTION);
        result = FALSE;
    } else {
        interface_full = g_strconcat(INTERFACE_PREFIX, interface_short, NULL);

        if (!g_dbus_is_interface_name(interface_full)) {
            g_print("'%s' is not a correct interface name.\n", interface_full);
            result = FALSE;
        }
    }

    /* Check a default action id for the interface. */
    default_action_id = g_key_file_get_string(backend_file,
                                              TOP_SECTION,
                                              ACTION_ID,
                                              NULL);

    if (default_action_id != NULL && !is_correct_action_id(default_action_id)) {
        g_print("'%s' is not a correct action id.\n", default_action_id);
        result = FALSE;
    }

    /* Check for the executor module. */
    if (!g_strcmp0(module, MODULE_NAME)) {
        sections = g_key_file_get_groups(backend_file, &length);

        for (gint i = 0; i < length; i++) {
            if (!g_strcmp0(sections[i], TOP_SECTION)) {
                continue;
            }

            if (!is_correct_name(sections[i])) {
                g_print("'%s' is not a correct method name.\n", sections[i]);
                result = FALSE;
            }

            if (sections[i]) {
                gchar *action_id = g_key_file_get_string(backend_file,
                                                         sections[i],
                                                         ACTION_ID,
                                                         NULL);

                if (action_id) {
                    if (!is_correct_action_id(action_id)) {
                        g_print("'%s' is not a correct action id "
                                "(section: %s).\n", action_id, sections[i]);
                    }
                    g_free(action_id);
                }
            }
        }
    }

    if (type) {
        g_free(type);
    }
    if (module) {
        g_free(module);
    }
    if (node) {
        g_free(node);
    }
    if (interface_short) {
        g_free(interface_short);
    }
    if (interface_full) {
        g_free(interface_full);
    }
    if (default_action_id) {
        g_free(default_action_id);
    }
    if (sections) {
        g_strfreev(sections);
    }

    return result;
}

/* Generates the contents of the policy file based on the passed file. */
static gchar* generate_content_for_policy(GKeyFile *backend_file) {
    GString *policy_xml = NULL;
    gchar *action_id = NULL;
    gchar *interface = NULL;
    gchar *interface_full = NULL;
    gchar **sections = NULL;
    gsize length = 0;

    if (backend_file == NULL) {
        g_print("generate_content_for_policy(): passed argument is NULL.\n");
        return NULL;
    }

    //action id for the interface
    action_id = g_key_file_get_string(backend_file,
                                      TOP_SECTION,
                                      ACTION_ID,
                                      NULL);

    if (action_id == NULL) {
        interface = g_key_file_get_string(backend_file,
                                          TOP_SECTION,
                                          INTERFACE_NAME,
                                          NULL);

        /* If action_id == NULL or it is not correct we use interface name. */
        if (interface) {
            interface_full = g_strconcat(INTERFACE_PREFIX, interface, NULL);
            action_id = g_strdelimit (g_strdup (interface_full), "_", '-');
        } else {
            g_print("generate_content_for_policy(): interface == NULL.\n");
            return NULL;
        }

        if (!is_correct_action_id(action_id)) {
            g_print("'%s' is not a correct action id.\n", action_id);
            g_free(action_id);
            g_free(interface);

            return NULL;
        }
    }

    policy_xml = g_string_new("");
    g_string_append(policy_xml, policy_top);
    //The default action id.
    g_string_append(policy_xml, policy_action);
    g_string_append(policy_xml, action_id);
    g_string_append(policy_xml, policy_message);
    g_string_append(policy_xml, action_id);
    g_string_append(policy_xml, policy_action_end);

    sections = g_key_file_get_groups(backend_file, &length);

    //The action id's from methods.
    for (gint i = 0; i < length; i++) {
        if (!g_strcmp0(sections[i], TOP_SECTION)) {
            continue;
        }

        gchar *methods_action_id = g_key_file_get_string(backend_file,
                                                         sections[i],
                                                         ACTION_ID,
                                                         NULL);

        if (methods_action_id) {
            gchar *methods_action_id_full = g_strconcat(action_id,
                                                        ".",
                                                        methods_action_id,
                                                        NULL);

            g_string_append(policy_xml, policy_action);
            g_string_append(policy_xml, methods_action_id_full);
            g_string_append(policy_xml, policy_message);
            g_string_append(policy_xml, methods_action_id_full);
            g_string_append(policy_xml, policy_action_end);

            g_free(methods_action_id);
            g_free(methods_action_id_full);
        }
    }

    //End of xml
    g_string_append(policy_xml, policy_bottom);

    if (action_id) {
        g_free(action_id);
    }
    if (interface) {
        g_free(interface);
    }
    if (interface_full) {
        g_free(interface_full);
    }
    if (sections) {
        g_strfreev(sections);
    }

    //Release the returned string when it is no longer needed.
    return g_string_free(policy_xml, FALSE);
}

static void print_help(GOptionContext *context) {
    gchar *help = g_option_context_get_help(context, TRUE, NULL);
    g_print("%s", help);
    g_free(help);
}

int main(int argc, char **argv) {
    GOptionContext *context;
    GError *error = NULL;

    context = g_option_context_new ("[backend file] [-o file]");
    g_option_context_set_summary(context,
      "Utility for generating '.policy' xml (polkit) from '.backend' files.\n"
      "\n"
      "Examples:"
      "\n"
      "$am-dev-tool -g <backend file> // Generate '.policy' xml.\n"
      "$am-dev-tool -g <backend file> -o <file> // Generate '.policy'"
      " xml and save it to a file.\n"
      "$am-dev-tool -t <backend file> // Checking the correctness of the "
      "'.backend' file.\n");

    g_option_context_add_main_entries (context, entries, NULL);

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_print ("Option parsing failed: %s.\n\n", error->message);

        print_help(context);

        return 3;
    }

    if (quiet) {
        g_set_print_handler(quiet_print_handler);
        g_set_printerr_handler(quiet_print_handler);
    }

    if (gen_policy) {
        gchar *xml = NULL;

        GKeyFile *backend_file = load_file(gen_policy);
        if (backend_file == NULL) {
            return 1;
        }

        if (validation(backend_file)) {
            xml = generate_content_for_policy(backend_file);

            if (output) {
                write_to_file(output, xml);
            } else {
                g_print(xml);
            }
        }

        if (xml) {
            g_free(xml);
        }
        if (backend_file) {
            g_key_file_free(backend_file);
        }
    } else if (validate) {
        GKeyFile *backend_file = load_file(validate);
        if (backend_file == NULL) {
            return 1;
        }

        if (validation(backend_file)) {
            g_print("File %s is OK!\n", validate);
            return 0;
        } else {
            return 1;
        }

        if (backend_file) {
            g_key_file_free(backend_file);
        }
    } else {
        print_help(context);
        return 1;
    }

    g_option_context_free(context);
    if (gen_policy) {
        g_free(gen_policy);
    }
    if (validate) {
        g_free(validate);
    }
    if (output) {
        g_free(output);
    }
    return 0;
}
