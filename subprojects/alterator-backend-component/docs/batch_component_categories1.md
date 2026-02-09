[English](./batch_component_categories1.md) | [Русский](./batch_component_categories1.ru_RU.md)

# Interface **org.altlinux.alterator.batch_component_categories1**

Provides batch access to component category descriptors.

| Method | Summary |
|--------|---------|
| [Info](#method-Info) | Returns .category descriptors for all component categories. |


## Methods

### **Info**() -> ([stdout_string_array](#argument-stdout_string_array-of-Info) : `as`, [response](#argument-response-of-Info) : `i`)<a id="method-Info"></a>

Returns .category descriptors for all component categories.

#### Output arguments

##### **stdout_string_array** : `as` <a id="argument-stdout_string_array-of-Info"></a>

Array of .category file contents from /usr/share/alterator/components/categories.

##### **response** : `i` <a id="argument-response-of-Info"></a>

Exit code of the helper.

0 — success, != 0 — error.


Current specification: https://altlinux.space/alterator/alterator-entry/src/branch/master/doc
