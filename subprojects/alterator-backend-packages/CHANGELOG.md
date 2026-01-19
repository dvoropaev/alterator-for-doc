# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Removed

- Method `Fetch` from `org.altlinux.alterator.apt1`.
- Method `FetchAsync` from `org.altlinux.alterator.apt1`.
- Method `FetchDistUpgrade` from `org.altlinux.alterator.apt1`.
- Method `FetchDistUpgradeAsync` from `org.altlinux.alterator.apt1`.
- Method `Clean` from `org.altlinux.alterator.apt1`.
- Method `Depends` from `org.altlinux.alterator.apt1`.
- Method `InverselyDepends` from `org.altlinux.alterator.apt1`.
- Method `Show` from `org.altlinux.alterator.apt1`.
- Method `Status` from `org.altlinux.alterator.apt1`.
- Method `Install` from `org.altlinux.alterator.apt1` (available only Async).
- Method `Reinstall` from `org.altlinux.alterator.apt1` (available only Async).
- Method `Remove` from `org.altlinux.alterator.apt1` (available only Async).
- Method `DistUpgrade` from `org.altlinux.alterator.apt1` (available only Async).

## [0.2.6] - 2025-06-17

### Fixed

- Localization for output of methods from `org.altlinux.alterator.apt1` 

## [0.2.3] - 2025-04-22

### Fixed

- Missing stdout_string in reply type for `CheckReinstall` method on `org.altlinux.alterator.apt1` 
  interface by @hromovpi.

## [0.2.2] - 2025-04-18

### Changed

- Split \`List\` output (RPM).
- Add group to \`List\` output (RPM).

## [0.2.1] - 2025-04-04

### Fixed

- Missing output for `DistUpgradeAsync` method on `org.altlinux.alterator.apt1` interface.
- Missing output for `FetchDistUpgradeAsync` method on `org.altlinux.alterator.apt1` interface.
- Missing output for `FetchAsync` method on `org.altlinux.alterator.apt1` interface.

## [0.2.0] - 2025-04-03

### Added

- Signals for async transactions on `org.altlinux.alterator.apt1` interface.
- `UpdateAsync` method on `org.altlinux.alterator.apt1` interface.
- `InstallAsync` method on `org.altlinux.alterator.apt1` interface.
- `ReinstallAsync` method on `org.altlinux.alterator.apt1` interface.
- `RemoveAsync` method on `org.altlinux.alterator.apt1` interface.
- `DistUpgradeAsync` method on `org.altlinux.alterator.apt1` interface.
- `CheckInstall` method on `org.altlinux.alterator.apt1` interface
  for getting packages list on installation and removal by @sharovkv.
- `CheckReinstall` method on `org.altlinux.alterator.apt1` interface by @sharovkv.
- `CheckRemove` method on `org.altlinux.alterator.apt1` interface by @sharovkv.
- `CheckRemove` method on `org.altlinux.alterator.apt1` interface by @sharovkv.
- `CheckDistUpgrade` method on `org.altlinux.alterator.apt1` interface by @sharovkv.
- `Clean` method on `org.altlinux.alterator.apt1` interface for erasing downloaded archives by @sharovkv.
- `Depends` method on `org.altlinux.alterator.apt1` interface
  for check dependencies of packages by @sharovkv.
- `InverselyDepends` method on `org.altlinux.alterator.apt1` interface by @sharovkv.
- `Fetch` method on `org.altlinux.alterator.apt1` interface
  for downloading packages archives by @sharovkv.
- `FetchAsync` method on `org.altlinux.alterator.apt1` interface by @sharovkv.
- `FetchDistUpgrade` method on `org.altlinux.alterator.apt1` interface by @sharovkv.
- `FetchDistUpgradeAsync` method on `org.altlinux.alterator.apt1` interface by @sharovkv.
- `DistUpgrade` method on `org.altlinux.alterator.apt1` interface by @sharovkv.
- `Show` method on `org.altlinux.alterator.apt1` interface
  for getting information about package by @sharovkv.
- `Status` method on `org.altlinux.alterator.apt1` interface
  for checking status of packages (one of 4: not installed, installed, outdated, missing) by @sharovkv.

## [0.1.4] - 2025-02-05

### Added

- `LastDistUpgrade` method on `org.altlinux.alterator.apt1` interface by @kozyrevid & @sharovkv.

### Fixed

- Now `LastUpdate` method result returns genuine data by @kozyrevid.
- Timeouts for `Install` and `Remove` methods on `org.altlinux.alterator.apt1` interface by @kozyrevid.

[unreleased]: https://gitlab.basealt.space/alt/alterator-backend-packages/-/compare/0.2.3-alt1...master
[0.2.3]: https://gitlab.basealt.space/alt/alterator-backend-packages/-/tags/0.2.3-alt1
[0.2.2]: https://gitlab.basealt.space/alt/alterator-backend-packages/-/tags/0.2.2-alt1
[0.2.1]: https://gitlab.basealt.space/alt/alterator-backend-packages/-/tags/0.2.1-alt1
[0.2.0]: https://gitlab.basealt.space/alt/alterator-backend-packages/-/tags/0.2.0-alt1
[0.1.4]: https://gitlab.basealt.space/alt/alterator-backend-packages/-/tags/0.1.4-alt1
