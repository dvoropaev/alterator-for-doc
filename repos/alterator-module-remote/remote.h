/**/

#ifndef _REMOTE_H
#define _REMOTE_H

#include <alterator/alterator_manager_module_info.h>
#include <sys/socket.h>
#include <fcntl.h>
//#include <gio/gio.h>
#include <glib/gstdio.h>
//#include <signal.h>
//#include <errno.h>
#include <ctype.h>

G_BEGIN_DECLS

#define PLUGIN_NAME "remote"
#define NODE_REMOTE "remote"
#define NODE_CONNECTION "connection"
#define DBUS_NAME "org.altlinux.alterator"
#define INTERFACE_REMOTE "org.altlinux.alterator.remote"
#define INTERFACE_INTROSPECTABLE "org.freedesktop.DBus.Introspectable"
#define INTERFACE_MANAGER "org.altlinux.alterator.manager"
#define INTERFACE_PROPERTIES "org.freedesktop.DBus.Properties"
#define INTERFACE_INTROSPECTABLE "org.freedesktop.DBus.Introspectable"
#define INTERFACE_PEER "org.freedesktop.DBus.Peer"
#define INTERFACE_PASSWORD_AGENT "org.altlinux.PasswordAgent"
#define OBJECT_ALTERATOR "/org/altlinux/alterator"
#define PART_OF_OBJECT_FOR_CONNECTION "/connection/"
#define OBJECT_PASSWORD_AGENT "/org/altlinux/PasswordAgent"
#define METHOD_INTROSPECT "Introspect"
#define METHOD_GETALLOBJECTS "GetAllObjects"
#define METHOD_SHOWRESULT "ShowResult"
#define METHOD_SELECTUSER "SelectUser"
#define POLKIT_AGENT_NAME "remote-polkit-agent"
#define DBUS_BRIDGE "systemd-stdio-bridge"
#define ROOT_NODE "-"
#define JSON_SELECTED_USER "{ \"selected user\" : \"%s\"}\n"
#define JSON_KEY_ACTION_ID "\"action_id\""
#define JSON_KEY_MESSAGE "\"message\""
#define JSON_KEY_USERS "\"users\""
#define JSON_KEY_REQUEST "\"request\""
#define JSON_KEY_RESULT "\"result\""
#define JSON_KEY_ERROR "\"error\""
#define JSON_KEY_INFO "\"info\""
#define JSON_KEY_INVALID_RESPONSE "\"invalid response\""
#define JSON_VALUE_PASSWORD "password"
#define JSON_RESPONSE "{ \"response\" : \"%s\"}\n"

typedef struct {
    GVariant *parameters;
    GDBusMethodInvocation *invocation;
} ThreadFuncParam;

typedef struct {
    /* node_name -> GDBusNodeInfo* */
    GHashTable *nodes_info;
    GMainLoop *loop;
    GPid pid_pkagent;
} SubtreeInfo;

typedef struct {
    GDBusMethodInvocation *invocation;
    gchar *object_path;
    gchar *agent_bus_name;
    gchar *connection_name;
    gchar *pty;
    gchar *password;
    GDBusConnection *connection;
    gboolean pkagent_stdout_done;
    gint pkstdin;
} HandlersData;

G_END_DECLS

#endif
