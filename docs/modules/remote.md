**alterator-module-backend3**\
[English](remote.md) | [Русский](remote-ru.md)

A module for [alterator-manager](../../README.md) to connect to remote machines
(remote alterator-managers).

- [Remote module](#remote-module)
- [Password agent](#password-agent)

___

# Remote module

Connection is established via SSH.\
The remote module is only available in `--user` mode and does not use `.backend`
files. The module's methods are located in the `org.altlinux.alterator.remote`
interface of the `/org/altlinux/alterator/remote` object.

- Method `Connect`

  - in:
    - string "remote_address"
    - string "connection_name"
    - string "agent_bus_name"
    - string "pty"
  - out:
    - boolean "response"
    - string "object_path"

  This method establishes a connection to a remote machine. The connection
  consists of two SSH connections.\
  The first connection connects to the remote D-Bus bus using
  `systemd-stdio-bridge`.\
  The second connection launches a special polkit agent on the remote machine -
  `remote-polkit-agent`, and if polkit on the remote machine requires user
  rights confirmation to perform an action, this agent will notify the remote
  module, and remote will ask the user to enter the necessary authentication
  data using the [password agent](#password-agent).\
  The `bus name` of the password agent is specified by the `agent_bus_name`
  parameter.

  *remote_address*: address of the remote machine (\<name>@\<ip>).

  *connection_name*: connection name can only contain `[A-Z][a-z][0-9]_`. It
  will be used to form the root object of the subtree representing the remote
  machine's object tree.\
  For example, if `connection_name` is "777", the remote machine's tree will be
  mapped to a subtree with root `/org/altlinux/alterator/connection/777`.

  *agent_bus_name*: this is the `bus_name` of the password agent on D-Bus. The
  [password agent](#password-agent) is a service used by the remote module to
  ask the user for a password.

  *pty*:

  *response*: completion code. `TRUE` - successful method completion, `FALSE` -
  unsuccessful completion.

  *object_path*: root object of the subtree representing the remote machine's
  D-Bus object tree.\
  For example, if `connection_name` is "777", the object
  `/org/altlinux/alterator/connection/777` should be returned.

- Method `Disconnect`

  - in:
    - string "connection_name"
  - out:
    - boolean "response"

  This method disconnects from the remote machine.

  *connection_name*: connection name (see Connect method).

  *response*: completion code. `TRUE` - successful method completion, `FALSE` -
  unsuccessful completion.

- Method `GetConnections`

  - out:
    - array of strings "connections"

  *connections*: list of established connection names.

# Password agent

**Password agent** - a service used by the remote module for user authentication
upon request from the remote polkit agent (`remote-polkit-agent`).\
The service's bus name is specified in the `agent_bus_name` parameter of the
remote module's `Connect` method, meaning a separate agent can be used for each
connection.\
The password agent must have an object `/org/altlinux/PasswordAgent` with the
interface `org.altlinux.PasswordAgent`.

- Method `SelectUser`

  - in:
    - array of strings "users"
    - string "action_id"
    - string "message"
    - string "pty"
  - out:
    - string "selected_user"
    - string "password"

  *users*: list of users provided by the remote polkit agent for authentication.

  *action_id*: action_id of the action provided by the remote polkit agent.

  *message*: message for the action being performed, provided by the remote
  polkit agent.

  *pty*:

  *selected_user*: user selected from the users list.

  *password*: password for the selected user.

- Method `ShowResult`

  - in:
    - string "result"

  *result*: result string returned by the polkit agent.
