/*  */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>

#include <termios.h>
#include <unistd.h>

#include <polkitagent/polkitagentlistener.h>
#include "polkitagentremotelistener.h"
#include <polkitagent/polkitagentsession.h>

/**
 * PolkitAgentRemoteListener:
 *
 * The #PolkitAgentRemoteListener struct should not be accessed directly.
 */
struct _PolkitAgentRemoteListener
{
  PolkitAgentListener parent_instance;

  GSimpleAsyncResult *simple;
  PolkitAgentSession *active_session;
  gulong cancel_id;
  GCancellable *cancellable;

  FILE *in;
  FILE *out;
};

typedef struct _PolkitAgentRemoteListener PolkitAgentRemoteListener;

typedef struct
{
  PolkitAgentListenerClass parent_class;
} PolkitAgentRemoteListenerClass;

static void polkit_agent_remote_listener_initiate_authentication (PolkitAgentListener  *_listener,
                                                                  const gchar          *action_id,
                                                                  const gchar          *message,
                                                                  const gchar          *icon_name,
                                                                  PolkitDetails        *details,
                                                                  const gchar          *cookie,
                                                                  GList                *identities,
                                                                  GCancellable         *cancellable,
                                                                  GAsyncReadyCallback   callback,
                                                                  gpointer              user_data);

static gboolean polkit_agent_remote_listener_initiate_authentication_finish (PolkitAgentListener  *_listener,
                                                                             GAsyncResult         *res,
                                                                             GError              **error);

static void initable_iface_init (GInitableIface *initable_iface);

G_DEFINE_TYPE_WITH_CODE (PolkitAgentRemoteListener, polkit_agent_remote_listener, POLKIT_AGENT_TYPE_LISTENER,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, initable_iface_init));

static void
polkit_agent_remote_listener_init (PolkitAgentRemoteListener *listener)
{
}

static void
polkit_agent_remote_listener_finalize (GObject *object)
{
  PolkitAgentRemoteListener *listener = POLKIT_AGENT_REMOTE_LISTENER (object);

  if (listener->in != NULL)
    fclose (listener->in);

  if (listener->out != NULL)
    fclose (listener->out);

  if (listener->active_session != NULL)
    g_object_unref (listener->active_session);

  if (G_OBJECT_CLASS (polkit_agent_remote_listener_parent_class)->finalize != NULL)
    G_OBJECT_CLASS (polkit_agent_remote_listener_parent_class)->finalize (object);
}

static void
polkit_agent_remote_listener_set_property (GObject      *object,
                                           guint         prop_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
}

static void
polkit_agent_remote_listener_get_property (GObject    *object,
                                           guint       prop_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
}

static void
polkit_agent_remote_listener_class_init (PolkitAgentRemoteListenerClass *klass)
{
  GObjectClass *gobject_class;
  PolkitAgentListenerClass *listener_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = polkit_agent_remote_listener_finalize;
  gobject_class->get_property = polkit_agent_remote_listener_get_property;
  gobject_class->set_property = polkit_agent_remote_listener_set_property;

  listener_class = POLKIT_AGENT_LISTENER_CLASS (klass);
  listener_class->initiate_authentication        = polkit_agent_remote_listener_initiate_authentication;
  listener_class->initiate_authentication_finish = polkit_agent_remote_listener_initiate_authentication_finish;
}

/**
 * polkit_agent_remote_listener_new:
 * @cancellable: A #GCancellable or %NULL.
 * @error: Return location for error or %NULL.
 *
 * Returns: A #PolkitAgentRemoteListener or %NULL if @error is set. Free with g_object_unref() when done with it.
 */
PolkitAgentListener *
polkit_agent_remote_listener_new (GCancellable  *cancellable,
                                  GError       **error)
{
  g_return_val_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  return POLKIT_AGENT_LISTENER (g_initable_new (POLKIT_AGENT_TYPE_REMOTE_LISTENER,
                                                cancellable,
                                                error,
                                                NULL));
}

/* ---------------------------------------------------------------------------------------------------- */

static gboolean
initable_init (GInitable     *initable,
               GCancellable  *cancellable,
               GError       **error)
{
  PolkitAgentRemoteListener *listener = POLKIT_AGENT_REMOTE_LISTENER (initable);
  gboolean ret;

  ret = FALSE;

  listener->in = fdopen (STDIN_FILENO, "r");
  if (listener->in == NULL)
    {
      g_set_error (error,
                   POLKIT_ERROR,
                   POLKIT_ERROR_FAILED,
                   "Error opening current controlling terminal for the process (`%s'): %s",
                   "stdin",
                   strerror (errno));
      goto out;
    }

  listener->out = fdopen (STDOUT_FILENO, "w");
  if (listener->out == NULL)
    {
      fclose (listener->in);
      g_set_error (error,
                   POLKIT_ERROR,
                   POLKIT_ERROR_FAILED,
                   "Error opening current controlling terminal for the process (`%s'): %s",
                   "stdout",
                   strerror (errno));
      goto out;
    }

  ret = TRUE;

 out:
  return ret;
}

static void
initable_iface_init (GInitableIface *initable_iface)
{
  initable_iface->init = initable_init;
}

/* ---------------------------------------------------------------------------------------------------- */

static void
on_completed (PolkitAgentSession *session,
              gboolean            gained_authorization,
              gpointer            user_data)
{
  PolkitAgentRemoteListener *listener = POLKIT_AGENT_REMOTE_LISTENER (user_data);

  if (gained_authorization)
    fprintf (listener->out, JSON_COMPLETE);
  else
    fprintf (listener->out, JSON_FAILED);

  fflush (listener->out);

  g_simple_async_result_complete_in_idle (listener->simple);

  g_object_unref (listener->simple);
  g_object_unref (listener->active_session);
  g_cancellable_disconnect (listener->cancellable, listener->cancel_id);
  g_object_unref (listener->cancellable);

  listener->simple = NULL;
  listener->active_session = NULL;
  listener->cancel_id = 0;
}

static void
on_request (PolkitAgentSession *session,
            const gchar        *request,
            gboolean            echo_on,
            gpointer            user_data)
{
  PolkitAgentRemoteListener *listener = POLKIT_AGENT_REMOTE_LISTENER (user_data);
  GString *str;
  gchar **response_parts = NULL;
  gchar *response_value = NULL;

  if (g_ascii_strncasecmp (request, "password:", 9) == 0)
    {
      fprintf (listener->out, JSON_REQUEST_PASSWORD);
    }
  else
    {
      fprintf (listener->out, JSON_REQUEST_OTHER, request);
    }

  fflush (listener->out);

  setbuf (listener->out, NULL);

  str = g_string_new (NULL);
  while (TRUE)
    {
      gint c;
      c = getc (listener->in);
      if (c == '\n')
        {
          /* ok, done */
          break;
        }
      else if (c == EOF)
        {
          g_error ("Got unexpected EOF while reading from stdin.");
          abort ();
          break;
        }
      else
        {
          g_string_append_c (str, c);
        }
    }

  putc ('\n', listener->out);

  /* We got: "{ \"response\" : \"<text>\" }\n". <text> is a password or
     something instead of a password (in case the request is not a password). */
  response_parts = g_strsplit (str->str, ":", 2);
  if (response_parts &&
      *response_parts &&
      *(response_parts + 1) &&
      g_strstr_len (*response_parts, -1, JSON_FIELD_RESPONSE))
    {
      /* Get <text> form " \"<text>\" }\n". */
      gchar *first = g_strstr_len (*(response_parts + 1), -1, "\"");
      gchar *last = g_strrstr (*(response_parts + 1), "\"");

      if (first && last && last > first && (last - first == 1))
        {
          response_value = g_strdup ("");
        }
      else if (first && last && last > first && (last - first > 1))
        {
          response_value = g_strndup (first + 1, last - (first + 1));
        }
      else
        {
          g_warning ("Failed to get value from response.");
        }
    }
  else
    {
      g_warning ("Got unexpected response from alterator-manager.");
      response_value = g_strdup ("");
    }

  polkit_agent_session_response (session, response_value);
  memset (str->str, '\0', str->len);
  g_string_free (str, TRUE);
  g_strfreev (response_parts);
  g_free (response_value);
}

static void
on_show_error (PolkitAgentSession *session,
               const gchar        *text,
               gpointer            user_data)
{
  PolkitAgentRemoteListener *listener = POLKIT_AGENT_REMOTE_LISTENER (user_data);
  fprintf (listener->out, JSON_ERROR, text);
  fflush (listener->out);
}

static void
on_show_info (PolkitAgentSession *session,
              const gchar        *text,
              gpointer            user_data)
{
  PolkitAgentRemoteListener *listener = POLKIT_AGENT_REMOTE_LISTENER (user_data);
  fprintf (listener->out, JSON_INFO, text);
  fflush (listener->out);
}

static void
on_cancelled (GCancellable *cancellable,
              gpointer      user_data)
{
  PolkitAgentRemoteListener *listener = POLKIT_AGENT_REMOTE_LISTENER (user_data);
  fprintf (listener->out, JSON_CANCELED);
  fflush (listener->out);

  polkit_agent_session_cancel (listener->active_session);
}

static gchar *
identity_to_human_readable_string (PolkitIdentity *identity)
{
  gchar *ret;

  g_return_val_if_fail (POLKIT_IS_IDENTITY (identity), NULL);

  ret = NULL;
  if (POLKIT_IS_UNIX_USER (identity))
    {
      struct passwd pw;
      struct passwd *ppw;
      char buf[2048];
      int res;

      res = getpwuid_r (polkit_unix_user_get_uid (POLKIT_UNIX_USER (identity)),
                        &pw,
                        buf,
                        sizeof buf,
                        &ppw);
      if (res != 0)
        {
          g_warning ("Error calling getpwuid_r: %s", strerror (res));
        }
      else
        {
          if (ppw->pw_gecos == NULL || strlen (ppw->pw_gecos) == 0 || strcmp (ppw->pw_gecos, ppw->pw_name) == 0)
            {
              ret = g_strdup_printf ("%s", ppw->pw_name);
            }
          else
            {
              ret = g_strdup_printf ("%s (%s)", ppw->pw_gecos, ppw->pw_name);
            }
        }
    }
  if (ret == NULL)
    ret = polkit_identity_to_string (identity);
  return ret;
}

static PolkitIdentity *
get_selected_identity (PolkitAgentRemoteListener *listener,
                       GList                     *identities)
{
  PolkitIdentity *ret;
  GString *json_answer;
  gchar *user_name;
  gchar **answer_parts = NULL;

  ret = NULL;

  json_answer = g_string_new (NULL);
  while (TRUE)
    {
      gint c;
      c = getc (listener->in);
      if (c == '\n')
        {
          /* ok, done */
          break;
        }
      else if (c == EOF)
        {
          g_error ("Got unexpected EOF while reading selected user.");
          abort ();
          break;
        }
      else
        {
          g_string_append_c (json_answer, c);
        }
    }

  /* We got: "{ \"selected_user\" : \"<user name>\" }\n".
     We split it by "\"" into five parts. Username is the fourth part. */
  if (!g_strstr_len(json_answer->str, -1, JSON_FIELD_SELECTED_USER))
    {
      fprintf (listener->out, JSON_INVALID_RESPONSE,
               "no field \"selected user\"");
      goto out;
    }
  answer_parts = g_strsplit (json_answer->str, "\"", 5);
  for (gint n = 0; n < 4; n++)
    {
      if (*(answer_parts + n) == NULL)
        {
          gchar *escp = g_strescape(json_answer->str, NULL);
          fprintf (listener->out, JSON_INVALID_RESPONSE, escp);
          g_free(escp);
          goto out;
        }
      else if (n == 3)
        {
          user_name = *(answer_parts + n);
          break;
        }
    }

  /* We compare whether the selected user matches the existing ones. */
  for (GList *l = identities; l != NULL; l = l->next)
    {
      PolkitIdentity *identity = POLKIT_IDENTITY (l->data);
      gchar *s;
      s = identity_to_human_readable_string (identity);
      if (!g_strcmp0 (s, user_name))
        {
          ret = identity;
          g_free (s);
          break;
        }
      g_free (s);
    }

  if (!ret)
    {
      gchar *escp = g_strescape(json_answer->str, NULL);
      fprintf (listener->out, JSON_INVALID_RESPONSE, escp);
      g_free(escp);
    }

 out:
  g_string_free (json_answer, TRUE);
  g_strfreev(answer_parts);

  return ret;
}

static void
polkit_agent_remote_listener_initiate_authentication (PolkitAgentListener  *_listener,
                                                      const gchar          *action_id,
                                                      const gchar          *message,
                                                      const gchar          *icon_name,
                                                      PolkitDetails        *details,
                                                      const gchar          *cookie,
                                                      GList                *identities,
                                                      GCancellable         *cancellable,
                                                      GAsyncReadyCallback   callback,
                                                      gpointer              user_data)
{
  PolkitAgentRemoteListener *listener = POLKIT_AGENT_REMOTE_LISTENER (_listener);
  GSimpleAsyncResult *simple;
  PolkitIdentity *identity;
  GString *json_str;

  simple = g_simple_async_result_new (G_OBJECT (listener),
                                      callback,
                                      user_data,
                                      polkit_agent_remote_listener_initiate_authentication);
  if (listener->active_session != NULL)
    {
      g_simple_async_result_set_error (simple,
                                       POLKIT_ERROR,
                                       POLKIT_ERROR_FAILED,
                                       "An authentication session is already underway.");
      g_simple_async_result_complete_in_idle (simple);
      g_object_unref (simple);
      goto out;
    }

  g_assert (g_list_length (identities) >= 1);

  json_str = g_string_new (JSON_AUTH_ACTION_ID);
  g_string_append (json_str, action_id);
  g_string_append (json_str, JSON_AUTH_MESSAGE);
  g_string_append (json_str, message);
  g_string_append (json_str, JSON_AUTH_USERS);

  gint n = 0;
  for (GList *l = identities; l != NULL; l = l->next, n++)
    {
      PolkitIdentity *identity = POLKIT_IDENTITY (l->data);
      gchar *s;
      s = identity_to_human_readable_string (identity);
      if (n > 0)
        {
          g_string_append (json_str, ", ");
        }
      g_string_append (json_str, "\"");
      g_string_append (json_str, s);
      g_string_append (json_str, "\"");
      g_free (s);
    }

  g_string_append (json_str, JSON_AUTH_END);

  /* Send json string to the alterator-manager (remote). */
  fprintf (listener->out, "%s", json_str->str);
  fflush (listener->out);
  g_string_free (json_str, TRUE);

  /* Get a selected user from the alterator-manager (remote). */
  identity = get_selected_identity (listener, identities);
  /* If identity is NULL then send "AUTHENTICATION CANCELED". */
  if (identity == NULL)
    {
      fprintf (listener->out, JSON_CANCELED);
      fflush (listener->out);
      g_simple_async_result_set_error (simple,
                                       POLKIT_ERROR,
                                       POLKIT_ERROR_FAILED,
                                       "Authentication was canceled.");
      g_simple_async_result_complete_in_idle (simple);
      g_object_unref (simple);
      goto out;
    }

  listener->active_session = polkit_agent_session_new (identity, cookie);
  g_signal_connect (listener->active_session,
                    "completed",
                    G_CALLBACK (on_completed),
                    listener);
  g_signal_connect (listener->active_session,
                    "request",
                    G_CALLBACK (on_request),
                    listener);
  g_signal_connect (listener->active_session,
                    "show-info",
                    G_CALLBACK (on_show_info),
                    listener);
  g_signal_connect (listener->active_session,
                    "show-error",
                    G_CALLBACK (on_show_error),
                    listener);

  listener->simple = simple;
  listener->cancellable = g_object_ref (cancellable);
  listener->cancel_id = g_cancellable_connect (cancellable,
                                               G_CALLBACK (on_cancelled),
                                               listener,
                                               NULL);

  polkit_agent_session_initiate (listener->active_session);

 out:
  ;
}

static gboolean
polkit_agent_remote_listener_initiate_authentication_finish (PolkitAgentListener  *_listener,
                                                             GAsyncResult         *res,
                                                             GError              **error)
{
  gboolean ret;

  g_warn_if_fail (g_simple_async_result_get_source_tag (G_SIMPLE_ASYNC_RESULT (res)) ==
                  polkit_agent_remote_listener_initiate_authentication);

  ret = FALSE;

  if (g_simple_async_result_propagate_error (G_SIMPLE_ASYNC_RESULT (res), error))
    goto out;

  ret = TRUE;

 out:
  return ret;
}
