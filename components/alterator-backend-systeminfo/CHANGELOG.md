# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.3.2] - 2025-04-09

### Changed

- Alterator application object name is About System now.

## [0.3.1] - 2025-04-04

### Changed

- Alterator application object category is System now.
- Alterator application object name is Properties now.

## [0.3.0] - 2025-03-28

### Added

- `GetLocale` method for getting system locale.
- `ListDesktopEnvironments` method for list installed DE on system.
- `GetReleaseNotes` method for getting ALT distro release notes.

## [0.2.2] - 2025-03-17

### Fixed

- `GetAll` method which returns usage instead expected data.
- `GetLicense` method which returns license with incorrect language. 

## [0.2.1] - 2025-03-14

### Fixed

- `GetDrive` method which returns incorrect result in some cases.
- `GetCPU` method which returns incorrect result in some cases.
- `GetLicense` method which returns result with incorrect language if edition is selected.

## [0.2.0] - 2025-03-12

### Added

- `GetLicense` method which returns OS license (supports edition).

## [0.1.2] - 2025-03-03

### Added

- `GetAll` method which returns all properties values.
- `Info` method returns object file of application interface for Alterator Explorer.

## [0.1.1] - 2025-02-20

### Changed

- `GetDeviceName` method name to `GetHostName`.
- `GetArch` method result appearance to authentic.
- `GetCPU` method returns base frequency instead max frequency.
- `GetMemory` and `GetDrive` methods returns volume in bytes instead kibibytes.

### Removed

- `GetLastUpdate` method (now this is `GetLastDistUpgrade` method from `org.altlinux.alterator.apt1`).
- APT loggers (now `alterator-backend-packages` contains it).

## [0.1.0] - 2024-12-09

### Added

- `org.altlinux.alterator.systeminfo1` D-Bus interface that includes methods for getting system information (see below).
- `GetDeviceName` method that returns hostname.
- `GetOperationSystemName` method that returns pretty OS name.
- `GetBranch` method that returns dist branch name.
- `GetLastUpdate` method that returns datetime of last dist-upgrade in UTC.
- `GetKernel` method that returns version and release of Linux kernel.
- `GetCPU` method that returns vendor, name, number of cores and max frequency on KHz of CPU.
- `GetArch` method that returns name of arch.
- `GetGPU` method that returns vendor and name of GPU.
- `GetMemory` method that returns RAM volume in kibibytes.
- `GetDrive` method that returns drive volume in kibibytes.
- `GetMotherboard` method that returns vendor and name of motherboard.
- `GetMonitor` method that returns pairs of port and resolution.
- `/org/altlinux/alterator/systeminfo` object of system bus that implements new interface.

[unreleased]: https://gitlab.basealt.space/alt/alterator-backend-systeminfo/-/compare/v0.3.2...main
[0.3.2]: https://gitlab.basealt.space/alt/alterator-backend-systeminfo/-/tags/v0.3.2
[0.3.1]: https://gitlab.basealt.space/alt/alterator-backend-systeminfo/-/tags/v0.3.1
[0.3.0]: https://gitlab.basealt.space/alt/alterator-backend-systeminfo/-/tags/v0.3.0
[0.2.2]: https://gitlab.basealt.space/alt/alterator-backend-systeminfo/-/tags/v0.2.2
[0.2.1]: https://gitlab.basealt.space/alt/alterator-backend-systeminfo/-/tags/v0.2.1
[0.2.0]: https://gitlab.basealt.space/alt/alterator-backend-systeminfo/-/tags/v0.2.0
[0.1.2]: https://gitlab.basealt.space/alt/alterator-backend-systeminfo/-/tags/v0.1.2
[0.1.1]: https://gitlab.basealt.space/alt/alterator-backend-systeminfo/-/tags/v0.1.1
[0.1.0]: https://gitlab.basealt.space/alt/alterator-backend-systeminfo/-/tags/v0.1.0
