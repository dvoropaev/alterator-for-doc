## Overview

The `org.altlinux.alterator.systeminfo1` interface exposes system data through `/usr/lib/alterator/backends/systeminfo`. Registration on the bus is handled by the `alterator-module-executor` subsystem of `alterator-manager` as described in `/usr/share/alterator/backends/systeminfo.backend`. Methods return the response code of the command they run (`0` — success, `!= 0` — failure).

## Info — `Info() -> (stdout_bytes: ay, response: i)`

- Purpose: publish the `systeminfo` object description from `/usr/share/alterator/objects/systeminfo.object`.
- Parameters: none; returns `stdout_bytes` and `response`.
- Expected behavior (example): `stdout_bytes` holds the TOML description, `response = 0`.

## GetAll — `GetAll() -> (stdout_bytes: ay, response: i)`

- Purpose: run `/usr/lib/alterator/backends/systeminfo --all` and aggregate characteristics.
- Parameters: no arguments; returns `stdout_bytes` and `response`.
- `stdout_bytes` format: lines `KEY="value"` with keys `HOSTNAME`, `OS_NAME`, `BRANCH`, `KERNEL`, `CPU`, `ARCH`, `GPU`, `MEMORY`, `DRIVE`, `MOTHERBOARD`, `MONITOR` in this order.
- Expected behavior (example): `response = 0`, output contains every listed key-value pair.

## GetHostName — `GetHostName() -> (stdout_strings: as, response: i)`

- Purpose: start `systeminfo host-name`, which prints the result of `hostname`.
- Parameters: none; returns `stdout_strings` and `response`.
- Expected behavior (example): `stdout_strings` contains a single host name, `response = 0`.

## GetOperationSystemName — `GetOperationSystemName() -> (stdout_strings: as, response: i)`

- Purpose: execute `systeminfo os-name`, reading `PRETTY_NAME` from `/etc/os-release`.
- Parameters: no arguments; returns `stdout_strings` and `response`.
- Expected behavior (example): `stdout_strings` holds a human-readable distribution name, `response = 0`.

## GetLicense — `GetLicense() -> (stdout_bytes: ay, response: i)`

- Purpose: call `systeminfo os-license`, which uses `get_notes_file_path` to locate the license file and output it.
- Parameters: none; returns `stdout_bytes` and `response` (`LC_ALL` unset).
- Expected behavior (example): `stdout_bytes` contains the license text of the current edition, `response = 0`; missing files lead to an error.

## GetReleaseNotes — `GetReleaseNotes() -> (stdout_bytes: ay, response: i)`

- Purpose: run `systeminfo release-notes` to output release notes.
- Parameters: none; returns `stdout_bytes` and `response` (`LC_ALL` unset).
- Expected behavior (example): `stdout_bytes` carries HTML or plain text notes; absent files produce a non-zero `response`.

## GetFinalNotes — `GetFinalNotes() -> (stdout_bytes: ay, response: i)`

- Purpose: call `systeminfo final-notes` to provide installer final hints.
- Parameters: none; returns `stdout_bytes` and `response` (`LC_ALL` unset).
- Expected behavior (example): `stdout_bytes` holds the final notes; missing data cause an error.

## GetArch — `GetArch() -> (stdout_strings: as, response: i)`

- Purpose: execute `systeminfo arch`, reading the architecture from `/proc/sys/kernel/arch`.
- Parameters: none; returns `stdout_strings` and `response`.
- Expected behavior (example): `stdout_strings` contains the architecture (for example, `x86_64`), `response = 0`.

## GetBranch — `GetBranch() -> (stdout_strings: as, response: i)`

- Purpose: run `systeminfo branch`, returning `%_priority_distbranch` from RPM macros.
- Parameters: none; returns `stdout_strings` and `response`.
- Expected behavior (example): `stdout_strings` includes the repository branch, `response = 0`.

## GetKernel — `GetKernel() -> (stdout_strings: as, response: i)`

- Purpose: call `systeminfo kernel`, outputting `osrelease` from `/proc/sys/kernel/osrelease`.
- Parameters: none; returns `stdout_strings` and `response`.
- Expected behavior (example): `stdout_strings` lists the kernel version, `response = 0`.

## GetCPU — `GetCPU() -> (stdout_strings: as, response: i)`

- Purpose: execute `systeminfo cpu`, extracting model, logical cores, and frequency from `/proc/cpuinfo` and sysfs.
- Parameters: none; returns `stdout_strings` and `response`.
- `stdout_strings` format: `[0]` — CPU model, `[1]` — logical core count, `[2]` — frequency in MHz.
- Expected behavior (example): `response = 0`, all three strings present.

## GetGPU — `GetGPU() -> (stdout_strings: as, response: i)`

- Purpose: run `systeminfo gpu`, parsing `lspci` output for VGA adapters.
- Parameters: none; returns `stdout_strings` and `response`.
- Expected behavior (example): `stdout_strings` lists VGA adapters, `response = 0`.

## GetMemory — `GetMemory() -> (stdout_strings: as, response: i)`

- Purpose: execute `systeminfo memory`, computing RAM size from `/proc/meminfo` (`MemTotal * 1024`).
- Parameters: none; returns `stdout_strings` and `response`.
- Expected behavior (example): `stdout_strings` provides memory size in bytes, `response = 0`.

## GetDrive — `GetDrive() -> (stdout_strings: as, response: i)`

- Purpose: call `systeminfo drive`, which sums partition sizes from `/proc/partitions`.
- Parameters: none; returns `stdout_strings` and `response`.
- Expected behavior (example): `stdout_strings` shows total storage size in bytes, `response = 0`.

## GetMonitor — `GetMonitor() -> (stdout_strings: as, response: i)`

- Purpose: run `systeminfo monitor`, reading `/sys/class/drm/*/modes` and matching resolutions.
- Parameters: none; returns `stdout_strings` and `response`.
- Expected behavior (example): `stdout_strings` lists entries like `card0-HDMI-A-1 1920x1080`, `response = 0`.

## GetMotherboard — `GetMotherboard() -> (stdout_strings: as, response: i)`

- Purpose: execute `systeminfo motherboard`, combining `/sys/devices/virtual/dmi/id/board_{vendor,name,version}` contents.
- Parameters: none; returns `stdout_strings` and `response`.
- `stdout_strings` format: `[0]` — board vendor, `[1]` — model, `[2]` — version.
- Expected behavior (example): `response = 0`, all three strings returned.

## GetLocale — `GetLocale() -> (stdout_strings: as, response: i)`

- Purpose: start `systeminfo locale`, which reads `LANG` from `/etc/locale.conf`.
- Parameters: none; returns `stdout_strings` and `response`.
- `stdout_strings` format: locale value `<language>_<region>.<encoding>` (for example, `ru_RU.UTF-8`).
- Expected behavior (example): `response = 0`, locale string present.

## ListDesktopEnvironments — `ListDesktopEnvironments() -> (stdout_strings: as, response: i)`

- Purpose: execute `systeminfo list-desktop-environments`, iterating desktop `.desktop` entries (plasma, gnome, mate, cinnamon, xfce).
- Parameters: none; returns `stdout_strings` and `response`.
- Expected behavior (example): `stdout_strings` lists values from `CINNAMON`, `GNOME`, `KDE<index>`, `MATE`, `XFCE`; `response = 0`.
