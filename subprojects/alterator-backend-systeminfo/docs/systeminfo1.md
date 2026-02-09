[English](./systeminfo1.md) | [Русский](./systeminfo1.ru_RU.md)

# Interface **org.altlinux.alterator.systeminfo1**

Expose systeminfo backend methods that provide system data by running /usr/lib/alterator/backends/systeminfo helpers; each method returns stdout and the underlying command response code.

| Method | Summary |
|--------|---------|
| [GetAll](#method-GetAll) | Collect consolidated system data in key/value form. |
| [GetCPU](#method-GetCPU) | Return CPU model, logical core count, and frequency via systeminfo cpu. |
| [GetHostName](#method-GetHostName) | Return the host name from systeminfo host-name. |
| [GetDrive](#method-GetDrive) | Return total storage size via systeminfo drive. |
| [GetGPU](#method-GetGPU) | List VGA adapters reported by systeminfo gpu. |
| [GetMemory](#method-GetMemory) | Report RAM size using systeminfo memory. |
| [GetBranch](#method-GetBranch) | Return the distribution branch via systeminfo branch. |
| [GetKernel](#method-GetKernel) | Return the kernel version via systeminfo kernel. |
| [GetOperationSystemName](#method-GetOperationSystemName) | Return a human-readable operating system name via systeminfo os-name. |
| [GetLicense](#method-GetLicense) | Return the license text selected by locale and edition via systeminfo os-license. |
| [GetReleaseNotes](#method-GetReleaseNotes) | Return release notes produced by systeminfo release-notes. |
| [GetFinalNotes](#method-GetFinalNotes) | Return installer final notes via systeminfo final-notes. |
| [GetArch](#method-GetArch) | Return system architecture via systeminfo arch. |
| [GetMonitor](#method-GetMonitor) | List connected monitors and resolutions via systeminfo monitor. |
| [GetMotherboard](#method-GetMotherboard) | Return motherboard vendor, model, and version via systeminfo motherboard. |
| [GetLocale](#method-GetLocale) | Return the system locale via systeminfo locale. |
| [ListDesktopEnvironments](#method-ListDesktopEnvironments) | List available desktop sessions via systeminfo list-desktop-environments. |


## Methods

### **GetAll**() -> ([stdout_bytes](#argument-stdout_bytes-of-GetAll) : `ay`, [response](#argument-response-of-GetAll) : `i`)<a id="method-GetAll"></a>

Collect consolidated system data in key/value form.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-GetAll"></a>

Raw output of the systeminfo get-all helper.

Lines KEY="value" ordered as HOSTNAME, OS_NAME, BRANCH, KERNEL, CPU, ARCH, GPU, MEMORY, DRIVE, MOTHERBOARD, MONITOR.
##### **response** : `i` <a id="argument-response-of-GetAll"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **GetCPU**() -> ([stdout_strings](#argument-stdout_strings-of-GetCPU) : `as`, [response](#argument-response-of-GetCPU) : `i`)<a id="method-GetCPU"></a>

Return CPU model, logical core count, and frequency via systeminfo cpu.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetCPU"></a>

CPU properties reported by the helper.

stdout_strings entries: model; logical cores; frequency in MHz.
##### **response** : `i` <a id="argument-response-of-GetCPU"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **GetHostName**() -> ([stdout_strings](#argument-stdout_strings-of-GetHostName) : `as`, [response](#argument-response-of-GetHostName) : `i`)<a id="method-GetHostName"></a>

Return the host name from systeminfo host-name.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetHostName"></a>

Hostname string from the hostname command.

##### **response** : `i` <a id="argument-response-of-GetHostName"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **GetDrive**() -> ([stdout_strings](#argument-stdout_strings-of-GetDrive) : `as`, [response](#argument-response-of-GetDrive) : `i`)<a id="method-GetDrive"></a>

Return total storage size via systeminfo drive.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetDrive"></a>

Aggregated storage size from /proc/partitions.

Single entry with size in bytes.
##### **response** : `i` <a id="argument-response-of-GetDrive"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **GetGPU**() -> ([stdout_strings](#argument-stdout_strings-of-GetGPU) : `as`, [response](#argument-response-of-GetGPU) : `i`)<a id="method-GetGPU"></a>

List VGA adapters reported by systeminfo gpu.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetGPU"></a>

Adapter descriptions parsed from lspci output.

##### **response** : `i` <a id="argument-response-of-GetGPU"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **GetMemory**() -> ([stdout_strings](#argument-stdout_strings-of-GetMemory) : `as`, [response](#argument-response-of-GetMemory) : `i`)<a id="method-GetMemory"></a>

Report RAM size using systeminfo memory.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetMemory"></a>

Total memory calculated as MemTotal * 1024.

Single entry with size in bytes.
##### **response** : `i` <a id="argument-response-of-GetMemory"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **GetBranch**() -> ([stdout_strings](#argument-stdout_strings-of-GetBranch) : `as`, [response](#argument-response-of-GetBranch) : `i`)<a id="method-GetBranch"></a>

Return the distribution branch via systeminfo branch.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetBranch"></a>

Value of the RPM macro %_priority_distbranch.

##### **response** : `i` <a id="argument-response-of-GetBranch"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **GetKernel**() -> ([stdout_strings](#argument-stdout_strings-of-GetKernel) : `as`, [response](#argument-response-of-GetKernel) : `i`)<a id="method-GetKernel"></a>

Return the kernel version via systeminfo kernel.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetKernel"></a>

Kernel release string read from /proc/sys/kernel/osrelease.

##### **response** : `i` <a id="argument-response-of-GetKernel"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **GetOperationSystemName**() -> ([stdout_strings](#argument-stdout_strings-of-GetOperationSystemName) : `as`, [response](#argument-response-of-GetOperationSystemName) : `i`)<a id="method-GetOperationSystemName"></a>

Return a human-readable operating system name via systeminfo os-name.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetOperationSystemName"></a>

PRETTY_NAME value from /etc/os-release.

##### **response** : `i` <a id="argument-response-of-GetOperationSystemName"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **GetLicense**() -> ([stdout_bytes](#argument-stdout_bytes-of-GetLicense) : `ay`, [response](#argument-response-of-GetLicense) : `i`)<a id="method-GetLicense"></a>

Return the license text selected by locale and edition via systeminfo os-license.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-GetLicense"></a>

License content chosen by the helper.

Search order: /usr/lib/alterator/backends/edition (when alterator-backend-edition-utils is present), license.<lang>.html in /usr/share/alt-notes, /usr/share/alt-license, /var/lib/install3/licenses; fallback to license.all.html. LC_ALL is unset before execution.
##### **response** : `i` <a id="argument-response-of-GetLicense"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **GetReleaseNotes**() -> ([stdout_bytes](#argument-stdout_bytes-of-GetReleaseNotes) : `ay`, [response](#argument-response-of-GetReleaseNotes) : `i`)<a id="method-GetReleaseNotes"></a>

Return release notes produced by systeminfo release-notes.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-GetReleaseNotes"></a>

Release notes text emitted by the helper.

Notes may be HTML or plain text; LC_ALL is unset before execution.
##### **response** : `i` <a id="argument-response-of-GetReleaseNotes"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **GetFinalNotes**() -> ([stdout_bytes](#argument-stdout_bytes-of-GetFinalNotes) : `ay`, [response](#argument-response-of-GetFinalNotes) : `i`)<a id="method-GetFinalNotes"></a>

Return installer final notes via systeminfo final-notes.

#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-GetFinalNotes"></a>

Final notes produced by the helper.

LC_ALL is unset before execution.
##### **response** : `i` <a id="argument-response-of-GetFinalNotes"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **GetArch**() -> ([stdout_strings](#argument-stdout_strings-of-GetArch) : `as`, [response](#argument-response-of-GetArch) : `i`)<a id="method-GetArch"></a>

Return system architecture via systeminfo arch.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetArch"></a>

Architecture string read from /proc/sys/kernel/arch.

##### **response** : `i` <a id="argument-response-of-GetArch"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **GetMonitor**() -> ([stdout_strings](#argument-stdout_strings-of-GetMonitor) : `as`, [response](#argument-response-of-GetMonitor) : `i`)<a id="method-GetMonitor"></a>

List connected monitors and resolutions via systeminfo monitor.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetMonitor"></a>

Connector names and resolutions from /sys/class/drm/*/modes.

Entries formatted as <device>-<connector> <width>x<height>.
##### **response** : `i` <a id="argument-response-of-GetMonitor"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **GetMotherboard**() -> ([stdout_strings](#argument-stdout_strings-of-GetMotherboard) : `as`, [response](#argument-response-of-GetMotherboard) : `i`)<a id="method-GetMotherboard"></a>

Return motherboard vendor, model, and version via systeminfo motherboard.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetMotherboard"></a>

Board identification read from DMI.

stdout_strings entries: vendor; model; version.
##### **response** : `i` <a id="argument-response-of-GetMotherboard"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **GetLocale**() -> ([stdout_strings](#argument-stdout_strings-of-GetLocale) : `as`, [response](#argument-response-of-GetLocale) : `i`)<a id="method-GetLocale"></a>

Return the system locale via systeminfo locale.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetLocale"></a>

Locale string read from /etc/locale.conf.

##### **response** : `i` <a id="argument-response-of-GetLocale"></a>

Exit code of the helper.

0 — success, != 0 — error.
### **ListDesktopEnvironments**() -> ([stdout_strings](#argument-stdout_strings-of-ListDesktopEnvironments) : `as`, [response](#argument-response-of-ListDesktopEnvironments) : `i`)<a id="method-ListDesktopEnvironments"></a>

List available desktop sessions via systeminfo list-desktop-environments.

#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-ListDesktopEnvironments"></a>

Desktop session identifiers discovered from .desktop entries.

stdout_strings may include CINNAMON, GNOME, KDE5, KDE6, MATE, XFCE entries.
##### **response** : `i` <a id="argument-response-of-ListDesktopEnvironments"></a>

Exit code of the helper.

0 — success, != 0 — error.


Current specification: https://altlinux.space/alterator/alterator-entry/src/branch/master/doc
