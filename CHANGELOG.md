# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.4.5] - 2025-04-19

### Changed

- Size of edition wizard window.

### Fixed

- Missing names of editions if locale is not en or ru.

### Removed

- Extra text about components from edition wizard.

## [0.4.4] - 2025-04-09

### Added

- Support Cinnamon DE settings.
- License application for Alterator Explorer.

### Changed

- Alterator Explorer application display name Properties to About System.
- Display names of branches.

### Fixed

- Wrong DE settings running.
- Size of License dialog.

## [0.4.3] - 2025-04-04

### Added

- Command line option -r or --release-notes for application launching
  with Useful Information tab.
- Alterator Explorer application Release Notes.

### Changed

- Alterator Explorer application display name Systeminfo to Properties.
- Useful Information section does not contains Support section.
  This information expected in release-notes document.
- Useful Information section has been renamed to Release Notes.

### Fixed

- Command line option -v or --version displays version now.

## [0.4.2] - 2025-03-28

### Changed

- Display ALT distro release notes with Support section instead own entries.

## [0.4.1] - 2025-03-18

### Added

- Desktop entry.

### Changed

- Application name is `ALT Systeminfo` now.
- Current edition is selected by default in wizard.
- Format last update date (without day of week and time zone).
- Switch tool button has an icon now.

### Fixed

- Incorrect style of Switch tool button (@alekseevam)
- Multiple DE running if there are more than one.
- Waiting to Settings child process.

## [0.4.0] - 2025-03-17

### Added

- Useful shortcuts:
  CTRL + Q for quitting everywhere,
  CTRL + Left / CTRL + Right for edition wizard, 
  CTRL + L for open license in About System tab.
- New tab Local Settings for ACC and DE settings launching.
- Icons for tabs and buttons.

### Fixed

- Bug with ALT Starterkit distro.
- Alignment of license button.
- Docs by language (RU).

### Changed

- Logo of distros.

## [0.3.0] - 2025-03-15

### Added

- BaseALT logo if edition or BaseALT distro is selected.
- Displaying default license if edition is not selected.
- Tab with useful information.

### Changed

- Properties section renamed to Operating System (en).
- Not found properties are not displaying (except: requirable for display properties (Edition)).
- Properties moved to tabs.
- Arch property moved to Operating System.
- Hostname is selectable.
- Translated display names for distros.
- Translated display names for branches.

### Fixed

- Incorrect displayable tool button for edition selection.
- Properties values alignment.
- Scroll bar in table if property value is wide.
- Graphic common layout.
- Some warnings.

### Removed

- Hardware section.

## [0.2.1] - 2025-03-09

### Changed

- Edition selection dialog like wizard with license agreement.

### Fixed

- Segmentation fault if edition is not installed in some cases.
- Segmentation fault on startup in some cases.

## [0.2.0] - 2025-03-07

### Added

- Editions selection with description and license information.

## [0.1.3] - 2025-03-03

### Added

- Support application for Alterator Explorer.

### Changed

- New main window title.

### Fixed

- Vertical resizing of properties contents.
- Date and time according system locale (on Qt 6).

## [0.1.2] - 2025-02-25

### Added

- Build support for Qt 6.

## [0.1.1] - 2025-02-20

### Added

- Displaying an operation system property: edition.
- Opening an edition license dialog using GUI or CLI long option "--show-license" and short option "-l".

### Fixed

- Drive and memory volume displays on megabytes/gigabytes instead mebibytes/gibibytes.

## [0.1.0] - 2024-12-04

### Added

- `/org/altlinux/alterator/systeminfoApp` object of system bus
  that implements `org.altlinux.alterator.application1` interface.
- Displaying a hostname.
- Displaying an operation system properties: name, dist branch, updated on, kernel.
- Displaying a hardware properties: processor, arch, graphics, memory, drive,
  motherboard and displays.
- English internationalization.
- Russian internationalization.
- ALT Linux logo.

[unreleased]: https://altlinux.space/alterator/alt-systeminfo/compare/v0.4.5...master
[0.4.5]: https://altlinux.space/alterator/alt-systeminfo/src/tag/v0.4.5
[0.4.4]: https://altlinux.space/alterator/alt-systeminfo/src/tag/v0.4.4
[0.4.3]: https://altlinux.space/alterator/alt-systeminfo/src/tag/v0.4.3
[0.4.2]: https://altlinux.space/alterator/alt-systeminfo/src/tag/v0.4.2
[0.4.1]: https://altlinux.space/alterator/alt-systeminfo/src/tag/v0.4.1
[0.4.0]: https://altlinux.space/alterator/alt-systeminfo/src/tag/v0.4.0
[0.3.0]: https://altlinux.space/alterator/alt-systeminfo/src/tag/v0.3.0
[0.2.1]: https://altlinux.space/alterator/alt-systeminfo/src/tag/v0.2.1
[0.2.0]: https://altlinux.space/alterator/alt-systeminfo/src/tag/v0.2.0
[0.1.3]: https://altlinux.space/alterator/alt-systeminfo/src/tag/v0.1.3
[0.1.2]: https://altlinux.space/alterator/alt-systeminfo/src/tag/v0.1.2
[0.1.1]: https://altlinux.space/alterator/alt-systeminfo/src/tag/v0.1.1
[0.1.0]: https://altlinux.space/alterator/alt-systeminfo/src/tag/v0.1.0
