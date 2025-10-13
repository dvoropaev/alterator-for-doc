**alterator-manager**\
[Русский](README-ru.md) | [English](../README.md)

Модульный инструмент для конфигурации системы через D-Bus.

- [Файлы `.backend`](#%D1%84%D0%B0%D0%B9%D0%BB%D1%8B-backend)
- [Хранение данных](#%D1%85%D1%80%D0%B0%D0%BD%D0%B5%D0%BD%D0%B8%D0%B5-%D0%B4%D0%B0%D0%BD%D0%BD%D1%8B%D1%85)
- [Валидация интерфейсов](#%D0%B2%D0%B0%D0%BB%D0%B8%D0%B4%D0%B0%D1%86%D0%B8%D1%8F-%D0%B8%D0%BD%D1%82%D0%B5%D1%80%D1%84%D0%B5%D0%B9%D1%81%D0%BE%D0%B2)
- [Переменные окружения](#%D0%BF%D0%B5%D1%80%D0%B5%D0%BC%D0%B5%D0%BD%D0%BD%D1%8B%D0%B5-%D0%BE%D0%BA%D1%80%D1%83%D0%B6%D0%B5%D0%BD%D0%B8%D1%8F)
- [am-dev-tool](#am-dev-tool)
- [Модули](#%D0%BC%D0%BE%D0%B4%D1%83%D0%BB%D0%B8)

___

# Файлы `.backend`

alterator-manager для предоставления интерфейсов на D-Bus использует
конфигурационные `.backend` файлы. На их основе формируется интроспекция для
D-Bus и правила polkit.

`backend` файлы имеют суффикс `.backend` и хранятся в следующих директориях:

- `/usr/share/alterator/backends/user/` и `/etc/alterator/backends/user/` - для
  пользовательского режима (`--user`)
- `/usr/share/alterator/backends/`, `/usr/share/alterator/backends/system/` и
  `/etc/alterator/backends/`, `/etc/alterator/backends/system/` - для обычного
  режима.

Директории названы в том порядке, в котором они обрабатываются, т.е. сначала
считываются файлы из `/usr/share/alterator/backends/`, затем из
`/usr/share/alterator/backends/system/`, затем из `/etc/alterator/backends/`, и
потом из `/etc/alterator/backends/system/`.

Для пользовательского режима сначала из `/usr/share/alterator/backends/user/`,
потом из `/etc/alterator/backends/user/`.

Один `backend` файл описывает один интерфейс некоторого объекта. Поскольку на
одном объекте не может быть интерфейсов с одинаковыми именами, то 'дубликаты'
игнорируются при загрузке. Данные в `backend` файлах хранятся в формате toml:

```
type = "Backend"
module = "executor"
name = "first2"
interface = "second"
#interface = "org.altlinux.alterator.second" //Интерфейс можно задать полным именем.
thread_limit = 5
action_id = "org.freedesktop.test"


[methods.make_ls2]
execute = "ls -al {param}"

[methods.make_ls3]
execute = "ls -al {param}"
```

Корневая таблица имеет следующие пары ключ-значение:

1. `type` - зависит от назначения файла. Пока заполняется словом Backend.
2. `module` - имя модуля который будет обрабатывать запросы.
3. `name` - имя узла (объекта) в поддереве D-Bus.
4. `interface` - имя интерфейса D-Bus.
5. `thread_limit` - максимальное число тредов для данного интерфейса.
6. `action_id` - action id для polkit.

Если имя интерфейса задано одной секцией (т.е. без точек, second в примере
выше), то к `interface_name` автоматически добавляется префикс
`org.altlinux.alterator.`. Если оно состоит из нескольких секций (разделённых
точками, например `org.altlinux.alterator.second`), то префикс не добавляется.
Пустые секции не допускаются. Каждая секция может содержать только
`[A-Z][a-z][0-9]_`.

Опция `action_id` не обязательная. Это action id для методов интерфейса
(polkit). Если actiont id не указан, то он формируется из имени интерфейса, для
чего подчёркивания заменяются на тире.

Опция `thread_limit` не обязательная. Если её явно не задать, то её значение
будет равно 10 (максимальное число методов выполняемых одновременно).

Методы описываются в таблице `methods`, для каждого метода отдельная подтаблица.
Имя подтаблицы - имя метода. Имя метода может состоять только из латинских букв,
цифр и символа подчёркивания. Пробелы не допускаются. Содержимое секции зависит
от модуля который будет обрабатывать запрос.

Выше приведён пример для модуля `executor`. Поле `execute` содержит строку для
`bash` с исполняемым файлом. Внутри фигурных скобок могут задаваться параметры
для метода D-Bus. Параметров может не быть. Имя параметра может содержать только
латинские буквы, цифры и символ подчёркивания. В примере выше метод `make_ls2`
будет выполнять команду `ls -al` с параметром `param`, в который можно будет
передать имя директории или файла.

# Хранение данных

Данные, полученные от модулей и считанные из backend файлов, alterator-manager
хранит в хеш-таблице (`backends_data`). Ключами в ней являются имена узлов в
дереве D-Bus, а значениями хеш-таблицы (`interfaces`). Ключами в interface'ах
являются имена интерфейсов, а значениями - структуры `InterfaceObjectInfo`.

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

1. `module_name` - имя модуля.
2. `action_id` - action id для интерфейса (для polkit).
3. `interface_introspection` - GDBusInterfaceInfo из модуля.
4. `thread_limit` - лимит тредов для интерфейса.
5. `interface_vtable` - GDBusInterfaceVTable из модуля.
6. `methods` - хеш-таблица с методами.

В `methods` ключами являются имена методов (`[methods.*]` в backend), а
значениями - структуры `MethodObjectInfo`. В каждой такой структуре хеш-таблица
`method_data` с информацией о методе взятой из backend файла и хеш-таблица
`environment` с информацией о переменных окружения для этого метода.\
В таблице `method_data` ключи - это ключи из таблицы описывающей метод, а
значения - это значения соответствующих ключей. Например, для executor'а ключом
будет `execute`, а значением - исполняемая строка. В таблице `environment` ключи
\- имена переменных окружения, значения - структуры `EnvironmentObjectInfo`.

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

# Валидация интерфейсов

Валидация интерфейсов - это проверка соответствия интерфейса, сформированного из
файла с расширением `.backend`, шаблону.\
Шаблон - это файл интроспекции проверяемого интерфейса, имя этого файла состоит
из имени интерфейса и расширения `.xml`. Файлы-шаблоны лежат в
`/usr/share/dbus-1/interfaces`.

Валидация считается не пройденной, если для интерфейса существует шаблон, но не
все методы описанные в шаблоне описаны в проверяемом интерфейсе, или не
совпадают описания методов.

# Переменные окружения

В корневом объекте, в интерфейсе `org.altlinux.alterator.manager`, есть методы с
помощью которых можно добавлять и удалять переменные окружения. Эти переменные
хранятся в хеш-таблице `sender_environment_data`, она устроена следующим
образом:

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

`sender` - это уникальное имя на D-Bus (например :1.951). Пользователь
подключается к D-Bus, получает уникальное имя и добавляет переменные окружения.
В этот момент в таблице `sender_environment_data` создаётся пара
`sender->variables`, где `variables` это тоже хеш-таблица в которой и хранятся
переменные в виде пар `name->value`.\
Эти переменные могут использоваться модулями при запуске новых процессов.
Например, если пользователь (`sender`) запускает новый процесс с помощью модуля
executor, то набор переменных окружения, унаследованный этим процессом от предка
(alterator-manager), может дополниться переменными указанными в backend файле в
подтаблице `environment` этого метода (см. модуль executor).\
При этом, значения переменные получают из хеш-таблицы для этого пользователя
(`sender'а`) в `sender_environment_data`. Если для переменной там значения нет,
то используется значение по умолчанию из подтабицы `environment` запускаемого
метода. Если и там для переменной значения нет, то переменная не
устанавливается.

Раз в 10 минут происходит проверка. Уникальные имена (`sender`'ы), которых нет
среди подключённых к D-Bus, удаляются из таблицы `sender_environment_data`.
Удаляются и связанные с ними переменные.

# am-dev-tool

`am-dev-tool` - утилита из пакета alterator-manager-tools, она предназначена для
генерации и валидации policy файлов.

Пример генерации с сохранением результата в файле:

```
$am-dev-tool -g /usr/share/alterator/backends/file.backend -o ~/file.policy
```

Пример валидации:

```
$am-dev-tool -t /usr/share/alterator/backends/file.backend
```

Опция `-q` подавляет вывод.

# Модули

Документация для модулей alterator-manager:

- [executor](./modules/executor-ru.md)
- [remote](./modules/remote-ru.md)
- [backend3](./modules/backend3-ru.md)
