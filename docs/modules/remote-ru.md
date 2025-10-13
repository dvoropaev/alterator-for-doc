**alterator-module-backend3**\
[Русский](remote-ru.md) | [English](remote.md)

Модуль [alterator-manager'а](../README-ru.md) для подключения к удаленным
машинам (удаленным alterator-manager'ам).

- [Модуль remote](#модуль-remote)
- [Password agent](#password-agent)

___

# Модуль remote

Подключение происходит по ssh.\
remote доступен только в режиме `--user` и не использует `.backend` файлы.
Методы модуля находятся в интерфейсе `org.altlinux.alterator.remote` объекта
`/org/altlinux/alterator/remote`.

- Метод `Connect`

  - in:
    - string "remote_address"
    - string "connection_name"
    - string "agent_bus_name"
    - string "pty"
  - out:
    - boolean "response"
    - string "object_path"

  Данный метод выполняет подключение к удалённой машине. Подключение состоит из
  двух ssh соединений.\
  Первое соединение подключается к удалённой шине d-bus с помощью
  `systemd-stdio-bridge`.\
  Второе соединение запускает на удалённой машине специальный polkit-agent -
  `remote-polkit-agent`, и если polkit на удалённой машине потребует
  подтверждения прав пользователя на выполнение действия, то этот агент даст об
  этом знать модулю remote, а remote попросит пользователя ввести необходимые
  для аутентификации данные используя [password agent](#password-agent).\
  `bus name` password agent'а задаётся параметром `agent_bus_name`.

  *remote_address*: адрес удалённой машины (\<name>@\<ip>).

  *connection_name*: имя соединения может содержать только `[A-Z][a-z][0-9]_`.
  Оно будет использовано для формирования корневого объекта поддерева,
  отображающего дерево объектов удалённой машины.\
  Например, если `connection_name` - "777", то дерево удалённой машины будет
  отображено в поддереве с корнем `/org/altlinux/alterator/connection/777`.

  *agent_bus_name*: это `bus_name` password agent'a на d-bus.
  [password agent](#password-agent) - сервис, с помощью которого модуль remote
  спрашивает пароль у пользователя.

  *pty*:

  *response*: код завершения. `TRUE` - удачное завершение метода, `FALSE` -
  неудачное завершение.

  *object_path*: корневой объект поддерева отображающего дерево объектов d-bus
  удалённой машины.\
  Например, если `connection_name` - "777", то должен вернуться объект
  `/org/altlinux/alterator/connection/777`.

- Метод `Disconnect`

  - in:
    - string "connection_name"
  - out:
    - boolean "response"

  Данный метод выполняет отключение от удалённой машины.

  *connection_name*: имя соединения (см. метод Connect).

  *response*: код завершения. `TRUE` - удачное завершение метода, `FALSE` -
  неудачное завершение.

- Метод `GetConnections`

  - out:
    - array of strings "connections"

  *connections*: список имён установленных соединений.

# Password agent

**password agent** - сервис, используемый модулем remote, для аутентификации
пользователя по запросу от удалённого polkit-agent'a (`remote-polkit-agent`).\
bus name сервиса указывается в параметре `agent_bus_name` метода `Connect`
модуля remote, т.е. для каждого соединения можно использовать отдельный агент.\
У password agent'а должен быть объект `/org/altlinux/PasswordAgent` с
интерфейсом `org.altlinux.PasswordAgent`.

- Метод `SelectUser`

  - in:
    - array of strings "users"
    - string "action_id"
    - string "message"
    - string "pty"
  - out:
    - string "selected_user"
    - string "password"

  *users*: список пользователей предоставленный удалённым polkit аgent'ом, для
  аутентификации.

  *action_id*: action_id действия, предоставленный удалённым polkit аgent'ом.

  *message*: сообщение для выполняемого действия, предоставленное удалённым
  polkit аgent'ом.

  *pty*:

  *selected_user*: выбранный из списка users пользователь.

  *password*: пароль для выбранного пользователя.

- Метод `ShowResult`

  - in:
    - string "result"

  *result*: строка-результат, возвращаемая polkit agent'ом.
