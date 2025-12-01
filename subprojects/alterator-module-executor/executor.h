/**/

#ifndef _EXECUTOR_H
#define _EXECUTOR_H

#include <gio/gio.h>
#include <glib/gstdio.h>
#include <signal.h>
#include <errno.h>
#include <toml.h>
#include <json_tokener.h>
#include <json_object.h>

G_BEGIN_DECLS

#define INFO_EXECUTE "execute"
#define INFO_STDIN_STRING "stdin_string"
#define INFO_STDOUT_STRINGS "stdout_strings"
#define INFO_STDOUT_BYTES "stdout_bytes"
#define INFO_STDOUT_BYTE_ARRAYS "stdout_byte_arrays"
#define INFO_STDOUT_STRING_ARRAY "stdout_string_array"
#define INFO_STDOUT_JSON "stdout_json"
#define INFO_STDERR_STRINGS "stderr_strings"
#define INFO_STDOUT_SIGNAL_NAME "stdout_signal_name"
#define INFO_STDERR_SIGNAL_NAME "stderr_signal_name"
#define INFO_STDOUT_BYTE_LIMIT "stdout_byte_limit"
#define INFO_STDOUT_STRINGS_LIMIT "stdout_strings_limit"
#define INFO_STDERR_STRINGS_LIMIT "stderr_strings_limit"
#define INFO_THREAD_LIMIT "thread_limit"
#define INFO_ACTION_ID "action_id"
#define INFO_TIMEOUT "timeout"
#define INFO_EXIT_STATUS "exit_status"
#define PRM_STDIN "stdin"
#define THREAD_LIMIT 1
#define PLUGIN_NAME "executor"
#define DBUS_NAME "org.altlinux.alterator.executor"
#define OBJECT_PATH_PREFIX "/org/altlinux/alterator/"
#define MAX_NAME_LENGTH 100
#define BYTE_ARRAYS_AND_ARRAY "(aayas)"
#define BYTE_ARRAYS_AND_ARRAY_I "(aayasi)"
#define BYTE_ARRAYS_ONLY "(aay)"
#define BYTE_ARRAYS_ONLY_I "(aayi)"
#define TWO_ARRAYS "(asas)"
#define TWO_ARRAYS_I "(asasi)"
#define BYTES_AND_ARRAY "(ayas)"
#define BYTES_AND_ARRAY_I "(ayasi)"
#define ARRAY_ONLY "(as)"
#define ARRAY_ONLY_I "(asi)"
#define BYTES_ONLY "(ay)"
#define BYTES_ONLY_I "(ayi)"
#define EXIT_STATUS_ONLY "(i)"
#define EMPTY "()"
#define DEFAULT_BYTE_LIMIT 524288
#define DEFAULT_TIMEOUT 60

typedef struct {
    gchar       *method_name;
    gchar       *execute;
    gchar       *stdout_signal_name;
    gchar       *stderr_signal_name;
    gchar       *action_id;
    GHashTable  *environment;
    gchar      **data;
    gchar      **stdout_param_names;
    gboolean     stdin_string_enabled;
    gboolean     stdout_strings_enabled;
    gboolean     stdout_bytes_enabled;
    gboolean     stdout_byte_arrays_enabled;
    gboolean     stdout_string_array_enabled;
    gboolean     stdout_json_enabled;
    gboolean     stdout_signals_enabled;
    gboolean     stderr_strings_enabled;
    gboolean     stderr_signals_enabled;
    gboolean     exit_status_enabled;
    gint         stdout_strings_limit;
    gint         stdout_byte_limit;
    gint         stderr_strings_limit;
    gint         thread_limit;
    gint         thread_counter;
    gint         timeout;
} MethodInfo;

typedef struct {
    gint         thread_limit;
    gint         thread_counter;
    GHashTable   *methods;
} InterfaceInfo;

typedef struct {
    gchar                 *command_line;
    GDBusMethodInvocation *invocation;
    GDBusConnection       *connection;
    gchar                 *sender;
    gchar                **envp; //environment
    InterfaceInfo         *interface_info; //It doesn't need to be freed,
    MethodInfo            *method_info;    //it belongs to methods_table.
    const gchar           *object_path;
    const gchar           *interface_name;
    gchar                 *stdout_signal_name;
    gchar                 *stderr_signal_name;
    gchar                 *stdin_string;
    gchar                **stdout_param_names;
    gint                   stdout_strings_limit;
    gint                   stdout_byte_limit;
    gint                   stderr_strings_limit;
    gint                   stdout_strings_counter;
    gint                   stdout_byte_counter;
    gint                   stderr_strings_counter;
    /* Enable or disable a special parameter of the method (the last one in the
       parameter list). The string from this parameter is passed to stdin of the
       running process. */
    gboolean               stdin_string_enabled;
    /* Enable or disable returning out/err as a string array or byte array (only
       stdout), or line by line via signals. */
    gboolean               stdout_strings_enabled;
    gboolean               stdout_bytes_enabled;
    gboolean               stdout_byte_arrays_enabled;
    gboolean               stdout_string_array_enabled;
    gboolean               stdout_json_enabled;
    gboolean               stdout_signals_enabled;
    gboolean               stderr_strings_enabled;
    gboolean               stderr_signals_enabled;
    gboolean               exit_status_enabled;
    GList                 *stdout_strings;
    GList                 *stderr_strings;
    GString               *stdout_bytes;
    /* These fields is used to signal buffer overruns (stdout/stderr) and the
       like. */
    gint                   stdout_exit_status;
    gint                   stderr_exit_status;
    gint                   exit_status;
} ThreadFuncParam;

typedef struct {
    GMainLoop       *loop;
    gboolean         child_exited;
    gboolean         stdout_done;
    gboolean         stderr_done;
    ThreadFuncParam *thread_func_param;
    gint             stdout_pipe_out;
    gint             stderr_pipe_out;
} ChildData;

G_END_DECLS

#endif
