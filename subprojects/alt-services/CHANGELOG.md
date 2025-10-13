# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Fixed
- build with qt < 6.9
- editors being opened on disabled items
- margins in table mode
- missing translations
- "remove" button position in table mode
- parameters selection page being skipped when parameters are incorrect
- wizard could be closed during operation
- missing icon on the start page of wizard
- highlighting of disabled parameters
- incorrect application name & icon in Gnome

### Changed
- updated to latest alterator-entry changes
- preserve initial parameters order
- text is no more elided inside table 
- error messages are now tracked for all actions
- unified look and behaviour of all selectable items inside tables

### Added
- an option to put non-required parameters at the end of the list
- service's Status() debug output
- service, diagnostic tools and test icons
- overridable resource highlighting
- array item prefixes

### Removed
- Start() & Stop() calls during Configure()

## 0.1.5-alt1

### Fixed
- parsing issues
- "parameters" tab icon
- improved keyboard navigation
- added word-wrap on confirmation page
- restricted app to a single instance

## 0.1.4-alt1

### Fixed
- composite editor children not being displayed
- modal dialogs behavior in gnome
- icons in gnome
- building with lcc
- "select all" button is now disabled correctly
- handling of authorization errors
- checkbox editor label not being shown
- display disabled child parameters correctly
- removed empty tooltips
- empty cells background
- toolbar actions being enabled during model (re-)loading
- added dbus timeout
- parameters order
- passwords being displayed

### Added
- a spacer at the bottom of the scroll area
- implemented diag
- group parameters
- compact form
- validation hints
- menubar
  - export and import parameters
  - refresh current service
  - reload all services
  - switching between compact and detailed parameter form
  - application info
- backup/restore methods
- undeploy parameters
- password parameters
- configurable toolbar
- deployment wizard
- additional diagnostic tool selection
- table verbocity toggle
- translatable enums

### Changed
- improved tables
- darken inactive parameters
- application updated for latest alterator-entry changes
  - Services now may have required tests in pre- and post-deploy diagnostic mode
- render non-interactable table checkboxes as disabled
- adjusted mainwindow margins

## 0.1.3-alt1

### Fixed
- tooltip in service table header
- service will not be displayed if any errors were encountered during parsing

### Added
- role icons
- composite parameters and arrays
- deployment/configuration dialog
- start/stop methods
- desktop file
- force_deploy parameter

### Removed
- "initial" configuration

### Changed
- parameter icon
- improved properties table
- resources are now displayed in groups
- various UI improvements


## 0.1.2-alt2

### Changed
- application mostly rewritten for latest service1 interface changes

### Added
- search bar

## 0.1.1-alt1

### Added

- translations
- tab icons

### Fixed

- missing icons on some desktop environments
- write initial values of QComboBox (and others) immediately
- display editable values correctly
- adjust columns to contents

### Changed

- display resource type as an icon with a tooltip

## 0.1.0-alt2

### Fixed

- main window title

### Added

- .gitingnore


## 0.1.0-alt1
- Initial build
