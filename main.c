/*  */

#include <stdio.h>

#include <polkit/polkit.h>
#define POLKIT_AGENT_I_KNOW_API_IS_SUBJECT_TO_CHANGE
#include <polkitagent/polkitagent.h>

#include "polkitagentremotelistener.h"

int main (int argc, char *argv[]) {
    GOptionContext *context;
    PolkitSubject *subject = NULL;
    gpointer local_agent_handle = NULL;
    PolkitAgentListener *listener = NULL;
    GVariant *listener_options = NULL;
    GError *error = NULL;
    GMainLoop *loop = NULL;
    gchar *opt_process = NULL;

    GOptionEntry options[] = {
        {
            "process", 'p', 0, G_OPTION_ARG_STRING, &opt_process,
            "Register the agent for the specified process",
            NULL
        },
        { NULL }
    };

    context = g_option_context_new ("Remote polkit agent");
    g_option_context_add_main_entries (context, options, NULL);

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_printerr ("Error parsing options: %s\n", error->message);
        g_error_free (error);
        return 1;
    }


    if (opt_process != NULL)
    {
        gint pid;
        guint64 pid_start_time;

        if (sscanf (opt_process, "%i,%" G_GUINT64_FORMAT, &pid, &pid_start_time)
            == 2)
        {
            G_GNUC_BEGIN_IGNORE_DEPRECATIONS
            subject = polkit_unix_process_new_full (pid, pid_start_time);
            G_GNUC_END_IGNORE_DEPRECATIONS
        } else if (sscanf (opt_process, "%i", &pid) == 1) {
            G_GNUC_BEGIN_IGNORE_DEPRECATIONS
            subject = polkit_unix_process_new (pid);
            G_GNUC_END_IGNORE_DEPRECATIONS
        } else {
            g_print("opt parse error\n");
            return 1;
        }
    }

    if (subject == NULL) {
        g_print("subject == NULL\n");
        return 1;
    }

    listener = polkit_agent_remote_listener_new (NULL, &error);
    if (listener == NULL)
    {
        g_printerr ("Error creating textual authentication agent: %s (%s, %d)\n",
                    error->message,
                    g_quark_to_string (error->domain),
                    error->code);
        g_clear_error (&error);
        goto out;
    }
    local_agent_handle = polkit_agent_listener_register (listener,
                                                         POLKIT_AGENT_REGISTER_FLAGS_RUN_IN_THREAD,
                                                         subject,
                                                         NULL, /* object_path */
                                                         NULL, /* GCancellable */
                                                         &error);
    if (local_agent_handle == NULL)
    {
        g_printerr ("Error registering local authentication agent: %s (%s, %d)\n",
                    error->message,
                    g_quark_to_string (error->domain),
                    error->code);
        g_clear_error (&error);
        goto out;
    }

    loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (loop);

    out:
        if (subject != NULL)
            g_object_unref (subject);

        if (loop != NULL)
            g_main_loop_unref (loop);

        if (local_agent_handle != NULL)
            polkit_agent_listener_unregister (local_agent_handle);

        if (listener_options != NULL)
            g_variant_unref (listener_options);

        g_free (opt_process);
        g_option_context_free (context);
}
