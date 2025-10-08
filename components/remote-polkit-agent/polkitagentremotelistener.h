/*  */

#if !defined(_POLKIT_AGENT_INSIDE_POLKIT_AGENT_H) && !defined (_POLKIT_AGENT_COMPILATION)
#error "Only <polkitagent/polkitagent.h> can be included directly, this file may disappear or change contents"
#endif

#ifndef __POLKIT_AGENT_REMOTE_LISTENER_H
#define __POLKIT_AGENT_REMOTE_LISTENER_H

#include <polkit/polkit.h>
#include <polkitagent/polkitagenttypes.h>

G_BEGIN_DECLS

#define POLKIT_AGENT_TYPE_REMOTE_LISTENER          (polkit_agent_remote_listener_get_type())
#define POLKIT_AGENT_REMOTE_LISTENER(o)            (G_TYPE_CHECK_INSTANCE_CAST ((o), POLKIT_AGENT_TYPE_REMOTE_LISTENER, PolkitAgentRemoteListener))
#define POLKIT_AGENT_IS_REMOTE_LISTENER(o)         (G_TYPE_CHECK_INSTANCE_TYPE ((o), POLKIT_AGENT_TYPE_REMOTE_LISTENER))

#define JSON_AUTH_ACTION_ID "{\"action_id\": \""
#define JSON_AUTH_MESSAGE "\", \"message\": \""
#define JSON_AUTH_USERS "\", \"users\": ["
#define JSON_AUTH_END "] }\n"
#define JSON_CANCELED "{ \"result\" : \"AUTHENTICATION CANCELED\" }\n"
#define JSON_COMPLETE "{ \"result\" : \"AUTHENTICATION COMPLETE\" }\n"
#define JSON_FAILED "{ \"result\" : \"AUTHENTICATION FAILED\" }\n"
#define JSON_REQUEST_PASSWORD "{ \"request\" : \"password\" }\n"
#define JSON_REQUEST_OTHER "{ \"request\" : \"%s\" }\n"
#define JSON_ERROR "{ \"error\" : \"%s\" }\n"
#define JSON_INFO "{ \"info\" : \"%s\" }\n"
#define JSON_INVALID_RESPONSE "{ \"invalid response\" : \"%s\"}\n"
#define JSON_FIELD_SELECTED_USER "\"selected user\""
#define JSON_FIELD_RESPONSE "\"response\""


GType                polkit_agent_remote_listener_get_type (void) G_GNUC_CONST;
PolkitAgentListener *polkit_agent_remote_listener_new      (GCancellable   *cancellable,
                                                            GError        **error);


G_END_DECLS

#endif /* __POLKIT_AGENT_REMOTE_LISTENER_H */
