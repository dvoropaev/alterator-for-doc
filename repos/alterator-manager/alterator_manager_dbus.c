/**/

#include "alterator_manager_dbus.h"
#include "alterator_manager_backends.h"
#include "alterator_manager_default_interfaces.h"
#include "alterator_manager_data.h"

static gchar **
subtree_enumerate (GDBusConnection       *connection,
                   const gchar           *sender,
                   const gchar           *object_path,
                   gpointer               user_data)
{
    gchar **nodes;
    GPtrArray *p;
    GHashTable *backends_data;
    GList *keys;

    p = g_ptr_array_new ();
    backends_data = alterator_manager_backends_get_data();

    keys = g_hash_table_get_keys(backends_data);

    for (GList *a = keys; a; a = a->next) {
        g_ptr_array_add (p, g_strdup ((gchar *)a->data));
    }

    g_ptr_array_add (p, NULL);
    nodes = (gchar **) g_ptr_array_free (p, FALSE);

    g_list_free(keys);

    return nodes;
}

static GDBusInterfaceInfo **
subtree_introspect (GDBusConnection       *connection,
                    const gchar           *sender,
                    const gchar           *object_path,
                    const gchar           *node,
                    gpointer               user_data)
{
    GPtrArray *p;
    GHashTable *backends_data;
    GHashTable *interfaces;
    gchar *interface;
    InterfaceObjectInfo *info;
    GHashTableIter iter;

    p = g_ptr_array_new ();

    if (node == NULL) {
        /* An introspection for the manager. */
        GList *list = alterator_manager_default_interfaces_get_intorspections();

        for (; list; list = list->next) {
            GDBusInterfaceInfo *itrospect = (GDBusInterfaceInfo *) list->data;
            g_ptr_array_add (p, g_dbus_interface_info_ref (itrospect));
        }
    } else {

        backends_data = alterator_manager_backends_get_data();
        interfaces = g_hash_table_lookup(backends_data, node);

        g_hash_table_iter_init (&iter, interfaces);
        while (g_hash_table_iter_next (&iter,
                                       (gpointer *) &interface,
                                       (gpointer *) &info))
        {
            if (info != NULL && info->interface_introspection != NULL) {

                g_ptr_array_add (p, g_dbus_interface_info_ref (
                                                info->interface_introspection));

            }/* else if (info == NULL) {
                g_debug("subtree_introspect: info == NULL (node:%s, "
                        "interface:%s).", node, interface);
            } else if (info->interface_introspection == NULL) {
                g_debug("subtree_introspect: info->interface_introspection == "
                        "NULL (node:%s, interface:%s)", node, interface);
            }*/
        }
    }
    g_ptr_array_add (p, NULL);

    return (GDBusInterfaceInfo **) g_ptr_array_free (p, FALSE);
}

static const GDBusInterfaceVTable *
subtree_dispatch (GDBusConnection             *connection,
                  const gchar                 *sender,
                  const gchar                 *object_path,
                  const gchar                 *interface_name,
                  const gchar                 *node,
                  gpointer                    *out_user_data,
                  gpointer                     user_data)
{
    GHashTable *backends_data;
    GHashTable *interfaces;
    InterfaceObjectInfo *info;

    if (node == NULL) {
        return alterator_manager_default_interfaces_get_vtable();
    } else {
        backends_data = alterator_manager_backends_get_data();
        interfaces = g_hash_table_lookup(backends_data, node);
        info = g_hash_table_lookup(interfaces, interface_name);

        if (info != NULL && info->interface_vtable != NULL) {
            return info->interface_vtable;
        } else {
            g_debug("subtree_dispatch returnd NULL. node = %s, interface = %s",
                   node, interface_name);
            return NULL;
        }
    }
}

static const GDBusSubtreeVTable subtree_vtable =
{
  subtree_enumerate,
  subtree_introspect,
  subtree_dispatch,
  { 0 }
};

static void on_bus_acquired(GDBusConnection *connection,
                            const gchar     *name,
                            gpointer         user_data)
{
    guint id;

    id = g_dbus_connection_register_subtree (connection,
                                             DBUS_OBJECT_PATH,
                                             &subtree_vtable,
                                             G_DBUS_SUBTREE_FLAGS_NONE,
                                             NULL,  /* user_data */
                                             NULL,  /* user_data_free_func */
                                             NULL); /* GError** */

    if (id == 0) {
        g_warning("g_dbus_connection_register_subtree() returned 0.");
    } else {
        ManagerData* manager_data = alterator_manager_data_get_data();
        manager_data->connection = connection;
    }
}

static void on_name_acquired(GDBusConnection *connection,
                             const gchar     *name,
                             gpointer         user_data)
{
}

static void on_name_lost(GDBusConnection *connection,
                         const gchar     *name,
                         gpointer         user_data)
{
}

guint alterator_manager_dbus_init(gboolean is_session) {
    GBusType bus_type = G_BUS_TYPE_SYSTEM;

    if (is_session) {
        bus_type = G_BUS_TYPE_SESSION;
    }

    guint owner_id = g_bus_own_name(bus_type,
                                    DBUS_OBJECT_NAME,
                                    G_BUS_NAME_OWNER_FLAGS_DO_NOT_QUEUE,
                                    on_bus_acquired,
                                    on_name_acquired,//on_name_acquired
                                    on_name_lost,//on_name_lost
                                    NULL,//gpointer user data
                                    NULL);

    return owner_id;
}
