[English](./apt1.md) | [Русский](./apt1.ru_RU.md)

# Interface **org.altlinux.alterator.apt1**

Expose apt backend commands for package search, install, reinstall, and upgrade with streaming signals for long-running operations.

| Method | Summary |
|--------|---------|
| [Info](#method-Info) | Return static descriptor of the apt backend object. |
| [UpdateAsync](#method-UpdateAsync) | Run apt-get update in background; progress is emitted through signals. |
| [ApplyAsync](#method-ApplyAsync) | Apply install/remove transaction via apt-wrapper apply using generated pkgpriorities. |
| [ReinstallAsync](#method-ReinstallAsync) | Reinstall packages asynchronously via apt-get reinstall -y -q. |
| [ListAllPackages](#method-ListAllPackages) | List all available package names using apt-cache search . --names-only. |
| [Search](#method-Search) | Search packages by pattern via apt-wrapper search (apt-cache search). |
| [LastUpdate](#method-LastUpdate) | Report last update timestamp for /var/lib/apt/lists in UTC. |
| [LastDistUpgrade](#method-LastDistUpgrade) | Return last dist-upgrade log entry from /var/log/alterator/apt/dist-upgrades.log. |
| [CheckApply](#method-CheckApply) | Simulate install/remove transaction and return planned changes. |
| [CheckReinstall](#method-CheckReinstall) | Simulate reinstall transaction for selected packages. |
| [CheckDistUpgrade](#method-CheckDistUpgrade) | Simulate dist-upgrade and report planned installs/removals. |
| [DistUpgradeAsync](#method-DistUpgradeAsync) | Perform dist-upgrade asynchronously via apt-get dist-upgrade -y -q. |


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

Return static descriptor of the apt backend object.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Info"></a>

Contents of /usr/share/alterator/objects/apt.object.

TOML object definition with display_name and comments.
##### **response** : `i` <a id="argument-response-of-Info"></a>

Exit code of the cat helper.

0 — success, != 0 — error.
### **UpdateAsync**() -> ([response](#argument-response-of-UpdateAsync) : `i`)<a id="method-UpdateAsync"></a>

Run apt-get update in background; progress is emitted through signals.

Signals emitted: apt1_update_stdout_signal, apt1_update_stderr_signals.
#### Output arguments

##### **response** : `i` <a id="argument-response-of-UpdateAsync"></a>

Exit code of apt-get update.

0 — success, != 0 — error.
### **ApplyAsync**([exclude_pkgnames](#argument-exclude_pkgnames-of-ApplyAsync) : `s`, [pkgnames](#argument-pkgnames-of-ApplyAsync) : `s`) -> ([response](#argument-response-of-ApplyAsync) : `i`)<a id="method-ApplyAsync"></a>

Apply install/remove transaction via apt-wrapper apply using generated pkgpriorities.

Creates a temporary pkgpriorities file (mktemp "${TMPDIR:-/tmp}/alterator-pkgpriorities.XXXXXXXXXXXX") that lists packages marked manual by apt-mark showmanual; prevents implicit removal of those packages unless they are listed in exclude_pkgnames. The temporary file is removed when apt-wrapper exits.
Signals: apt1_install_stdout_signal, apt1_install_stderr_signal, apt1_remove_stdout_signal, apt1_remove_stderr_signal.
#### Input arguments

##### **exclude_pkgnames** : `s` <a id="argument-exclude_pkgnames-of-ApplyAsync"></a>

Whitespace-separated package names to exclude from pkgpriorities.

Use this list to allow removal of manual packages by dependencies. When calling via busctl, pass a literal '' string for an empty list; an empty string shifts the first pkgnames entry into exclude_pkgnames.
##### **pkgnames** : `s` <a id="argument-pkgnames-of-ApplyAsync"></a>

Whitespace-separated packages to process.

Names ending with "-" are removed; others are installed with apt-get install -y -q.
#### Output arguments

##### **response** : `i` <a id="argument-response-of-ApplyAsync"></a>

Exit code of apt-wrapper apply.

0 — success, != 0 — error.
### **ReinstallAsync**([pkgnames](#argument-pkgnames-of-ReinstallAsync) : `s`) -> ([response](#argument-response-of-ReinstallAsync) : `i`)<a id="method-ReinstallAsync"></a>

Reinstall packages asynchronously via apt-get reinstall -y -q.

#### Input arguments

##### **pkgnames** : `s` <a id="argument-pkgnames-of-ReinstallAsync"></a>

Whitespace-separated package names to reinstall.

#### Output arguments

##### **response** : `i` <a id="argument-response-of-ReinstallAsync"></a>

Exit code of the reinstall command.

0 — success, != 0 — error.
### **ListAllPackages**() -> ([stdout_strings](#argument-stdout_strings-of-ListAllPackages) : `as`, [stderr_strings](#argument-stderr_strings-of-ListAllPackages) : `as`, [response](#argument-response-of-ListAllPackages) : `i`)<a id="method-ListAllPackages"></a>

List all available package names using apt-cache search . --names-only.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-ListAllPackages"></a>

Package names produced by apt-cache.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-ListAllPackages"></a>

Errors or warnings from apt-cache.

##### **response** : `i` <a id="argument-response-of-ListAllPackages"></a>

Exit code of the listall helper.

0 — success, != 0 — error.
### **Search**([pattern](#argument-pattern-of-Search) : `s`) -> ([stdout_strings](#argument-stdout_strings-of-Search) : `as`, [stderr_strings](#argument-stderr_strings-of-Search) : `as`, [response](#argument-response-of-Search) : `i`)<a id="method-Search"></a>

Search packages by pattern via apt-wrapper search (apt-cache search).

#### Input arguments

##### **pattern** : `s` <a id="argument-pattern-of-Search"></a>

Search expression passed to apt-cache search.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-Search"></a>

Matching package names extracted from apt-cache output.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-Search"></a>

Errors from the search command.

##### **response** : `i` <a id="argument-response-of-Search"></a>

Exit code of the search helper.

0 — success, != 0 — error.
### **LastUpdate**() -> ([stdout_strings](#argument-stdout_strings-of-LastUpdate) : `as`, [stderr_strings](#argument-stderr_strings-of-LastUpdate) : `as`, [response](#argument-response-of-LastUpdate) : `i`)<a id="method-LastUpdate"></a>

Report last update timestamp for /var/lib/apt/lists in UTC.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-LastUpdate"></a>

Single entry with date-time string (YYYY-MM-DD HH:MM:SS UTC).

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-LastUpdate"></a>

Errors when stat or parsing fails.

##### **response** : `i` <a id="argument-response-of-LastUpdate"></a>

Exit code of the timestamp helper.

0 — success, != 0 — error.
### **LastDistUpgrade**() -> ([stdout_strings](#argument-stdout_strings-of-LastDistUpgrade) : `as`, [stderr_strings](#argument-stderr_strings-of-LastDistUpgrade) : `as`, [response](#argument-response-of-LastDistUpgrade) : `i`)<a id="method-LastDistUpgrade"></a>

Return last dist-upgrade log entry from /var/log/alterator/apt/dist-upgrades.log.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-LastDistUpgrade"></a>

Tail line with timestamp from dist-upgrades.log.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-LastDistUpgrade"></a>

Errors when the log is missing or unreadable.

##### **response** : `i` <a id="argument-response-of-LastDistUpgrade"></a>

Exit code of the log reader.

0 — success, != 0 — error.
### **CheckApply**([pkgnames](#argument-pkgnames-of-CheckApply) : `s`) -> ([stdout_strings](#argument-stdout_strings-of-CheckApply) : `as`, [stderr_strings](#argument-stderr_strings-of-CheckApply) : `as`, [response](#argument-response-of-CheckApply) : `i`)<a id="method-CheckApply"></a>

Simulate install/remove transaction and return planned changes.

#### Input arguments

##### **pkgnames** : `s` <a id="argument-pkgnames-of-CheckApply"></a>

Whitespace-separated packages for simulation.

Names ending with "-" are treated as removals.
#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-CheckApply"></a>

JSON line with install_packages, remove_packages, extra_remove_packages arrays.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-CheckApply"></a>

Diagnostic output from apt-get --just-print.

##### **response** : `i` <a id="argument-response-of-CheckApply"></a>

Exit code of the simulation.

0 — success, != 0 — error.
### **CheckReinstall**([pkgnames](#argument-pkgnames-of-CheckReinstall) : `s`) -> ([stdout_strings](#argument-stdout_strings-of-CheckReinstall) : `as`, [stderr_strings](#argument-stderr_strings-of-CheckReinstall) : `as`, [response](#argument-response-of-CheckReinstall) : `i`)<a id="method-CheckReinstall"></a>

Simulate reinstall transaction for selected packages.

#### Input arguments

##### **pkgnames** : `s` <a id="argument-pkgnames-of-CheckReinstall"></a>

Whitespace-separated package names to reinstall.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-CheckReinstall"></a>

Packages scheduled for installation from apt-get -s output.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-CheckReinstall"></a>

Packages scheduled for removal or warnings.

##### **response** : `i` <a id="argument-response-of-CheckReinstall"></a>

Exit code of the simulation.

0 — success, != 0 — error.
### **CheckDistUpgrade**() -> ([stdout_strings](#argument-stdout_strings-of-CheckDistUpgrade) : `as`, [stderr_strings](#argument-stderr_strings-of-CheckDistUpgrade) : `as`, [response](#argument-response-of-CheckDistUpgrade) : `i`)<a id="method-CheckDistUpgrade"></a>

Simulate dist-upgrade and report planned installs/removals.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-CheckDistUpgrade"></a>

Packages planned for installation.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-CheckDistUpgrade"></a>

Packages planned for removal or errors.

##### **response** : `i` <a id="argument-response-of-CheckDistUpgrade"></a>

Exit code of apt-get dist-upgrade -s -q.

0 — success, != 0 — error.
### **DistUpgradeAsync**() -> ([response](#argument-response-of-DistUpgradeAsync) : `i`)<a id="method-DistUpgradeAsync"></a>

Perform dist-upgrade asynchronously via apt-get dist-upgrade -y -q.

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

