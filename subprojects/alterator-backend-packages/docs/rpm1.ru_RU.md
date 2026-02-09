[English](./rpm1.md) | [Русский](./rpm1.ru_RU.md)

# Интерфейс **org.altlinux.alterator.rpm1**

Предоставляет команды бэкенда rpm для перечисления, установки и удаления RPM-пакетов.

| Метод | Описание |
|--------|---------|
| [Info](#method-Info) | Возвращает статический описатель объекта бэкенда rpm. |
| [List](#method-List) | Перечисляет установленные пакеты через `rpm -qa` с именем, версией, релизом, архитектурой и группой. |
| [Install](#method-Install) | Устанавливает или обновляет пакетный файл через `rpm -U`. |
| [Remove](#method-Remove) | Удаляет установленный пакет через `rpm -e`. |
| [PackageInfo](#method-PackageInfo) | Показывает метаданные пакета через `rpm -qi`. |
| [Files](#method-Files) | Перечисляет файлы, установленные пакетом, через `rpm -ql`. |


## Методы

### **Info**() -> ([stdout_bytes](#argument-stdout_bytes-of-Info) : `ay`, [response](#argument-response-of-Info) : `i`)<a id="method-Info"></a>

Возвращает статический описатель объекта бэкенда rpm.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Info"></a>

Содержимое `/usr/share/alterator/objects/rpm.object`.

TOML-описание объекта с display_name и comments.
##### **response** : `i` <a id="argument-response-of-Info"></a>

Код завершения помощника cat.

0 — успех, != 0 — ошибка.
### **List**() -> ([stdout_strings](#argument-stdout_strings-of-List) : `as`, [stderr_strings](#argument-stderr_strings-of-List) : `as`, [response](#argument-response-of-List) : `i`)<a id="method-List"></a>

Перечисляет установленные пакеты через `rpm -qa` с именем, версией, релизом, архитектурой и группой.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-List"></a>

Записи пакетов в формате “name version release arch group”.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-List"></a>

Ошибки выполнения `rpm -qa`.

##### **response** : `i` <a id="argument-response-of-List"></a>

Код завершения команды списка.

0 — успех, != 0 — ошибка.
### **Install**([pkgpath](#argument-pkgpath-of-Install) : `s`) -> ([stderr_strings](#argument-stderr_strings-of-Install) : `as`, [response](#argument-response-of-Install) : `i`)<a id="method-Install"></a>

Устанавливает или обновляет пакетный файл через `rpm -U`.

#### Входные аргументы

##### **pkgpath** : `s` <a id="argument-pkgpath-of-Install"></a>

Путь к RPM-файлу в файловой системе.

#### Выходные аргументы

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-Install"></a>

Ошибки установки rpm.

##### **response** : `i` <a id="argument-response-of-Install"></a>

Код завершения `rpm -U`.

0 — успех, != 0 — ошибка.
### **Remove**([pkgname](#argument-pkgname-of-Remove) : `s`) -> ([stderr_strings](#argument-stderr_strings-of-Remove) : `as`, [response](#argument-response-of-Remove) : `i`)<a id="method-Remove"></a>

Удаляет установленный пакет через `rpm -e`.

#### Входные аргументы

##### **pkgname** : `s` <a id="argument-pkgname-of-Remove"></a>

Имя пакета для удаления.

#### Выходные аргументы

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-Remove"></a>

Ошибки удаления rpm.

##### **response** : `i` <a id="argument-response-of-Remove"></a>

Код завершения `rpm -e`.

0 — успех, != 0 — ошибка.
### **PackageInfo**([pkgname](#argument-pkgname-of-PackageInfo) : `s`) -> ([stdout_strings](#argument-stdout_strings-of-PackageInfo) : `as`, [stderr_strings](#argument-stderr_strings-of-PackageInfo) : `as`, [response](#argument-response-of-PackageInfo) : `i`)<a id="method-PackageInfo"></a>

Показывает метаданные пакета через `rpm -qi`.

#### Входные аргументы

##### **pkgname** : `s` <a id="argument-pkgname-of-PackageInfo"></a>

Имя установленного пакета для запроса.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-PackageInfo"></a>

Вывод `rpm -qi`, разбитый на строки.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-PackageInfo"></a>

Ошибки запроса rpm.

##### **response** : `i` <a id="argument-response-of-PackageInfo"></a>

Код завершения запроса.

0 — успех, != 0 — ошибка.
### **Files**([pkgname](#argument-pkgname-of-Files) : `s`) -> ([stdout_strings](#argument-stdout_strings-of-Files) : `as`, [stderr_strings](#argument-stderr_strings-of-Files) : `as`, [response](#argument-response-of-Files) : `i`)<a id="method-Files"></a>

Перечисляет файлы, установленные пакетом, через `rpm -ql`.

#### Входные аргументы

##### **pkgname** : `s` <a id="argument-pkgname-of-Files"></a>

Имя установленного пакета.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-Files"></a>

Пути файлов, возвращаемые `rpm -ql`.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-Files"></a>

Ошибки rpm при чтении базы пакетов.

##### **response** : `i` <a id="argument-response-of-Files"></a>

Код завершения выдачи списка файлов.

0 — успех, != 0 — ошибка.


Актуальная спецификация: https://altlinux.space/alterator/alterator-entry/src/branch/master/doc
