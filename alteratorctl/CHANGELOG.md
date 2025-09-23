# Changelog


All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.13-alt1] - 2025-08-05

### Added
* Line breaks for better readability of `components install/remove`. (by nexi@)
* `--force-yes` option in components module.
* Handling warnings in `diag run` command.
* Spliting `packages apt` andd `components install/remove` output to columns.
* Handling `^C` during apt-get transaction.
* `components search` command.
* Pager for `components list`, `packages rpm list/files` and `packages apt list/search` output.
* Spliting tests output in `--verbose` mode during `diag run`.

### Fixed
* Handling attempts to remove uninstalled packages and install already installed ones.
* Names of components removing options.
* Line breaks in `--help` option output.

### Changed
* Split target components and affected components in install and remove commands.
* More detailed help in the components module.

## [0.1.12-alt1] - 2025-07-26

### Added
* Calculating affected components for installing or removing of specified component.
* Packages filtering in `components status`, `components install` or `components remove` by `exclude_arch` flags.
* Show tests display name in `diag run` by default. (by nexi@)

### Fixed
* `packages apt list` command description. (by nexi@)
* Settint timeouts for dbus calls with signals. 

## [0.1.11-alt1] - 2025-07-18

### Added
* Prevent manual removal of installed packages by default in `components remove` and `packages apt`.

### Fixed
* Bash-completion working with `actl` symbolic link to `alteratorctl`.
* Send edition path to `edition set`.

## [0.1.10-alt1] - 2025-07-01

### Added
* Bash and Fish completion by @kozyrevid. ([#1])

### Fixed
* Alteratorctl translation by @lepata. ([#3])

## [0.1.9-alt1] - 2025-05-31

### Added 
* Apt-get output streaming in `alteratorctl packages apt install`. ([#84])
* Apt-get output streaming in `alteratorctl packages apt remove`. ([#84])
* Apt-get output streaming in `alteratorctl packages apt reinstall`. ([#84])
* Apt-get output streaming in `alteratorctl packages apt update`. ([#86])
* Packages apt submodule methods `install`, `remove` and `reinstall` output sorting. ([#87])
* Systeminfo module output sorting in `description` method. ([#87]))
* Handling dbus errors from connection. ([#88])
* Simplify checking of existance of dbus objects. ([#88])
* Choose edition sections parameters in components module cmdl.
* Hide installed status markers in components module.
* Hide installed edition marker.

### Fixed 
* `alteratorctl manager getsignals` fix double free signals names and memory leak. ([#85])
* Registration of polkit-agent in tty. ([#88])
* Components installed status with architecturally depends packages.
* Extra printing of specified category in components module.

### Changed
* Renaming `--show-display-name` option to `--enable-display-name` in components module.

## [0.1.8-alt1] - 2025-04-18

### Fixed
* `alteratorctl components install` and `remove` doesn't works by @kozyrevid.

## [0.1.7-alt1] - 2025-04-16

### Added
* `--verbose` option in editions, diag and components usage help.

### Fixed
* Running diagnostic tools from root.
* Components options translation.
* Handling set of noexistance edition.

### Changed
* Diagnostic tool tests status names.
* Usage helps alignment.

## [0.1.6-alt1] - 2025-04-10

### Fixed
* Getting components sections while empty result.
* Getting components list while DE's is empty.
* Systeminfo desktop and locales ouptput.
* Double free in diag list tests.
* Glib warning in components components while edition doesn't set.
* Diag test status output.
* Manager getobjects and getifaces output.
* Remove packages filtering in components list (this functional moved to batch components backend).

## [0.1.5-alt1] - 2025-04-01

### Added
* Systeminfo `desktop` and `locales` methods. ([#80])
* Filter components packages by languages and desktop environments. ([#80])

### Fixed
* Component packages filtering in `components list`. ([#81])

## [0.1.4-alt1] - 2025-03-27

### Added
* Textual authentication agent. ([#75])

### Fixed
* Fallback in usage polkit agent. ([#79])
* Handling incorrect category name with --category option in components module. ([#78])
* Memory leaks in `components install/remove`. ([#76])
* Memory leaks in `components list`. ([#76])
* Memory leaks in `components info`. ([#76])
* Memory leaks in components printing list helper function. ([#76])

## [0.1.3-alt1] - 2025-03-26

### Added
* Component packages filtering by arch option. ([#77])
* Include kernel name in package name when kernel_module option is set. ([#77])
* Components packages sorting in `components status` command. ([#77])

### Fixed
* Components packages installed status markers. ([#77])
* Memory leaks. ([#74])
* Editions ru. usage help translation. ([#72])
* Packages submodules errors handling. ([#72])
* Packages rpm list method name. ([#72])
* Systeminfo description ignore unknown properties. ([#72])
* Components apt update function. ([#72])
* Display all components from current edition in default mode. ([#72])
* Components installed markers position. ([#72])
* Various segfaults and memory leaks
* Handling of invalid or missing locales

## Removed
* Warning about the absence of the current edition. ([#77])

## [0.1.2-alt1] - 2025-03-18

### Added
* Sorting result of editions module. ([#71])
* Sorting result of diag list method. ([#71])
* Sorting result of packages submodules. ([#71])
* Components --draft and no-update options. ([#72])
* Current edition marker in editions list. ([#72])

### Removed
* Systeminfo license. ([#72])

## [0.1.1-alt1] - 2025-03-17

### Added 
* Option to hide the components module legend. ([#69])
* Selecting the license text in the systeminfo module depending on the system language. ([#69])
* Marker for unknown installed status. ([#70])
* Sorting in alteratorctl manager module. ([#70])

### Security
* Changed the user tracking criteria for running commands that require appropriate permissions. ([#69])

### Fixed
* --path-only option usage in components --list. ([#69])
* Correct handling of missing edits. ([#69])

## [0.1.0-alt1] - 2025-03-13

### Added
* Installing status markers legend. ([#68])
* Output sorting by names. ([#68])
* Systeminfo license. ([#68])

### Changed
* Simple tree output. ([#68])
* Options names. ([#68])

### Fixed
* Printing components while edition isn't valid. ([#68])
* Check running via sudo. ([#68])

## [0.0.11-alt2] - 2025-03-10

### Changed
* Adding polkit to dependencies in .spec file.

## [0.0.11-alt1] - 2025-03-07

### Added
* Editions list and license. ([#67])
* Spliting categories and components by editions sections. ([#67])
* Toml array parsing. ([#67])
* New html tags parsing. ([#67])
* Adding automatic substitution of current default editions into command arguments of editions module. ([#67])
* Checking user rules. ([#67])

### Fixed
* Editions info and description methods. ([#67])

## [0.0.10-alt1] - 2025-03-06

### Added
* Options for hiding and displaying part of the information and components and categories. ([#65])
* Different modes of displaying component and category hierarchy. ([#65])
* Ability to display components of only a certain category. ([#65])

### Fixed
* Optimization of components list method
* Optimization of components install method
* Optimization of components remove method

## [0.0.9-alt2] - 2025-02-25

### Added
* Handling html description. ([#62])
* Markers for established, unestablished and partially established components and categories. ([#62])

### Changed
* Move old description output to components status. ([#62])
* Deleting colorize text. ([#62])

### Fixed
* Components install, remove and status methods. ([#62])

## [0.0.9-alt1] - 2025-02-17

### Added

* CHANGELOG.md. ([#56])
* Support for working with diag module tool names. ([#57])
* Handling options related to modules or submodules. ([#59])
* Optimize diag module interface. ([#59])
* Systeminfo default description command. ([#60]) 
* Working with components names. ([#61])
* Displaying the list of components with categories in the form of a hierarchical tree. ([#61])

### Changed

* Update using of methods in systeminfo module according to newest version of interface ([#54])
* The output of a diag test when it is executed is now only displayed with the --verbose option ([#59])

## [0.0.8-alt2] - 2024-12-13

### Changed

* Output the list of components when accessing the components module without specifying a command. ([#52])
* Output the list of editions when accessing the editions module without specifying a command. ([#52])
* Output the list of diagnostic tools and his tests when accessing the diag module without specifying a command. ([#52])

### Fixed

* Interface validation in diag module. ([#53])
* Dependencies after moving to alterator entry parsing from .ini to toml files.

## [0.0.8-alt1] - 2024-12-11

### Added

* Remote module. ([#47])

### Changed 

* Moving alterator entry info files parsing from .ini to toml format. ([#51])
* Synchronization with alterator-backends-systeminfo changes: Remove the logic of using environment variables to
    retrieve monitor information. CPU frequency is now displayed in Hz. ([#50])

## [0.0.7-alt1] - 2024-11-11

### Added

* Packages apt last-update method. ([#39])
* Description method in editions module. ([#41])
* Getting new error messages from DBus in packages module. ([#40])
* Timedate parsing in systeminfo module. ([#43])
* Implement diag and manager internal modules to work with objects on system and session buses. 

### Changed

* Synchronization with alterator-backends-systeminfo changes: renaming systeminfo methods. ([#43]) 
* Output formatting of CPU, monitor and motherboard info. ([#44])

### Fixed

* Blocking the repetition of the same DBus bus selection flags. ([#48])

## [0.0.6-alt1] - 2024-10-23

### Added

* Systeminfo module. ([#36])

### Changed

* Change prefix from ru.basealt to org.altlinux.

## [0.0.5-alt3] - 2024-10-16

### Changed

* Refactor return types of info in packages and components modules. ([#35])

### Fixed

* Calling packages rpm and apt submodules help. ([#35])

## [0.0.5-alt2] - 2024-09-27

### Added 

* Get alterator entry info text in gdbus source. ([#32])
* Validation of objects and interfaces. ([#32])

### Changed

* Printing usage help in packages submodules. ([#32])

### Fixed

* Writing diag report file. ([#32])

## [0.0.5-alt1] - 2024-09-17

### Added

* Alterator entry parsing in gdbus source. ([#21])
* Refactor getting alterator entry info in alteratorctl modules. ([#22])

### Removed

* Some messages in verbose mode. ([#18])

### Fixed

* Reply type in components info ([#20]).
* Remove catched memory leaks. ([#23])
* Validation args in manager module. ([#24])

## [0.0.4-alt1] - 2024-08-22

### Added

* Verbose mode messages. ([#12])
* Checking the existence of objects and interfaces functional in gdbus source.
* Objects and interfaces validation in alteratorctl modules. ([#17])
* Make universal context for alteratorctl modules. ([#16])

### Changed

* Store subcommand ID in modules object. ([#14])
* Store packages submodules and subcommands ID's. ([#15])

### Fixed

* Improved the usage help to a more pretty and user-friendly format. ([#13])

## [0.0.3-alt1] - 2024-07-31

### Added

* Internationalization via xgettext. ([#8])
* Editions module. ([#9])
* Diag module.

## [0.0.2-alt1] - 2024-07-24

### Added

* Implementation of manager module.
* Alteratorctl module interface.
* Alteratorctl options entries.
* Packages and components modules.
* Alteratorctl and submodules usage help.
* Register modules mechanism.

### Changed

* Add 'alteratorctl' prefix to all source files.
* Renaming GDBusSource class to AlteratorGDBusSource.
* Redesigning alteratorctlapp as a class.

### Removed

* Diagctl and managerctl.

## [0.0.1-alt1] - 2024-06-02

### Added

* Managerctl module.
* GDBusSource class.
* Diagctl module.
* Alteratorctlapp.
* .gitignore, README.md, .spec file, CMakeList.txt and gear rules.

[unreleased]: https://altlinux.space/alterator/alteratorctl/compare/0.1.13-alt1...main
[0.1.13-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.1.13-alt1
[0.1.12-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.1.12-alt1
[0.1.11-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.1.11-alt1
[0.1.10-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.1.10-alt1
[0.1.9-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.1.9-alt1
[0.1.8-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.1.8-alt1
[0.1.7-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.1.7-alt1
[0.1.6-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.1.6-alt1
[0.1.5-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.1.5-alt1
[0.1.4-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.1.4-alt1
[0.1.3-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.1.3-alt1
[0.1.2-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.1.2-alt1
[0.1.1-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.1.1-alt1
[0.1.0-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.1.0-alt1
[0.0.11-alt2]: https://altlinux.space/alterator/alteratorctl/src/tag/0.0.11-alt2
[0.0.11-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.0.11-alt1
[0.0.10-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.0.10-alt1
[0.0.9-alt2]: https://altlinux.space/alterator/alteratorctl/src/tag/0.0.9-alt2
[0.0.9-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.0.9-alt1
[0.0.8-alt2]: https://altlinux.space/alterator/alteratorctl/src/tag/0.0.8-alt2
[0.0.8-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.0.8-alt1
[0.0.7-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.0.7-alt1
[0.0.6-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.0.6-alt1
[0.0.5-alt3]: https://altlinux.space/alterator/alteratorctl/src/tag/0.0.5-alt3
[0.0.5-alt2]: https://altlinux.space/alterator/alteratorctl/src/tag/0.0.5-alt2
[0.0.5-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.0.5-alt1
[0.0.4-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.0.4-alt1
[0.0.3-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.0.3-alt1
[0.0.2-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.0.2-alt1
[0.0.1-alt1]: https://altlinux.space/alterator/alteratorctl/src/tag/0.0.1-alt1

[#88]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/88
[#87]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/87
[#86]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/86
[#85]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/85
[#84]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/84
[#81]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/81
[#80]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/80
[#79]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/79
[#78]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/78
[#77]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/77
[#76]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/76
[#75]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/75
[#74]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/74
[#72]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/72
[#71]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/71
[#70]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/70
[#69]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/69
[#68]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/68
[#67]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/67
[#65]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/65
[#63]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/63
[#62]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/62
[#61]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/61
[#60]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/60
[#59]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/59
[#57]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/57
[#56]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/56
[#54]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/54
[#53]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/53
[#52]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/52
[#51]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/51
[#50]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/50
[#47]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/47
[#44]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/44
[#43]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/43
[#41]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/41
[#40]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/40
[#39]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/39
[#36]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/36
[#35]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/35
[#32]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/32
[#24]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/24
[#23]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/23
[#22]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/22
[#21]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/21
[#20]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/20
[#18]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/18
[#17]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/17
[#16]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/16
[#15]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/15
[#14]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/14
[#13]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/13
[#12]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/12
[#9]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/9
[#8]: https://gitlab.basealt.space/alt/alteratorctl/-/merge_requests/8
