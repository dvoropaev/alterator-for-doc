#ifndef GDBUSCONNECTION_H
#define GDBUSCONNECTION_H

#include <gio/gio.h>
#include <glib-object.h>
#include <glib/gvariant.h>

#include "alteratorctlcommon.h"
#include "alteratorctlmoduleinfoparser.h"

#define TYPE_ALTERATOR_GDBUS_SOURCE (alterator_gdbus_source_get_type())
#define ALTERATOR_GDBUS_SOURCE(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_ALTERATOR_GDBUS_SOURCE, GDBusSource))
#define IS_ALTERATOR_GDBUS_SOURCE(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_ALTERATOR_GDBUS_SOURCE))
#define ALTERATOR_GDBUS_SOURCE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_ALTERATOR_GDBUS_SOURCE, GDBusSourceClass))
#define IS_ALTERATOR_GDBUS_SOURCE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_ALTERATOR_GDBUS_SOURCE))
#define ALTERATOR_GDBUS_SOURCE_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_ALTERATOR_GDBUS_SOURCE, GDBusSourceClass))

typedef struct
{
    GObjectClass parent_class;

} AlteratorGDBusSourceClass;

typedef struct
{
    GObject parent_instance;

    gboolean verbose;

    GDBusConnection* dbus_connection;

    GBusType bus_type;

    AlteratorCtlModuleInfoParser* info_parser;

    void (*call)(gpointer self, dbus_ctx_t* dbus_ctx, GError** dbus_call_error);

    void (*call_with_signals)(gpointer self, dbus_ctx_t* dbus_ctx, GPtrArray* signals,
                              GError** dbus_call_error);

    int (*alterator_gdbus_source_get_all_objects)(gpointer self, GHashTable** objects);

    int (*alterator_gdbus_source_get_all_ifaces)(gpointer self, GHashTable** objects);

    int (*alterator_gdbus_source_get_object_ifaces)(gpointer self, const gchar* path,
                                                    GHashTable** ifaces);

    int (*alterator_gdbus_source_get_iface_objects)(gpointer self, const gchar* iface,
                                                    GHashTable** objects);

    int (*alterator_gdbus_source_get_signals)(gpointer self, const gchar* object_str_id,
                                              const gchar* interface, const gchar* method,
                                              GHashTable** signals);

    /*!
     *  \brief checks if the object exists on the specified path
     *  \returns 0 - if object isn't exists, not null - if object exists, 2 if path contains a path
     * other than path. This may be the name of an object, no further verification of the object's
     * existence is performed
     */
    int (*alterator_gdbus_source_check_object_by_path)(gpointer self, const gchar* object_str_id,
                                                       int* result);

    int (*alterator_gdbus_source_check_object_by_iface)(gpointer self, const gchar* object_str_id,
                                                        const gchar* iface, int* result);

    int (*alterator_gdbus_source_set_env_value)(gpointer self, const gchar* name, gchar* value);

    int (*alterator_gdbus_source_get_env_value)(gpointer self, const gchar* name, gchar** result);

    int (*alterator_gdbus_source_get_text_of_alterator_entry_by_path)(gpointer self,
                                                                      const gchar* object_str_id,
                                                                      const gchar* iface,
                                                                      gchar** alterator_entry);

    int (*alterator_gdbus_source_get_services_names)(gpointer self, GHashTable** result);

    int (*alterator_gdbus_source_get_introspection)(gpointer self, const gchar* service,
                                                    const gchar* path, gchar** introspection);

    const gchar* (*alterator_gdbus_source_get_name_by_path)(gpointer self, const gchar* path,
                                                            const gchar* iface);

    const gchar* (*alterator_gdbus_source_get_path_by_name)(gpointer self, const gchar* name,
                                                            const gchar* iface);

} AlteratorGDBusSource;

typedef struct subscribe_signals_t
{
    gchar* signal_name;
    void* callback;
    gpointer user_data;
} subscribe_signals_t;

subscribe_signals_t* subscribe_signals_init(gchar* signal_name, GDBusSignalCallback callback,
                                            gpointer user_data);

void subscribe_signals_free(subscribe_signals_t* signal);

AlteratorGDBusSource* alterator_gdbus_source_new(gboolean is_verbose, GBusType type);

void alterator_gdbus_source_free(AlteratorGDBusSource* gdbus_source);

gboolean alterator_ctl_check_object_is_exist(const GError* error);

void call(gpointer self, dbus_ctx_t* dbus_ctx, GError** dbus_call_error);

void call_with_signals(gpointer self, dbus_ctx_t* dbus_ctx, GPtrArray* signals,
                       GError** dbus_call_error);

#endif // GDBUSCONNECTION_H
