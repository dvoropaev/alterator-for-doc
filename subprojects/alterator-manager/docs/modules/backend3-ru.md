**alterator-module-backend3**\
[Русский](backend3-ru.md) | [English](backend3.md)

Модуль [alterator-manager'а](../README-ru.md) для использования скриптов
backend3.

___

Модуль backend3 во многом похож на [executor](./executor-ru.md) и предназначен
для использования бэкендов из backend3 alterator'а.\
D-Bus интерфейсы, использующие этот модуль, описываются в конфигурационных
файлах с расширением `backend`. Один такой файл описывает один интерфейс. Для
описания используется язык TOML. Ниже приведён пример описывающий интерфейс
`org.altlinux.alterator.backend3`:

```
type = "Backend"
module = "backend3"
name = "example1"
interface = "backend3"
#interface = "org.altlinux.alterator.backend3" //Интерфейс можно задать полным именем.
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

Поле `backend3` обязательное. Остальные поля не обязательные.

- В поле `backend3` указывается имя бэкенда из директории
  `/usr/lib/alterator/backend3/`.

- Про `action_id` можно почитать в документации к модулю
  [executor](./executor-ru.md).

Методы имеют один параметр типа `a{ss}`, и возвращаемое значение типа `aа{ss}`.\
В параметре передаются данные в виде пар ключ - значение. Для примера выше это
может быть `{"ui":"qt", "expert_mode":"0"}`. Данные из входного параметра могут
быть дополнены с помощью переменных описанных в подтаблице `environment` в
описании метода. Значения этим переменным присваиваются через интерфейс
`org.altlinux.alterator.manager` в корневом объекте, также им может быть
присвоено значение по умолчанию с помощью поля `default`.\

В примере выше:\
- Переменная `_objects` содержит тоже, что одноимённая переменная в бэкендах из
  backend3.

  Ещё можно встретить название `path`, например, в нашем случае этот path был бы
  `/menu/avail_modules`, но в `_objects` первая часть не записывается.

- В переменной `language` указана локаль.

- В переменной `action` указано действие - `list`.

  Обычно, действие это `list`, `write`, или `read`, но бывают и другие.

Весь вывод метода упаковывается в тип `aа{ss}`.
