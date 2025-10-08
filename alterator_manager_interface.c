/**/

#include "alterator_manager_interface.h"
#include "alterator_manager_backends.h"
#include "alterator_manager_polkit.h"
#include "alterator_manager_data.h"

static GList *module_interfaces = NULL;

static gboolean manager_register_module (const AlteratorModuleInterface* module_interface)
{
    gboolean res;

    if (module_interface == NULL) {
        g_warning("manager_register_module(): module_interface == NULL.");
        return FALSE;
    }
    if (module_interface->interface_version == NULL) {
        g_warning("manager_register_module(): module_interface->interface_version == NULL.");
        return FALSE;
    }

    gint module_interface_version = module_interface->interface_version();
    gint manager_interface_version = alterator_module_interface_version();

    if (manager_interface_version != module_interface_version) {
        g_warning("manager_register_module(): manager (%d) and module (%d) interface versions not equal.",
                  manager_interface_version, module_interface_version);
        return FALSE;
    }
    if (module_interface->init == NULL) {
        g_warning("manager_register_module(): module_interface->init == NULL.");
        return FALSE;
    }
    if (module_interface->destroy == NULL) {
        g_warning("manager_register_module(): module_interface->destroy == NULL.");
        return FALSE;
    }
    if (module_interface->is_module_busy == NULL) {
        g_warning("manager_register_module(): module_interface->is_module_busy"
                  " == NULL.");
        return FALSE;
    }

    res = module_interface->init(alterator_manager_data_get_data());
    if (!res) {
        g_warning("manager_register_module(): module_interface->init() failed.");
        return FALSE;
    }

    AlteratorModuleInterface *i = (AlteratorModuleInterface *) module_interface;
    module_interfaces = g_list_prepend(module_interfaces, i);

    return TRUE;
}

static gboolean arguments_cmp (GDBusArgInfo **interface_args,
                               GDBusArgInfo **template_args)
{
    if (interface_args == NULL && template_args == NULL) {
        return TRUE;
    } else if (interface_args == NULL || template_args == NULL) {
        return FALSE;
    }

    while (*template_args != NULL && *interface_args != NULL) {
        if (g_strcmp0((*interface_args)->name, (*template_args)->name)) {
            return FALSE;
        }

        if (g_strcmp0((*interface_args)->signature,
                      (*template_args)->signature))
        {
            return FALSE;
        }

        template_args++;
        interface_args++;
    }

    if (*template_args != NULL || *interface_args != NULL) {
        return FALSE;
    }

    return TRUE;
}

static gboolean method_cmp (const GDBusMethodInfo *interface_method,
                            const GDBusMethodInfo *template_method)
{
    if (interface_method == NULL || template_method == NULL) {
        return FALSE;
    }

    if (g_strcmp0(interface_method->name, template_method->name)) {
        return FALSE;
    }

    if (!arguments_cmp(interface_method->in_args, template_method->in_args)) {
        return FALSE;
    }

    if (!arguments_cmp(interface_method->out_args, template_method->out_args)) {
        return FALSE;
    }

    return TRUE;
}

static gboolean interface_cmp (GDBusInterfaceInfo *interface_info,
                               GDBusInterfaceInfo *template)
{
    GDBusMethodInfo *interface_method = NULL;

    if (interface_info == NULL || template == NULL) {
        return TRUE;
    }

    if (g_strcmp0(interface_info->name, template->name)) {
        return TRUE;
    }

    /* If interface name matched, then the method names, names and types of
       their arguments and return values must match. Else we return FALSE. */
    GDBusMethodInfo **template_methods = template->methods;
    while (*template_methods != NULL) {
        interface_method = g_dbus_interface_info_lookup_method(interface_info,
                                                     (*template_methods)->name);

        if (interface_method == NULL) {
            return FALSE;
        }

        if (!method_cmp(interface_method, (*template_methods))) {
            return FALSE;
        }

        interface_method = NULL;
        template_methods++;
    }

    return TRUE;
}

/* Checking if introspection matches a pattern in a directory
   /usr/share/dbus-1/interfaces. This returns FALSE if the interface name
   matched, but the method names, the names and types of input and output
   parameters of the methods did not match, or if there are methods in the
   template that are not in the introspection being checked. */
static gboolean interface_validation (GDBusInterfaceInfo *interface_info,
                                      gchar *interface_name) {
    GDBusNodeInfo *node_info = NULL;
    GDBusInterfaceInfo *template = NULL; //it belongs to node_info
    gchar *file_contents = NULL;
    gchar *file_path = NULL;
    gboolean result = TRUE;
    GError *error = NULL;

    if (interface_info == NULL || interface_name == NULL) {
        return TRUE;
    }

    file_path = g_strconcat(INTROSPECTION_FILES_PATH,
                            interface_name,
                            ".xml",
                            NULL);

    if (g_file_get_contents(file_path, &file_contents, NULL, &error)) {
        node_info = g_dbus_node_info_new_for_xml(file_contents, &error);

        if (error) {
            g_warning("Interface validation: g_dbus_node_info_new_for_xml,\n"
                      "error: %s.", error->message);
            g_error_free(error);
            g_free(file_path);
            g_free(file_contents);

            return TRUE;
        }

        template = g_dbus_node_info_lookup_interface(node_info,
                                                     interface_name);
        if (template == NULL) {
            g_dbus_node_info_unref(node_info);
            g_free(file_path);
            g_free(file_contents);
            return TRUE;
        }

        result = interface_cmp(interface_info, template);
    } else if (error) {
//        g_debug("Interface validation: failed to get contents of %s. Error: "
//                  "%s.", file_path, error->message);
        g_error_free(error);
    }

    g_free(file_path);
    if (file_contents) {
        g_free(file_contents);
    }

    return result;
}

static AlteratorManagerInterface manager_interface =
{
    .register_module = manager_register_module,
    .interface_validation = interface_validation
};

const AlteratorManagerInterface* alterator_manager_interface(void) {
    return &manager_interface;
}

gboolean alterator_manager_interface_modules_are_busy(void) {
    for (GList *a = module_interfaces; a; a = a->next) {
        AlteratorModuleInterface *interface =
                                           (AlteratorModuleInterface *) a->data;

        if (interface->is_module_busy()) {
            return TRUE;
        }
    }

    return FALSE;
}

