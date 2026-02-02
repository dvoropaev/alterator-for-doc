[English](./repo1.md) | [Русский](./repo1.ru_RU.md)

# Interface **org.altlinux.alterator.repo1**

Expose repository management commands backed by apt-repo.

| Method | Summary |
|--------|---------|
| [Info](#method-Info) | Return static descriptor of the repo backend object. |
| [List](#method-List) | List configured repositories via apt-repo list. |
| [Add](#method-Add) | Add repository source via apt-repo add. |
| [Remove](#method-Remove) | Remove repository source via apt-repo rm. |


## Methods

### **Info**() -> ([stdout_bytes](#argument-stdout_bytes-of-Info) : `ay`, [response](#argument-response-of-Info) : `i`)<a id="method-Info"></a>

Return static descriptor of the repo backend object.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Info"></a>

Contents of /usr/share/alterator/objects/repo.object.

TOML object definition with display_name and comments.
##### **response** : `i` <a id="argument-response-of-Info"></a>

Exit code of the cat helper.

0 — success, != 0 — error.
### **List**() -> ([stdout_strings](#argument-stdout_strings-of-List) : `as`, [stderr_strings](#argument-stderr_strings-of-List) : `as`, [response](#argument-response-of-List) : `i`)<a id="method-List"></a>

List configured repositories via apt-repo list.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-List"></a>

Repository entries printed by apt-repo list.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-List"></a>

Errors from apt-repo list.

##### **response** : `i` <a id="argument-response-of-List"></a>

Exit code of the list command.

0 — success, != 0 — error.
### **Add**([source](#argument-source-of-Add) : `s`) -> ([stderr_strings](#argument-stderr_strings-of-Add) : `as`, [response](#argument-response-of-Add) : `i`)<a id="method-Add"></a>

Add repository source via apt-repo add.

#### Input arguments

##### **source** : `s` <a id="argument-source-of-Add"></a>

Source definition passed directly to apt-repo add.

Supports file paths or URI strings accepted by apt-repo.
#### Output arguments

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-Add"></a>

apt-repo warnings or errors.

##### **response** : `i` <a id="argument-response-of-Add"></a>

Exit code of the add command.

0 — success, != 0 — error.
### **Remove**([source](#argument-source-of-Remove) : `s`) -> ([stderr_strings](#argument-stderr_strings-of-Remove) : `as`, [response](#argument-response-of-Remove) : `i`)<a id="method-Remove"></a>

Remove repository source via apt-repo rm.

#### Input arguments

##### **source** : `s` <a id="argument-source-of-Remove"></a>

Identifier or path of the repository to drop.

#### Output arguments

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-Remove"></a>

apt-repo errors during removal.

##### **response** : `i` <a id="argument-response-of-Remove"></a>

Exit code of the remove command.

0 — success, != 0 — error.
