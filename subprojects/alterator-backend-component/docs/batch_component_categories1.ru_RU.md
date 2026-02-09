[English](./batch_component_categories1.md) | [Русский](./batch_component_categories1.ru_RU.md)

# Интерфейс **org.altlinux.alterator.batch_component_categories1**

Предоставляет пакетный доступ к описателям категорий компонентов.

| Метод | Описание |
|--------|---------|
| [Info](#method-Info) | Возвращает описатели .category для всех категорий компонентов. |


## Методы

### **Info**() -> ([stdout_string_array](#argument-stdout_string_array-of-Info) : `as`, [response](#argument-response-of-Info) : `i`)<a id="method-Info"></a>

Возвращает описатели .category для всех категорий компонентов.

#### Выходные аргументы

##### **stdout_string_array** : `as` <a id="argument-stdout_string_array-of-Info"></a>

Массив содержимого файлов .category из `/usr/share/alterator/components/categories`.

##### **response** : `i` <a id="argument-response-of-Info"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.


Актуальная спецификация: https://altlinux.space/alterator/alterator-entry/src/branch/master/doc
