[English](./component1.md) | [Русский](./component1.ru_RU.md)

# Интерфейс **org.altlinux.alterator.component1**

Предоставляет доступ к описателю, описанию и статусу установки одного компонента.

| Метод | Описание |
|--------|---------|
| [Info](#method-Info) | Возвращает описатель текущего компонента в формате .component. |
| [Description](#method-Description) | Возвращает локализованное описание компонента. |
| [Status](#method-Status) | Возвращает установленные пакеты, которые предоставляют пакеты компонента. |


## Методы

### **Info**() -> ([stdout_bytes](#argument-stdout_bytes-of-Info) : `ay`, [response](#argument-response-of-Info) : `i`)<a id="method-Info"></a>

Возвращает описатель текущего компонента в формате .component.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Info"></a>

Содержимое `/usr/share/alterator/components/<name>/<name>.component`.

##### **response** : `i` <a id="argument-response-of-Info"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **Description**() -> ([stdout_bytes](#argument-stdout_bytes-of-Description) : `ay`, [response](#argument-response-of-Description) : `i`)<a id="method-Description"></a>

Возвращает локализованное описание компонента.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Description"></a>

Содержимое `description.<locale>.html`, `description.<lang>.html` или `description.html` (выбор по LC_ALL, по умолчанию en_US).

##### **response** : `i` <a id="argument-response-of-Description"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **Status**() -> ([stdout_strings](#argument-stdout_strings-of-Status) : `as`, [response](#argument-response-of-Status) : `i`)<a id="method-Status"></a>

Возвращает установленные пакеты, которые предоставляют пакеты компонента.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-Status"></a>

Имена пакетов из `rpm -q --whatprovides`.

##### **response** : `i` <a id="argument-response-of-Status"></a>

Код завершения `rpm -q --whatprovides`.

0 — успех, != 0 — ошибка или компонент не установлен.


Актуальная спецификация: https://altlinux.space/alterator/alterator-entry/src/branch/master/doc
