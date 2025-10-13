#include "alteratorctlcommon.h"

#include <errno.h>
#include <fcntl.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>

static int stdout_backup, stderr_backup;

#define TEXT_COLOR_DEFAULT "\033[0m"
#define TEXT_COLOR_RED "\033[31m"
#define TEXT_COLOR_GREEN "\033[32m"
#define TEXT_COLOR_YELLOW "\033[33m"

#define SELF_PROCESS_STDIN_LINK_PATH "/proc/self/fd/0"

static void alterator_ctl_print_html_node(GumboNode *node, gboolean is_newline);

dbus_ctx_t *dbus_ctx_init(
    const gchar *service, const gchar *path, const gchar *interface, const gchar *method, gboolean verbose)
{
    int ret = 0;

    dbus_ctx_t *ctx = g_malloc0(sizeof(dbus_ctx_t));

    if (!ctx)
        return NULL;

    ctx->verbose = verbose;

    // Default
    ctx->timeout_msec = -1;

    if (service && !(ctx->service_name = g_strdup(service)))
    {
        g_printerr(_("D-Bus context init error: unvaliable service name.\n"));
        ERR_EXIT();
    }

    if (path && !(ctx->path = g_strdup(path)))
    {
        g_printerr(_("D-Bus context init error: unvaliable path to object.\n"));
        ERR_EXIT();
    }

    if (interface && !(ctx->interface = g_strdup(interface)))
    {
        g_printerr(_("D-Bus context init error: unvaliable interface name.\n"));
        ERR_EXIT();
    }

    if (method && !(ctx->method = g_strdup(method)))
    {
        g_printerr(_("D-Bus context init error: unvaliable method name.\n"));
        ERR_EXIT();
    }

    return ctx;

end:
    if (ctx)
        g_free(ctx);

    return NULL;
}

int dbus_ctx_set_timeout(dbus_ctx_t *ctx, gint milliseconds)
{
    int ret = 0;
    if (milliseconds < -1)
    {
        g_printerr(_("Unvalid D-Bus connection timeout value.\n"));
        ERR_EXIT();
    }
    ctx->timeout_msec = milliseconds;

end:
    return ret;
}

void dbus_ctx_free(dbus_ctx_t *ctx)
{
    if (!ctx)
        return;

    g_free(ctx->service_name);
    g_free(ctx->path);
    g_free(ctx->interface);
    g_free(ctx->method);

    if (ctx->result)
        g_variant_unref(ctx->result);

    g_free(ctx);
}

alteratorctl_ctx_t *alteratorctl_context_init(GVariant *subcommands_ids,
                                              GVariant *parameters,
                                              void (*free_results)(gpointer results),
                                              void *additional_data)
{
    alteratorctl_ctx_t *ctx = g_malloc0(sizeof(alteratorctl_ctx_t));
    if (!ctx)
        return NULL;

    ctx->subcommands_ids = subcommands_ids;
    ctx->parameters      = parameters;
    ctx->free_results    = free_results;
    ctx->additional_data = additional_data;

    return ctx;
}

alteratorctl_ctx_t *alteratorctl_ctx_init_manager(int subcommand_id,
                                                  const gchar *param1,
                                                  const gchar *param2,
                                                  const gchar *param3,
                                                  const gchar *param4,
                                                  void (*free_results)(gpointer results),
                                                  void *additional_data)
{
    GVariant *ids        = g_variant_new("i", subcommand_id);
    GVariant *parameters = g_variant_new("(msmsmsms)", param1, param2, param3, param4);

    alteratorctl_ctx_t *ctx = alteratorctl_context_init(ids, parameters, free_results, additional_data);
    if (!ctx)
    {
        g_printerr(_("Error of creating alteratorctl_ctx_t to manager module.\n"));
        return NULL;
    }

    return ctx;
}

alteratorctl_ctx_t *alteratorctl_ctx_init_packages(gint submodule_id,
                                                   gint subcommand_id,
                                                   const gchar *param1,
                                                   void (*free_results)(gpointer results),
                                                   void *additional_data)
{
    GVariant *ids        = g_variant_new("(ii)", submodule_id, subcommand_id);
    GVariant *parameters = g_variant_new("(ms)", param1);

    alteratorctl_ctx_t *ctx = alteratorctl_context_init(ids, parameters, free_results, additional_data);
    if (!ctx)
    {
        g_printerr(_("Error of creating alteratorctl_ctx_t to packages module.\n"));
        return NULL;
    }

    return ctx;
}

alteratorctl_ctx_t *alteratorctl_ctx_init_editions(gint subcommand_id,
                                                   const gchar *param1,
                                                   void (*free_results)(gpointer results),
                                                   void *additional_data)
{
    GVariant *ids        = g_variant_new("i", subcommand_id);
    GVariant *parameters = g_variant_new("(ms)", param1);

    alteratorctl_ctx_t *ctx = alteratorctl_context_init(ids, parameters, free_results, additional_data);
    if (!ctx)
    {
        g_printerr(_("Error of creating alteratorctl_ctx_t to editions module.\n"));
        return NULL;
    }

    return ctx;
}

alteratorctl_ctx_t *alteratorctl_ctx_init_components(gint subcommand_id,
                                                     const gchar *param1,
                                                     void (*free_results)(gpointer results),
                                                     void *additional_data)
{
    GVariant *ids        = g_variant_new("i", subcommand_id);
    GVariant *parameters = g_variant_new("(ms)", param1);

    alteratorctl_ctx_t *ctx = alteratorctl_context_init(ids, parameters, free_results, additional_data);
    if (!ctx)
    {
        g_printerr(_("Error of creating alteratorctl_ctx_t to components module.\n"));
        return NULL;
    }

    return ctx;
}

alteratorctl_ctx_t *alteratorctl_ctx_init_diag(gint subcommand_id,
                                               const gchar *param1,
                                               const gchar *param2,
                                               const gchar *param3,
                                               void (*free_results)(gpointer results),
                                               void *additional_data)
{
    GVariant *ids        = g_variant_new("i", subcommand_id);
    GVariant *parameters = g_variant_new("(msmsms)", param1, param2, param3);

    alteratorctl_ctx_t *ctx = alteratorctl_context_init(ids, parameters, free_results, additional_data);
    if (!ctx)
    {
        g_printerr(_("Error of creating alteratorctl_ctx_t to diag module.\n"));
        return NULL;
    }

    return ctx;
}

alteratorctl_ctx_t *alteratorctl_ctx_init_systeminfo(gint subcommand_id,
                                                     void (*free_results)(gpointer results),
                                                     void *additional_data)
{
    GVariant *ids           = g_variant_new("i", subcommand_id);
    alteratorctl_ctx_t *ctx = alteratorctl_context_init(ids, NULL, free_results, additional_data);
    if (!ctx)
    {
        g_printerr(_("Error of creating alteratorctl_ctx_t to systeminfo module.\n"));
        return NULL;
    }

    return ctx;
}

alteratorctl_ctx_t *alteratorctl_ctx_init_remote(gint subcommand_id,
                                                 const gchar *param1,
                                                 const gchar *param2,
                                                 const gchar *param3,
                                                 const gchar *param4,
                                                 void (*free_results)(gpointer results),
                                                 void *additional_data)
{
    GVariant *ids        = g_variant_new("i", subcommand_id);
    GVariant *parameters = g_variant_new("(msmsmsms)", param1, param2, param3, param4);

    alteratorctl_ctx_t *ctx = alteratorctl_context_init(ids, parameters, free_results, additional_data);
    if (!ctx)
    {
        g_printerr(_("Error of creating alteratorctl_ctx_t to remote module.\n"));
        return NULL;
    }

    return ctx;
}

void alteratorctl_ctx_free(alteratorctl_ctx_t *ctx)
{
    if (ctx->subcommands_ids)
        g_variant_unref(ctx->subcommands_ids);

    if (ctx->parameters)
        g_variant_unref(ctx->parameters);

    if (ctx->results && ctx->free_results)
        ctx->free_results(ctx->results);

    g_free(ctx->additional_data);
    g_free(ctx);
}

gboolean alterator_ctl_is_utf8_locale()
{
    const char *charset = NULL;
    return g_get_charset(&charset);
}

gchar *alterator_ctl_get_effective_locale()
{
    if (!alterator_ctl_is_utf8_locale())
        return g_strdup("C");
    return alterator_ctl_get_locale();
}

gchar *alterator_ctl_get_effective_language()
{
    if (!alterator_ctl_is_utf8_locale())
        return g_strdup("en");
    return alterator_ctl_get_language();
}

void print_hash_table(GHashTable *table, gboolean with_values)
{
    if (!table)
        goto end;

    GHashTableIter iter;
    gpointer key = NULL, value = NULL;

    g_hash_table_iter_init(&iter, table);
    while (g_hash_table_iter_next(&iter, &key, &value))
        if (!with_values)
            g_print("%s\n", (gchar *) key);
        else
            g_print("%s: %s\n", (gchar *) key, (gchar *) value);

end:
    return;
}

static int redirecting_stream(uint32_t target_id, uint32_t stream_id)
{
    int ret             = 0;
    gchar *fd_proc_path = NULL;
    gchar *target_str   = g_strdup_printf("%u", target_id);
    char path[512];

    if (dup2(target_id, stream_id) == -1)
    {
        fd_proc_path = g_strconcat("/proc/self/fd/", target_str, NULL);
        ssize_t len  = readlink(fd_proc_path, path, sizeof(path) - 1);
        if (len != -1)
        {
            path[len] = '\0';
            g_printerr(_("In dup2 to redirect the thread from id %u to \"%s\" failed. Return code: -1\n"),
                       stream_id,
                       path);
            ERR_EXIT();
        }
        else
        {
            g_printerr(_("In dup2 to redirect the thread from id %u to file descriptor %u. Return code: -1\n"),
                       stream_id,
                       target_id);
            ERR_EXIT();
        }
    }

end:
    g_free(fd_proc_path);

    g_free(target_str);

    return ret;
}

int disable_output()
{
    int ret = 0;
    int devnull;

    if ((stdout_backup = dup(STDOUT_FILENO)) == -1)
    {
        g_printerr(_("Can't make STDOUT descriptor backup\n"));
        ERR_EXIT();
    }

    if ((stderr_backup = dup(STDERR_FILENO)) == -1)
    {
        g_printerr(_("Can't make STDERR descriptor backup\n"));
        ERR_EXIT();
    }

    if ((devnull = open("/dev/null", O_WRONLY)) == -1)
    {
        g_printerr(_("Error while open \"/dev/null\" file descriptor: %s\n"), strerror(errno));
        ERR_EXIT();
    }

    // Redirecting STDOUT and STDERR to /dev/null;
    if (redirecting_stream(devnull, STDOUT_FILENO) < 0)
        ERR_EXIT();

    if (redirecting_stream(devnull, STDERR_FILENO) < 0)
    {
        redirecting_stream(stdout_backup, STDOUT_FILENO);
        ERR_EXIT();
    }

end:
    return ret;
}

int enable_output()
{
    int ret = 0;
    if (redirecting_stream(stdout_backup, STDOUT_FILENO) < 0)
        ERR_EXIT();

    if (redirecting_stream(stderr_backup, STDERR_FILENO) < 0)
        ERR_EXIT();

end:
    return ret;
}

gchar *colorize_text(const gchar *text, text_color color)
{
    gchar *result = NULL;

    switch (color)
    {
    case DEFAULT:
        result = g_strdup(text);
        break;

    case RED:
        result = g_strconcat(TEXT_COLOR_RED, text, TEXT_COLOR_DEFAULT, NULL);
        break;

    case GREEN:
        result = g_strconcat(TEXT_COLOR_GREEN, text, TEXT_COLOR_DEFAULT, NULL);
        break;

    case YELLOW:
        result = g_strconcat(TEXT_COLOR_YELLOW, text, TEXT_COLOR_DEFAULT, NULL);
        break;
    };

    return result;
}

gchar *call_bash_command(const gchar *cmd, GError **error)
{
    gchar *output = NULL;

    if (!g_spawn_command_line_sync(cmd, &output, NULL, NULL, error))
        return NULL;

    return output;
}

gchar *alterator_ctl_get_locale()
{
    gchar *locale           = NULL;
    gchar *full_locale_name = getenv("LC_ALL");

    if (!full_locale_name || (full_locale_name && !strlen(full_locale_name)))
        full_locale_name = getenv("LANG");

    if (!full_locale_name || (full_locale_name && !strlen(full_locale_name)))
        full_locale_name = getenv("LC_MESSAGES");

    if (!full_locale_name || (full_locale_name && !strlen(full_locale_name)) || 0 == strcmp(full_locale_name, "C")
        || 0 == strcmp(full_locale_name, "C.utf-8") || 0 == strcmp(full_locale_name, "C.UTF-8")
        || 0 == strcmp(full_locale_name, "POSIX"))
    {
        //g_printerr(_("System locale data is empty.\n"));
        locale = g_strdup(LOCALE_FALLBACK);
    }
    else
        locale = full_locale_name;

    return locale;
}

gchar *alterator_ctl_get_language()
{
    gchar *locale   = alterator_ctl_get_locale();
    gchar *language = g_utf8_substring(locale, 0, 2);
    return language;
}

int alterator_ctl_print_html(const gchar *html)
{
    int ret             = 0;
    GumboOutput *output = gumbo_parse(html);
    if (!output)
    {
        g_printerr(_("Failed to parse html text.\n"));
        ERR_EXIT();
    }
    alterator_ctl_print_html_node(output->root, true);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
end:
    return ret;
}

static void alterator_ctl_print_html_node(GumboNode *node, gboolean is_newline)
{
    if (node->type == GUMBO_NODE_TEXT)
    {
        g_print("%s", node->v.text.text);
        if (is_newline)
            g_print("\n");
    }
    else if (node->type == GUMBO_NODE_ELEMENT)
    {
        if (node->v.element.tag == GUMBO_TAG_TITLE)
            return;

        gboolean add_newline  = FALSE;
        gboolean is_h1        = (node->v.element.tag == GUMBO_TAG_H1);
        gboolean is_h2        = (node->v.element.tag == GUMBO_TAG_H2);
        gboolean is_bold      = (node->v.element.tag == GUMBO_TAG_B);
        gboolean is_italic    = (node->v.element.tag == GUMBO_TAG_EM);
        gboolean is_paragraph = (node->v.element.tag == GUMBO_TAG_P);
        gboolean is_list_item = (node->v.element.tag == GUMBO_TAG_LI);

        if (is_h1 || is_h2 || is_paragraph || is_list_item)
            add_newline = TRUE;

        if (is_h1)
            g_print("\033[1m"); // Bold
        if (is_h2)
            g_print("\033[4m"); // Blue color
        if (is_bold)
            g_print("\033[1m"); // Bold
        if (is_italic)
            g_print("\033[3m"); // Italic
        if (is_list_item)
            g_print("    * ");

        for (size_t i = 0; i < node->v.element.children.length; ++i)
        {
            GumboNode *child = node->v.element.children.data[i];
            alterator_ctl_print_html_node(child, add_newline);
        }

        if (is_h1 || is_h2 || is_bold || is_italic)
            g_print("\033[0m"); // Reset
        if (add_newline)
            g_print("\n");
    }
}

gboolean alterator_ctl_is_root()
{
    return getuid() == 0;
}

is_tty_status isatty_safe(guint fd)
{
    int int_fd = (int) fd;
    if (int_fd < 0)
        return INCORRECT_FILE_DESCRIPTOR;

    if (isatty((int) fd))
        return TTY;

    if (errno == EIO)
        return TTY;

    return NOT_TTY;
}

int print_with_pager(const gchar *text)
{
    if (!text)
        return 0;

    /* Step 1: Detect interactive shell */
    if (!isatty(STDOUT_FILENO))
    {
        /* Not a TTY, print directly */
        fputs(text, stdout);
        return 0;
    }

    /* Step 2: Get terminal height */
    struct winsize w;
    int term_lines = 24; // fallback
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_row > 0)
        term_lines = w.ws_row;

    /* Step 3: Count lines in text */
    int line_count = 0;
    for (const char *p = text; *p; ++p)
        if (*p == '\n')
            ++line_count;
    // If text does not end with newline, count last line
    if (line_count == 0 || (text[strlen(text) - 1] != '\n' && strlen(text) > 0))
        ++line_count;

    /* Step 4: Decide whether to use pager */
    if (line_count < term_lines)
    {
        fputs(text, stdout);
        return 0;
    }

    /* Step 5: Buffer output to a temporary file */
    char tmpname[] = "/tmp/alteratorctl_pager_XXXXXX";
    int fd         = mkstemp(tmpname);
    if (fd == -1)
    {
        /* Fallback: print directly */
        fputs(text, stdout);
        return 0;
    }
    FILE *tmpf = fdopen(fd, "w+");
    if (!tmpf)
    {
        close(fd);
        unlink(tmpname);
        fputs(text, stdout);
        return 0;
    }
    fputs(text, tmpf);
    fflush(tmpf);
    rewind(tmpf);

    /* Step 6: Pager invocation */
    const char *pager = getenv("PAGER");
    if (!pager || !*pager)
        pager = "less";

    pid_t pid = fork();
    if (pid == 0)
    {
        /* Child: exec pager */
        dup2(fileno(tmpf), STDIN_FILENO);
        fclose(tmpf);
        execlp(pager, pager, NULL);
        /* If exec fails, fallback */
        _exit(127);
    }
    else if (pid > 0)
    {
        /* Parent: wait for pager */
        fclose(tmpf);
        waitpid(pid, NULL, 0);
    }
    else
    {
        /* Fork failed, fallback */
        fclose(tmpf);
        fputs(text, stdout);
    }
    unlink(tmpname);
    return 0;
}

gchar *columnize_text(gchar **text)
{
    if (!text || !g_strv_length(text))
        return NULL;

    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) < 0)
    {
        g_printerr(_("Unable to format output into columns: %s\n"), strerror(errno));
        return NULL;
    }

    gsize strings_amount = g_strv_length((gchar **) text);
    gint terminal_width  = w.ws_col;

    gsize max_columns = strings_amount;
    gsize columns = 1, rows = strings_amount;

    for (gsize column = max_columns; column > 0; column--)
    {
        rows              = (strings_amount + column - 1) / column;
        gsize *col_widths = g_new0(gsize, column);
        for (gsize i = 0; i < strings_amount; i++)
        {
            gsize column_idx       = i / rows;
            col_widths[column_idx] = MAX(col_widths[column_idx], strlen(text[i]));
        }

        gsize total_width = 0;
        for (gsize i = 0; i < column; i++)
            total_width += col_widths[i] + (i < column - 1 ? 2 : 0); // 2 space symbols between columns

        g_free(col_widths);

        if (total_width <= (gsize) terminal_width)
        {
            columns = column;
            break;
        }
    }

    rows = (strings_amount + columns - 1) / columns;

    gsize *col_widths = g_new0(gsize, columns);
    for (gsize i = 0; i < strings_amount; i++)
    {
        gsize column_idx       = i / rows;
        col_widths[column_idx] = MAX(col_widths[column_idx], strlen(text[i]));
    }

    GString *result_builder = g_string_new(NULL);
    for (gsize row_idx = 0; row_idx < rows; row_idx++)
    {
        for (gsize column_idx = 0; column_idx < columns; column_idx++)
        {
            gsize idx = column_idx * rows + row_idx;
            if (idx >= strings_amount)
                continue;

            if (column_idx == 0)
                g_string_append(result_builder, "  ");
            g_string_append_printf(result_builder, "%-*s", (int) col_widths[column_idx], text[idx]);

            if (column_idx < columns - 1)
                g_string_append(result_builder, "  ");
        }
        g_string_append(result_builder, "\n");
    }

    gchar *result = g_strdup(result_builder->str);
    g_string_free(result_builder, TRUE);
    g_free(col_widths);

    return result;
}
