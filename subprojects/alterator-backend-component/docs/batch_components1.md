[English](./batch_components1.md) | [Русский](./batch_components1.ru_RU.md)

# Interface **org.altlinux.alterator.batch_components1**

Provides batch access to component descriptors and installation status.

| Method | Summary |
|--------|---------|
| [Info](#method-Info) | Returns .component descriptors for all components. |
| [Status](#method-Status) | Returns packages that are not installed for all components. |


## Methods

### **Info**() -> ([stdout_string_array](#argument-stdout_string_array-of-Info) : `as`, [response](#argument-response-of-Info) : `i`)<a id="method-Info"></a>

Returns .component descriptors for all components.

#### Output arguments

##### **stdout_string_array** : `as` <a id="argument-stdout_string_array-of-Info"></a>

Array of .component file contents from /usr/share/alterator/components.

##### **response** : `i` <a id="argument-response-of-Info"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **Status**() -> ([stdout_bytes](#argument-stdout_bytes-of-Status) : `ay`, [response](#argument-response-of-Status) : `i`)<a id="method-Status"></a>

Returns packages that are not installed for all components.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Status"></a>

Newline-separated package names from rpm -q output (includes packages filtered by arch, language, or desktop).

##### **response** : `i` <a id="argument-response-of-Status"></a>

Exit code of the helper.

0 — success, != 0 — error.


Current specification: https://altlinux.space/alterator/alterator-entry/src/branch/master/doc
