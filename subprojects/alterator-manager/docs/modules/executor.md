**alterator-module-executor**\
[English](executor.md) | [Русский](executor-ru.md)

[Alterator-manager](../../README.md) module for running executable files and
scripts.

___

Method options for the module:

```
[methods.method_name]
execute = "ls -l {param} {param_array[]}"
stdin_string = true
stdout_strings = true
stdout_bytes = true
stdout_byte_arrays = true
stdout_string_array = true
stdout_json = ["arg1", "arg2", "arg3[]"]
stderr_strings = true
stdout_signal_name = "stdout_signal_example"
stderr_signal_name = "stderr_signal_example"
stdout_byte_limit = 200000
stdout_strings_limit = 200000
stderr_strings_limit = 200000
thread_limit = 3
action_id = "method-name"
timeout = 60
[methods.method_name.environment.VARNAME]
default = "default value"
required = false
[methods.method_name.environment.VARNAME2]
```

The `execute` field is required, others are optional.

- The `execute` field contains a string for `bash` with the executable command.

  Parameters for the D-Bus method can be specified inside curly braces. There
  might be no parameters. The parameter name can only contain Latin letters,
  numbers, and the underscore character. By default, the parameter type is
  `string`, but parameters of type "array of strings" can also be specified. For
  this, square brackets are added to the name (as in the example above). Strings
  from such parameter are quoted and used sequentially, one after another.

- The `stdout_strings` field enables returning an array of strings from stdout.

- The `stdout_bytes` field enables returning a byte array from stdout.

- The `stdout_byte_arrays` field enables returning an array of byte arrays from
  stdout. In this case, the byte stream from stdout is split by null bytes into
  separate arrays.

- `stdout_string_array` is the same as `stdout_byte_arrays`, but the method's
  return value is not an array of byte arrays, but an array of strings.

- The `stdout_json` field enables the method to return multiple values (of type
  *array of strings* or *string*).

  The mode is enabled if this field is assigned an array of strings.\
  This array specifies the names of the method's return values; they can only
  contain characters `[A-Z][a-z][0-9]_`, but to indicate that a return value has
  the array of strings type, a pair of square brackets is added to its name (see
  example above), which are discarded when forming the return value name.\
  In this mode, the method expects a JSON object string from the process's
  stdout. Elements whose keys match the return value names are extracted from
  this object. If the extracted element has a type that doesn't match the return
  value type, it is replaced with an empty string.

> [!WARNING] Only one of the above modes can be enabled for a method. If
> multiple modes are specified in the method description, the one with higher
> priority is enabled, with priorities as follows (in ascending order):\
> `stdout_strings`, `stdout_bytes`, `stdout_byte_arrays`, `stdout_string_array`,
> and `stdout_json`.

- The fields `stdout_bytes`, `stdout_byte_arrays`, `stdout_string_array`, and
  `stdout_json` also disable returning strings from stdout via signals.

- The `stderr_strings` field enables returning an array of strings from the
  stderr stream.

- The `exit_status` field enables returning the exit code of the process.

- The fields `stdout_byte_limit`, `stdout_strings_limit`, and
  `stderr_strings_limit` set the maximum size in bytes for the corresponding
  array.

  Default value is 524288. Valid range is from 0 to 2147483647. Working with
  arrays larger than 524288 has not been tested.

- Output from stdout and stderr can also be received via signals. The signal
  name is obtained by concatenating the sender (bus name) and the value of the
  corresponding field (`stdout_signal_name` or `stderr_signal_name`). In the
  sender, `':'` and `'.'` are replaced with `'_'`.\
  The values of the `stdout_signal_name` and `stderr_signal_name` fields can
  only contain Latin letters, numbers, and the underscore character. Signals for
  stdout and stderr are enabled if the corresponding field has a value.

- The `stdin_string` field with value `true` adds one more parameter to the
  method's parameter list - stdin.

  This parameter appears last in the parameter list. The string passed through
  it is sent to the process's standard input immediately after launch.

- The `thread_limit` field can be used to set a limit on the number of threads
  for the method, i.e., how many instances of this method can run in parallel.
  Default value is 1.

- The `action_id` field can be used to set the polkit action_id for the method.
  The field can only contain the following characters `[A-Z][a-z][0-9].-`. A
  prefix formed from the interface name will be automatically added to the value
  of this field.

- The `timeout` field sets the time interval after which the SIGKILL signal will
  be sent to the process spawned to execute this method.

  Time is specified in seconds. Default value is 60 seconds.\
  If the value of the `timeout` field is zero or less, no timeout is set and the
  SIGKILL signal will not be sent.\
  If the value is set incorrectly - no timeout is set and the SIGKILL signal
  will not be sent.

- The `environment` subtable lists environment variables that can be added to
  the list of environment variables inherited from the parent process (The
  parent process is alterator-manager. For executing each method, executor
  spawns a new process).

  Each variable in `environment` is described in a separate table. The table
  name is the variable name. The string field `default` is used to set the
  default value. The `required` field is not currently used. The variable table
  can be empty, then its value is assigned via the
  `org.altlinux.alterator.manager` interface in the root object.\
  Variables not listed in `environment` cannot be added to the process
  environment.
