# ALT Services
Graphical front-end for Alterator services module.

# Documentation

## Service management
See [ALT Wiki page](https://www.altlinux.org/ALT_Services).

## import & export
This application allows to save parameters for later reuse.

#### file format
Alterator service playfile format is based on json (`.playfile.json` extension is usually used).
It contains current service name and action, parameters, and additional options.

Example:

```json
{
    "version": 1,
    "service": "sample_service",
    "action": "deploy",
    "options": {
        "autostart": false,
        "force": false,
        "postdiag": {
            "enable": true,
            "options": {
                "tool1": [
                    "test3"
                ]
            }
        },
        "prediag": {
            "enable": true,
            "options": {
                "tool1": [
                    "test1",
                    "test2"
                ],
                "tool2": [
                    "test1"
                ],
            }
        }
    },
    "parameters": {
        "foo": "bar",
        ....
    }
}

```

See [playfile JSON-schema](doc/service.playfile.schema.json).

#### exporting parameters
- There is a menu bar on the "Parameters" page of a wizard.
- `File` -> `Export...` -> chose where to save a file.

#### importing parameters
- use `File` -> `Import...`.
- or just drag and drop a file into the application window.

## Service developement & debugging
You can enable debug output to:
- see Status() json output
- see json that is actually sent to Deploy()/Undeploy()/Configure()/Backup()/Restore()

Add the following lines to `/etc/qt6/qtlogging.ini` 
```ini
[Rules]
*.debug=false
default.debug=true
```


# Building
- install the following dependencies (see `BuildRequirements` section of the [spec file](.gear/alt-services.spec))
  - cmake extra-cmake-modules
  - gcc-c++
  - qt6-base-devel
  - qt6-tools-devel
  - qt6-base-common
  - libtoml11-devel
  - boost-devel-headers
  - kf6-kwidgetsaddons-devel
  - libqtsingleapplication-qt6-devel

- Clone the repository:

  `git clone https://altlinux.space/alterator/alt-services.git`

- Configure the project:

  `cd alt-services && cmake . -B build`

- Build:

  `cmake --build build`
