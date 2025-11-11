/**/

#ifndef _BACKEND3_H
#define _BACKEND3_H

#include <alterator/alterator_manager_module_info.h>
#include <glib/gstdio.h>
#include <toml.h>
#include <ctype.h>

G_BEGIN_DECLS

#define BACKEND3_PATH "/usr/lib/alterator/backend3/"
#define OBJECT_PATH_PREFIX "/org/altlinux/alterator/"
#define DBUS_NAME "org.altlinux.alterator.backend3"
#define PLUGIN_NAME "backend3"
#define NODE_BACKEND3 "backend3"
//#define INTERFACE_BACKEND3 "org.altlinux.alterator.backend3"
#define INFO_BACKEND3 "backend3"
#define INFO_ACTION_ID "action_id"
#define INFO_THREAD_LIMIT "thread_limit"
#define INFO_TIMEOUT "timeout"
#define INFO_STDOUT_BYTE_LIMIT "stdout_byte_limit"
#define THREAD_LIMIT 1
#define DEFAULT_BYTE_LIMIT 524288
#define DEFAULT_TIMEOUT 60
#define CHILD_TIMEOUT 600
#define MESSAGE_BEGIN "_message:begin\n"
#define MESSAGE_END "_message:end\n"
#define MESSAGE_TEMPLATE "%s:%s\n"
#define KEY_OBJECTS "_objects"
#define KEY_ACTION "action"
#define KEY_LANGUAGE "language"
#define MAX_NAME_LENGTH 100
#define OUTPUT_TYPE "(aa{ss})"

typedef struct {
    gchar       *method_name;
    gchar       *backend3;
    gchar       *action_id;
    /* In the protocol it is used with underscore. Here the underscore is
       preserved to avoid confusion. */
    GHashTable  *environment;
    gint         thread_limit;
    gint         stdout_byte_limit;
    gint         thread_counter;
    gint         timeout;
} MethodInfo;

typedef struct {
    gint         thread_limit;
    gint         thread_counter;
    GHashTable   *methods;
} InterfaceInfo;

typedef struct {
    GDBusMethodInvocation *invocation; // do not free up it
    GMainContext          *context;
    GMainLoop             *loop;
    gchar                 *to_stdin;
    gchar                 *backend3;
    gchar                 *object_path;
    gchar                 *interface_name;
    gchar                 *method_name;
    InterfaceInfo         *interface_info; //It doesn't need to be freed,
    MethodInfo            *method_info;    //it belongs to methods_table.
    GString               *stdout_bytes;
    GSource               *timeout_source;
    /* Variables for output validation. */
    gboolean               stdout_done;
    gboolean               inner_brackets;
    gboolean               outer_brackets;
    gboolean               double_quotes;
    gboolean               quotes;
    /* ----------------------------------- */
    gint                   stdout_byte_limit;
} MethodData;

typedef struct {
    GMainContext    *context;
    GMainLoop       *loop;
    MethodData      *method_data;
    gchar           *backend3;
    GMutex           mutex;
    GCond            cond_data;
    gint             stdout_pipe_out;
    gint             stdin_pipe_out;
    GPid             pid;
} ChildData;

G_END_DECLS

#endif
