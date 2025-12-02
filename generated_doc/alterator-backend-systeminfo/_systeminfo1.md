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

Output format: lines KEY="value" ordered as HOSTNAME, OS_NAME, BRANCH, KERNEL, CPU, ARCH, GPU, MEMORY, DRIVE, MOTHERBOARD, MONITOR. Response: 0 — success, != 0 — error.
#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-GetAll"></a>

##### **response** : `i` <a id="argument-response-of-GetAll"></a>

### **GetCPU**() -> ([stdout_strings](#argument-stdout_strings-of-GetCPU) : `as`, [response](#argument-response-of-GetCPU) : `i`)<a id="method-GetCPU"></a>

Return CPU model, logical core count, and frequency via systeminfo cpu.

stdout_strings[0] — model; stdout_strings[1] — logical cores; stdout_strings[2] — frequency in MHz. Response: 0 — success, != 0 — error.
#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetCPU"></a>

##### **response** : `i` <a id="argument-response-of-GetCPU"></a>

### **GetHostName**() -> ([stdout_strings](#argument-stdout_strings-of-GetHostName) : `as`, [response](#argument-response-of-GetHostName) : `i`)<a id="method-GetHostName"></a>

Return the host name from systeminfo host-name.

stdout_strings carries a single hostname string produced by the hostname command. Response: 0 — success, != 0 — error.
#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetHostName"></a>

##### **response** : `i` <a id="argument-response-of-GetHostName"></a>

### **GetDrive**() -> ([stdout_strings](#argument-stdout_strings-of-GetDrive) : `as`, [response](#argument-response-of-GetDrive) : `i`)<a id="method-GetDrive"></a>

Return total storage size via systeminfo drive.

Computes the sum of partition sizes from /proc/partitions; stdout_strings holds the size in bytes. Response: 0 — success, != 0 — error.
#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetDrive"></a>

##### **response** : `i` <a id="argument-response-of-GetDrive"></a>

### **GetGPU**() -> ([stdout_strings](#argument-stdout_strings-of-GetGPU) : `as`, [response](#argument-response-of-GetGPU) : `i`)<a id="method-GetGPU"></a>

List VGA adapters reported by systeminfo gpu.

Parses lspci output for VGA devices; stdout_strings contains adapter descriptions. Response: 0 — success, != 0 — error.
#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetGPU"></a>

##### **response** : `i` <a id="argument-response-of-GetGPU"></a>

### **GetMemory**() -> ([stdout_strings](#argument-stdout_strings-of-GetMemory) : `as`, [response](#argument-response-of-GetMemory) : `i`)<a id="method-GetMemory"></a>

Report RAM size using systeminfo memory.

Calculates MemTotal * 1024 from /proc/meminfo; stdout_strings returns the size in bytes. Response: 0 — success, != 0 — error.
#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetMemory"></a>

##### **response** : `i` <a id="argument-response-of-GetMemory"></a>

### **GetBranch**() -> ([stdout_strings](#argument-stdout_strings-of-GetBranch) : `as`, [response](#argument-response-of-GetBranch) : `i`)<a id="method-GetBranch"></a>

Return the distribution branch via systeminfo branch.

Provides the RPM macro %_priority_distbranch value for service use. Response: 0 — success, != 0 — error.
#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetBranch"></a>

##### **response** : `i` <a id="argument-response-of-GetBranch"></a>

### **GetKernel**() -> ([stdout_strings](#argument-stdout_strings-of-GetKernel) : `as`, [response](#argument-response-of-GetKernel) : `i`)<a id="method-GetKernel"></a>

Return the kernel version via systeminfo kernel.

Reads /proc/sys/kernel/osrelease; stdout_strings holds the kernel release string. Response: 0 — success, != 0 — error.
#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetKernel"></a>

##### **response** : `i` <a id="argument-response-of-GetKernel"></a>

### **GetOperationSystemName**() -> ([stdout_strings](#argument-stdout_strings-of-GetOperationSystemName) : `as`, [response](#argument-response-of-GetOperationSystemName) : `i`)<a id="method-GetOperationSystemName"></a>

Return a human-readable operating system name via systeminfo os-name.

Reads PRETTY_NAME from /etc/os-release; stdout_strings contains the display name. Response: 0 — success, != 0 — error.
#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetOperationSystemName"></a>

##### **response** : `i` <a id="argument-response-of-GetOperationSystemName"></a>

### **GetLicense**() -> ([stdout_bytes](#argument-stdout_bytes-of-GetLicense) : `ay`, [response](#argument-response-of-GetLicense) : `i`)<a id="method-GetLicense"></a>

Return the license text selected by locale and edition via systeminfo os-license.

Search order: /usr/lib/alterator/backends/edition (when alterator-backend-edition-utils is present), then license.<lang>.html in /usr/share/alt-notes, /usr/share/alt-license, /var/lib/install3/licenses; falls back to license.all.html. LC_ALL is unset before execution. Response: 0 — success, != 0 — error.
#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-GetLicense"></a>

##### **response** : `i` <a id="argument-response-of-GetLicense"></a>

### **GetReleaseNotes**() -> ([stdout_bytes](#argument-stdout_bytes-of-GetReleaseNotes) : `ay`, [response](#argument-response-of-GetReleaseNotes) : `i`)<a id="method-GetReleaseNotes"></a>

Return release notes produced by systeminfo release-notes.

Emits HTML or plain-text notes; LC_ALL is unset. Response: 0 — success, != 0 — error.
#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-GetReleaseNotes"></a>

##### **response** : `i` <a id="argument-response-of-GetReleaseNotes"></a>

### **GetFinalNotes**() -> ([stdout_bytes](#argument-stdout_bytes-of-GetFinalNotes) : `ay`, [response](#argument-response-of-GetFinalNotes) : `i`)<a id="method-GetFinalNotes"></a>

Return installer final notes via systeminfo final-notes.

Provides final hints for installers; LC_ALL is unset. Response: 0 — success, != 0 — error.
#### Output arguments

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-GetFinalNotes"></a>

##### **response** : `i` <a id="argument-response-of-GetFinalNotes"></a>

### **GetArch**() -> ([stdout_strings](#argument-stdout_strings-of-GetArch) : `as`, [response](#argument-response-of-GetArch) : `i`)<a id="method-GetArch"></a>

Return system architecture via systeminfo arch.

Reads /proc/sys/kernel/arch; stdout_strings holds the architecture string such as x86_64. Response: 0 — success, != 0 — error.
#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetArch"></a>

##### **response** : `i` <a id="argument-response-of-GetArch"></a>

### **GetMonitor**() -> ([stdout_strings](#argument-stdout_strings-of-GetMonitor) : `as`, [response](#argument-response-of-GetMonitor) : `i`)<a id="method-GetMonitor"></a>

List connected monitors and resolutions via systeminfo monitor.

Reads /sys/class/drm/*/modes and matches connectors; stdout_strings entries look like card0-HDMI-A-1 1920x1080. Response: 0 — success, != 0 — error.
#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetMonitor"></a>

##### **response** : `i` <a id="argument-response-of-GetMonitor"></a>

### **GetMotherboard**() -> ([stdout_strings](#argument-stdout_strings-of-GetMotherboard) : `as`, [response](#argument-response-of-GetMotherboard) : `i`)<a id="method-GetMotherboard"></a>

Return motherboard vendor, model, and version via systeminfo motherboard.

Combines /sys/devices/virtual/dmi/id/board_vendor, board_name, board_version. stdout_strings[0] — vendor; stdout_strings[1] — model; stdout_strings[2] — version. Response: 0 — success, != 0 — error.
#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetMotherboard"></a>

##### **response** : `i` <a id="argument-response-of-GetMotherboard"></a>

### **GetLocale**() -> ([stdout_strings](#argument-stdout_strings-of-GetLocale) : `as`, [response](#argument-response-of-GetLocale) : `i`)<a id="method-GetLocale"></a>

Return the system locale via systeminfo locale.

Reads LANG from /etc/locale.conf; stdout_strings holds the locale string such as ru_RU.UTF-8. Response: 0 — success, != 0 — error.
#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetLocale"></a>

##### **response** : `i` <a id="argument-response-of-GetLocale"></a>

### **ListDesktopEnvironments**() -> ([stdout_strings](#argument-stdout_strings-of-ListDesktopEnvironments) : `as`, [response](#argument-response-of-ListDesktopEnvironments) : `i`)<a id="method-ListDesktopEnvironments"></a>

List available desktop sessions via systeminfo list-desktop-environments.

Iterates .desktop entries for plasma, gnome, mate, cinnamon, xfce; stdout_strings may include CINNAMON, GNOME, KDE<index>, MATE, XFCE entries. Response: 0 — success, != 0 — error.
#### Output arguments

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-ListDesktopEnvironments"></a>

##### **response** : `i` <a id="argument-response-of-ListDesktopEnvironments"></a>

