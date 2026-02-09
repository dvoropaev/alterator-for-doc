[English](./rpm1.md) | [Русский](./rpm1.ru_RU.md)

# Interface **org.altlinux.alterator.rpm1**

Expose rpm backend commands for listing, installing, and removing RPM packages.

| Method | Summary |
|--------|---------|
| [Info](#method-Info) | Return static descriptor of the rpm backend object. |
| [List](#method-List) | List installed packages via rpm -qa with name, version, release, arch, and group. |
| [Install](#method-Install) | Install or upgrade a package file via rpm -U. |
| [Remove](#method-Remove) | Erase an installed package via rpm -e. |
| [PackageInfo](#method-PackageInfo) | Show package metadata via rpm -qi. |
| [Files](#method-Files) | List files installed by a package via rpm -ql. |


## Methods

### **Info**() -> ([stdout_bytes](#argument-stdout_bytes-of-Info) : `ay`, [response](#argument-response-of-Info) : `i`)<a id="method-Info"></a>

Return static descriptor of the rpm backend object.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Info"></a>

Contents of /usr/share/alterator/objects/rpm.object.

TOML object definition with display_name and comments.
##### **response** : `i` <a id="argument-response-of-Info"></a>

Exit code of the cat helper.

0 — success, != 0 — error.
### **List**() -> ([stdout_strings](#argument-stdout_strings-of-List) : `as`, [stderr_strings](#argument-stderr_strings-of-List) : `as`, [response](#argument-response-of-List) : `i`)<a id="method-List"></a>

List installed packages via rpm -qa with name, version, release, arch, and group.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-List"></a>

Package entries formatted as “name version release arch group”.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-List"></a>

Errors from rpm -qa.

##### **response** : `i` <a id="argument-response-of-List"></a>

Exit code of the list command.

0 — success, != 0 — error.
### **Install**([pkgpath](#argument-pkgpath-of-Install) : `s`) -> ([stderr_strings](#argument-stderr_strings-of-Install) : `as`, [response](#argument-response-of-Install) : `i`)<a id="method-Install"></a>

Install or upgrade a package file via rpm -U.

#### Input arguments

##### **pkgpath** : `s` <a id="argument-pkgpath-of-Install"></a>

Filesystem path to the RPM file.

#### Output arguments

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-Install"></a>

rpm installation errors.

##### **response** : `i` <a id="argument-response-of-Install"></a>

Exit code of rpm -U.

0 — success, != 0 — error.
### **Remove**([pkgname](#argument-pkgname-of-Remove) : `s`) -> ([stderr_strings](#argument-stderr_strings-of-Remove) : `as`, [response](#argument-response-of-Remove) : `i`)<a id="method-Remove"></a>

Erase an installed package via rpm -e.

#### Input arguments

##### **pkgname** : `s` <a id="argument-pkgname-of-Remove"></a>

Name of the package to remove.

#### Output arguments

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-Remove"></a>

rpm removal errors.

##### **response** : `i` <a id="argument-response-of-Remove"></a>

Exit code of rpm -e.

0 — success, != 0 — error.
### **PackageInfo**([pkgname](#argument-pkgname-of-PackageInfo) : `s`) -> ([stdout_strings](#argument-stdout_strings-of-PackageInfo) : `as`, [stderr_strings](#argument-stderr_strings-of-PackageInfo) : `as`, [response](#argument-response-of-PackageInfo) : `i`)<a id="method-PackageInfo"></a>

Show package metadata via rpm -qi.

#### Input arguments

##### **pkgname** : `s` <a id="argument-pkgname-of-PackageInfo"></a>

Name of the installed package to query.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-PackageInfo"></a>

rpm -qi output split into lines.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-PackageInfo"></a>

rpm query errors.

##### **response** : `i` <a id="argument-response-of-PackageInfo"></a>

Exit code of the query.

0 — success, != 0 — error.
### **Files**([pkgname](#argument-pkgname-of-Files) : `s`) -> ([stdout_strings](#argument-stdout_strings-of-Files) : `as`, [stderr_strings](#argument-stderr_strings-of-Files) : `as`, [response](#argument-response-of-Files) : `i`)<a id="method-Files"></a>

List files installed by a package via rpm -ql.

#### Input arguments

##### **pkgname** : `s` <a id="argument-pkgname-of-Files"></a>

Name of the installed package.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-Files"></a>

File paths reported by rpm -ql.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-Files"></a>

rpm errors while reading the package database.

##### **response** : `i` <a id="argument-response-of-Files"></a>

Exit code of the file listing.

0 — success, != 0 — error.


Current specification: https://altlinux.space/alterator/alterator-entry/src/branch/master/doc
