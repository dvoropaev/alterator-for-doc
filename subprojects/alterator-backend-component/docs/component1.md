[English](./component1.md) | [Русский](./component1.ru_RU.md)

# Interface **org.altlinux.alterator.component1**

Provides access to a single component descriptor, description, and installation status.

| Method | Summary |
|--------|---------|
| [Info](#method-Info) | Returns the .component descriptor of the current component. |
| [Description](#method-Description) | Returns the localized component description. |
| [DescriptionRaw](#method-DescriptionRaw) | Returns the localized component description in raw format. |
| [Status](#method-Status) | Returns installed packages that provide the component packages. |


## Methods

### **Info**() -> ([stdout_bytes](#argument-stdout_bytes-of-Info) : `ay`, [response](#argument-response-of-Info) : `i`)<a id="method-Info"></a>

Returns the .component descriptor of the current component.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Info"></a>

Contents of /usr/share/alterator/components/<name>/<name>.component.

##### **response** : `i` <a id="argument-response-of-Info"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **Description**() -> ([stdout_bytes](#argument-stdout_bytes-of-Description) : `ay`, [response](#argument-response-of-Description) : `i`)<a id="method-Description"></a>

Returns the localized component description.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Description"></a>

Contents of description.<locale>.html, description.<lang>.html, or description.html (selected by LC_ALL, default en_US).

##### **response** : `i` <a id="argument-response-of-Description"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **DescriptionRaw**() -> ([stdout_bytes](#argument-stdout_bytes-of-DescriptionRaw) : `ay`, [response](#argument-response-of-DescriptionRaw) : `i`)<a id="method-DescriptionRaw"></a>

Returns the localized component description in raw format.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-DescriptionRaw"></a>

Contents of description.<locale>.md, description.<lang>.md, or description.md (selected by LC_ALL, default en_US).

##### **response** : `i` <a id="argument-response-of-DescriptionRaw"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **Status**() -> ([stdout_strings](#argument-stdout_strings-of-Status) : `as`, [response](#argument-response-of-Status) : `i`)<a id="method-Status"></a>

Returns installed packages that provide the component packages.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-Status"></a>

Package names from rpm -q --whatprovides.

##### **response** : `i` <a id="argument-response-of-Status"></a>

Exit code of rpm -q --whatprovides.

0 — success, != 0 — error or component not installed.


Current specification: https://altlinux.space/alterator/alterator-entry/src/branch/master/doc
