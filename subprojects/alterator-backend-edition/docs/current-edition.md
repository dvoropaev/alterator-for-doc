[English](./current-edition.md) | [Русский](./current-edition.ru_RU.md)

# Interface **org.altlinux.alterator.current-edition1**

Exposes backend methods for reading and updating the current product edition.

| Method | Summary |
|--------|---------|
| [Info](#method-Info) | Return the .edition descriptor of the current edition. |
| [Description](#method-Description) | Return the description text for the current edition. |
| [License](#method-License) | Return the license text for the current edition. |
| [ReleaseNotes](#method-ReleaseNotes) | Return release notes for the current edition. |
| [FinalNotes](#method-FinalNotes) | Return final notes for the current edition. |
| [Get](#method-Get) | Return identifier of the current edition. |
| [Set](#method-Set) | Set the current edition and update system metadata. |


## Methods

### **Info**() -> ([stdout_bytes](#argument-stdout_bytes-of-Info) : `ay`, [response](#argument-response-of-Info) : `i`)<a id="method-Info"></a>

Return the .edition descriptor of the current edition.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Info"></a>

Contents of the current .edition file.

TOML text with fields like name, display_name, license.
##### **response** : `i` <a id="argument-response-of-Info"></a>

Exit code.

0 — success, != 0 — error.
### **Description**() -> ([stdout_bytes](#argument-stdout_bytes-of-Description) : `ay`, [response](#argument-response-of-Description) : `i`)<a id="method-Description"></a>

Return the description text for the current edition.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Description"></a>

Localized description file content.

Lookup order: description.<locale>.html, description.<lang>.html, description.all.html, description.html in the edition directory.
##### **response** : `i` <a id="argument-response-of-Description"></a>

Exit code.

0 — success, != 0 — error.
### **License**() -> ([stdout_bytes](#argument-stdout_bytes-of-License) : `ay`, [response](#argument-response-of-License) : `i`)<a id="method-License"></a>

Return the license text for the current edition.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-License"></a>

Localized license file content.

Lookup in /usr/share/distro-licenses/<license> using license.<locale>.html, license.<lang>.html, license.all.html, license.html.
##### **response** : `i` <a id="argument-response-of-License"></a>

Exit code.

0 — success, != 0 — error.
### **ReleaseNotes**() -> ([stdout_bytes](#argument-stdout_bytes-of-ReleaseNotes) : `ay`, [response](#argument-response-of-ReleaseNotes) : `i`)<a id="method-ReleaseNotes"></a>

Return release notes for the current edition.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-ReleaseNotes"></a>

Localized release-notes file content.

Lookup order: release-notes.<locale>.html, release-notes.<lang>.html, release-notes.all.html, release-notes.html in the edition directory.
##### **response** : `i` <a id="argument-response-of-ReleaseNotes"></a>

Exit code.

0 — success, != 0 — error.
### **FinalNotes**() -> ([stdout_bytes](#argument-stdout_bytes-of-FinalNotes) : `ay`, [response](#argument-response-of-FinalNotes) : `i`)<a id="method-FinalNotes"></a>

Return final notes for the current edition.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-FinalNotes"></a>

Localized final-notes file content.

Lookup order: final-notes.<locale>.html, final-notes.<lang>.html, final-notes.all.html, final-notes.html in the edition directory.
##### **response** : `i` <a id="argument-response-of-FinalNotes"></a>

Exit code.

0 — success, != 0 — error.
### **Get**() -> ([stdout_strings](#argument-stdout_strings-of-Get) : `as`, [response](#argument-response-of-Get) : `i`)<a id="method-Get"></a>

Return identifier of the current edition.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-Get"></a>

Edition identifier reported by the backend.

Single string with the edition name.
##### **response** : `i` <a id="argument-response-of-Get"></a>

Exit code.

0 — success, != 0 — error.
### **Set**([edition_id](#argument-edition_id-of-Set) : `s`) -> ([response](#argument-response-of-Set) : `i`)<a id="method-Set"></a>

Set the current edition and update system metadata.

#### Input arguments

##### **edition_id** : `s` <a id="argument-edition_id-of-Set"></a>

Edition identifier to set.

Must match the name field from a .edition file.
#### Output arguments

##### **response** : `i` <a id="argument-response-of-Set"></a>

Exit code.

0 — success, != 0 — error.


Current specification: https://altlinux.space/alterator/alterator-entry/src/branch/master/doc
