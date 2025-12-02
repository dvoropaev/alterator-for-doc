**alterator-backend-systeminfo**

[English](./README.md) | [Русский](./docs/README.ru_RU.md)

Provides Alterator clients with system details through the D-Bus interface.

---

# Overview
| Component | Location | Purpose |
| --------- | -------- | ------- |
| Script `systeminfo` | `/usr/lib/alterator/backends/systeminfo` | Provides system information about the computer and the operating system. |
| File `systeminfo.backend` | `/usr/share/alterator/backends/systeminfo.backend` | Describes the `org.altlinux.alterator.systeminfo1` interface of the `/org/altlinux/alterator/systeminfo` object. |
| File `systeminfo.object` | `/usr/share/alterator/objects/systeminfo.object` | Stores data for registering the Alterator object "About system" for clients. |
| Script `systeminfo.d/notes` | `/usr/lib/alterator/backends/systeminfo.d/notes` | Provides helper routines that locate license texts, release notes, and distribution records for use by the `systeminfo` script. |

# Capabilities
- Retrieve textual system attributes: host name, repository branch, kernel version, locale.
- Return hardware details: CPU, GPU, architecture, memory volume, storage, monitors, motherboard.
- Return the contents of the license, release notes, and final installation notes with locale awareness.
- Produce aggregated output for `GetAll` with system and hardware attributes.
- List available desktop environments.

# Integration
- Methods for retrieving machine and system data are available to clients through the `org.altlinux.alterator.systeminfo1` interface.
- `alterator-module-executor` from `alterator-manager` registers the backend using `/usr/share/alterator/backends/systeminfo.backend`.
- License, release notes, and final notes lookup is performed by `systeminfo.d/notes` with edition awareness: when `alterator-backend-edition-utils` is installed, `/usr/lib/alterator/backends/edition` is consulted and files in `/usr/share/alt-notes` are checked first; otherwise the search scans `/usr/share/alt-notes/license.<lang>.html`, `/usr/share/alt-license/license.<lang>.html`, `/var/lib/install3/licenses/license.<lang>.html` and falls back to `license.all.html` in these locations.
- `systeminfo.object` declares the object category `X-Alterator-System`, enabling the module in `alterator-explorer`.

# `systeminfo` commands
| Command | Output | Sources |
| ------- | ------ | ------- |
| `host-name`, `os-name`, `branch`, `kernel` | Text strings with general system data. | `hostname`, `/etc/os-release`, the RPM macro `%_priority_distbranch`, `/proc/sys/kernel/osrelease`. |
| `os-license`, `release-notes`, `final-notes` | Contents of HTML files with license and notes. | Lookup through `systeminfo.d/notes`: without edition data — sequential scan of `/usr/share/alt-notes/license.<lang>.html`, `/usr/share/alt-license/license.<lang>.html`, `/var/lib/install3/licenses/license.<lang>.html` with `license.all.html` fallback; with `alterator-backend-edition-utils` — `/usr/lib/alterator/backends/edition` and `/usr/share/alt-notes` are tried first. |
| `locale`, `list-desktop-environments` | Current locale, list of desktop environments. | Reads `/etc/locale.conf`, scans desktop `.desktop` entries. |
| `arch`, `cpu`, `gpu`, `memory`, `drive`, `motherboard`, `monitor` | Hardware information processed from `/proc` and `/sys`. | Reads system files and parses `arch`, `lspci`/`lscpu` output. |

Important: every command runs with `set -o pipefail`; errors return a non-zero code with diagnostics sent to stderr.

# Interface documentation
- `systeminfo1` — exposes system information including hardware characteristics and service notes. See [systeminfo1.md](/docs/systeminfo1.md).
