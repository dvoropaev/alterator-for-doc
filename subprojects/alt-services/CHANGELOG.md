# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Fixed
- segmentation fault while editing parameters in table mode
- "Copy"/"Select All" default actions not being displayed in Log Page context menu
- "table mode" parameter editor was losing its scroll position while adding/removing parameters
- optional parameter selection was only possible outside of table checkboxes

## 0.1.7-alt1

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
- incorrect text/background colors in some table cells
- disabled checkboxes were drawn as active in "compact mode
- "word wrap" option did not affect some table cells
- "select all" & "clear" buttons were not working
- diagnostic tools selection was not possible in keyboard-only naviggation
- incorrect enum parameters processing
- "Refresh" action not working
- all actions are now properly disabled while another operation in progress

### Changed
- updated to latest alterator-entry changes
- preserve initial parameters order
- text is no more elided inside table 
- error messages are now tracked for all actions
- unified look and behaviour of all selectable items inside tables
- important messages now have background color depending on message type
- "force deploy" now allowes to ignore resource conflicts
- "refresh all" option position in services table context menu
- start & stop actions are now blocked until prefious call is finished
- import/export file format
- improved keyboard navigation
- wizard's log page now displays entries hierarchically
- diagnostic will run all selected tests, even if some of them failed

### Added
- an option to put non-required parameters at the end of the list
- an option to configure word wrap in tables
- service's Status() debug output
- service, diagnostic tools and test icons
- overridable resource highlighting
- array item prefixes
- a table of resources with current owners on the start wizard page
- check for unresolvable resource conflicts on the start wizard page
- check for resource conflicts while validating parameters that are linked to resources
- a warning message, when Status() returns a non-zero exit code
- a search bar on the Parameters page of a wizard
- ability to export journals

### Removed
- Start() & Stop() calls during Configure()
- old resource conflict message

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
