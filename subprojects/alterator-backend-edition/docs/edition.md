[English](./edition.md) | [Русский](./edition.ru_RU.md)

# Interface **org.altlinux.alterator.edition1**

Exposes edition-specific backend methods that return metadata and texts for a selected product edition.

| Method | Summary |
|--------|---------|
| [Info](#method-Info) | Return the .edition descriptor for the requested edition. |
| [Description](#method-Description) | Return the description text for the edition. |
| [License](#method-License) | Return the license text for the edition. |
| [ReleaseNotes](#method-ReleaseNotes) | Return release notes for the edition. |
| [FinalNotes](#method-FinalNotes) | Return final notes for the edition. |


## Methods

### **Info**() -> ([stdout_bytes](#argument-stdout_bytes-of-Info) : `ay`, [response](#argument-response-of-Info) : `i`)<a id="method-Info"></a>

Return the .edition descriptor for the requested edition.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Info"></a>

Contents of the .edition file.

TOML text with fields like name, display_name, license.
##### **response** : `i` <a id="argument-response-of-Info"></a>

Exit code.

0 — success, != 0 — error.
### **Description**() -> ([stdout_bytes](#argument-stdout_bytes-of-Description) : `ay`, [response](#argument-response-of-Description) : `i`)<a id="method-Description"></a>

Return the description text for the edition.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Description"></a>

Localized description file content.

Lookup order: description.<locale>.html, description.<lang>.html, description.all.html, description.html in the edition directory.
##### **response** : `i` <a id="argument-response-of-Description"></a>

Exit code.

0 — success, != 0 — error.
### **License**() -> ([stdout_bytes](#argument-stdout_bytes-of-License) : `ay`, [response](#argument-response-of-License) : `i`)<a id="method-License"></a>

Return the license text for the edition.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-License"></a>

Localized license file content.

Lookup in /usr/share/distro-licenses/<license> using license.<locale>.html, license.<lang>.html, license.all.html, license.html.
##### **response** : `i` <a id="argument-response-of-License"></a>

Exit code.

0 — success, != 0 — error.
### **ReleaseNotes**() -> ([stdout_bytes](#argument-stdout_bytes-of-ReleaseNotes) : `ay`, [response](#argument-response-of-ReleaseNotes) : `i`)<a id="method-ReleaseNotes"></a>

Return release notes for the edition.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-ReleaseNotes"></a>

Localized release-notes file content.

Lookup order: release-notes.<locale>.html, release-notes.<lang>.html, release-notes.all.html, release-notes.html in the edition directory.
##### **response** : `i` <a id="argument-response-of-ReleaseNotes"></a>

Exit code.

0 — success, != 0 — error.
### **FinalNotes**() -> ([stdout_bytes](#argument-stdout_bytes-of-FinalNotes) : `ay`, [response](#argument-response-of-FinalNotes) : `i`)<a id="method-FinalNotes"></a>

Return final notes for the edition.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-FinalNotes"></a>

Localized final-notes file content.

Lookup order: final-notes.<locale>.html, final-notes.<lang>.html, final-notes.all.html, final-notes.html in the edition directory.
##### **response** : `i` <a id="argument-response-of-FinalNotes"></a>

Exit code.

0 — success, != 0 — error.


Current specification: https://altlinux.space/alterator/alterator-entry/src/branch/master/doc
