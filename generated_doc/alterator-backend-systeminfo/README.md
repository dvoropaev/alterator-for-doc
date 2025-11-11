**alterator-backend-systeminfo**

[English](README.md) | [Русский](README.ru_RU.md)

Provides Alterator clients with system details through the D-Bus interface.

---

# Overview
| Component | Location | Purpose |
| --------- | -------- | ------- |
| Script `systeminfo` | `/usr/lib/alterator/backends/systeminfo` | Runs commands that collect information about the installed system. |
| File `systeminfo.backend` | `/usr/share/alterator/backends/systeminfo.backend` | Describes the backend with methods of the `org.altlinux.alterator.systeminfo1` interface. |
| File `systeminfo.object` | `/usr/share/alterator/objects/systeminfo.object` | Registers the Alterator object "About system" for clients. |
| Script `systeminfo.d/notes` | `/usr/lib/alterator/backends/systeminfo.d/notes` | Supplies helper routines that locate license texts, release notes, and distribution records. |

# Capabilities
- Retrieve textual system attributes: host name, repository branch, kernel version, locale.
- Return hardware details: CPU, GPU, memory volume, storage, monitors, motherboard.
- Provide links to license text, release notes, and final installation notes with locale awareness.
- Produce aggregated output for the `--all` command consumed by `GetAll`.
- List available desktop environments via the `list-desktop-environments` command.

# Integration
- The backend implements `org.altlinux.alterator.systeminfo1` and delivers data to clients over D-Bus.
- `alterator-module-executor` from `alterator-manager` registers the backend using `/usr/share/alterator/backends/systeminfo.backend`.
- `GetLicense` and related commands delegate file lookup to `systeminfo.d/notes`, which invokes `/usr/lib/alterator/backends/edition` when edition data are required.
- `systeminfo.object` declares the object category `X-Alterator-System`, enabling the module in `alterator-explorer`.

# `systeminfo` commands
| Command | Output | Sources |
| ------- | ------ | ------- |
| `host-name`, `os-name`, `branch`, `kernel` | Text strings with general system data. | `/etc/os-release`, `%_priority_distbranch`, `/proc/sys/kernel/osrelease`. |
| `os-license`, `release-notes`, `final-notes` | Contents of HTML files with license and notes. | Lookup through `systeminfo.d/notes` (including `/usr/share/alt-notes`, `/usr/share/distro-licenses`). |
| `locale`, `list-desktop-environments` | Current locale, list of desktop environments. | Reads `/etc/locale.conf`, scans desktop `.desktop` entries. |
| `cpu`, `gpu`, `memory`, `drive`, `motherboard`, `monitor` | Hardware information processed from `/proc` and `/sys`. | Reads system files and parses `lspci`/`lscpu` output. |

Important: every command runs with `set -o pipefail`; errors return a non-zero code with diagnostics sent to stderr.

# Interface documentation
- `systeminfo1` — exposes system information including hardware characteristics and service notes. See [systeminfo1.md](./systeminfo1.md).
