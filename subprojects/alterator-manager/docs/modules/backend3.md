**alterator-module-backend3**\
[English](backend3.md) | [Русский](backend3-ru.md)

[Alterator-manager](../../README.md) module for using backend3 scripts.

___

The backend3 module is largely similar to [executor](./executor.md) and is
designed to use backends from alterator's backend3.\
D-Bus interfaces using this module are described in configuration files with the
`backend` extension. One such file describes one interface. TOML language is
used for description. Below is an example describing the
`org.altlinux.alterator.backend3` interface:

```
type = "Backend"
module = "backend3"
name = "example1"
interface = "backend3"
#interface = "org.altlinux.alterator.backend3" //The interface can be specified by its full name.
action_id = "org.freedesktop.test"

[methods.method_name]
backend3 = "menu"
action_id = "test1"
[methods.method_name.environment._objects]
default = "avail_modules"
[methods.method_name.environment.language]
default = "ru_RU"
[methods.method_name.environment.action]
default = "list"
```

The `backend3` field is required. Other fields are optional.

- The `backend3` field specifies the name of the backend from the
  `/usr/lib/alterator/backend3/` directory.

- About `action_id` you can read in the documentation for the
  [executor](./executor.md) module.

Methods have one parameter of type `a{ss}`, and a return value of type `a{ss}`.\
The parameter passes data in the form of key-value pairs. For the example above,
this could be `{"ui":"qt", "expert_mode":"0"}`. The data from the input
parameter can be supplemented using variables described in the `environment`
subtable in the method's description. These variables are assigned values
through the `org.altlinux.alterator.manager` interface in the root object, and
can also be assigned a default value using the `default` field.\

In the example above:\
- The `_objects` variable contains the same values as the variable of the same
  name in the backends from backend3.

  You may also encounter the name `path`; for example, in our case, this `path`
  would be `/menu/avail_modules`, but the first part is not written to
  `_objects`.

- The `language` variable specifies the locale.

- The `action` variable specifies the action - `list`.

  Usually, the action is `list`, `write`, or `read`, but others can also be
  used.

All method output is packaged into the `a{ss}` type.
