[English](./component-categories1.md) | [Русский](./component-categories1.ru_RU.md)

# Интерфейс **org.altlinux.alterator.component-categories1**

Предоставляет доступ к описателям категорий компонентов, их описаниям и списку категорий.

| Метод | Описание |
|--------|---------|
| [Info](#method-Info) | Возвращает описатель категории в формате .category. |
| [Description](#method-Description) | Возвращает локализованное описание категории. |
| [List](#method-List) | Перечисляет категории компонентов. |


## Методы

### **Info**([name](#argument-name-of-Info) : `s`) -> ([stdout_bytes](#argument-stdout_bytes-of-Info) : `ay`, [response](#argument-response-of-Info) : `i`)<a id="method-Info"></a>

Возвращает описатель категории в формате .category.

#### Входные аргументы

##### **name** : `s` <a id="argument-name-of-Info"></a>

Имя категории.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Info"></a>

Содержимое `/usr/share/alterator/components/categories/<name>/<name>.category`.

##### **response** : `i` <a id="argument-response-of-Info"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **Description**([name](#argument-name-of-Description) : `s`) -> ([stdout_bytes](#argument-stdout_bytes-of-Description) : `ay`, [response](#argument-response-of-Description) : `i`)<a id="method-Description"></a>

Возвращает локализованное описание категории.

#### Входные аргументы

##### **name** : `s` <a id="argument-name-of-Description"></a>

Имя категории.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Description"></a>

Содержимое `description.<locale>.html`, `description.<lang>.html` или `description.html` (выбор по LC_ALL, по умолчанию en_US).

##### **response** : `i` <a id="argument-response-of-Description"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **List**() -> ([stdout_strings](#argument-stdout_strings-of-List) : `as`, [response](#argument-response-of-List) : `i`)<a id="method-List"></a>

Перечисляет категории компонентов.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-List"></a>

Имена категорий.

##### **response** : `i` <a id="argument-response-of-List"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.


Актуальная спецификация: https://altlinux.space/alterator/alterator-entry/src/branch/master/doc
