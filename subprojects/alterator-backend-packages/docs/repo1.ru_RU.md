[English](./repo1.md) | [Русский](./repo1.ru_RU.md)

# Интерфейс **org.altlinux.alterator.repo1**

Предоставляет команды управления репозиториями на основе apt-repo.

| Метод | Описание |
|--------|---------|
| [Info](#method-Info) | Возвращает статический описатель объекта бэкенда repo. |
| [List](#method-List) | Перечисляет настроенные репозитории через `apt-repo list`. |
| [Add](#method-Add) | Добавляет источник репозитория через `apt-repo add`. |
| [Remove](#method-Remove) | Удаляет источник репозитория через `apt-repo rm`. |


## Методы

### **Info**() -> ([stdout_bytes](#argument-stdout_bytes-of-Info) : `ay`, [response](#argument-response-of-Info) : `i`)<a id="method-Info"></a>

Возвращает статический описатель объекта бэкенда repo.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Info"></a>

Содержимое `/usr/share/alterator/objects/repo.object`.

TOML-описание объекта с display_name и comments.
##### **response** : `i` <a id="argument-response-of-Info"></a>

Код завершения помощника cat.

0 — успех, != 0 — ошибка.
### **List**() -> ([stdout_strings](#argument-stdout_strings-of-List) : `as`, [stderr_strings](#argument-stderr_strings-of-List) : `as`, [response](#argument-response-of-List) : `i`)<a id="method-List"></a>

Перечисляет настроенные репозитории через `apt-repo list`.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-List"></a>

Записи репозиториев, выводимые `apt-repo list`.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-List"></a>

Ошибки выполнения `apt-repo list`.

##### **response** : `i` <a id="argument-response-of-List"></a>

Код завершения команды списка.

0 — успех, != 0 — ошибка.
### **Add**([source](#argument-source-of-Add) : `s`) -> ([stderr_strings](#argument-stderr_strings-of-Add) : `as`, [response](#argument-response-of-Add) : `i`)<a id="method-Add"></a>

Добавляет источник репозитория через `apt-repo add`.

#### Входные аргументы

##### **source** : `s` <a id="argument-source-of-Add"></a>

Определение источника, передаваемое напрямую в `apt-repo add`.

Поддерживает пути к файлам или URI-строки, принимаемые apt-repo.
#### Выходные аргументы

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-Add"></a>

Предупреждения или ошибки apt-repo.

##### **response** : `i` <a id="argument-response-of-Add"></a>

Код завершения команды добавления.

0 — успех, != 0 — ошибка.
### **Remove**([source](#argument-source-of-Remove) : `s`) -> ([stderr_strings](#argument-stderr_strings-of-Remove) : `as`, [response](#argument-response-of-Remove) : `i`)<a id="method-Remove"></a>

Удаляет источник репозитория через `apt-repo rm`.

#### Входные аргументы

##### **source** : `s` <a id="argument-source-of-Remove"></a>

Идентификатор или путь к удаляемому репозиторию.

#### Выходные аргументы

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-Remove"></a>

Ошибки apt-repo при удалении.

##### **response** : `i` <a id="argument-response-of-Remove"></a>

Код завершения команды удаления.

0 — успех, != 0 — ошибка.


Актуальная спецификация: https://altlinux.space/alterator/alterator-entry/src/branch/master/doc
