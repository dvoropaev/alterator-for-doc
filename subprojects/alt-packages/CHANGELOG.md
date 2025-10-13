# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.3.3] - 2025-07-11

### Added

- Safe mode from newest backend.

### Changed

- APT Transaction applying dialog replaced to dialog with scrollable package list.

### Fixed

- Suggestion when upgrading if package list on transaction is empty.
  In this case, user gets a notification.

## [0.3.2] - 2025-04-25

### Fixed

- Fix deletion of update sources.
- Fix dialog closing after package list update.
- Bump required backend version.
- Add rpm action messages.

## [0.3.1] - 2025-04-25

### Fixed

- Clean not needed BuildRequires.
- Fix APT check for last update from /var/log/alterator/apt/updates.log

## [0.3.0] - 2025-04-18

### Added

- Filter by Group (RPM).
- Filter by Arch (RPM).

### Changed

- Version & Release columns has been merged to Version-Release
  single column (RPM).
- Repo Type column has been renamed to Type (Repo).
- Branch column has been renamed to Sign (Repo).
- `Upgrade All` button has been retranslated (APT).

### Fixed

- Fix deletion of update sources
- Fix dialog closing after package list update
- Alignment of packages list on applying transaction (APT).

## [0.2.0] - 2025-04-14

### Added

- Showing APT transaction progress.
- Suggestion to update if last update date is not found.
- Displaying an actual packages list of APT transaction.
- `Upgrade All` button for upgrading an all obsoleted packages.

### Changed

- Application name has been changed to ALT Packages.
- `Update` button text (i18n/ru).

### Fixed

- Broken Update.
- Selection resetting if filter is changed.

## [0.1.5] - 2025-02-25

### Added

- Error dialogs by @alekseevam.
- Menu bar with actions: Manual, About, Language selection by @sharovkv.
- Multiple files and info of packages dialogs by @sharovkv.
- Build support for Qt 6.

### Fixed

- Faster application running and workflow by @sharovkv.
- Disabling buttons if selection is empty by @sharovkv.
- Data is now not updated if authentication is rejected by @sharovkv.

### Changed

- Graphical UI by @sharovkv.

## [0.1.4] - 2024-12-09

### Added

- Waiting dialogs by @sa.

### Fixed

- Updating of packages list by @sharovkv.

[unreleased]: https://altlinux.space/alterator/alt-packages/compare/0.3.3-alt1...master
[0.3.3]: https://altlinux.space/alterator/alt-packages/src/tag/0.3.3-alt1
[0.3.2]: https://altlinux.space/alterator/alt-packages/src/tag/0.3.2-alt1
[0.3.1]: https://altlinux.space/alterator/alt-packages/src/tag/0.3.1-alt1
[0.3.0]: https://altlinux.space/alterator/alt-packages/src/tag/0.3.0-alt1
[0.2.0]: https://altlinux.space/alterator/alt-packages/src/tag/0.2.0-alt1
[0.1.5]: https://altlinux.space/alterator/alt-packages/src/tag/0.1.5-alt1
[0.1.4]: https://altlinux.space/alterator/alt-packages/src/tag/0.1.4-alt1
