# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## 0.1.17-alt1

### Fixed
- window was delayed until all objects are built
- launched apps are now keeped after acc window closed
- application coud not raise its window on Wayland
- reference text
- the input cursor is removed in the reference

### Added
- dbus-activation mechanism
- xdg-activation protocol
- --replace commandline option for replacing existing window

## 0.1.16-alt1

### Fixed
- correct icon handling in Wayland
- use standard Alterator icon

## 0.1.15-alt1

### Fixed
- restricted to single instance
- unknown objects not being displayed

### Added
- branding style

## 0.1.14-alt1

### Added

- get objects interfaces from available applications

## 0.1.13-alt1

### Added

- refresh button
- support release-notes application
- add control interface

### Fixed

- category icons size
- hide legacy switch button if acc-legacy not available
- toolbar appearance in different themes 

## 0.1.12-alt1

### Fixed

- empty categories

## 0.1.11-alt1

### Added

- Support systeminfo application

## 0.1.10-alt1

- add qt6 support
- remove comments

## 0.1.9-alt1

### Fixed

- Infinite connection to DBus while alterator interface is not present, or not responding
- Category order
- Module order
- Object parsing
- Removed empty space in the bottom of scroll area
- Icons and styles to display correctly on different desktop environments

### Added

- A notification, telling that alterator-manager service does not respond
- Objects override functionality
