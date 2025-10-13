/**/

#include "alterator_manager_backend_monitor.h"


static gboolean backends_changed = FALSE;

static void change_flag(void) {
    backends_changed = TRUE;
}

static GFile *file_new_dir(gchar *dir_path) {
    GFile *dir;

    if (!dir_path) {
        return NULL;
    }

    dir = g_file_new_for_path(dir_path);
    if (!g_file_query_exists(dir, NULL)) {
        g_debug("Dir %s doesn't exist.", dir_path);
        g_object_unref(dir);
        return NULL;
    }

    return dir;
}

static GFileMonitor *create_monitor(GFile *dir) {
    GFileMonitor *monitor;
    GError *error = NULL;

    if (!dir) {
        return NULL;
    }

    monitor = g_file_monitor_directory(dir,
                                       G_FILE_MONITOR_NONE,
                                       NULL,
                                       &error);

    if (error) {
        g_debug("Create monitor error %s.\n", error->message);
        g_error_free(error);
        g_free(monitor);
        return NULL;
    }

    //This function is called 6 times when the file is created,
    //5 times when copied to this directory, and once when a file is deleted.
    //Why? I don't know.
    g_signal_connect(monitor,
                     "changed",
                     change_flag,
                     NULL);

    return monitor;
}

void alterator_manager_backend_monitor_init(void) {
    GFile *etc;
    GFile *etc_system;
    GFile *etc_user;
    GFile *share;
    GFile *share_system;
    GFile *share_user;

    backends_changed = FALSE;

    etc = file_new_dir(BACKENDS_DIR_ETC);
    etc_system = file_new_dir(BACKENDS_DIR_ETC_SYSTEM);
    etc_user = file_new_dir(BACKENDS_DIR_ETC_USER);
    share = file_new_dir(BACKENDS_DIR_SHARE);
    share_system = file_new_dir(BACKENDS_DIR_SHARE_SYSTEM);
    share_user = file_new_dir(BACKENDS_DIR_SHARE_USER);

    create_monitor(etc);
    create_monitor(etc_system);
    create_monitor(etc_user);
    create_monitor(share);
    create_monitor(share_system);
    create_monitor(share_user);

    g_clear_object(&etc);
    g_clear_object(&etc_system);
    g_clear_object(&etc_user);
    g_clear_object(&share);
    g_clear_object(&share_system);
    g_clear_object(&share_user);
}

gboolean alterator_manager_backend_monitor_backends_changed(gpointer data) {
    GMainLoop *mainLoop = (GMainLoop *) data;

    if (backends_changed && !alterator_manager_interface_modules_are_busy()) {
        g_main_loop_quit(mainLoop);
        g_debug("Backend files changed, exit.");
        return FALSE;
    }

    return TRUE;
}

