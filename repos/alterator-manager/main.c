/**/
#define G_LOG_USE_STRUCTURED
#include "alterator_manager_modules.h"
#include "alterator_manager_dbus.h"
#include "alterator_manager_backends.h"
#include "alterator_manager_polkit.h"
#include "alterator_manager_default_interfaces.h"
#include "alterator_manager_data.h"
#include "alterator_manager_sender_environment.h"
#include "alterator_manager_backend_monitor.h"

static gboolean debug = FALSE;
static gboolean user_mode = FALSE;

static GOptionEntry entries[] =
{
  { "debug", 'd', 0, G_OPTION_ARG_NONE, &debug, "Run in debug level", NULL },
  { "user", 0, 0, G_OPTION_ARG_NONE, &user_mode, "Run in user mode", NULL },
  { NULL }
};

int main(int argc, char **argv) {
    GError *error = NULL;
    GOptionContext *context;

    context = g_option_context_new ("AlteratorManager");
    g_option_context_add_main_entries (context, entries, NULL);

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_print ("option parsing failed: %s\n", error->message);
        return 3;
    }

    if (debug) {
#if GLIB_CHECK_VERSION(2,72,0)
        g_log_set_debug_enabled(TRUE);
        g_debug("alterator manager: start.");
#endif
    } else {
        g_log_set_writer_func(g_log_writer_journald, NULL, NULL);
    }

    if (!g_module_supported()) {
        g_critical("g_module isn't supported!");
        return 1;
    }

    gboolean result = alterator_manager_backends_init(user_mode);
    if (!result) {
        g_critical("Alterator backends load failed!");
        return 2;
    }

    if (!user_mode) {
        result = alterator_manager_polkit_init();
        if (!result) {
            g_critical("Alterator polkit init failed!");
            return 2;
        }
    }

    result = alterator_manager_sender_environment_init();
    if (!result) {
        g_critical("Alterator sender environment failed!");
        return 2;
    }

    alterator_manager_data_init(user_mode);

    result = alterator_manager_modules_init();
    if (!result) {
        g_critical("Alterator modules load failed!");
        return 2;
    }

    alterator_manager_default_interfaces_init(user_mode);

    alterator_manager_backend_monitor_init();

    GMainLoop* mainLoop = g_main_loop_new(NULL, FALSE);

    g_timeout_add_seconds(600,
                          alterator_manager_sender_environment_check_senders,
                          NULL);

    /* If a change occurs in one of the directories with backend files, the
       service will be stopped. The service will be restarted through
       systemd. */
    g_timeout_add_seconds(1,
                          alterator_manager_backend_monitor_backends_changed,
                          mainLoop);

    guint owner_id = alterator_manager_dbus_init(user_mode);

    g_main_loop_run(mainLoop);
    alterator_manager_sender_environment_clean();
    g_bus_unown_name(owner_id);
    g_main_loop_unref(mainLoop);
    return 0;
}
