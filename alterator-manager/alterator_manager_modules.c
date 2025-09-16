/**/

#include "alterator_manager_modules.h"
#include "alterator_manager_interface.h"
#include "alterator_manager_polkit.h"
#include "alterator_manager_module_info.h"

typedef gboolean (*AlteratorModuleInit) (const AlteratorManagerInterface *manager_interface);

static void load_module(const gchar *module_library_name) {
    GModule *module;
    AlteratorModuleInit alterator_module_init;
    gchar *module_library_path = NULL;
    gboolean res = FALSE;

    module_library_path = g_strconcat(MODULES_DIR, module_library_name, NULL);
    if (module_library_path == NULL) {
        g_warning("module_library_path == NULL.");
        return;
    }

    module = g_module_open(module_library_path, G_MODULE_BIND_LAZY);

    g_free(module_library_path);

    if (module == NULL) {
        g_warning("Module library not open (%s).", module_library_name);
        return;
    }

    res = g_module_symbol(module,
                          "alterator_module_init",
                          (gpointer *)&alterator_module_init);
    if (!res) {
        g_warning("Can't get symbol \'alterator_module_init\' (%s).", module_library_name);
        if (!g_module_close(module)) {
            g_warning("g_module_close() returned FALSE (%s): %s",
            module_library_name, g_module_error ());
        }
        return;
    }

    if (alterator_module_init == NULL) {
        g_warning("alterator_module_init == NULL (%s).", module_library_name);
        if (!g_module_close(module)) {
            g_warning("g_module_close() returned FALSE (%s): %s",
            module_library_name, g_module_error ());
        }
        return;
    }

    res = alterator_module_init(alterator_manager_interface());
    if (!res) {
        g_warning("alterator_module_init() failed (%s).", module_library_name);
        if (!g_module_close(module)) {
            g_warning("g_module_close() returned FALSE (%s): %s",
            module_library_name, g_module_error ());
        }
        return;
    }

    /* We don't close modules yet. */
}

static void update_modules_list(void) {
    GDir *dir = NULL;
    GError *error = NULL;

    dir = g_dir_open(MODULES_DIR, 0, &error);

    if (error) {
        g_critical("Open directory %s, %s.", MODULES_DIR, error->message);
        g_error_free(error);
        return;
    }

    const gchar *file_name = NULL;
    while ((file_name = g_dir_read_name(dir)) != NULL) {
        g_debug("Load module: %s.", file_name);
        load_module(file_name);
    }

    g_dir_close(dir);
}

gboolean alterator_manager_modules_init(void) {
    GFile *dir;
//    GFileMonitor *monitor;
//    GError *error = NULL;

    dir = g_file_new_for_path(MODULES_DIR);

    if (!g_file_query_exists(dir, NULL)) {
        g_critical("Dir %s doesn't exist.", MODULES_DIR);
        g_object_unref(dir);
        return FALSE;
    }

    update_modules_list();

/* Maybe one day */
/*    monitor = g_file_monitor_directory(dir,
                                       G_FILE_MONITOR_NONE,
                                       NULL,
                                       &error);

    if (error) {
        g_print("ERROR: create monitor %s, %s.\n", MODULES_DIR, error->message);
        g_error_free(error);
        g_free(monitor);
        return 1;
    }

    //This function is called 6 times when the file is created,
    //5 times when copied to this directory, and once when a file is deleted.
    //Why? I don't know.
    g_signal_connect(monitor, "changed", update_module_list, NULL);*/

    g_object_unref(dir);

    return TRUE;
}
