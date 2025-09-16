/**/

#include "alterator_manager_sender_environment.h"

static GHashTable *sender_environment_data = NULL;
static guint watcher_id = 0;
static gboolean is_interface_available = FALSE;
static GDBusConnection *current_connection = NULL;

static void environment_table_free(void *environment_table) {
    GHashTable *table = (GHashTable *)environment_table;

    if (table == NULL) {
        return;
    }

    g_hash_table_unref(table);
}

void alterator_manager_sender_environment_clean(void) {
    if (watcher_id) {
        g_bus_unwatch_name(watcher_id);
    }
}

static void remove_absent_senders (GObject *source,
                                   GAsyncResult *res,
                                   gpointer user_data)
{
    GDBusConnection *connection = G_DBUS_CONNECTION (source);
    GError *error = NULL;
    GVariant *result;
    GVariantIter *iter;
    GList *sender_names = NULL;
    GList *senders_to_be_deleted = NULL;
    gchar *sender_name_table;
    gchar *str;
    GHashTableIter table_iter;

    result = g_dbus_connection_call_finish (connection, res, &error);
    if (error) {
        g_warning("Senders environment callback error: %s.", error->message);
        g_error_free(error);

        return;
    }

    if (sender_environment_data) {
        /* Get a list of all senders. */
        /* Do not free str, it belongs to g_variant. */
        g_variant_get (result, "(as)", &iter);
        while (g_variant_iter_loop (iter, "s", &str)) {
            sender_names = g_list_prepend(sender_names, strdup(str));
        }
        g_variant_iter_free (iter);

        /* Get a list of senders that need to be deleted. */
        g_hash_table_iter_init (&table_iter, sender_environment_data);
        while (g_hash_table_iter_next (&table_iter,
                                       (gpointer *) &sender_name_table,
                                       NULL))
        {
            GList *name = g_list_find_custom(sender_names,
                                             sender_name_table,
                                             (GCompareFunc) g_strcmp0);
            if (name == NULL) {
                senders_to_be_deleted =
                             g_list_prepend(senders_to_be_deleted,
                                            strdup((gchar*) sender_name_table));
            }
        }

        /* Remove senders. */
        for (GList *a = senders_to_be_deleted; a; a = a->next) {
            g_hash_table_remove(sender_environment_data, (gchar*) a->data);
        }
    }

    if (sender_names) {
        g_list_free_full (g_steal_pointer (&sender_names), g_free);
    }
    if (senders_to_be_deleted) {
        g_list_free_full (g_steal_pointer (&senders_to_be_deleted), g_free);
    }
    g_variant_unref (result);
}


/* This function is called on a timer. */
gboolean alterator_manager_sender_environment_check_senders(gpointer data) {
    if (is_interface_available == FALSE) {
        return TRUE;
    }

    g_dbus_connection_call (current_connection,
                            FREEDESKTOP_DBUS_BUS_NAME,
                            FREEDESKTOP_DBUS_OBJECT_PATH,
                            FREEDESKTOP_DBUS_INTERFACE_NAME,
                            FREEDESKTOP_DBUS_METHOD_NAME,
                            NULL,
                            G_VARIANT_TYPE("(as)"),
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            remove_absent_senders,
                            NULL);

    return TRUE;
}

static void on_name_appeared (GDBusConnection *connection,
                              const gchar     *name,
                              const gchar     *name_owner,
                              gpointer         user_data)
{
    is_interface_available = TRUE;
    current_connection = connection;
    g_debug("%s is available. Senders verification is possible.",
            FREEDESKTOP_DBUS_BUS_NAME);
}

static void on_name_vanished (GDBusConnection *connection,
                              const gchar     *name,
                              gpointer         user_data)
{
    is_interface_available = FALSE;
    current_connection = NULL;
    g_warning("%s is not available. Senders verification is not possible.",
              FREEDESKTOP_DBUS_BUS_NAME);
}

/* The senders are checked using a timer. Senders that are not included in the
   response will be removed from the table. */
static void establish_connection_to_check_names(void) {
    watcher_id = g_bus_watch_name (G_BUS_TYPE_SYSTEM,
                                   FREEDESKTOP_DBUS_BUS_NAME,
                                   G_BUS_NAME_WATCHER_FLAGS_NONE,
                                   on_name_appeared,
                                   on_name_vanished,
                                   NULL,
                                   NULL);
}

gboolean alterator_manager_sender_environment_init(void) {
    sender_environment_data = g_hash_table_new_full(g_str_hash,
                                                    g_str_equal,
                                                    g_free,
                                                    environment_table_free);
    if (sender_environment_data == NULL) {
        return FALSE;
    }

    establish_connection_to_check_names();

    return TRUE;
}

GHashTable* alterator_manager_sender_environment_get_table(const gchar* sender) {
    GHashTable* table = g_hash_table_lookup(sender_environment_data, sender);

    if (table == NULL) {
        table = g_hash_table_new_full(g_str_hash,
                                      g_str_equal,
                                      g_free,
                                      g_free);

        if (table == NULL) {
            return NULL;
        }

        g_hash_table_replace(sender_environment_data,
                             g_strdup(sender), table);
    }

    return g_hash_table_ref(table);
}

GHashTable* alterator_manager_sender_environment_get_data(void) {
    return sender_environment_data;
}

gboolean is_valid_environment_name(const gchar* name) {
    if (name == NULL) {
        return FALSE;
    }

    if (g_utf8_strchr(name, -1, '=') != NULL) {
        return FALSE;
    }

    return TRUE;
}
