[English](./batch_components1.md) | [Русский](./batch_components1.ru_RU.md)

# Интерфейс **org.altlinux.alterator.batch_components1**

Предоставляет пакетный доступ к описателям компонентов и статусу установки.

| Метод | Описание |
|--------|---------|
| [Info](#method-Info) | Возвращает описатели .component для всех компонентов. |
| [Status](#method-Status) | Возвращает пакеты, которые не установлены для всех компонентов. |


## Методы

### **Info**() -> ([stdout_string_array](#argument-stdout_string_array-of-Info) : `as`, [response](#argument-response-of-Info) : `i`)<a id="method-Info"></a>

Возвращает описатели .component для всех компонентов.

#### Выходные аргументы

##### **stdout_string_array** : `as` <a id="argument-stdout_string_array-of-Info"></a>

Массив содержимого файлов .component из `/usr/share/alterator/components`.

##### **response** : `i` <a id="argument-response-of-Info"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **Status**() -> ([stdout_bytes](#argument-stdout_bytes-of-Status) : `ay`, [response](#argument-response-of-Status) : `i`)<a id="method-Status"></a>

Возвращает пакеты, которые не установлены для всех компонентов.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Status"></a>

Имена пакетов, разделённые переводом строки, из `rpm -q` (включая пакеты, отфильтрованные по архитектуре, языку или окружению рабочего стола).

##### **response** : `i` <a id="argument-response-of-Status"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.


Актуальная спецификация: https://altlinux.space/alterator/alterator-entry/src/branch/master/doc
