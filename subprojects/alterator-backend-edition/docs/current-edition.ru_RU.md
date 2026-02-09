[English](./current-edition.md) | [Русский](./current-edition.ru_RU.md)

# Интерфейс **org.altlinux.alterator.current-edition1**

Предоставляет методы бэкенда для чтения и обновления текущей редакции продукта.

| Метод | Описание |
|--------|---------|
| [Info](#method-Info) | Возвращает файл-описатель `.edition` текущей редакции. |
| [Description](#method-Description) | Возвращает текст описания текущей редакции. |
| [License](#method-License) | Возвращает текст лицензии текущей редакции. |
| [ReleaseNotes](#method-ReleaseNotes) | Возвращает релиз-ноты текущей редакции. |
| [FinalNotes](#method-FinalNotes) | Возвращает финальные заметки текущей редакции. |
| [Get](#method-Get) | Возвращает идентификатор текущей редакции. |
| [Set](#method-Set) | Устанавливает текущую редакцию и обновляет системные метаданные. |


## Методы

### **Info**() -> ([stdout_bytes](#argument-stdout_bytes-of-Info) : `ay`, [response](#argument-response-of-Info) : `i`)<a id="method-Info"></a>

Возвращает файл-описатель `.edition` текущей редакции.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Info"></a>

Содержимое файла `.edition` текущей редакции.

TOML-текст с полями name, display_name, license.
##### **response** : `i` <a id="argument-response-of-Info"></a>

Код завершения.

0 — успех, != 0 — ошибка.
### **Description**() -> ([stdout_bytes](#argument-stdout_bytes-of-Description) : `ay`, [response](#argument-response-of-Description) : `i`)<a id="method-Description"></a>

Возвращает текст описания текущей редакции.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Description"></a>

Содержимое локализованного файла описания.

Порядок поиска: description.<locale>.html, description.<lang>.html, description.all.html, description.html в каталоге редакции.
##### **response** : `i` <a id="argument-response-of-Description"></a>

Код завершения.

0 — успех, != 0 — ошибка.
### **License**() -> ([stdout_bytes](#argument-stdout_bytes-of-License) : `ay`, [response](#argument-response-of-License) : `i`)<a id="method-License"></a>

Возвращает текст лицензии текущей редакции.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-License"></a>

Содержимое локализованного файла лицензии.

Поиск в `/usr/share/distro-licenses/<license>`: license.<locale>.html, license.<lang>.html, license.all.html, license.html.
##### **response** : `i` <a id="argument-response-of-License"></a>

Код завершения.

0 — успех, != 0 — ошибка.
### **ReleaseNotes**() -> ([stdout_bytes](#argument-stdout_bytes-of-ReleaseNotes) : `ay`, [response](#argument-response-of-ReleaseNotes) : `i`)<a id="method-ReleaseNotes"></a>

Возвращает релиз-ноты текущей редакции.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-ReleaseNotes"></a>

Содержимое локализованного файла release-notes.

Порядок поиска: release-notes.<locale>.html, release-notes.<lang>.html, release-notes.all.html, release-notes.html в каталоге редакции.
##### **response** : `i` <a id="argument-response-of-ReleaseNotes"></a>

Код завершения.

0 — успех, != 0 — ошибка.
### **FinalNotes**() -> ([stdout_bytes](#argument-stdout_bytes-of-FinalNotes) : `ay`, [response](#argument-response-of-FinalNotes) : `i`)<a id="method-FinalNotes"></a>

Возвращает финальные заметки текущей редакции.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-FinalNotes"></a>

Содержимое локализованного файла final-notes.

Порядок поиска: final-notes.<locale>.html, final-notes.<lang>.html, final-notes.all.html, final-notes.html в каталоге редакции.
##### **response** : `i` <a id="argument-response-of-FinalNotes"></a>

Код завершения.

0 — успех, != 0 — ошибка.
### **Get**() -> ([stdout_strings](#argument-stdout_strings-of-Get) : `as`, [response](#argument-response-of-Get) : `i`)<a id="method-Get"></a>

Возвращает идентификатор текущей редакции.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-Get"></a>

Идентификатор редакции, возвращаемый бэкендом.

Одна строка с именем редакции.
##### **response** : `i` <a id="argument-response-of-Get"></a>

Код завершения.

0 — успех, != 0 — ошибка.
### **Set**([edition_id](#argument-edition_id-of-Set) : `s`) -> ([response](#argument-response-of-Set) : `i`)<a id="method-Set"></a>

Устанавливает текущую редакцию и обновляет системные метаданные.

#### Входные аргументы

##### **edition_id** : `s` <a id="argument-edition_id-of-Set"></a>

Идентификатор редакции для установки.

Должен совпадать с полем name из файла `.edition`.
#### Выходные аргументы

##### **response** : `i` <a id="argument-response-of-Set"></a>

Код завершения.

0 — успех, != 0 — ошибка.


Актуальная спецификация: https://altlinux.space/alterator/alterator-entry/src/branch/master/doc
