[English](./edition.md) | [Русский](./edition.ru_RU.md)

# Интерфейс **org.altlinux.alterator.edition1**

Предоставляет методы бэкенда для выбранной редакции продукта; возвращает метаданные и тексты.

| Метод | Описание |
|--------|---------|
| [Info](#method-Info) | Возвращает файл-описатель `.edition` для запрошенной редакции. |
| [Description](#method-Description) | Возвращает текст описания редакции. |
| [License](#method-License) | Возвращает текст лицензии для редакции. |
| [ReleaseNotes](#method-ReleaseNotes) | Возвращает релиз-ноты для редакции. |
| [FinalNotes](#method-FinalNotes) | Возвращает финальные заметки для редакции. |


## Методы

### **Info**() -> ([stdout_bytes](#argument-stdout_bytes-of-Info) : `ay`, [response](#argument-response-of-Info) : `i`)<a id="method-Info"></a>

Возвращает файл-описатель `.edition` для запрошенной редакции.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Info"></a>

Содержимое файла `.edition`.

TOML-текст с полями name, display_name, license.
##### **response** : `i` <a id="argument-response-of-Info"></a>

Код завершения.

0 — успех, != 0 — ошибка.
### **Description**() -> ([stdout_bytes](#argument-stdout_bytes-of-Description) : `ay`, [response](#argument-response-of-Description) : `i`)<a id="method-Description"></a>

Возвращает текст описания редакции.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Description"></a>

Содержимое локализованного файла описания.

Порядок поиска: description.<locale>.html, description.<lang>.html, description.all.html, description.html в каталоге редакции.
##### **response** : `i` <a id="argument-response-of-Description"></a>

Код завершения.

0 — успех, != 0 — ошибка.
### **License**() -> ([stdout_bytes](#argument-stdout_bytes-of-License) : `ay`, [response](#argument-response-of-License) : `i`)<a id="method-License"></a>

Возвращает текст лицензии для редакции.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-License"></a>

Содержимое локализованного файла лицензии.

Поиск в `/usr/share/distro-licenses/<license>`: license.<locale>.html, license.<lang>.html, license.all.html, license.html.
##### **response** : `i` <a id="argument-response-of-License"></a>

Код завершения.

0 — успех, != 0 — ошибка.
### **ReleaseNotes**() -> ([stdout_bytes](#argument-stdout_bytes-of-ReleaseNotes) : `ay`, [response](#argument-response-of-ReleaseNotes) : `i`)<a id="method-ReleaseNotes"></a>

Возвращает релиз-ноты для редакции.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-ReleaseNotes"></a>

Содержимое локализованного файла release-notes.

Порядок поиска: release-notes.<locale>.html, release-notes.<lang>.html, release-notes.all.html, release-notes.html в каталоге редакции.
##### **response** : `i` <a id="argument-response-of-ReleaseNotes"></a>

Код завершения.

0 — успех, != 0 — ошибка.
### **FinalNotes**() -> ([stdout_bytes](#argument-stdout_bytes-of-FinalNotes) : `ay`, [response](#argument-response-of-FinalNotes) : `i`)<a id="method-FinalNotes"></a>

Возвращает финальные заметки для редакции.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-FinalNotes"></a>

Содержимое локализованного файла final-notes.

Порядок поиска: final-notes.<locale>.html, final-notes.<lang>.html, final-notes.all.html, final-notes.html в каталоге редакции.
##### **response** : `i` <a id="argument-response-of-FinalNotes"></a>

Код завершения.

0 — успех, != 0 — ошибка.


Актуальная спецификация: https://altlinux.space/alterator/alterator-entry/src/branch/master/doc
