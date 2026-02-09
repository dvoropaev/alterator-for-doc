[English](./apt1.md) | [Русский](./apt1.ru_RU.md)

# Interface **org.altlinux.alterator.apt1**

Provides apt backend commands to search, install, reinstall, and update packages with streaming signals for long-running operations.

| Method | Summary |
|--------|---------|
| [Info](#method-Info) | Returns contents of the apt.object descriptor file. |
| [UpdateAsync](#method-UpdateAsync) | Runs apt-get update in the background; progress is delivered through signals. |
| [ApplyAsync](#method-ApplyAsync) | Applies install/remove transaction using generated pkgpriorities. |
| [ReinstallAsync](#method-ReinstallAsync) | Reinstalls packages asynchronously via apt-get reinstall -y -q. |
| [ListAllPackages](#method-ListAllPackages) | Lists all available package names via apt-cache search . --names-only. |
| [Search](#method-Search) | Searches packages by pattern via apt-wrapper search (apt-cache search). |
| [LastUpdate](#method-LastUpdate) | Reports last update time from /var/lib/apt/lists in UTC. |
| [LastDistUpgrade](#method-LastDistUpgrade) | Gets date of the last system update. |
| [CheckApply](#method-CheckApply) | Simulates install/remove transaction and returns planned changes. |
| [CheckReinstall](#method-CheckReinstall) | Simulates reinstall transaction for selected packages. |
| [CheckDistUpgrade](#method-CheckDistUpgrade) | Simulates dist-upgrade and reports planned installs/removals. |
| [DistUpgradeAsync](#method-DistUpgradeAsync) | Performs dist-upgrade asynchronously via apt-get dist-upgrade -y -q. |


| Signal | Summary |
|--------|---------|
| [apt1_update_stderr_signal](#signal-apt1_update_stderr_signal) | stderr stream from apt-get update. |
| [apt1_update_stdout_signal](#signal-apt1_update_stdout_signal) | stdout stream from apt-get update. |
| [apt1_install_stderr_signal](#signal-apt1_install_stderr_signal) | stderr stream from apt-wrapper apply (install/remove). |
| [apt1_install_stdout_signal](#signal-apt1_install_stdout_signal) | stdout stream from apt-wrapper apply (install/remove). |
| [apt1_reinstall_stderr_signal](#signal-apt1_reinstall_stderr_signal) | stderr stream from apt-get reinstall. |
| [apt1_reinstall_stdout_signal](#signal-apt1_reinstall_stdout_signal) | stdout stream from apt-get reinstall. |
| [apt1_remove_stderr_signal](#signal-apt1_remove_stderr_signal) | stderr stream from removal transactions. |
| [apt1_remove_stdout_signal](#signal-apt1_remove_stdout_signal) | stdout stream from removal transactions. |
| [apt1_dist_upgrade_stderr_signal](#signal-apt1_dist_upgrade_stderr_signal) | stderr stream from apt-get dist-upgrade. |
| [apt1_dist_upgrade_stdout_signal](#signal-apt1_dist_upgrade_stdout_signal) | stdout stream from apt-get dist-upgrade. |

## Methods

### **Info**() -> ([stdout_bytes](#argument-stdout_bytes-of-Info) : `ay`, [response](#argument-response-of-Info) : `i`)<a id="method-Info"></a>

Returns contents of the apt.object descriptor file.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Info"></a>

Contents of /usr/share/alterator/objects/apt.object.

TOML description of the object.
##### **response** : `i` <a id="argument-response-of-Info"></a>

Exit code of the cat utility.

0 — success, != 0 — error.
### **UpdateAsync**() -> ([response](#argument-response-of-UpdateAsync) : `i`)<a id="method-UpdateAsync"></a>

Runs apt-get update in the background; progress is delivered through signals.

Signals: apt1_update_stdout_signal, apt1_update_stderr_signals.
#### Output arguments

##### **response** : `i` <a id="argument-response-of-UpdateAsync"></a>

Exit code of apt-get update.

0 — success, != 0 — error.
### **ApplyAsync**([exclude_pkgnames](#argument-exclude_pkgnames-of-ApplyAsync) : `s`, [pkgnames](#argument-pkgnames-of-ApplyAsync) : `s`) -> ([response](#argument-response-of-ApplyAsync) : `i`)<a id="method-ApplyAsync"></a>

Applies install/remove transaction using generated pkgpriorities.

Creates temporary pkgpriorities file (mktemp "${TMPDIR:-/tmp}/alterator-pkgpriorities.XXXXXXXXXXXX"), which lists packages marked as manual by apt via apt-mark showmanual. The file is used by apt to protect manual packages from implicit removal if they are not specified in exclude_pkgnames. The temporary file is removed after the method completes.
Signals: apt1_install_stdout_signal, apt1_install_stderr_signal, apt1_remove_stdout_signal, apt1_remove_stderr_signal.
#### Input arguments

##### **exclude_pkgnames** : `s` <a id="argument-exclude_pkgnames-of-ApplyAsync"></a>

Space-separated package names to exclude from pkgpriorities.

This list allows removing manual packages by dependencies.
##### **pkgnames** : `s` <a id="argument-pkgnames-of-ApplyAsync"></a>

Space-separated package names to process.

Names ending with "-" are removed; others are installed via apt-get install -y -q.
#### Output arguments

##### **response** : `i` <a id="argument-response-of-ApplyAsync"></a>

Exit code of the operation.

0 — success, != 0 — error.
### **ReinstallAsync**([pkgnames](#argument-pkgnames-of-ReinstallAsync) : `s`) -> ([response](#argument-response-of-ReinstallAsync) : `i`)<a id="method-ReinstallAsync"></a>

Reinstalls packages asynchronously via apt-get reinstall -y -q.

#### Input arguments

##### **pkgnames** : `s` <a id="argument-pkgnames-of-ReinstallAsync"></a>

Space-separated package names to reinstall.

#### Output arguments

##### **response** : `i` <a id="argument-response-of-ReinstallAsync"></a>

Exit code of the reinstall command.

0 — success, != 0 — error.
### **ListAllPackages**() -> ([stdout_strings](#argument-stdout_strings-of-ListAllPackages) : `as`, [stderr_strings](#argument-stderr_strings-of-ListAllPackages) : `as`, [response](#argument-response-of-ListAllPackages) : `i`)<a id="method-ListAllPackages"></a>

Lists all available package names via apt-cache search . --names-only.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-ListAllPackages"></a>

Package names produced by apt-cache.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-ListAllPackages"></a>

Errors or warnings from apt-cache.

##### **response** : `i` <a id="argument-response-of-ListAllPackages"></a>

Exit code of the listall helper.

0 — success, != 0 — error.
### **Search**([pattern](#argument-pattern-of-Search) : `s`) -> ([stdout_strings](#argument-stdout_strings-of-Search) : `as`, [stderr_strings](#argument-stderr_strings-of-Search) : `as`, [response](#argument-response-of-Search) : `i`)<a id="method-Search"></a>

Searches packages by pattern via apt-wrapper search (apt-cache search).

#### Input arguments

##### **pattern** : `s` <a id="argument-pattern-of-Search"></a>

Search expression passed to apt-cache search.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-Search"></a>

Matching package names extracted from apt-cache output.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-Search"></a>

Search command errors.

##### **response** : `i` <a id="argument-response-of-Search"></a>

Exit code of the search helper.

0 — success, != 0 — error.
### **LastUpdate**() -> ([stdout_strings](#argument-stdout_strings-of-LastUpdate) : `as`, [stderr_strings](#argument-stderr_strings-of-LastUpdate) : `as`, [response](#argument-response-of-LastUpdate) : `i`)<a id="method-LastUpdate"></a>

Reports last update time from /var/lib/apt/lists in UTC.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-LastUpdate"></a>

Date-time string from the helper (format is not validated).

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-LastUpdate"></a>

Errors during stat or parsing.

##### **response** : `i` <a id="argument-response-of-LastUpdate"></a>

Exit code of the time lookup.

0 — success, != 0 — error.
### **LastDistUpgrade**() -> ([stdout_strings](#argument-stdout_strings-of-LastDistUpgrade) : `as`, [stderr_strings](#argument-stderr_strings-of-LastDistUpgrade) : `as`, [response](#argument-response-of-LastDistUpgrade) : `i`)<a id="method-LastDistUpgrade"></a>

Gets date of the last system update.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-LastDistUpgrade"></a>

Last line with timestamp from dist-upgrades.log.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-LastDistUpgrade"></a>

Errors if the log is missing or unavailable.

##### **response** : `i` <a id="argument-response-of-LastDistUpgrade"></a>

Exit code of the log read.

0 — success, != 0 — error.
### **CheckApply**([pkgnames](#argument-pkgnames-of-CheckApply) : `s`) -> ([stdout_strings](#argument-stdout_strings-of-CheckApply) : `as`, [stderr_strings](#argument-stderr_strings-of-CheckApply) : `as`, [response](#argument-response-of-CheckApply) : `i`)<a id="method-CheckApply"></a>

Simulates install/remove transaction and returns planned changes.

#### Input arguments

##### **pkgnames** : `s` <a id="argument-pkgnames-of-CheckApply"></a>

Space-separated package names for simulation.

Names ending with "-" are treated as removals.
#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-CheckApply"></a>

JSON string with install_packages, remove_packages, extra_remove_packages arrays.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-CheckApply"></a>

Diagnostic output of apt-get --just-print.

##### **response** : `i` <a id="argument-response-of-CheckApply"></a>

Exit code of the simulation.

0 — success, != 0 — error.
### **CheckReinstall**([pkgnames](#argument-pkgnames-of-CheckReinstall) : `s`) -> ([stdout_strings](#argument-stdout_strings-of-CheckReinstall) : `as`, [stderr_strings](#argument-stderr_strings-of-CheckReinstall) : `as`, [response](#argument-response-of-CheckReinstall) : `i`)<a id="method-CheckReinstall"></a>

Simulates reinstall transaction for selected packages.

#### Input arguments

##### **pkgnames** : `s` <a id="argument-pkgnames-of-CheckReinstall"></a>

Space-separated package names for reinstall.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-CheckReinstall"></a>

Packages scheduled for installation from apt-get -s output.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-CheckReinstall"></a>

Packages scheduled for removal or warnings.

##### **response** : `i` <a id="argument-response-of-CheckReinstall"></a>

Exit code of the simulation.

0 — success, != 0 — error.
### **CheckDistUpgrade**() -> ([stdout_strings](#argument-stdout_strings-of-CheckDistUpgrade) : `as`, [stderr_strings](#argument-stderr_strings-of-CheckDistUpgrade) : `as`, [response](#argument-response-of-CheckDistUpgrade) : `i`)<a id="method-CheckDistUpgrade"></a>

Simulates dist-upgrade and reports planned installs/removals.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-CheckDistUpgrade"></a>

Packages planned for installation.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-CheckDistUpgrade"></a>

Packages planned for removal or errors.

##### **response** : `i` <a id="argument-response-of-CheckDistUpgrade"></a>

Exit code of apt-get dist-upgrade -s -q.

0 — success, != 0 — error.
### **DistUpgradeAsync**() -> ([response](#argument-response-of-DistUpgradeAsync) : `i`)<a id="method-DistUpgradeAsync"></a>

Performs dist-upgrade asynchronously via apt-get dist-upgrade -y -q.

#### Output arguments

##### **response** : `i` <a id="argument-response-of-DistUpgradeAsync"></a>

Exit code of the upgrade command.

0 — success, != 0 — error.
## Signals

### **apt1_update_stderr_signal**(`s`)<a id="signal-apt1_update_stderr_signal"></a>

stderr stream from apt-get update.

#### Output arguments

##### Argument `s`

### **apt1_update_stdout_signal**(`s`)<a id="signal-apt1_update_stdout_signal"></a>

stdout stream from apt-get update.

#### Output arguments

##### Argument `s`

### **apt1_install_stderr_signal**(`s`)<a id="signal-apt1_install_stderr_signal"></a>

stderr stream from apt-wrapper apply (install/remove).

#### Output arguments

##### Argument `s`

### **apt1_install_stdout_signal**(`s`)<a id="signal-apt1_install_stdout_signal"></a>

stdout stream from apt-wrapper apply (install/remove).

#### Output arguments

##### Argument `s`

### **apt1_reinstall_stderr_signal**(`s`)<a id="signal-apt1_reinstall_stderr_signal"></a>

stderr stream from apt-get reinstall.

#### Output arguments

##### Argument `s`

### **apt1_reinstall_stdout_signal**(`s`)<a id="signal-apt1_reinstall_stdout_signal"></a>

stdout stream from apt-get reinstall.

#### Output arguments

##### Argument `s`

### **apt1_remove_stderr_signal**(`s`)<a id="signal-apt1_remove_stderr_signal"></a>

stderr stream from removal transactions.

#### Output arguments

##### Argument `s`

### **apt1_remove_stdout_signal**(`s`)<a id="signal-apt1_remove_stdout_signal"></a>

stdout stream from removal transactions.

#### Output arguments

##### Argument `s`

### **apt1_dist_upgrade_stderr_signal**(`s`)<a id="signal-apt1_dist_upgrade_stderr_signal"></a>

stderr stream from apt-get dist-upgrade.

#### Output arguments

##### Argument `s`

### **apt1_dist_upgrade_stdout_signal**(`s`)<a id="signal-apt1_dist_upgrade_stdout_signal"></a>

stdout stream from apt-get dist-upgrade.

#### Output arguments

##### Argument `s`



Current specification: https://altlinux.space/alterator/alterator-entry/src/branch/master/doc
