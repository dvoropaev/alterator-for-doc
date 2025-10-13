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
_objects = "avail_modules"
language = "ru_RU"
action = "list"
action_id = "test1"
```

The `backend3` field is required. Other fields are optional.

- The `backend3` field specifies the name of the backend from the
  `/usr/lib/alterator/backend3/` directory.

- The `_objects` field contains the same as the variable of the same name in
  backend3 backends.

  You might also encounter the name `path`, for example, in our case this path
  would be `/menu/avail_modules`, but the first part is not written in
  `_objects`.

- The `language` field specifies the locale.

- The `action` field specifies the action:\
  `list`, `write`, `read`, etc.

- About `action_id` you can read in the documentation for the
  [executor](./executor.md) module.

Methods have one parameter of type `a{ss}`, and a return value of type `a{ss}`.\
The parameter passes data in the form of key-value pairs. For the example above,
this could be `{"ui":"qt", "expert_mode":"0"}`. All method output is packaged
into the `a{ss}` type.
