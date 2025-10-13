**alterator-manager**\
[English](README.md) | [Русский](./docs/README-ru.md)

A modular tool for system configuration via D-Bus.

- [`.backend` files](#backend-files)
- [Data Storage](#data-storage)
- [Interface Validation](#interface-validation)
- [Environment Variables](#environment-variables)
- [am-dev-tool](#am-dev-tool)
- [Modules](#modules)

___

# `.backend` files

alterator-manager uses configuration `.backend` files to provide interfaces on
D-Bus. Based on these, D-Bus introspection and polkit rules are generated.

`backend` files have the `.backend` suffix and are stored in the following
directories:

- `/usr/share/alterator/backends/user/` and `/etc/alterator/backends/user/` -
  for user mode (`--user`)
- `/usr/share/alterator/backends/`, `/usr/share/alterator/backends/system/` and
  `/etc/alterator/backends/`, `/etc/alterator/backends/system/` - for normal
  mode.

The directories are named in the order they are processed, i.e., files are first
read from `/usr/share/alterator/backends/`, then from
`/usr/share/alterator/backends/system/`, then from `/etc/alterator/backends/`,
and finally from `/etc/alterator/backends/system/`.

For user mode, first from `/usr/share/alterator/backends/user/`, then from
`/etc/alterator/backends/user/`.

One `backend` file describes one interface of a certain object. Since one object
cannot have interfaces with the same names, 'duplicates' are ignored during
loading. Data in `backend` files is stored in toml format:

```
type = "Backend"
module = "executor"
name = "first2"
interface = "second"
#interface = "org.altlinux.alterator.second" //The interface can be specified by its full name.
thread_limit = 5
action_id = "org.freedesktop.test"


[methods.make_ls2]
execute = "ls -al {param}"

[methods.make_ls3]
execute = "ls -al {param}"
```

The root table has the following key-value pairs:

1. `type` - depends on the file's purpose. Currently filled with the word
   Backend.
2. `module` - the name of the module that will handle requests.
3. `name` - the node (object) name in the D-Bus subtree.
4. `interface` - the D-Bus interface name.
5. `thread_limit` - the maximum number of threads for this interface.
6. `action_id` - the action id for polkit.

If the interface name is given as a single part (i.e., without dots, like
`second` in the example above), the prefix `org.altlinux.alterator.` is
automatically added to the `interface_name`. If it consists of multiple parts
(separated by dots, e.g., `org.altlinux.alterator.second`), the prefix is not
added. Empty parts are not allowed. Each part can only contain
`[A-Z][a-z][0-9]_`.

The `action_id` option is optional. This is the action id for the interface
methods (polkit). If the action id is not specified, it is generated from the
interface name by replacing underscores with hyphens.

The `thread_limit` option is optional. If not explicitly set, its value will be
10 (the maximum number of methods executed concurrently).

Methods are described in the `methods` table, with a separate subtable for each
method. The subtable name is the method name. The method name can only consist
of Latin letters, numbers, and the underscore character. Spaces are not allowed.
The content of the section depends on the module that will process the request.

The example above is for the `executor` module. The `execute` field contains a
string for `bash` with the executable command. Parameters for the D-Bus method
can be specified inside curly braces. There might be no parameters. The
parameter name can only contain Latin letters, numbers, and the underscore
character. In the example above, the `make_ls2` method will execute the `ls -al`
command with the `param` parameter, into which a directory or file name can be
passed.

# Data Storage

Data received from modules and read from backend files is stored by
alterator-manager in a hash table (`backends_data`). The keys in this table are
the node names in the D-Bus tree, and the values are hash tables (`interfaces`).
The keys in the interfaces are the interface names, and the values are
`InterfaceObjectInfo` structures.

```
typedef struct {
    gchar *module_name;
    gchar *action_id;
    GDBusInterfaceInfo *interface_introspection;
    GHashTable *methods;
    const GDBusInterfaceVTable *interface_vtable;
    gint thread_limit;
} InterfaceObjectInfo;
```

1. `module_name` - module name.
2. `action_id` - action id for the interface (for polkit).
3. `interface_introspection` - GDBusInterfaceInfo from the module.
4. `thread_limit` - thread limit for the interface.
5. `interface_vtable` - GDBusInterfaceVTable from the module.
6. `methods` - hash table with methods.

In `methods`, the keys are method names (`[methods.*]` in the backend), and the
values are `MethodObjectInfo` structures. Each such structure contains a hash
table `method_data` with information about the method taken from the backend
file and a hash table `environment` with information about environment variables
for this method.\
In the `method_data` table, the keys are the keys from the table describing the
method, and the values are the values of the corresponding keys. For example,
for the executor, the key would be `execute`, and the value would be the
executable string. In the `environment` table, the keys are environment variable
names, and the values are `EnvironmentObjectInfo` structures.

```
                backends_data
    key: node_name             value: interfaces
                                        |
                                        |
                                        |
                      -------------------
                      ||
                      ||
                      \/
                  interface
    key: interface_name        value: InterfaceObjectInfo->methods
                                                             |
                                                             |
                                                             |
                      ----------------------------------------
                      ||
                      ||
                      \/
                    method
    key: method_name           value: MethodObjectInfo
                                         |          |
                                         |          |
                                         |          |
                                         |          |
                     ---------------------          |
                     ||                             |
                     \/                             |
                method_data                         -----------------------
    key: method_name           value: method_data                        ||
                                        |                                ||
                                        |                                \/
                                        |                            environment
                      -------------------                 key: env_var      value: EnvironmentObjectInfo
                      ||
                      ||
                      \/
                 method_data
key: it depends on the module. value: it depends on the module
For example - executor:
key: execute                   value: executable file
```

# Interface Validation

Interface validation is the process of checking the compliance of an interface,
formed from a file with the `.backend` extension, against a template.\
A template is an introspection file of the interface being checked; the name of
this file consists of the interface name and the `.xml` extension. Template
files are located in `/usr/share/dbus-1/interfaces`.

Validation is considered failed if a template exists for the interface, but not
all methods described in the template are described in the checked interface, or
if the method descriptions do not match.

# Environment Variables

In the root object, in the `org.altlinux.alterator.manager` interface, there are
methods to add and remove environment variables. These variables are stored in
the `sender_environment_data` hash table, which is structured as follows:

```
                  sender_environment_data
           key: sender                value: variables
                                                 |
                                                 |
                                                 |
                            ----------------------
                            ||
                            ||
                            \/
                         variable
           key: name                  value: value
```

`sender` is a unique D-Bus name (e.g., `:1.951`). A user connects to D-Bus,
receives a unique name, and adds environment variables. At this point, a
`sender->variables` pair is created in the `sender_environment_data` table,
where `variables` is also a hash table that stores the variables as
`name->value` pairs.\
These variables can be used by modules when launching new processes. For
example, if a user (`sender`) launches a new process using the executor module,
the set of environment variables inherited by this process from its parent
(alterator-manager) can be supplemented with variables specified in the backend
file in the `environment` subtable of that method (see the executor module).\
In this case, the variable values are taken from the hash table for this user
(`sender`) in `sender_environment_data`. If a value for the variable is not
found there, the default value from the `environment` subtable of the launched
method is used. If no value is found there either, the variable is not set.

Every 10 minutes, a check is performed. Unique names (`sender`s) that are not
among those connected to D-Bus are removed from the `sender_environment_data`
table. The variables associated with them are also removed.

# am-dev-tool

`am-dev-tool` is a utility from the alterator-manager-tools package, intended
for generating and validating policy files.

Example of generation with the result saved to a file:

```
$am-dev-tool -g /usr/share/alterator/backends/file.backend -o ~/file.policy
```

Example of validation:

```
$am-dev-tool -t /usr/share/alterator/backends/file.backend
```

The `-q` option suppresses output.

# Modules

Documentation for alterator-manager modules:

- [executor](./docs/modules/executor.md)
- [remote](./docs/modules/remote.md)
- [backend3](./docs/modules/backend3.md)
