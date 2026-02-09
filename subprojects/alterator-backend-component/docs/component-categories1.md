[English](./component-categories1.md) | [Русский](./component-categories1.ru_RU.md)

# Interface **org.altlinux.alterator.component-categories1**

Provides access to component category descriptors, descriptions, and list of categories.

| Method | Summary |
|--------|---------|
| [Info](#method-Info) | Returns the .category descriptor of the specified category. |
| [Description](#method-Description) | Returns the localized category description. |
| [List](#method-List) | Lists component categories. |


## Methods

### **Info**([name](#argument-name-of-Info) : `s`) -> ([stdout_bytes](#argument-stdout_bytes-of-Info) : `ay`, [response](#argument-response-of-Info) : `i`)<a id="method-Info"></a>

Returns the .category descriptor of the specified category.

#### Input arguments

##### **name** : `s` <a id="argument-name-of-Info"></a>

Category name.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Info"></a>

Contents of /usr/share/alterator/components/categories/<name>/<name>.category.

##### **response** : `i` <a id="argument-response-of-Info"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **Description**([name](#argument-name-of-Description) : `s`) -> ([stdout_bytes](#argument-stdout_bytes-of-Description) : `ay`, [response](#argument-response-of-Description) : `i`)<a id="method-Description"></a>

Returns the localized category description.

#### Input arguments

##### **name** : `s` <a id="argument-name-of-Description"></a>

Category name.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Description"></a>

Contents of description.<locale>.html, description.<lang>.html, or description.html (selected by LC_ALL, default en_US).

##### **response** : `i` <a id="argument-response-of-Description"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **List**() -> ([stdout_strings](#argument-stdout_strings-of-List) : `as`, [response](#argument-response-of-List) : `i`)<a id="method-List"></a>

Lists component categories.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-List"></a>

Category names.

##### **response** : `i` <a id="argument-response-of-List"></a>

Exit code of the helper.

0 — success, != 0 — error.


Current specification: https://altlinux.space/alterator/alterator-entry/src/branch/master/doc
