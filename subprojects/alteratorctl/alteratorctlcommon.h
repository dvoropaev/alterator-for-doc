#ifndef ALTERATORCTL_COMMON_H
#define ALTERATORCTL_COMMON_H

#include <glib.h>
#include <glib/gi18n.h>
#include <gumbo.h>
#include <toml.h>

// TO DO add logging in ERR_EXIT
#define ERR_EXIT_MESSAGE(format, ...) \
    do \
    { \
        ret = -1; \
        g_printerr(format); \
        goto end; \
    } while (0)

#define ERR_EXIT() \
    do \
    { \
        ret = -1; \
        goto end; \
    } while (0)

typedef enum
{
    DEFAULT,
    RED,
    GREEN,
    YELLOW
} text_color;

typedef enum is_tty_status
{
    NOT_TTY = 0,
    TTY     = 1,
    INCORRECT_FILE_DESCRIPTOR
} is_tty_status;

#define DIAG_SUBCOMMAND_MAX_LENGTH 50
#define DIAG_PATH_MAX_LENGTH 255
#define DIAG_TEST_MAX_LENGTH 255
#define DIAG_RUN_WAIT_EVENTS_INTERVAL 1 * 1000
#define DIAG_RUN_MAIN_LOOP_BREAK_INTERVAL 10

#define MANAGER_SUBCOMMAND_MAX_LENGTH 50
#define MANAGER_PATH_MAX_LENGTH 255
#define MANAGER_PARAMETER_MAX_LENGTH 255

#define ALTERATOR_CTL_MODULE_NAME_MAX_LENGTH 255

#define ALTERATOR_CTL_MANAGER_MODULE_NAME "manager"
#define ALTERATOR_CTL_PACKAGES_MODULE_NAME "packages"
#define ALTERATOR_CTL_COMPONENTS_MODULE_NAME "components"
#define ALTERATOR_CTL_EDITIONS_MODULE_NAME "editions"
#define ALTERATOR_CTL_DIAG_MODULE_NAME "diag"
#define ALTERATOR_CTL_SYSTEMINFO_MODULE_NAME "systeminfo"
#define ALTERATOR_CTL_REMOTE_MODULE_NAME "remote"

#define ALTERATOR_SERVICE_NAME "org.altlinux.alterator"
#define ALTERATOR_MANAGER_PATH "/org/altlinux/alterator"
#define ALTERATOR_GLOBAL_PATH "/org/altlinux/alterator/global"
#define ALTERATOR_SYSTEMINFO_PATH "/org/altlinux/alterator/systeminfo"
#define ALTERATOR_REMOTE_PATH "/org/altlinux/alterator/remote"

#define ALTERATOR_ENTRY_GET_METHOD "Info"

#define TOML_ERROR_BUFFER_SIZE 512

#define LOCALE_FALLBACK "en_US.UTF-8"

enum alteratorctl_commands
{
    ALTERATORCTL_GET_VERSION  = 1,
    ALTERATORCTL_LIST_MODULES = 2,
    ALTERATORCTL_RUN_MODULE   = 3
};

enum alteratorctl_help_type
{
    ALTERATORCTL_NONE_HELP,
    ALTERATORCTL_MODULE_HELP,
    ALTERATORCTL_SUBMODULE_HELP
};

typedef struct alteratorctl_arguments_t
{
    enum alteratorctl_commands command;
    enum alteratorctl_help_type help_type;
    gboolean module_help;
    gboolean verbose;
    gchar *module;
} alteratorctl_arguments_t;

typedef struct alteratorctl_ctx_t
{
    GVariant *subcommands_ids;
    GVariant *parameters;
    gpointer results;
    void (*free_results)(gpointer results);
    gpointer additional_data;
} alteratorctl_ctx_t;

typedef struct alterator_ctl_module_t
{
    gchar id[ALTERATOR_CTL_MODULE_NAME_MAX_LENGTH];
    gpointer *(*new_object_func)(gpointer);
    void (*free_object_func)(gpointer);
} alterator_ctl_module_t;

typedef struct dbus_ctx_t
{
    gchar *service_name;
    gchar *path;
    gchar *interface;
    gchar *method;
    GVariant *parameters;
    const GVariantType *reply_type;
    GVariant *result;
    gint timeout_msec;
    gboolean verbose;
    gboolean disable_output;
} dbus_ctx_t;

dbus_ctx_t *dbus_ctx_init(
    const gchar *service, const gchar *path, const gchar *interface, const gchar *method, gboolean verbose);

int dbus_ctx_set_timeout(dbus_ctx_t *ctx, gint milliseconds);

void dbus_ctx_free(dbus_ctx_t *ctx);

alteratorctl_ctx_t *alteratorctl_context_init(GVariant *subcommands_ids,
                                              GVariant *parameters,
                                              void (*free_results)(gpointer results),
                                              void *additional_data);

alteratorctl_ctx_t *alteratorctl_ctx_init_manager(gint subcommand_id,
                                                  const gchar *param1,
                                                  const gchar *param2,
                                                  const gchar *param3,
                                                  const gchar *param4,
                                                  void (*free_results)(gpointer results),
                                                  void *additional_data);

alteratorctl_ctx_t *alteratorctl_ctx_init_packages(gint submodule_id,
                                                   gint subcommand_id,
                                                   const gchar *param1,
                                                   void (*free_results)(gpointer results),
                                                   void *additional_data);

alteratorctl_ctx_t *alteratorctl_ctx_init_editions(gint subcommand_id,
                                                   const gchar *param1,
                                                   void (*free_results)(gpointer results),
                                                   void *additional_data);

alteratorctl_ctx_t *alteratorctl_ctx_init_components(gint subcommand_id,
                                                     const gchar *param1,
                                                     void (*free_results)(gpointer results),
                                                     void *additional_data);

alteratorctl_ctx_t *alteratorctl_ctx_init_diag(gint subcommand_id,
                                               const gchar *param1,
                                               const gchar *param2,
                                               const gchar *param3,
                                               void (*free_results)(gpointer results),
                                               void *additional_data);

alteratorctl_ctx_t *alteratorctl_ctx_init_systeminfo(gint subcommand_id,
                                                     void (*free_results)(gpointer results),
                                                     void *additional_data);

alteratorctl_ctx_t *alteratorctl_ctx_init_remote(gint subcommand_id,
                                                 const gchar *param1,
                                                 const gchar *param2,
                                                 const gchar *param3,
                                                 const gchar *param4,
                                                 void (*free_results)(gpointer results),
                                                 void *additional_data);

void alteratorctl_ctx_free(alteratorctl_ctx_t *ctx);

int alterator_ctl_print_html(const gchar *html);

gchar *alterator_ctl_get_locale();

gchar *alterator_ctl_get_language();

// Returns TRUE if current locale encoding is UTF-8 according to GLib
gboolean alterator_ctl_is_utf8_locale();

// Effective helpers for ASCII-safe mode
// If current locale is not UTF-8, these functions return English/"C" values.
// Callers must g_free() the returned strings.
gchar *alterator_ctl_get_effective_locale();

gchar *alterator_ctl_get_effective_language();

void print_hash_table(GHashTable *table, gboolean with_values);

int disable_output();

int enable_output();

gchar *colorize_text(const gchar *text, text_color color);

gchar *call_bash_command(const gchar *cmd, GError **error);

gboolean alterator_ctl_is_root();

is_tty_status isatty_safe(guint fd);

gchar *columnize_text(gchar **text);

int print_with_pager(const gchar *text);

#endif // ALTERATORCTL_COMMON_H
